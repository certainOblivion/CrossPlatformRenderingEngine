#include "pch.h"
#include "TransparencyMappingRenderer.h"
#include "Cameras/Camera.h"
#include "Game.h"
#include "Lights\DirectionalLight.h"
#include <WICTextureLoader.h>
#include <WICTextureLoader.h>
#include "ProxyModel.h"
#include "Utility.h"
#include "RenderStateHelper.h"
#include "RasterizerStates.h"

using namespace DirectX;
using namespace Library;
using namespace DX;
using namespace std;
using namespace Rendering;

const size_t TransparencyMappingRenderer::Alignment = 16;
const XMFLOAT2 TransparencyMappingRenderer::LightRotationRate = XMFLOAT2(XM_2PI, XM_2PI);
const float TransparencyMappingRenderer::LightModulationRate = UCHAR_MAX;
const XMFLOAT4 TransparencyMappingRenderer::FogColor = XMFLOAT4(&ColorHelper::CornflowerBlue[0]);

TransparencyMappingRenderer::TransparencyMappingRenderer(Library::Game& game, Library::Camera& camera, bool& isControllingCamera):
GameDemo(camera, game), bIsCameraControlled(isControllingCamera), mRenderStateHelper(game)
{
}

void* TransparencyMappingRenderer::operator new(size_t size)
{
#if defined(DEBUG) || defined(_DEBUG)
	return _aligned_malloc_dbg(size, Alignment, __FILE__, __LINE__);
#else
	return _aligned_malloc(size, Alignment);
#endif
}

void TransparencyMappingRenderer::operator delete(void *p)
{
	if (p != nullptr)
	{
#if defined(DEBUG) || defined(_DEBUG)
		_aligned_free_dbg(p);
#else
		_aligned_free(p);
#endif
	}
}

void TransparencyMappingRenderer::Initialize(ID3D11Device2* device, ID3D11DeviceContext* deviceContext)
{
	GameDemo::Initialize(device, deviceContext);
	BlendStates::Initialize(device);
	// Load a compiled vertex shader
	std::vector<char> compiledVertexShader;
	Utility::LoadBinaryFile(L"Content\\Shaders\\TransparencyMappingVS.cso", compiledVertexShader);
	ThrowIfFailed(mGame.GetD3DDevice()->CreateVertexShader(&compiledVertexShader[0], compiledVertexShader.size(), nullptr, mVertexShader.GetAddressOf()), "ID3D11Device::CreatedVertexShader() failed.");

	// Load a compiled pixel shader
	std::vector<char> compiledPixelShader;
	Utility::LoadBinaryFile(L"Content\\Shaders\\TransparencyMappingPS.cso", compiledPixelShader);
	ThrowIfFailed(mGame.GetD3DDevice()->CreatePixelShader(&compiledPixelShader[0], compiledPixelShader.size(), nullptr, mPixelShader.GetAddressOf()), "ID3D11Device::CreatedPixelShader() failed.");

	// Create an input layout
	D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	ThrowIfFailed(mGame.GetD3DDevice()->CreateInputLayout(inputElementDescriptions, ARRAYSIZE(inputElementDescriptions), &compiledVertexShader[0], compiledVertexShader.size(), mInputLayout.GetAddressOf()), "ID3D11Device::CreateInputLayout() failed.");

	// Create a vertex buffer
	VertexPositionTextureNormal vertices[] =
	{
		VertexPositionTextureNormal(XMFLOAT4(-0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), Vector3Helper::Backward),
		VertexPositionTextureNormal(XMFLOAT4(-0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), Vector3Helper::Backward),
		VertexPositionTextureNormal(XMFLOAT4(0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), Vector3Helper::Backward),

		VertexPositionTextureNormal(XMFLOAT4(-0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), Vector3Helper::Backward),
		VertexPositionTextureNormal(XMFLOAT4(0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), Vector3Helper::Backward),
		VertexPositionTextureNormal(XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), Vector3Helper::Backward),
	};

	mVertexCount = ARRAYSIZE(vertices);
	CreateVertexBuffer(vertices, mVertexCount, mVertexBuffer.GetAddressOf());

	// Create constant buffers
	D3D11_BUFFER_DESC constantBufferDesc;
	ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));
	constantBufferDesc.ByteWidth = sizeof(mVertexCBufferPerObjectData);
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	ThrowIfFailed(mGame.GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, mVertexCBufferPerObject.GetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

	constantBufferDesc.ByteWidth = sizeof(mVertexCBufferPerFrameData);
	ThrowIfFailed(mGame.GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, mVertexCBufferPerFrame.GetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

	constantBufferDesc.ByteWidth = sizeof(mPixelCBufferPerObjectData);
	ThrowIfFailed(mGame.GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, mPixelCBufferPerObject.GetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

	constantBufferDesc.ByteWidth = sizeof(mPixelCBufferPerFrameData);
	ThrowIfFailed(mGame.GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, mPixelCBufferPerFrame.GetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

	// Load a texture
	std::wstring textureName = L"Content\\Textures\\Checkerboard.png";
	ThrowIfFailed(DirectX::CreateWICTextureFromFile(mGame.GetD3DDevice(), mGame.GetD3DDeviceContext(), textureName.c_str(), nullptr, mColorTexture.GetAddressOf()), "CreateWICTextureFromFile() failed.");

	textureName = L"Content\\Textures\\AlphaMask_32bpp.png";
	ThrowIfFailed(DirectX::CreateWICTextureFromFile(mGame.GetD3DDevice(), mGame.GetD3DDeviceContext(), textureName.c_str(), nullptr, mTransparencyMap.GetAddressOf()), "CreateWICTextureFromFile() failed.");

	// Create a texture sampler
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	ThrowIfFailed(mGame.GetD3DDevice()->CreateSamplerState(&samplerDesc, mTrilinearSampler.GetAddressOf()), "ID3D11Device::CreateSamplerState() failed.");

	mProxyModel = std::make_unique<ProxyModel>(mGame, mCamera, "Content\\Models\\DirectionalLightProxy.bin", 0.5f);
	mProxyModel->Initialize();
	mProxyModel->SetPosition(10.0f, 0.0, 0.0f);
	mProxyModel->ApplyRotation(XMMatrixRotationY(XM_PIDIV2));

	mDirectionalLight = std::make_unique<DirectionalLight>(mGame);
	const XMFLOAT3& lightdirection = mDirectionalLight->Direction();
	mVertexCBufferPerFrameData.LightDirection = XMFLOAT4(lightdirection.x, lightdirection.y, lightdirection.z, 0.0f);

	const XMFLOAT3& cameraPosition = mCamera.Position();
	mVertexCBufferPerFrameData.CameraPosition = XMFLOAT4(cameraPosition.x, cameraPosition.y, cameraPosition.z, 1.0f);
	mPixelCBufferPerFrameData.CameraPosition = mVertexCBufferPerFrameData.CameraPosition;

	mVertexCBufferPerFrameData.FogStart = mFogStart;
	mVertexCBufferPerFrameData.FogRange = mFogRange;
	mPixelCBufferPerFrameData.FogColor = FogColor;

	XMStoreFloat4x4(&mWorldMatrix, XMMatrixScaling(10.0f, 10.0f, 10.0f));

}

void TransparencyMappingRenderer::Draw(ID3D11DeviceContext* direct3DDeviceContext)
{
	direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	direct3DDeviceContext->IASetInputLayout(mInputLayout.Get());

	UINT stride = sizeof(VertexPositionTextureNormal);
	UINT offset = 0;
	direct3DDeviceContext->IASetVertexBuffers(0, 1, mVertexBuffer.GetAddressOf(), &stride, &offset);

	direct3DDeviceContext->VSSetShader(mVertexShader.Get(), nullptr, 0);
	direct3DDeviceContext->PSSetShader(mPixelShader.Get(), nullptr, 0);

	XMMATRIX worldMatrix = XMLoadFloat4x4(&mWorldMatrix);
	XMMATRIX wvp = worldMatrix * mCamera.ViewProjectionMatrix();
	XMStoreFloat4x4(&mVertexCBufferPerObjectData.WorldViewProjection, XMMatrixTranspose(wvp));
	XMStoreFloat4x4(&mVertexCBufferPerObjectData.World, XMMatrixTranspose(worldMatrix));

	const XMFLOAT3& cameraPosition = mCamera.Position();
	mVertexCBufferPerFrameData.CameraPosition = XMFLOAT4(cameraPosition.x, cameraPosition.y, cameraPosition.z, 1.0f);
	mPixelCBufferPerFrameData.CameraPosition = mVertexCBufferPerFrameData.CameraPosition;

	direct3DDeviceContext->UpdateSubresource(mVertexCBufferPerFrame.Get(), 0, nullptr, &mVertexCBufferPerFrameData, 0, 0);
	direct3DDeviceContext->UpdateSubresource(mVertexCBufferPerObject.Get(), 0, nullptr, &mVertexCBufferPerObjectData, 0, 0);
	direct3DDeviceContext->UpdateSubresource(mPixelCBufferPerFrame.Get(), 0, nullptr, &mPixelCBufferPerFrameData, 0, 0);
	direct3DDeviceContext->UpdateSubresource(mPixelCBufferPerObject.Get(), 0, nullptr, &mPixelCBufferPerObjectData, 0, 0);

	static ID3D11Buffer* VSConstantBuffers[] = { mVertexCBufferPerFrame.Get(), mVertexCBufferPerObject.Get() };
	direct3DDeviceContext->VSSetConstantBuffers(0, ARRAYSIZE(VSConstantBuffers), VSConstantBuffers);

	static ID3D11Buffer* PSConstantBuffers[] = { mPixelCBufferPerFrame.Get(), mPixelCBufferPerObject.Get() };
	direct3DDeviceContext->PSSetConstantBuffers(0, ARRAYSIZE(PSConstantBuffers), PSConstantBuffers);

	static ID3D11ShaderResourceView* PSShaderResources[] = { mColorTexture.Get(), mTransparencyMap.Get() };
	direct3DDeviceContext->PSSetShaderResources(0, ARRAYSIZE(PSShaderResources), PSShaderResources);
	direct3DDeviceContext->PSSetSamplers(0, 1, mTrilinearSampler.GetAddressOf());

	mRenderStateHelper.SaveAll();

	direct3DDeviceContext->OMSetBlendState(BlendStates::AlphaBlending.Get(), 0, 0xFFFFFFFF);
	direct3DDeviceContext->Draw(mVertexCount, 0);
	mRenderStateHelper.RestoreBlendState();

	mProxyModel->Draw();
	mRenderStateHelper.RestoreAll();
}

void TransparencyMappingRenderer::Update(const DX::StepTimer& gameTime)
{
	UpdateAmbientLight(gameTime);
	UpdateDirectionalLight(gameTime);
	UpdateSpecularLight(gameTime);
	mProxyModel->Update(gameTime);
}

void Rendering::TransparencyMappingRenderer::CreateVertexBuffer(VertexPositionTextureNormal* vertices, UINT vertexCount, ID3D11Buffer** vertexBuffer) const
{
	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
	vertexBufferDesc.ByteWidth = sizeof(VertexPositionTextureNormal) * vertexCount;
	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertexSubResourceData;
	ZeroMemory(&vertexSubResourceData, sizeof(vertexSubResourceData));
	vertexSubResourceData.pSysMem = vertices;
	ThrowIfFailed(mGame.GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, vertexBuffer), "ID3D11Device::CreateBuffer() failed.");
}

void Rendering::TransparencyMappingRenderer::UpdateAmbientLight(const DX::StepTimer& gameTime)
{
	static float ambientIntensity = 0.0f;

	auto gamePadState = mGamePad->CurrentState();
	if (gamePadState.IsConnected())
	{
		if (mGamePad->IsButtonDown(GamePadButton::A) && ambientIntensity <= 1.0f)
		{
			ambientIntensity += static_cast<float>(gameTime.GetElapsedSeconds());
			ambientIntensity = min(ambientIntensity, 1.0f);

			mPixelCBufferPerFrameData.AmbientColor = XMFLOAT4(ambientIntensity, ambientIntensity, ambientIntensity, 1.0f);
		}

		if (mGamePad->IsButtonDown(GamePadButton::B) && ambientIntensity >= 0.0f)
		{
			ambientIntensity -= (float)gameTime.GetElapsedSeconds();
			ambientIntensity = max(ambientIntensity, 0.0f);

			mPixelCBufferPerFrameData.AmbientColor = XMFLOAT4(ambientIntensity, ambientIntensity, ambientIntensity, 1.0f);
		}
	}
}

void Rendering::TransparencyMappingRenderer::UpdateDirectionalLight(const DX::StepTimer& gameTime)
{
	static float directionalIntensity = 1.0f;
	float elapsedTime = static_cast<float>(gameTime.GetElapsedSeconds());

	auto gamepadState = mGamePad->CurrentState();
	if (gamepadState.IsConnected())
	{
		// Update directional light intensity		
		if (mGamePad->IsButtonDown(GamePadButton::X) && directionalIntensity < 1.0f)
		{
			directionalIntensity += elapsedTime;
			directionalIntensity = min(directionalIntensity, 1.0f);

			mPixelCBufferPerFrameData.LightColor = XMFLOAT4(directionalIntensity, directionalIntensity, directionalIntensity, 1.0f);
			mDirectionalLight->SetColor(mPixelCBufferPerFrameData.LightColor);
		}
		if (mGamePad->IsButtonDown(GamePadButton::Y) && directionalIntensity > 0.0f)
		{
			directionalIntensity -= elapsedTime;
			directionalIntensity = max(directionalIntensity, 0.0f);

			mPixelCBufferPerFrameData.LightColor = XMFLOAT4(directionalIntensity, directionalIntensity, directionalIntensity, 1.0f);
			mDirectionalLight->SetColor(mPixelCBufferPerFrameData.LightColor);
		}

		// Rotate directional light
		XMFLOAT2 rotationAmount = Vector2Helper::Zero;
		if (!bIsCameraControlled)
		{
			if (gamepadState.IsLeftThumbStickLeft())
			{
				rotationAmount.x += LightRotationRate.x * elapsedTime;
			}
			if (gamepadState.IsLeftThumbStickRight())
			{
				rotationAmount.x -= LightRotationRate.x * elapsedTime;
			}
			if (gamepadState.IsLeftThumbStickUp())
			{
				rotationAmount.y += LightRotationRate.y * elapsedTime;
			}
			if (gamepadState.IsLeftThumbStickDown())
			{
				rotationAmount.y -= LightRotationRate.y * elapsedTime;
			}
		}

		XMMATRIX lightRotationMatrix = XMMatrixIdentity();
		if (rotationAmount.x != 0)
		{
			lightRotationMatrix = XMMatrixRotationY(rotationAmount.x);
		}

		if (rotationAmount.y != 0)
		{
			XMMATRIX lightRotationAxisMatrix = XMMatrixRotationAxis(mDirectionalLight->RightVector(), rotationAmount.y);
			lightRotationMatrix *= lightRotationAxisMatrix;
		}

		if (rotationAmount.x != 0.0f || rotationAmount.y != 0.0f)
		{
			mDirectionalLight->ApplyRotation(lightRotationMatrix);
			mProxyModel->ApplyRotation(lightRotationMatrix);

			const XMFLOAT3& lightdirection = mDirectionalLight->Direction();
			mVertexCBufferPerFrameData.LightDirection = XMFLOAT4(lightdirection.x, lightdirection.y, lightdirection.z, 0.0f);
		}
	}
}

void Rendering::TransparencyMappingRenderer::UpdateSpecularLight(const DX::StepTimer& gameTime)
{
	static float specularIntensity = 1.0f;
	GamePad::State gamePadState = mGamePad->CurrentState();
	if (gamePadState.IsConnected())
	{
		if (gamePadState.IsLeftTriggerPressed() && specularIntensity <= 1.0f)
		{
			specularIntensity += static_cast<float>(gameTime.GetElapsedSeconds());
			specularIntensity = min(specularIntensity, 1.0f);

			mPixelCBufferPerObjectData.SpecularColor = XMFLOAT3(specularIntensity, specularIntensity, specularIntensity);
		}

		if (gamePadState.IsRightTriggerPressed() && specularIntensity >= 0.0f)
		{
			specularIntensity -= (float)gameTime.GetElapsedSeconds();
			specularIntensity = max(specularIntensity, 0.0f);

			mPixelCBufferPerObjectData.SpecularColor = XMFLOAT3(specularIntensity, specularIntensity, specularIntensity);
		}

		static float specularPower = mPixelCBufferPerObjectData.SpecularPower;

		if (mGamePad->IsButtonDown(GamePadButton::DPadUp) && specularPower < UCHAR_MAX)
		{
			specularPower += LightModulationRate * static_cast<float>(gameTime.GetElapsedSeconds());
			specularPower = min(specularPower, static_cast<float>(UCHAR_MAX));

			mPixelCBufferPerObjectData.SpecularPower = specularPower;
		}

		if (mGamePad->IsButtonDown(GamePadButton::DPadDown) && specularPower > 1.0f)
		{
			specularPower -= LightModulationRate * static_cast<float>(gameTime.GetElapsedSeconds());
			specularPower = max(specularPower, 1.0f);

			mPixelCBufferPerObjectData.SpecularPower = specularPower;
		}
	}
}

