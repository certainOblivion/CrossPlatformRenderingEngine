#pragma once
#include "Game.h"

namespace Library
{
	class Skybox;
}

namespace Game
{
	class GameRenderer : public Library::Game
	{
	public:

#if (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
		GameRenderer(std::function<void*()> getWindow, std::function<uint32_t()>getWidth, std::function<uint32_t()>getHeight);
#else
		GameRenderer(void* window);
#endif // (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)


		// Basic game loop
		void Tick();
		virtual bool Render() override;

		// Rendering helpers
		void Clear();
		void Present();

		// Messages
		void OnActivated();
		void OnDeactivated();
		void OnSuspending();
		void OnResuming();
		void OnWindowSizeChanged();

		// Properties
		void GetDefaultSize(size_t& width, size_t& height) const;

		std::uint32_t GetWindowWidth();
		std::uint32_t GetWindowHeight();
		void* GetGameWindow();
#if (WINAPI_FAMILY!=WINAPI_FAMILY_DESKTOP_APP)
		void Trim();
		void SetLogicalSize(Windows::Foundation::Size size);
		void SetDpi(float dpi);
		void SetCurrentOrientation(Windows::Graphics::Display::DisplayOrientations currentOrientation);
		void SetWindow(void* window);
#endif

	private:

		virtual void Update() override;
		virtual void OnDeviceLost() override;
		virtual void OnDeviceRestored() override;
		void Initialize();
		// Application state
		void* m_window;

		// Direct3D Objects
		std::function<void*()> mGetWindowFunctor;
		std::function<double()>mGetWindowWidthFunctor;
		std::function<double()>mGetWindowHeightFunctor;
		//std::shared_ptr<Rendering::TexturedSphereRenderer> mTexturedSphereRenderer;
		std::shared_ptr<Library::Camera> mCamera;
		std::unique_ptr<Library::GamePadComponent> mGamePad;
		uint32_t mCurrentDemo;
		bool bIsResetInput;
		bool bIsControlCamera;
		std::unique_ptr<Library::Skybox> mSkybox;

#if !(WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
		Windows::Foundation::Size m_d3dRenderTargetSize;
		DXGI_MODE_ROTATION ComputeDisplayRotation();
		Windows::Graphics::Display::DisplayOrientations	m_nativeOrientation;
		Windows::Graphics::Display::DisplayOrientations	m_currentOrientation;
		float m_dpi;
		Windows::Foundation::Size m_outputSize;
		Windows::Foundation::Size m_logicalSize;
#endif
	};
}

