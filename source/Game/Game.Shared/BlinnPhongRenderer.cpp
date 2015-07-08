#include "pch.h"
#include "BlinnPhongRenderer.h"
#include "Utility.h"
#include <DDSTextureLoader.h>
#include "Cameras\Camera.h"
#include "VertexDeclarations.h"

using namespace DX;
using namespace DirectX;
using namespace Library;
using namespace Rendering;
using namespace std;

const XMFLOAT2 BlinnPhongRenderer::LightRotationRate = XMFLOAT2(XM_2PI, XM_2PI);
const float BlinnPhongRenderer::LightModulationRate = UCHAR_MAX;

void BlinnPhongRenderer::Initialize(ID3D11Device2* device, ID3D11DeviceContext* deviceContext)
{
	GameDemo::Initialize(device, deviceContext);
	// Load a compiled vertex shader
	std::vector<char> compiledVertexShader;
	Utility::LoadBinaryFile(L"Content\\Shaders\\BlinnPhongVS.cso", compiledVertexShader);
	ThrowIfFailed(device->CreateVertexShader(&compiledVertexShader[0], compiledVertexShader.size(), nullptr, mVertexShader.GetAddressOf()), "ID3D11Device::CreatedVertexShader() failed.");

	// Load a compiled pixel shader
	std::vector<char> compiledPixelShader;
	Utility::LoadBinaryFile(L"Content\\Shaders\\BlinnPhongPS.cso", compiledPixelShader);
	ThrowIfFailed(device->CreatePixelShader(&compiledPixelShader[0], compiledPixelShader.size(), nullptr, mPixelShader.GetAddressOf()), "ID3D11Device::CreatedPixelShader() failed.");

	// Create an input layout
	D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	ThrowIfFailed(device->CreateInputLayout(inputElementDescriptions, ARRAYSIZE(inputElementDescriptions), &compiledVertexShader[0], compiledVertexShader.size(), mInputLayout.GetAddressOf()), "ID3D11Device::CreateInputLayout() failed.");

	// Load the model
	std::unique_ptr<Model> model = std::make_unique<Model>("Content\\Models\\texturedSphere.bin", true);

	// Create vertex and index buffers for the model
	Mesh* mesh = model->Meshes().at(0);
	CreateVertexBuffer(device, *mesh, mVertexBuffer.GetAddressOf());
	mesh->CreateIndexBuffer(mIndexBuffer.GetAddressOf(), device);
	mIndexCount = (UINT)mesh->Indices().size();

	// Create constant buffers
	D3D11_BUFFER_DESC constantBufferDesc;
	ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));
	constantBufferDesc.ByteWidth = sizeof(mVertexCBufferPerObjectData);
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	ThrowIfFailed(device->CreateBuffer(&constantBufferDesc, nullptr, mVertexCBufferPerObject.GetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

	constantBufferDesc.ByteWidth = sizeof(mVertexCBufferPerFrameData);
	ThrowIfFailed(device->CreateBuffer(&constantBufferDesc, nullptr, mVertexCBufferPerFrame.GetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

	constantBufferDesc.ByteWidth = sizeof(mPixelCBufferPerObjectData);
	ThrowIfFailed(device->CreateBuffer(&constantBufferDesc, nullptr, mPixelCBufferPerObject.GetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

	constantBufferDesc.ByteWidth = sizeof(mPixelCBufferPerFrameData);
	ThrowIfFailed(device->CreateBuffer(&constantBufferDesc, nullptr, mPixelCBufferPerFrame.GetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

	// Load a texture
	std::wstring textureName = L"Content\\Textures\\Earthatday.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, textureName.c_str(), nullptr, mColorTexture.GetAddressOf()), "CreateDDSTextureFromFile() failed.");

	// Create a texture sampler
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	ThrowIfFailed(device->CreateSamplerState(&samplerDesc, mColorSampler.GetAddressOf()), "ID3D11Device::CreateSamplerState() failed.");

	mProxyModel = std::make_unique<ProxyModel>(mGame, mCamera, "Content\\Models\\DirectionalLightProxy.bin", 0.5f);
	mProxyModel->Initialize();
	mProxyModel->SetPosition(10.0f, 0.0, 0.0f);
	mProxyModel->ApplyRotation(DirectX::XMMatrixRotationY(DirectX::XM_PIDIV2));

	mDirectionalLight = std::make_unique<DirectionalLight>(mGame);
	const XMFLOAT3& lightdirection = mDirectionalLight->Direction();
	mVertexCBufferPerFrameData.LightDirection = XMFLOAT4(lightdirection.x, lightdirection.y, lightdirection.z, 0.0f);

	const XMFLOAT3& cameraPosition = mCamera.Position();
	mVertexCBufferPerFrameData.CameraPosition = XMFLOAT4(cameraPosition.x, cameraPosition.y, cameraPosition.z, 1.0f);
}

void BlinnPhongRenderer::Draw(ID3D11DeviceContext* direct3DDeviceContext)
{
	direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	direct3DDeviceContext->IASetInputLayout(mInputLayout.Get());

	UINT stride = sizeof(VertexPositionTextureNormal);
	UINT offset = 0;
	direct3DDeviceContext->IASetVertexBuffers(0, 1, mVertexBuffer.GetAddressOf(), &stride, &offset);
	direct3DDeviceContext->IASetIndexBuffer(mIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	direct3DDeviceContext->VSSetShader(mVertexShader.Get(), nullptr, 0);
	direct3DDeviceContext->PSSetShader(mPixelShader.Get(), nullptr, 0);

	XMMATRIX worldMatrix = XMLoadFloat4x4(&mWorldMatrix);
	XMMATRIX wvp = worldMatrix * mCamera.ViewProjectionMatrix();
	XMStoreFloat4x4(&mVertexCBufferPerObjectData.WorldViewProjection, XMMatrixTranspose(wvp));
	XMStoreFloat4x4(&mVertexCBufferPerObjectData.World, XMMatrixTranspose(worldMatrix));

	const XMFLOAT3& cameraPosition = mCamera.Position();
	mVertexCBufferPerFrameData.CameraPosition = XMFLOAT4(cameraPosition.x, cameraPosition.y, cameraPosition.z, 1.0f);

	direct3DDeviceContext->UpdateSubresource(mVertexCBufferPerFrame.Get(), 0, nullptr, &mVertexCBufferPerFrameData, 0, 0);
	direct3DDeviceContext->UpdateSubresource(mVertexCBufferPerObject.Get(), 0, nullptr, &mVertexCBufferPerObjectData, 0, 0);
	direct3DDeviceContext->UpdateSubresource(mPixelCBufferPerFrame.Get(), 0, nullptr, &mPixelCBufferPerFrameData, 0, 0);
	direct3DDeviceContext->UpdateSubresource(mPixelCBufferPerObject.Get(), 0, nullptr, &mPixelCBufferPerObjectData, 0, 0);

	static ID3D11Buffer* VSConstantBuffers[] = { mVertexCBufferPerFrame.Get(), mVertexCBufferPerObject.Get() };
	direct3DDeviceContext->VSSetConstantBuffers(0, ARRAYSIZE(VSConstantBuffers), VSConstantBuffers);

	static ID3D11Buffer* PSConstantBuffers[] = { mPixelCBufferPerFrame.Get(), mPixelCBufferPerObject.Get() };
	direct3DDeviceContext->PSSetConstantBuffers(0, ARRAYSIZE(PSConstantBuffers), PSConstantBuffers);

	direct3DDeviceContext->PSSetShaderResources(0, 1, mColorTexture.GetAddressOf());
	direct3DDeviceContext->PSSetSamplers(0, 1, mColorSampler.GetAddressOf());

	direct3DDeviceContext->DrawIndexed(mIndexCount, 0, 0);

	mProxyModel->Draw();
}

void BlinnPhongRenderer::Update(const DX::StepTimer& gameTime)
{
	UpdateAmbientLight(gameTime);
	UpdateDirectionalLight(gameTime);
	UpdateSpecularLight(gameTime);
	mProxyModel->Update(gameTime);
}

BlinnPhongRenderer::BlinnPhongRenderer(Library::Game& game, Library::Camera& camera, bool& bIsCameraBeingControlled) :
GameDemo(camera, game), bIsCameraControlled(bIsCameraBeingControlled)
{

}

void BlinnPhongRenderer::CreateVertexBuffer(ID3D11Device* device, const Mesh& mesh, ID3D11Buffer** vertexBuffer) const
{
	const std::vector<XMFLOAT3>& sourceVertices = mesh.Vertices();
	std::vector<XMFLOAT3>* textureCoordinates = mesh.TextureCoordinates().at(0);
	assert(textureCoordinates->size() == sourceVertices.size());
	const std::vector<XMFLOAT3>& normals = mesh.Normals();
	assert(normals.size() == sourceVertices.size());

	std::vector<VertexPositionTextureNormal> vertices;
	vertices.reserve(sourceVertices.size());
	for (UINT i = 0; i < sourceVertices.size(); i++)
	{
		XMFLOAT3 position = sourceVertices.at(i);
		XMFLOAT3 uv = textureCoordinates->at(i);
		XMFLOAT3 normal = normals.at(i);
		vertices.push_back(VertexPositionTextureNormal(XMFLOAT4(position.x, position.y, position.z, 1.0f), XMFLOAT2(uv.x, uv.y), normal));
	}

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
	vertexBufferDesc.ByteWidth = (UINT)(sizeof(VertexPositionTextureNormal) * vertices.size());
	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertexSubResourceData;
	ZeroMemory(&vertexSubResourceData, sizeof(vertexSubResourceData));
	vertexSubResourceData.pSysMem = &vertices[0];
	ThrowIfFailed(device->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, vertexBuffer), "ID3D11Device::CreateBuffer() failed.");
}

void BlinnPhongRenderer::UpdateAmbientLight(const DX::StepTimer& gameTime)
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

void BlinnPhongRenderer::UpdateDirectionalLight(const DX::StepTimer& gameTime)
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

void BlinnPhongRenderer::UpdateSpecularLight(const DX::StepTimer& gameTime)
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
