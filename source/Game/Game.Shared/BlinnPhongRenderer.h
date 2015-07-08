#pragma once
#include "Lights\DirectionalLight.h"
#include "ProxyModel.h"

namespace Rendering
{
	class BlinnPhongRenderer :
		public Library::GameDemo
	{
		typedef DirectX::XMFLOAT4X4 XMFLOAT4X4;
		typedef DirectX::XMFLOAT4 XMFLOAT4;
		typedef DirectX::XMFLOAT3 XMFLOAT3;
		typedef DirectX::XMFLOAT2 XMFLOAT2;
	public:
		BlinnPhongRenderer(Library::Game& game, Library::Camera& camera, bool& bIsCameraBeingControlled);
		virtual ~BlinnPhongRenderer() = default;

		BlinnPhongRenderer() = delete;
		BlinnPhongRenderer(const BlinnPhongRenderer& rhs) = delete;
		BlinnPhongRenderer& operator=(const BlinnPhongRenderer& rhs) = delete;

		virtual void Initialize(ID3D11Device2* device, ID3D11DeviceContext* deviceContext) override;
		virtual void Draw(ID3D11DeviceContext* direct3DDeviceContext) override;
		virtual void Update(const DX::StepTimer& gameTime) override;

	private:
		void CreateVertexBuffer(ID3D11Device* device, const Library::Mesh& mesh, ID3D11Buffer** vertexBuffer) const;
		struct VertexCBufferPerObject
		{
			XMFLOAT4X4 WorldViewProjection;
			XMFLOAT4X4 World;

			VertexCBufferPerObject() : WorldViewProjection(), World() { }

			VertexCBufferPerObject(const XMFLOAT4X4& wvp, const XMFLOAT4X4& world) : WorldViewProjection(wvp), World(world) { }
		};

		struct VertexCBufferPerFrame
		{
			XMFLOAT4 LightDirection;
			XMFLOAT4 CameraPosition;

			VertexCBufferPerFrame() : LightDirection(0.0f, 0.0f, -1.0f, 0.0f), CameraPosition(0.0f, 0.0f, 0.0f, 1.0f) { }

			VertexCBufferPerFrame(const XMFLOAT4& lightDirection, const XMFLOAT4& cameraPosition)
				: LightDirection(lightDirection), CameraPosition(cameraPosition)
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

			PixelCBufferPerFrame() : AmbientColor(0.0f, 0.0f, 0.0f, 0.0f), LightColor(1.0f, 1.0f, 1.0f, 1.0f) { }

			PixelCBufferPerFrame(const XMFLOAT4& ambientColor, const XMFLOAT4& lightColor) : AmbientColor(ambientColor), LightColor(lightColor) { }
		};

		void UpdateAmbientLight(const DX::StepTimer& gameTime);
		void UpdateDirectionalLight(const DX::StepTimer& gameTime);
		void UpdateSpecularLight(const DX::StepTimer& gameTime);

		static const XMFLOAT2 LightRotationRate;
		static const float LightModulationRate;

		Microsoft::WRL::ComPtr<ID3D11Buffer> mVertexCBufferPerObject;
		VertexCBufferPerObject mVertexCBufferPerObjectData;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mVertexCBufferPerFrame;
		VertexCBufferPerFrame mVertexCBufferPerFrameData;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mPixelCBufferPerObject;
		PixelCBufferPerObject mPixelCBufferPerObjectData;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mPixelCBufferPerFrame;
		PixelCBufferPerFrame mPixelCBufferPerFrameData;

		std::unique_ptr<Library::DirectionalLight> mDirectionalLight;
		std::unique_ptr<Library::ProxyModel> mProxyModel;

		bool& bIsCameraControlled;
	};
}

