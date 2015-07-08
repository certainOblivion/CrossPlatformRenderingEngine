#include "pch.h"
#include "GameRenderer.h"
#if (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
#include "Utility.h"  
#endif // (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
#include "TexturedSphereRenderer.h"
#include "ColoredTriangleRenderer.h"
#include "Cameras\FirstPersonCamera.h"
#include "BlinnPhongRenderer.h"
#include "RasterizerStates.h"
#include "PointLightRenderer.h"
#include "EnvironmentMappingRenderer.h"
#include "FogRenderer.h"
#include "TransparencyMappingRenderer.h"

using namespace DirectX;
using namespace Library;
using namespace Game;
using namespace Rendering;
using namespace std;
using Microsoft::WRL::ComPtr;
#if (WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP)
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;
using namespace Windows::Graphics::Display;
using namespace Windows::UI::Core;
#endif // (WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP)


#if (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
GameRenderer::GameRenderer(std::function<void*()> getWindow, std::function<uint32_t()>getWidth, std::function<uint32_t()>getHeight) :
m_window(0), mGetWindowFunctor(getWindow), mGetWindowWidthFunctor(getWidth), mGetWindowHeightFunctor(getHeight), mCurrentDemo(0), bIsResetInput(false), bIsControlCamera(true)
{
	mWindowWidth = getWidth();
	mWindowHeight = getHeight();

	Initialize();
	CreateResources(mGetWindowFunctor, mGetWindowWidthFunctor, mGetWindowHeightFunctor);
}
#else

GameRenderer::GameRenderer(void* window) :
m_window(window), m_d3dRenderTargetSize(), m_nativeOrientation(DisplayOrientations::None), m_currentOrientation(DisplayOrientations::None), bIsControlCamera(true)
{
	mWindowHeight = (uint32_t)(reinterpret_cast<Windows::UI::Core::CoreWindow^>(window))->Bounds.Height;
	mWindowWidth = (uint32_t)(reinterpret_cast<Windows::UI::Core::CoreWindow^>(window))->Bounds.Width;
	Initialize();


	mGetWindowFunctor = [&]()
	{
		return m_window;
	};

	mGetWindowWidthFunctor = [&]()
	{
		return GetWindowWidth();
	};

	mGetWindowHeightFunctor = [&]()
	{
		return GetWindowHeight();
	};
}

#endif

// Executes basic game loop.
void GameRenderer::Tick()
{
	m_timer.Tick([&]()
	{
		Update();
	});

	Render();
}
std::uint32_t GameRenderer::GetWindowWidth()
{
#if (WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP)
	// Calculate the necessary render target size in pixels.
	m_outputSize.Width = DX::ConvertDipsToPixels(m_logicalSize.Width, m_dpi);

	// Prevent zero size DirectX content from being created.
	m_outputSize.Width = max(m_outputSize.Width, 1);

	// The width and height of the swap chain must be based on the window's
	// natively-oriented width and height. If the window is not in the native
	// orientation, the dimensions must be reversed.
	DXGI_MODE_ROTATION displayRotation = ComputeDisplayRotation();

	bool swapDimensions = displayRotation == DXGI_MODE_ROTATION_ROTATE90 || displayRotation == DXGI_MODE_ROTATION_ROTATE270;
	m_d3dRenderTargetSize.Width = swapDimensions ? m_outputSize.Height : m_outputSize.Width;
	return (std::uint32_t)m_d3dRenderTargetSize.Width;
#else
	return 800;
#endif
}

std::uint32_t GameRenderer::GetWindowHeight()
{
#if (WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP)
	m_outputSize.Height = DX::ConvertDipsToPixels(m_logicalSize.Height, m_dpi);
	m_outputSize.Height = max(m_outputSize.Height, 1);

	DXGI_MODE_ROTATION displayRotation = ComputeDisplayRotation();

	bool swapDimensions = displayRotation == DXGI_MODE_ROTATION_ROTATE90 || displayRotation == DXGI_MODE_ROTATION_ROTATE270;

	m_d3dRenderTargetSize.Height = swapDimensions ? m_outputSize.Width : m_outputSize.Height;
	return (std::uint32_t)m_d3dRenderTargetSize.Height;
#else
	return 600;
#endif
}

void* GameRenderer::GetGameWindow()
{
	return mGetWindowFunctor();
}

// Updates the world
void GameRenderer::Update()
{
	for (auto component : mComponents)
	{
		if (!component->Is(Camera::TypeIdClass()))
		{
			component->Update(m_timer);
		}
		else
		{
			if (bIsControlCamera)
			{
				component->Update(m_timer);
			}
		}
	}

	auto gamePadState = mGamePad->CurrentState();
	if (gamePadState.IsConnected())
	{
		if (!bIsResetInput)
		{
			if (gamePadState.IsDPadLeftPressed())
			{
				bIsResetInput = true;
				mCurrentDemo = (uint32_t)(mCurrentDemo == 0 ? mDemos.size() - 1 : mCurrentDemo - 1);
			}

			if (gamePadState.IsDPadRightPressed())
			{
				bIsResetInput = true;
				mCurrentDemo = mCurrentDemo >= mDemos.size() - 1 ? 0 : mCurrentDemo + 1;
			}
			if (gamePadState.IsStartPressed())
			{
				bIsResetInput = true;
				bIsControlCamera = bIsControlCamera ? false : true;
			}
			if (gamePadState.IsLeftStickPressed())
			{
				bIsResetInput = true;
				if (mSkybox->Enabled())
				{
					mSkybox->SetEnabled(false);
					mSkybox->SetVisible(false);
				}
				else
				{
					mSkybox->SetEnabled(true);
					mSkybox->SetVisible(true);
				}
			}

		}
		else
		{

			if (!gamePadState.IsDPadLeftPressed() && !gamePadState.IsDPadRightPressed() && !gamePadState.IsStartPressed() && !gamePadState.IsLeftStickPressed())
			{
				bIsResetInput = false;
			}

		}

	}
	mDemos[mCurrentDemo]->Update(m_timer);
}

// Draws the scene
bool GameRenderer::Render()
{
	// Don't try to render anything before the first Update.
	if (m_timer.GetFrameCount() == 0)
		return false;

	Clear();

	// TODO: Add your rendering code here
	for (auto component : mComponents)
	{
		auto drawableComponent = component->As<DrawableGameComponent>();
		if (drawableComponent != nullptr)
		{
			if (drawableComponent->Visible())
			{
				drawableComponent->Draw();
			}
		}
	}

	mDemos[mCurrentDemo]->Draw(GetD3DDeviceContext());

	Present();
	return true;
}

// Helper method to clear the back buffers
void GameRenderer::Clear()
{
	// Clear the views
	m_d3dContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::CornflowerBlue);
	m_d3dContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	m_d3dContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());
}

// Presents the back buffer contents to the screen
void GameRenderer::Present()
{
	// The first argument instructs DXGI to block until VSync, putting the application
	// to sleep until the next VSync. This ensures we don't waste any cycles rendering
	// frames that will never be displayed to the screen.
	HRESULT hr = m_swapChain->Present(1, 0);

	// If the device was reset we must completely reinitialize the renderer.
	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
	{
		OnDeviceLost();
	}
	else
	{
		DX::ThrowIfFailed(hr);
	}
}

// Message handlers
void GameRenderer::OnActivated()
{
	// TODO: Game is becoming active window
}

void GameRenderer::OnDeactivated()
{
	// TODO: Game is becoming background window
}

void GameRenderer::OnSuspending()
{
	// TODO: Game is being power-suspended (or minimized)
}

void GameRenderer::OnResuming()
{
	m_timer.ResetElapsedTime();

	// TODO: Game is being power-resumed (or returning from minimize)
}

void GameRenderer::OnWindowSizeChanged()
{
	CreateResources(mGetWindowFunctor, mGetWindowWidthFunctor, mGetWindowHeightFunctor);
	mCamera->UpdateProjectionMatrix();
}

// Properties
void GameRenderer::GetDefaultSize(size_t& width, size_t& height) const
{
	// TODO: Change to desired default window size (note minimum size is 320x200)
	width = 800;
	height = 600;
}

void GameRenderer::OnDeviceLost()
{
	m_depthStencilView.Reset();
	m_renderTargetView.Reset();
	m_swapChain.Reset();
	m_d3dContext.Reset();
	m_d3dDevice.Reset();

	CreateDevice();

	CreateResources(mGetWindowFunctor, mGetWindowWidthFunctor, mGetWindowHeightFunctor);
}

void GameRenderer::OnDeviceRestored()
{
	CreateResources(mGetWindowFunctor, mGetWindowWidthFunctor, mGetWindowHeightFunctor);
}

void GameRenderer::Initialize()
{
	CreateDevice();
	mGamePad = std::unique_ptr<GamePadComponent>(new GamePadComponent(*this));
	mServices.AddService(GamePadComponent::TypeIdClass(), mGamePad.get());
	mCamera = make_shared<FirstPersonCamera>(*this);
	mServices.AddService(Camera::TypeIdClass(), mCamera.get());

	RasterizerStates::Initialize(m_d3dDevice.Get());
	mSkybox = make_unique<Skybox>(*this, *mCamera, L"Content\\Textures\\Maskonaive2_1024.dds", 500.0f);

	mDemos.push_back(make_shared<Rendering::ColoredTriangleRenderer>(*this, *(mCamera.get())));
	mDemos.push_back(make_shared<Rendering::TexturedSphereRenderer>(*this, *(mCamera.get())));
	mDemos.push_back(shared_ptr<Rendering::PointLightRenderer>(new Rendering::PointLightRenderer(*this, *(mCamera.get()), bIsControlCamera)));
	mDemos.push_back(shared_ptr<Rendering::BlinnPhongRenderer>(new Rendering::BlinnPhongRenderer(*this, *(mCamera.get()), bIsControlCamera)));
	mDemos.push_back(shared_ptr<Rendering::EnvironmentMappingRenderer>(new Rendering::EnvironmentMappingRenderer(*this, *(mCamera.get()))));
	mDemos.push_back(shared_ptr<Rendering::FogRenderer>(new Rendering::FogRenderer(*this, *(mCamera.get()), bIsControlCamera)));
	mDemos.push_back(shared_ptr<Rendering::TransparencyMappingRenderer>(new Rendering::TransparencyMappingRenderer(*this, *(mCamera.get()), bIsControlCamera)));

	mComponents.push_back(mGamePad.get());
	mComponents.push_back(mCamera.get());
	mComponents.push_back(mSkybox.get());

	for (auto component : mComponents)
	{
		component->Initialize();
	}

	for (auto demo : mDemos)
	{
		demo->Initialize(GetD3DDevice(), GetD3DDeviceContext());
	}

	mCamera->SetPosition(DirectX::XMFLOAT3(0.f, 0.f, 20.f));
}
#if (WINAPI_FAMILY!=WINAPI_FAMILY_DESKTOP_APP)
DXGI_MODE_ROTATION GameRenderer::ComputeDisplayRotation()
{
	DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_UNSPECIFIED;

	// Note: NativeOrientation can only be Landscape or Portrait even though
	// the DisplayOrientations enum has other values.
	switch (m_nativeOrientation)
	{
	case DisplayOrientations::Landscape:
		switch (m_currentOrientation)
		{
		case DisplayOrientations::Landscape:
			rotation = DXGI_MODE_ROTATION_IDENTITY;
			break;

		case DisplayOrientations::Portrait:
			rotation = DXGI_MODE_ROTATION_ROTATE270;
			break;

		case DisplayOrientations::LandscapeFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE180;
			break;

		case DisplayOrientations::PortraitFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE90;
			break;
		}
		break;

	case DisplayOrientations::Portrait:
		switch (m_currentOrientation)
		{
		case DisplayOrientations::Landscape:
			rotation = DXGI_MODE_ROTATION_ROTATE90;
			break;

		case DisplayOrientations::Portrait:
			rotation = DXGI_MODE_ROTATION_IDENTITY;
			break;

		case DisplayOrientations::LandscapeFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE270;
			break;

		case DisplayOrientations::PortraitFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE180;
			break;
		}
		break;
	}
	return rotation;
}
void GameRenderer::Trim()
{
		ComPtr<IDXGIDevice3> dxgiDevice;
		m_d3dDevice.As(&dxgiDevice);

		dxgiDevice->Trim();
}

void GameRenderer::SetLogicalSize(Windows::Foundation::Size size)
{
	m_logicalSize = size;
	CreateResources(mGetWindowFunctor, mGetWindowWidthFunctor, mGetWindowHeightFunctor);
}

void GameRenderer::SetDpi(float dpi)
{
	if (dpi != m_dpi)
	{
		m_dpi = dpi;

		// When the display DPI changes, the logical size of the window (measured in Dips) also changes and needs to be updated.
		m_logicalSize = Windows::Foundation::Size((reinterpret_cast<Windows::UI::Core::CoreWindow^>(m_window))->Bounds.Width, (reinterpret_cast<Windows::UI::Core::CoreWindow^>(m_window))->Bounds.Height);

		m_d2dContext->SetDpi(m_dpi, m_dpi);
		CreateResources(mGetWindowFunctor, mGetWindowWidthFunctor, mGetWindowHeightFunctor);
	}
}

void GameRenderer::SetCurrentOrientation(Windows::Graphics::Display::DisplayOrientations currentOrientation)
{
	if (m_currentOrientation != currentOrientation)
	{
		m_currentOrientation = currentOrientation;
		CreateResources(mGetWindowFunctor, mGetWindowWidthFunctor, mGetWindowHeightFunctor);
	}
}

void GameRenderer::SetWindow(void* window)
{
	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();
	m_window = window;
	m_logicalSize = Windows::Foundation::Size((reinterpret_cast<Windows::UI::Core::CoreWindow^>(m_window))->Bounds.Width, (reinterpret_cast<Windows::UI::Core::CoreWindow^>(m_window))->Bounds.Height);
	m_nativeOrientation = currentDisplayInformation->NativeOrientation;
	m_currentOrientation = currentDisplayInformation->CurrentOrientation;
	m_dpi = currentDisplayInformation->LogicalDpi;
	CreateResources(mGetWindowFunctor, mGetWindowWidthFunctor, mGetWindowHeightFunctor);
}

#endif
