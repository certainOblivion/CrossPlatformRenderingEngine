#pragma once
#include <DirectXMath.h>
#include "ProxyModel.h"
#include "Lights\PointLight.h"
#include <wrl\client.h>
#include "GameDemo.h"


namespace Rendering
{
	class PointLightRenderer :	public Library::GameDemo
	{
	public:
		static void* operator new(size_t size);
		static void operator delete(void *p);
		PointLightRenderer(Library::Game& game, Library::Camera& camera, bool& bIsCameraBeingControlled);
		virtual ~PointLightRenderer();

		PointLightRenderer() = delete;
		PointLightRenderer(const PointLightRenderer& rhs) = delete;
		PointLightRenderer& operator=(const PointLightRenderer& rhs) = delete;

		virtual void Initialize(ID3D11Device2* device, ID3D11DeviceContext* deviceContext) override;
		virtual void Draw(ID3D11DeviceContext* direct3DDeviceContext) override;
		virtual void Update(const DX::StepTimer& gameTime) override;
	private:
		static const size_t Alignment;
		static const float LightModulationRate;
		static const float LightMovementRate;
		void CreateVertexBuffer(ID3D11Device* device, const Library::Mesh& mesh, ID3D11Buffer** vertexBuffer) const;
		struct VertexCBufferPerObject
		{
			DirectX::XMFLOAT4X4 WorldViewProjection;
			DirectX::XMFLOAT4X4 World;

			VertexCBufferPerObject() : WorldViewProjection(), World() { }

			VertexCBufferPerObject(const DirectX::XMFLOAT4X4& wvp, const DirectX::XMFLOAT4X4& world) : WorldViewProjection(wvp), World(world) { }
		};

		struct VertexCBufferPerFrame
		{
			DirectX::XMFLOAT3 LightPosition;
			float LightRadius;

			VertexCBufferPerFrame() : LightPosition(0.0f, 0.0f, 0.0f), LightRadius(10.0f) { }

			VertexCBufferPerFrame(const DirectX::XMFLOAT3& lightPosition, float lightRadius)
				: LightPosition(lightPosition), LightRadius(lightRadius)
			{
			}
		};

		struct PixelCBufferPerObject
		{
			DirectX::XMFLOAT3 SpecularColor;
			float SpecularPower;

			PixelCBufferPerObject() : SpecularColor(1.0f, 1.0f, 1.0f), SpecularPower(25.0f) { }

			PixelCBufferPerObject(const DirectX::XMFLOAT3& specularColor, float specularPower)
				: SpecularColor(specularColor), SpecularPower(specularPower)
			{
			}
		};

		__declspec(align(16))
		struct PixelCBufferPerFrame
		{
			DirectX::XMFLOAT4 AmbientColor;
			DirectX::XMFLOAT4 LightColor;
			DirectX::XMFLOAT3 LightPosition;
			float Padding;
			DirectX::XMFLOAT3 CameraPosition;

			PixelCBufferPerFrame()
				: AmbientColor(0.0f, 0.0f, 0.0f, 0.0f), LightColor(1.0f, 1.0f, 1.0f, 1.0f),
				LightPosition(0.0f, 0.0f, 0.0f), Padding(D3D11_FLOAT32_MAX), CameraPosition(0.0f, 0.0f, 0.0f)
			{
			}

			PixelCBufferPerFrame(const DirectX::XMFLOAT4& ambientColor, const DirectX::XMFLOAT4& lightColor, const DirectX::XMFLOAT3& lightPosition, const DirectX::XMFLOAT3& cameraPosition)
				: AmbientColor(ambientColor), LightColor(lightColor),
				LightPosition(lightPosition), CameraPosition(cameraPosition)
			{
			}
		};

		void UpdateAmbientLight(const DX::StepTimer& gameTime);
		void UpdatePointLight(const DX::StepTimer& gameTime);
		void UpdateSpecularLight(const DX::StepTimer& gameTime);

		Microsoft::WRL::ComPtr<ID3D11Buffer> mVertexCBufferPerObject;
		VertexCBufferPerObject mVertexCBufferPerObjectData;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mVertexCBufferPerFrame;
		VertexCBufferPerFrame mVertexCBufferPerFrameData;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mPixelCBufferPerObject;
		PixelCBufferPerObject mPixelCBufferPerObjectData;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mPixelCBufferPerFrame;
		PixelCBufferPerFrame mPixelCBufferPerFrameData;

		std::unique_ptr<Library::ProxyModel> mProxyModel;
		std::unique_ptr<Library::PointLight> mPointLight;
		bool& bIsCameraControlled;
	};
}

