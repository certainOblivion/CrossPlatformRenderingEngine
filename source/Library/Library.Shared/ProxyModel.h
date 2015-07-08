#pragma once

#include "pch.h"
#include "DrawableGameComponent.h"

namespace Library
{
	class Mesh;

	class ProxyModel : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(ProxyModel, DrawableGameComponent)

	public:
		ProxyModel(Game& game, Camera& camera, const std::string& modelFileName, float scale = 1.0f);
		~ProxyModel() = default;

		ProxyModel() = delete;
		ProxyModel(const ProxyModel& rhs) = delete;
		ProxyModel& operator=(const ProxyModel& rhs) = delete;

		const DirectX::XMFLOAT3& Position() const;
        const DirectX::XMFLOAT3& Direction() const;
        const DirectX::XMFLOAT3& Up() const;
        const DirectX::XMFLOAT3& Right() const;

		DirectX::XMVECTOR PositionVector() const;
		DirectX::XMVECTOR DirectionVector() const;
		DirectX::XMVECTOR UpVector() const;
		DirectX::XMVECTOR RightVector() const;

		bool& DisplayWireframe();

		void SetPosition(FLOAT x, FLOAT y, FLOAT z);
		void SetPosition(DirectX::FXMVECTOR position);
        void SetPosition(const DirectX::XMFLOAT3& position);

		void ApplyRotation(DirectX::CXMMATRIX transform);
		void ApplyRotation(const DirectX::XMFLOAT4X4& transform);

		virtual void Initialize() override;
		virtual void Update(const DX::StepTimer& gameTime) override;
		virtual void Draw() override;

	private:
		struct VertexCBufferPerObject
		{
			DirectX::XMFLOAT4X4 WorldViewProjection;

			VertexCBufferPerObject() : WorldViewProjection() { }

			VertexCBufferPerObject(const DirectX::XMFLOAT4X4& wvp) : WorldViewProjection(wvp) { }
		};

		void CreateVertexBuffer(ID3D11Device* device, const Mesh& mesh, ID3D11Buffer** vertexBuffer) const;

		std::string mModelFileName;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> mVertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> mPixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> mInputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mIndexBuffer;
		UINT mIndexCount;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mVertexCBufferPerObject;
		VertexCBufferPerObject mVertexCBufferPerObjectData;
        
		DirectX::XMFLOAT4X4 mWorldMatrix;
		DirectX::XMFLOAT4X4 mScaleMatrix;

		bool mDisplayWireframe;
		DirectX::XMFLOAT3 mPosition;
		DirectX::XMFLOAT3 mDirection;
        DirectX::XMFLOAT3 mUp;
        DirectX::XMFLOAT3 mRight;
	};
}