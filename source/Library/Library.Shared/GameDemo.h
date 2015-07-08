#pragma once
#include "Mesh.h"
#include "pch.h"
namespace Library
{
	class Game;
	class Camera;
	class GamePadComponent;

	struct VertexPositionTexture
	{
		DirectX::XMFLOAT4 Position;
		DirectX::XMFLOAT2 TextureCoordinates;

		VertexPositionTexture() { }

		VertexPositionTexture(const DirectX::XMFLOAT4& position, const DirectX::XMFLOAT2& textureCoordinates)
			: Position(position), TextureCoordinates(textureCoordinates) { }
	};

	class GameDemo
	{
	public:
		GameDemo(Camera& camera, Game& game);
		~GameDemo() = default;

		virtual void Initialize(ID3D11Device2* device, ID3D11DeviceContext* deviceContext);
		virtual void Draw(ID3D11DeviceContext* direct3DDeviceContext) = 0;
		virtual void Update(const DX::StepTimer& gameTime) = 0;

		GameDemo(const GameDemo&) = delete;
		GameDemo& operator=(const GameDemo&) = delete;

	protected:

		Microsoft::WRL::ComPtr<ID3D11VertexShader> mVertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> mPixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> mInputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mIndexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mConstantBuffer;
		Library::Camera& mCamera;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mColorTexture;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> mColorSampler;
		
		DirectX::XMFLOAT4X4 mWorldMatrix;
		UINT mIndexCount;
		Game& mGame;
		GamePadComponent* mGamePad;

	};
}