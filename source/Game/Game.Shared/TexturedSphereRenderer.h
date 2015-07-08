#pragma once
#include "Mesh.h"
#include "Cameras\Camera.h"
#include "GameDemo.h"
#include "Game.h"

namespace Rendering
{
	class TexturedSphereRenderer : public Library::GameDemo
	{
	public:
		TexturedSphereRenderer(Library::Game& game, Library::Camera& camera);
		virtual ~TexturedSphereRenderer() = default;

		TexturedSphereRenderer() = delete;
		TexturedSphereRenderer(const TexturedSphereRenderer& rhs) = delete;
		TexturedSphereRenderer& operator=(const TexturedSphereRenderer& rhs) = delete;

		virtual void Initialize(ID3D11Device2* device, ID3D11DeviceContext* deviceContext) override;
		virtual void Draw(ID3D11DeviceContext* direct3DDeviceContext) override;
		virtual void Update(const DX::StepTimer& gameTime) override;
		void CreateVertexBuffer(ID3D11Device* device, const Library::Mesh& mesh, ID3D11Buffer** vertexBuffer) const;
	private:
		struct CBufferPerObject
		{
			DirectX::XMFLOAT4X4 WorldViewProjection;

			CBufferPerObject() : WorldViewProjection() { }

			CBufferPerObject(const DirectX::XMFLOAT4X4& wvp) : WorldViewProjection(wvp) { }
		};

		CBufferPerObject mCBufferPerObject;
	};
}

