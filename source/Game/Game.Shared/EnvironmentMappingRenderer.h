#pragma once
#include "GameDemo.h"
namespace Rendering
{
	class EnvironmentMappingRenderer : public Library::GameDemo
	{
		typedef DirectX::XMFLOAT4X4 XMFLOAT4X4;
		typedef DirectX::XMFLOAT4 XMFLOAT4;
		typedef DirectX::XMFLOAT3 XMFLOAT3;
		typedef DirectX::XMFLOAT2 XMFLOAT2;
	public:
		static void* operator new(size_t size);
		static void operator delete(void *p);

		EnvironmentMappingRenderer(Library::Game& game, Library::Camera& camera);
		~EnvironmentMappingRenderer() = default;

		EnvironmentMappingRenderer() = delete;
		EnvironmentMappingRenderer(const EnvironmentMappingRenderer& rhs) = delete;
		EnvironmentMappingRenderer& operator=(const EnvironmentMappingRenderer& rhs) = delete;

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
			XMFLOAT3 CameraPosition;

			VertexCBufferPerFrame() : CameraPosition(0.0f, 0.0f, 0.0f) { }

			VertexCBufferPerFrame(const XMFLOAT3& cameraPosition) : CameraPosition(cameraPosition) { }
		};

		__declspec(align(16))
		struct PixelCBufferPerObject
		{
			float ReflectionAmount;

			PixelCBufferPerObject() : ReflectionAmount(25.0f) { }

			PixelCBufferPerObject(float reflectionAmount) : ReflectionAmount(reflectionAmount) { }
		}; 

		struct PixelCBufferPerFrame
		{
			XMFLOAT4 AmbientColor;
			XMFLOAT4 EnvColor;

			PixelCBufferPerFrame() : AmbientColor(0.0f, 0.0f, 0.0f, 0.0f), EnvColor(1.0f, 1.0f, 1.0f, 1.0f) { }

			PixelCBufferPerFrame(const XMFLOAT4& ambientColor, const XMFLOAT4& envColor)
				: AmbientColor(ambientColor), EnvColor(envColor)
			{
			}
		};

		void CreateVertexBuffer(ID3D11Device* device, const Library::Mesh& mesh, ID3D11Buffer** vertexBuffer) const;

		static const size_t Alignment;
		static const XMFLOAT4 EnvColor;
		static const XMFLOAT4 AmbientColor;

		Microsoft::WRL::ComPtr<ID3D11Buffer> mVertexCBufferPerObject;
		VertexCBufferPerObject mVertexCBufferPerObjectData;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mVertexCBufferPerFrame;
		VertexCBufferPerFrame mVertexCBufferPerFrameData;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mPixelCBufferPerObject;
		PixelCBufferPerObject mPixelCBufferPerObjectData;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mPixelCBufferPerFrame;
		PixelCBufferPerFrame mPixelCBufferPerFrameData;

		float mReflectionAmount;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mEnvironmentMap;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> mTrilinearSampler;
	};
}

