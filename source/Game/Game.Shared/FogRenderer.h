#pragma once
#include "GameDemo.h"
#include "Lights\DirectionalLight.h"
#include "ColorHelper.h"

namespace Library
{
	class ProxyModel;
}

namespace Rendering
{
	class FogRenderer :
		public Library::GameDemo
	{
		typedef DirectX::XMFLOAT4X4 XMFLOAT4X4;
		typedef DirectX::XMFLOAT4 XMFLOAT4;
		typedef DirectX::XMFLOAT3 XMFLOAT3;
		typedef DirectX::XMFLOAT2 XMFLOAT2;
	public:
		static void* operator new(size_t size);
		static void operator delete(void *p);

		FogRenderer(Library::Game& game, Library::Camera& camera, bool& isControllingCamera);
		~FogRenderer() = default;

		FogRenderer() = delete;
		FogRenderer(const FogRenderer& rhs) = delete;
		FogRenderer& operator=(const FogRenderer& rhs) = delete;

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

			PixelCBufferPerObject() : SpecularColor(1.0f, 1.0f, 1.0f), SpecularPower(25.0f) { }

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

		void CreateVertexBuffer(ID3D11Device* device, const Library::Mesh& mesh, ID3D11Buffer** vertexBuffer) const;
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
		std::unique_ptr<Library::DirectionalLight> mDirectionalLight;
		float mFogStart;
		float mFogRange;
		std::unique_ptr<Library::ProxyModel> mProxyModel;
		bool& bIsCameraControlled;
	};
}

