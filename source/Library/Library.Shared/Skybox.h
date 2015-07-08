#pragma once

#include "DrawableGameComponent.h"

namespace Library
{
	class Mesh;

	class Skybox : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(Skybox, DrawableGameComponent)
			typedef DirectX::XMFLOAT4X4 XMFLOAT4X4;

	public:
		Skybox(Game& game, Camera& camera, const std::wstring& cubeMapFileName, float scale);
		~Skybox();

		Skybox() = delete;
		Skybox(const Skybox& rhs) = delete;
		Skybox& operator=(const Skybox& rhs) = delete;

		virtual void Initialize() override;
		virtual void Update(const DX::StepTimer& gameTime) override;
		virtual void Draw() override;

	private:
		struct VertexCBufferPerObject
		{
			XMFLOAT4X4 WorldViewProjection;

			VertexCBufferPerObject() : WorldViewProjection() { }

			VertexCBufferPerObject(const XMFLOAT4X4& wvp) : WorldViewProjection(wvp) { }
		};

		void CreateVertexBuffer(ID3D11Device* device, const Mesh& mesh, ID3D11Buffer** vertexBuffer) const;

		std::wstring mCubeMapFileName;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> mVertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> mPixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> mInputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mIndexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mVertexCBufferPerObject;
		VertexCBufferPerObject mVertexCBufferPerObjectData;		
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mSkyboxTexture;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> mTrilinearSampler;
		UINT mIndexCount;        
		XMFLOAT4X4 mWorldMatrix;
		XMFLOAT4X4 mScaleMatrix;
	};
}