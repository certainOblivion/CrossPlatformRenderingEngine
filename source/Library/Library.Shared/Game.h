#pragma once
#include "pch.h"

namespace Library
{
	struct VertexPositionColor
	{
		DirectX::XMFLOAT4 Position;
		DirectX::XMFLOAT4 Color;

		VertexPositionColor() {}

		VertexPositionColor(const DirectX::XMFLOAT4& position, const DirectX::XMFLOAT4& color)
			: Position(position), Color(color) {}
	};

	class Game
	{
	public:
		Game();
		~Game() = default;

		virtual void Update() = 0;
		virtual bool Render() = 0;
		virtual void OnDeviceLost() = 0;
		virtual void OnDeviceRestored() = 0;
		float AspectRatio() const;
		// D3D Accessors.
		ID3D11Device2* GetD3DDevice() const								{ return m_d3dDevice.Get(); }
		ID3D11DeviceContext2* GetD3DDeviceContext() const				{ return m_d3dContext.Get(); }
		IDXGISwapChain1* GetSwapChain() const							{ return m_swapChain.Get(); }
		D3D_FEATURE_LEVEL GetDeviceFeatureLevel() const					{ return m_featureLevel; }
		ID3D11RenderTargetView*	GetBackBufferRenderTargetView() const	{ return m_renderTargetView.Get(); }
		ID3D11DepthStencilView* GetDepthStencilView() const				{ return m_depthStencilView.Get(); }
		const ServiceContainer& Services() const;
		const std::vector<GameComponent*>& Components() const;

	protected:
		void CreateDevice();
		void CreateResources(std::function<void*()> getWindow, std::function<double()>getWidth, std::function<double()>getHeight);

		DX::StepTimer m_timer;
		Microsoft::WRL::ComPtr<ID3D11Device2> m_d3dDevice;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext2> m_d3dContext;
		Microsoft::WRL::ComPtr<IDXGISwapChain1> m_swapChain;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilView;
		D3D_FEATURE_LEVEL m_featureLevel;
		D3D11_VIEWPORT m_screenViewport;

		Microsoft::WRL::ComPtr<ID2D1Factory2> m_d2dFactory;
		Microsoft::WRL::ComPtr<ID2D1Device1> m_d2dDevice;
		Microsoft::WRL::ComPtr<ID2D1DeviceContext1> m_d2dContext;
		Microsoft::WRL::ComPtr<ID2D1Bitmap1> m_d2dTargetBitmap;
		uint32_t mWindowWidth;
		uint32_t mWindowHeight;

		std::vector<GameComponent*> mComponents;
		std::vector<std::shared_ptr<GameDemo>> mDemos;
		ServiceContainer mServices;
	};
}

