#include "pch.h"
#include "Game.h"
#include <d2d1helper.h>
#include <d2d1.h>

using namespace Library;
using namespace DirectX;
using Microsoft::WRL::ComPtr;

Game::Game() :
m_featureLevel(D3D_FEATURE_LEVEL_9_1)
{
	// Initialize Direct2D resources.
	D2D1_FACTORY_OPTIONS options;
	ZeroMemory(&options, sizeof(D2D1_FACTORY_OPTIONS));

#if defined(_DEBUG)
	// If the project is in a debug build, enable Direct2D debugging via SDK Layers.
	options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

	// Initialize the Direct2D Factory.
	DX::ThrowIfFailed(
		D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		__uuidof(ID2D1Factory2),
		&options,
		&m_d2dFactory
		)
		);
}

// These are the resources that depend on the device.
void Game::CreateDevice()
{
#if (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
	// This flag adds support for surfaces with a different color channel ordering than the API default.
	UINT creationFlags = 0;
#else
	// This flag adds support for surfaces with a different color channel ordering
	// than the API default. It is required for compatibility with Direct2D.
	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#endif

#ifdef _DEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	static const D3D_FEATURE_LEVEL featureLevels[] =
	{
		// TODO: Modify for supported Direct3D feature levels (see code below related to 11.1 fallback handling)
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};

	// Create the DX11 API device object, and get a corresponding context.
	HRESULT hr = D3D11CreateDevice(
		nullptr,                                // specify null to use the default adapter
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,                                // leave as nullptr unless software device
		creationFlags,                          // optionally set debug and Direct2D compatibility flags
		featureLevels,                          // list of feature levels this app can support
		_countof(featureLevels),                // number of entries in above list
		D3D11_SDK_VERSION,                      // always set this to D3D11_SDK_VERSION
		(ID3D11Device**)m_d3dDevice.ReleaseAndGetAddressOf(),   // returns the Direct3D device created
		&m_featureLevel,                        // returns feature level of device created
		(ID3D11DeviceContext**)m_d3dContext.ReleaseAndGetAddressOf()   // returns the device immediate context
		);

	if (hr == E_INVALIDARG)
	{
		// DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
		hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
			creationFlags, &featureLevels[1], _countof(featureLevels) - 1,
			D3D11_SDK_VERSION, (ID3D11Device**)m_d3dDevice.ReleaseAndGetAddressOf(),
			&m_featureLevel, (ID3D11DeviceContext**)m_d3dContext.ReleaseAndGetAddressOf());
	}

	DX::ThrowIfFailed(hr);

#ifndef NDEBUG
	ComPtr<ID3D11Debug> d3dDebug;
	hr = m_d3dDevice.As(&d3dDebug);
	if (SUCCEEDED(hr))
	{
		ComPtr<ID3D11InfoQueue> d3dInfoQueue;
		hr = d3dDebug.As(&d3dInfoQueue);
		if (SUCCEEDED(hr))
		{
#ifdef _DEBUG
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif
			D3D11_MESSAGE_ID hide[] =
			{
				D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
				// TODO: Add more message IDs here as needed 
			};
			D3D11_INFO_QUEUE_FILTER filter;
			memset(&filter, 0, sizeof(filter));
			filter.DenyList.NumIDs = _countof(hide);
			filter.DenyList.pIDList = hide;
			d3dInfoQueue->AddStorageFilterEntries(&filter);
		}
	}
#endif
#if (WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP)
	// Create the Direct2D device object and a corresponding context.
	ComPtr<IDXGIDevice3> dxgiDevice;
	DX::ThrowIfFailed(
		m_d3dDevice.As(&dxgiDevice)
		);
	DX::ThrowIfFailed(
		m_d2dFactory->CreateDevice(dxgiDevice.Get(), &m_d2dDevice)
		);

	DX::ThrowIfFailed(
		m_d2dDevice->CreateDeviceContext(
		D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
		&m_d2dContext
		)
		);

#endif
	// TODO: Initialize device dependent objects here (independent of window size)
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateResources(std::function<void*()> getWindow, std::function<double()>getWidth, std::function<double()>getHeight)
{
#if (WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
	// Windows Phone does not support resizing the swap chain, so clear it instead of resizing.
	m_swapChain = nullptr;
#elif (WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP)
	//m_d2dContext->SetTarget(nullptr);
	m_d2dTargetBitmap = nullptr;
#endif
	// Clear the previous window size specific context.
	ID3D11RenderTargetView* nullViews[] = { nullptr };
	m_d3dContext->OMSetRenderTargets(_countof(nullViews), nullViews, nullptr);
	m_renderTargetView.Reset();
	m_depthStencilView.Reset();
	m_d3dContext->Flush();

	UINT backBufferWidth = (UINT)getWidth();
	UINT backBufferHeight = (UINT)getHeight();
	DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
	DXGI_FORMAT depthBufferFormat = (m_featureLevel >= D3D_FEATURE_LEVEL_10_0) ? DXGI_FORMAT_D32_FLOAT : DXGI_FORMAT_D16_UNORM;

	// If the swap chain already exists, resize it, otherwise create one.
	if (m_swapChain)
	{
		HRESULT hr = m_swapChain->ResizeBuffers(2, backBufferWidth, backBufferHeight, backBufferFormat, 0);

		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
			// If the device was removed for any reason, a new device and swap chain will need to be created.
			OnDeviceLost();

			// Everything is set up now. Do not continue execution of this method. OnDeviceLost will reenter this method 
			// and correctly set up the new device.
			return;
		}
		else
		{
			DX::ThrowIfFailed(hr);
		}
	}
	else
	{
		// First, retrieve the underlying DXGI Device from the D3D Device
		ComPtr<IDXGIDevice3> dxgiDevice;
		DX::ThrowIfFailed(m_d3dDevice.As(&dxgiDevice));

		// Identify the physical adapter (GPU or card) this device is running on.
		ComPtr<IDXGIAdapter> dxgiAdapter;
		DX::ThrowIfFailed(dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf()));

		// And obtain the factory object that created it.
		ComPtr<IDXGIFactory2> dxgiFactory;
		DX::ThrowIfFailed(dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), &dxgiFactory));

		ComPtr<IDXGIFactory2> dxgiFactory2;
		HRESULT hr = dxgiFactory.As(&dxgiFactory2);
		if (SUCCEEDED(hr))
		{
			DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
#if (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
			// DirectX 11.1 or later
			//m_d3dDevice.As( &m_d3dDevice1 );
			//m_d3dContext.As( &m_d3dContext1 );

			// Create a descriptor for the swap chain.
			swapChainDesc.Width = backBufferWidth;
			swapChainDesc.Height = backBufferHeight;
			swapChainDesc.Format = backBufferFormat;
			swapChainDesc.SampleDesc.Count = 1;
			swapChainDesc.SampleDesc.Quality = 0;
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.BufferCount = 2;

			DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = { 0 };
			fsSwapChainDesc.Windowed = TRUE;


			// Create a SwapChain from a CoreWindow.
			DX::ThrowIfFailed(dxgiFactory2->CreateSwapChainForHwnd(
				m_d3dDevice.Get(), reinterpret_cast<HWND>(getWindow()), &swapChainDesc,
				&fsSwapChainDesc,
				nullptr, m_swapChain.ReleaseAndGetAddressOf()));

			// This template does not support 'full-screen' mode and prevents the ALT+ENTER shortcut from working
			dxgiFactory->MakeWindowAssociation(reinterpret_cast<HWND>(getWindow()), DXGI_MWA_NO_ALT_ENTER);
		}
	}
	ComPtr<ID3D11Texture2D> backBuffer;
	DX::ThrowIfFailed(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer));


#endif // (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)

#if (WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP)
	swapChainDesc.Width = lround(getWidth()); // Match the size of the window.
	swapChainDesc.Height = lround(getHeight());
	swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // This is the most common swap chain format.
	swapChainDesc.Stereo = false;
	swapChainDesc.SampleDesc.Count = 1; // Don't use multi-sampling.
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2; // Use double-buffering to minimize latency.
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // All Windows Store apps must use this SwapEffect.
	swapChainDesc.Flags = 0;
	swapChainDesc.Scaling = DXGI_SCALING_NONE;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

	// This sequence obtains the DXGI factory that was used to create the Direct3D device above.
	ComPtr<IDXGIDevice3> dxgiDevice;
	DX::ThrowIfFailed(
		m_d3dDevice.As(&dxgiDevice)
		);

	ComPtr<IDXGIAdapter> dxgiAdapter;
	DX::ThrowIfFailed(
		dxgiDevice->GetAdapter(&dxgiAdapter)
		);

	ComPtr<IDXGIFactory2> dxgiFactory;
	DX::ThrowIfFailed(
		dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory))
		);

	DX::ThrowIfFailed(
		dxgiFactory->CreateSwapChainForCoreWindow(
		m_d3dDevice.Get(),
		reinterpret_cast<IUnknown*>(getWindow()),
		&swapChainDesc,
		nullptr,
		&m_swapChain
		)
		);
		}
	}
	// Create a render target view of the swap chain back buffer.
	ComPtr<ID3D11Texture2D> backBuffer;
	DX::ThrowIfFailed(
		m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer))
		);

	depthBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
#endif // (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)

	// Create a view interface on the render target to use on bind.
	DX::ThrowIfFailed(m_d3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, m_renderTargetView.ReleaseAndGetAddressOf()));

	// Allocate a 2-D surface as the depth/stencil buffer and
	// create a DepthStencil view on this surface to use on bind.
	CD3D11_TEXTURE2D_DESC depthStencilDesc(depthBufferFormat, backBufferWidth, backBufferHeight, 1, 1, D3D11_BIND_DEPTH_STENCIL);

	ComPtr<ID3D11Texture2D> depthStencil;
	DX::ThrowIfFailed(m_d3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, depthStencil.GetAddressOf()));

	CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
	DX::ThrowIfFailed(m_d3dDevice->CreateDepthStencilView(depthStencil.Get(), &depthStencilViewDesc, m_depthStencilView.ReleaseAndGetAddressOf()));

	// Create a view port descriptor of the full window size.
	m_screenViewport = CD3D11_VIEWPORT(0.0f, 0.0f, static_cast<float>(backBufferWidth), static_cast<float>(backBufferHeight));

	// Set the current view port using the descriptor.
	m_d3dContext->RSSetViewports(1, &m_screenViewport);
	//m_swapChain1.As( &m_swapChain );

	// Obtain the back buffer for this window which will be the final 3D render target.

	// TODO: Initialize windows-size dependent objects here
}

float Game::AspectRatio() const
{
	return static_cast<float>(mWindowWidth) / mWindowHeight;
}

const ServiceContainer& Game::Services() const
{
	return mServices;
}

const std::vector<GameComponent*>& Library::Game::Components() const
{
	return mComponents;
}
