#pragma once
#include "ColorHelper.h"
#include "GameDemo.h"

namespace Library
{
	class DirectionalLight;
	class ProxyModel;
	struct VertexPositionTextureNormal;
	class Game;
	class Camera;
	class RenderStateHelper;
}

namespace Rendering
{
	class TransparencyMappingRenderer :
		public Library::GameDemo
	{
		typedef DirectX::XMFLOAT4X4 XMFLOAT4X4;
		typedef DirectX::XMFLOAT4 XMFLOAT4;
		typedef DirectX::XMFLOAT3 XMFLOAT3;
		typedef DirectX::XMFLOAT2 XMFLOAT2;
	public:
		static void* operator new(size_t size);
		static void operator delete(void *p);

		TransparencyMappingRenderer(Library::Game& game, Library::Camera& camera, bool& isControllingCamera);
		~TransparencyMappingRenderer() = default;

		TransparencyMappingRenderer() = delete;
		TransparencyMappingRenderer(const TransparencyMappingRenderer& rhs) = delete;
		TransparencyMappingRenderer& operator=(const TransparencyMappingRenderer& rhs) = delete;

		virtual void Initialize(ID3D11Device2* device, ID3D11DeviceContext* deviceContext) override;
		virtual void Draw(ID3D11DeviceContext* direct3DDeviceContext) override;
		virtual void Update(const DX::StepTimer& gameTime) override;

	private:
		struct VertexCBufferPerObject
		{
			XMFLOAT4X4 WorldViewProjection;
			XMFLOAT4X4 World;

			VertexCBufferPerObject() : WorldViewProjection(), World() { }

			VertexCBufferPerObject(const XMFLOAT4X4& wvp, const XMFLOAT4X4& world) : WorldViewProjection(wvp), World(world) { }
		};

		__declspec(align(16))
		struct VertexCBufferPerFrame
		{
			XMFLOAT4 LightDirection;
			XMFLOAT4 CameraPosition;
			float FogStart;
			float FogRange;

			VertexCBufferPerFrame()
				: LightDirection(0.0f, 0.0f, -1.0f, 0.0f), CameraPosition(0.0f, 0.0f, 0.0f, 1.0f), FogStart(20.0f), FogRange(40.0f)
			{
			}

			VertexCBufferPerFrame(const XMFLOAT4& lightDirection, const XMFLOAT4& cameraPosition, float fogStart, float fogRange)
				: LightDirection(lightDirection), CameraPosition(cameraPosition), FogStart(fogStart), FogRange(fogRange)
			{
			}
		};

		struct PixelCBufferPerObject
		{
			XMFLOAT3 SpecularColor;
			float SpecularPower;

			PixelCBufferPerObject() : SpecularColor(0.5f, 0.5f, 0.5f), SpecularPower(255.0f) { }

			PixelCBufferPerObject(const XMFLOAT3& specularColor, float specularPower)
				: SpecularColor(specularColor), SpecularPower(specularPower)
			{
			}
		};

		struct PixelCBufferPerFrame
		{
			XMFLOAT4 AmbientColor;
			XMFLOAT4 LightColor;
			XMFLOAT4 FogColor;
			XMFLOAT4 CameraPosition;

			PixelCBufferPerFrame()
				: AmbientColor(0.0f, 0.0f, 0.0f, 0.0f), LightColor(1.0f, 1.0f, 1.0f, 1.0f), FogColor(&Library::ColorHelper::CornflowerBlue[0]),
				CameraPosition(0.0f, 0.0f, 0.0f, 1.0f)
			{
			}

			PixelCBufferPerFrame(const XMFLOAT4& ambientColor, const XMFLOAT4& lightColor, const XMFLOAT4& fogColor, const XMFLOAT4& cameraPosition)
				: AmbientColor(ambientColor), LightColor(lightColor), FogColor(fogColor), CameraPosition(cameraPosition)
			{
			}
		};

		void CreateVertexBuffer(Library::VertexPositionTextureNormal* vertices, UINT vertexCount, ID3D11Buffer** vertexBuffer) const;
		void UpdateAmbientLight(const DX::StepTimer& gameTime);
		void UpdateDirectionalLight(const DX::StepTimer& gameTime);
		void UpdateSpecularLight(const DX::StepTimer& gameTime);

		static const size_t Alignment;
		static const XMFLOAT2 LightRotationRate;
		static const float LightModulationRate;
		static const XMFLOAT4 FogColor;

		Microsoft::WRL::ComPtr<ID3D11Buffer> mVertexCBufferPerObject;
		VertexCBufferPerObject mVertexCBufferPerObjectData;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mVertexCBufferPerFrame;
		VertexCBufferPerFrame mVertexCBufferPerFrameData;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mPixelCBufferPerObject;
		PixelCBufferPerObject mPixelCBufferPerObjectData;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mPixelCBufferPerFrame;
		PixelCBufferPerFrame mPixelCBufferPerFrameData;
		UINT mVertexCount;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mTransparencyMap;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> mTrilinearSampler;
		std::unique_ptr<Library::DirectionalLight> mDirectionalLight;
		float mFogStart;
		float mFogRange;
		std::unique_ptr<Library::ProxyModel> mProxyModel;
		bool& bIsCameraControlled;
		Library::RenderStateHelper mRenderStateHelper;
	};
}

