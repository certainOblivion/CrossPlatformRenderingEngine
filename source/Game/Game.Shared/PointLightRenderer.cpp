#include "pch.h"
#include "PointLightRenderer.h"
#include <DDSTextureLoader.h>
#include "Utility.h"  
#include "ColorHelper.h"
#include "Cameras\Camera.h"
#include "VertexDeclarations.h"
#include <algorithm>
#include "Game.h"

using namespace Rendering;
using namespace Library;
using namespace DirectX;
using namespace std;

const size_t PointLightRenderer::Alignment = 16;
const float PointLightRenderer::LightModulationRate = UCHAR_MAX;
const float PointLightRenderer::LightMovementRate = 10.0f;

void* PointLightRenderer::operator new(size_t size)
{
#if defined(DEBUG) || defined(_DEBUG)
	return _aligned_malloc_dbg(size, Alignment, __FILE__, __LINE__);
#else
	return _aligned_malloc(size, Alignment);
#endif
}

void PointLightRenderer::operator delete(void *p)
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

PointLightRenderer::PointLightRenderer(Library::Game& game, Library::Camera& camera, bool& bIsCameraBeingControlled) :
GameDemo(camera, game), bIsCameraControlled(bIsCameraBeingControlled)
{

}

PointLightRenderer::~PointLightRenderer()
{
}

void PointLightRenderer::Initialize(ID3D11Device2* device, ID3D11DeviceContext* deviceContext)
{
	GameDemo::Initialize(device, deviceContext);
	// Load a compiled vertex shader
	std::vector<char> compiledVertexShader;
	Utility::LoadBinaryFile(L"Content\\Shaders\\PointLightVS.cso", compiledVertexShader);
	DX::ThrowIfFailed(device->CreateVertexShader(&compiledVertexShader[0], compiledVertexShader.size(), nullptr, mVertexShader.GetAddressOf()), "ID3D11Device::CreatedVertexShader() failed.");

	// Load a compiled pixel shader
	std::vector<char> compiledPixelShader;
	Utility::LoadBinaryFile(L"Content\\Shaders\\PointLightPS.cso", compiledPixelShader);
	DX::ThrowIfFailed(device->CreatePixelShader(&compiledPixelShader[0], compiledPixelShader.size(), nullptr, mPixelShader.GetAddressOf()), "ID3D11Device::CreatedPixelShader() failed.");

	// Create an input layout
	D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	DX::ThrowIfFailed(device->CreateInputLayout(inputElementDescriptions, ARRAYSIZE(inputElementDescriptions), &compiledVertexShader[0], compiledVertexShader.size(), mInputLayout.GetAddressOf()), "ID3D11Device::CreateInputLayout() failed.");

	// Load the model
	std::unique_ptr<Model> model = std::make_unique<Model>("Content\\Models\\texturedSphere.bin", true);

	// Create vertex and index buffers for the model
	Mesh* mesh = model->Meshes().at(0);
	CreateVertexBuffer(device, *mesh, mVertexBuffer.GetAddressOf());
	mesh->CreateIndexBuffer(&mIndexBuffer,device);
	mIndexCount = (UINT)mesh->Indices().size();

	// Create constant buffers
	D3D11_BUFFER_DESC constantBufferDesc;
	ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));
	constantBufferDesc.ByteWidth = sizeof(mVertexCBufferPerObjectData);
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	DX::ThrowIfFailed(device->CreateBuffer(&constantBufferDesc, nullptr, mVertexCBufferPerObject.GetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

	constantBufferDesc.ByteWidth = sizeof(mVertexCBufferPerFrameData);
	DX::ThrowIfFailed(device->CreateBuffer(&constantBufferDesc, nullptr, mVertexCBufferPerFrame.GetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

	constantBufferDesc.ByteWidth = sizeof(mPixelCBufferPerObjectData);
	DX::ThrowIfFailed(device->CreateBuffer(&constantBufferDesc, nullptr, mPixelCBufferPerObject.GetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

	constantBufferDesc.ByteWidth = sizeof(mPixelCBufferPerFrameData);
	DX::ThrowIfFailed(device->CreateBuffer(&constantBufferDesc, nullptr, mPixelCBufferPerFrame.GetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

	// Load a texture
	std::wstring textureName = L"Content\\Textures\\Earthatday.dds";
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, textureName.c_str(), nullptr, mColorTexture.GetAddressOf()), "CreateDDSTextureFromFile() failed.");

	// Create a texture sampler
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	DX::ThrowIfFailed(device->CreateSamplerState(&samplerDesc, mColorSampler.GetAddressOf()), "ID3D11Device::CreateSamplerState() failed.");

	mProxyModel = std::make_unique<ProxyModel>(mGame, mCamera, "Content\\Models\\pointlightproxy.bin", 0.5f);
	mProxyModel->Initialize();

	mPointLight = std::unique_ptr<Library::PointLight>(new PointLight(mGame));
	mPointLight->SetRadius(10.0f);
	mPointLight->SetPosition(5.0f, 0.0f, 10.0f);
	mVertexCBufferPerFrameData.LightPosition = mPointLight->Position();
	mVertexCBufferPerFrameData.LightRadius = mPointLight->Radius();

	mPixelCBufferPerFrameData.LightPosition = mPointLight->Position();
	mPixelCBufferPerFrameData.LightColor = ColorHelper::ToFloat4(mPointLight->Color(), true);
	mPixelCBufferPerFrameData.CameraPosition = mCamera.Position();
}

void PointLightRenderer::Draw(ID3D11DeviceContext* direct3DDeviceContext)
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

	mPixelCBufferPerFrameData.CameraPosition = mCamera.Position();

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

void PointLightRenderer::Update(const DX::StepTimer& gameTime)
{
	UpdateAmbientLight(gameTime);
	UpdatePointLight(gameTime);
	UpdateSpecularLight(gameTime);
	mProxyModel->Update(gameTime);
}

void PointLightRenderer::UpdateAmbientLight(const DX::StepTimer& gameTime)
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

void PointLightRenderer::UpdatePointLight(const DX::StepTimer& gameTime)
{
 	static float pointLightIntensity = 1.0f;
	float elapsedTime = static_cast<float>(gameTime.GetElapsedSeconds());
 
	GamePad::State gamePadState = mGamePad->CurrentState();
	if (gamePadState.IsConnected())
	{
		// Update point light intensity		
		if (mGamePad->IsButtonDown(GamePadButton::X) && pointLightIntensity <= 1.0f)
		{
			pointLightIntensity += elapsedTime;
			pointLightIntensity = min(pointLightIntensity, 1.0f);

			mPixelCBufferPerFrameData.LightColor = XMFLOAT4(pointLightIntensity, pointLightIntensity, pointLightIntensity, 1.0f);
			mPointLight->SetColor(mPixelCBufferPerFrameData.LightColor);
		}
		if (mGamePad->IsButtonDown(GamePadButton::Y) && pointLightIntensity >= 0.0f)
		{
			pointLightIntensity -= elapsedTime;
			pointLightIntensity = max(pointLightIntensity, 0.0f);

			mPixelCBufferPerFrameData.LightColor = XMFLOAT4(pointLightIntensity, pointLightIntensity, pointLightIntensity, 1.0f);
			mPointLight->SetColor(mPixelCBufferPerFrameData.LightColor);
		}

		// Move point light
		XMFLOAT3 movementAmount = Vector3Helper::Zero;
		if (!bIsCameraControlled)
		{
			if (gamePadState.IsLeftThumbStickLeft())
			{
				movementAmount.x = -1.0f;
			}

			if (gamePadState.IsLeftThumbStickRight())
			{
				movementAmount.x = 1.0f;
			}

			if (gamePadState.IsLeftThumbStickUp())
			{
				movementAmount.y = 1.0f;
			}

			if (gamePadState.IsLeftThumbStickDown())
			{
				movementAmount.y = -1.0f;
			}

			if (gamePadState.IsRightThumbStickLeft())
			{
				movementAmount.z = -1.0f;
			}

			if (gamePadState.IsRightThumbStickRight())
			{
				movementAmount.z = 1.0f;
			}
		}
		XMVECTOR movement = XMLoadFloat3(&movementAmount) * LightMovementRate * elapsedTime;
		mPointLight->SetPosition(mPointLight->PositionVector() + movement);
		mProxyModel->SetPosition(mPointLight->Position());
		mVertexCBufferPerFrameData.LightPosition = mPointLight->Position();

		mPixelCBufferPerFrameData.LightPosition = mPointLight->Position();
	}

}

void PointLightRenderer::UpdateSpecularLight(const DX::StepTimer& gameTime)
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

void PointLightRenderer::CreateVertexBuffer(ID3D11Device* device, const Mesh& mesh, ID3D11Buffer** vertexBuffer) const
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
	DX::ThrowIfFailed(device->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, vertexBuffer), "ID3D11Device::CreateBuffer() failed.");
}
