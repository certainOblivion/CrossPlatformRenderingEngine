#include "pch.h"
#include "EnvironmentMappingRenderer.h"
#include <DDSTextureLoader.h>
#include <WICTextureLoader.h>
#include "Utility.h"
#include "Game.h"
#include "Cameras\Camera.h"

using namespace Rendering;
using namespace DirectX;
using namespace Library;
using namespace DX;
using namespace std;

const size_t EnvironmentMappingRenderer::Alignment = 16;
const XMFLOAT4 EnvironmentMappingRenderer::AmbientColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
const XMFLOAT4 EnvironmentMappingRenderer::EnvColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

void* EnvironmentMappingRenderer::operator new(size_t size)
{
#if defined(DEBUG) || defined(_DEBUG)
	return _aligned_malloc_dbg(size, Alignment, __FILE__, __LINE__);
#else
	return _aligned_malloc(size, Alignment);
#endif
}

void EnvironmentMappingRenderer::operator delete(void *p)
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

void EnvironmentMappingRenderer::CreateVertexBuffer(ID3D11Device* device, const Library::Mesh& mesh, ID3D11Buffer** vertexBuffer) const
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
	ThrowIfFailed(mGame.GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, vertexBuffer), "ID3D11Device::CreateBuffer() failed.");
}

void EnvironmentMappingRenderer::Initialize(ID3D11Device2* device, ID3D11DeviceContext* deviceContext)
{
	GameDemo::Initialize(device, deviceContext);
	// Load a compiled vertex shader
	std::vector<char> compiledVertexShader;
	Utility::LoadBinaryFile(L"Content\\Shaders\\EnvironmentMappingVS.cso", compiledVertexShader);
	ThrowIfFailed(mGame.GetD3DDevice()->CreateVertexShader(&compiledVertexShader[0], compiledVertexShader.size(), nullptr, mVertexShader.GetAddressOf()), "ID3D11Device::CreatedVertexShader() failed.");

	// Load a compiled pixel shader
	std::vector<char> compiledPixelShader;
	Utility::LoadBinaryFile(L"Content\\Shaders\\EnvironmentMappingPS.cso", compiledPixelShader);
	ThrowIfFailed(mGame.GetD3DDevice()->CreatePixelShader(&compiledPixelShader[0], compiledPixelShader.size(), nullptr, mPixelShader.GetAddressOf()), "ID3D11Device::CreatedPixelShader() failed.");

	// Create an input layout
	D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	ThrowIfFailed(mGame.GetD3DDevice()->CreateInputLayout(inputElementDescriptions, ARRAYSIZE(inputElementDescriptions), &compiledVertexShader[0], compiledVertexShader.size(), mInputLayout.GetAddressOf()), "ID3D11Device::CreateInputLayout() failed.");

	// Load the model
	std::unique_ptr<Model> model = std::unique_ptr<Model>(new Model("Content\\Models\\texturedSphere.bin", true));

	// Create vertex and index buffers for the model
	Mesh* mesh = model->Meshes().at(0);
	CreateVertexBuffer(mGame.GetD3DDevice(), *mesh, mVertexBuffer.GetAddressOf());
	mesh->CreateIndexBuffer(mIndexBuffer.GetAddressOf(), device);
	mIndexCount = (UINT)mesh->Indices().size();

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
	std::wstring textureName = L"Content\\Textures\\Maskonaive2_1024.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile(mGame.GetD3DDevice(), textureName.c_str(), nullptr, mEnvironmentMap.GetAddressOf()), "CreateDDSTextureFromFile() failed.");

	textureName = L"Content\\Textures\\Checkerboard.png";
	ThrowIfFailed(DirectX::CreateWICTextureFromFile(mGame.GetD3DDevice(), mGame.GetD3DDeviceContext(), textureName.c_str(), nullptr, mColorTexture.GetAddressOf()), "CreateWICTextureFromFile() failed.");

	// Create a texture sampler
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

	ThrowIfFailed(mGame.GetD3DDevice()->CreateSamplerState(&samplerDesc, mTrilinearSampler.GetAddressOf()), "ID3D11Device::CreateSamplerState() failed.");

	mPixelCBufferPerObjectData.ReflectionAmount = mReflectionAmount;
	mPixelCBufferPerFrameData.AmbientColor = AmbientColor;
	mPixelCBufferPerFrameData.EnvColor = EnvColor;
}

void EnvironmentMappingRenderer::Draw(ID3D11DeviceContext* direct3DDeviceContext)
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

	mVertexCBufferPerFrameData.CameraPosition = mCamera.Position();

	direct3DDeviceContext->UpdateSubresource(mVertexCBufferPerFrame.Get(), 0, nullptr, &mVertexCBufferPerFrameData, 0, 0);
	direct3DDeviceContext->UpdateSubresource(mVertexCBufferPerObject.Get(), 0, nullptr, &mVertexCBufferPerObjectData, 0, 0);
	direct3DDeviceContext->UpdateSubresource(mPixelCBufferPerFrame.Get(), 0, nullptr, &mPixelCBufferPerFrameData, 0, 0);
	direct3DDeviceContext->UpdateSubresource(mPixelCBufferPerObject.Get(), 0, nullptr, &mPixelCBufferPerObjectData, 0, 0);

	static ID3D11Buffer* VSConstantBuffers[] = { mVertexCBufferPerFrame.Get(), mVertexCBufferPerObject.Get() };
	direct3DDeviceContext->VSSetConstantBuffers(0, ARRAYSIZE(VSConstantBuffers), VSConstantBuffers);

	static ID3D11Buffer* PSConstantBuffers[] = { mPixelCBufferPerFrame.Get(), mPixelCBufferPerObject.Get() };
	direct3DDeviceContext->PSSetConstantBuffers(0, ARRAYSIZE(PSConstantBuffers), PSConstantBuffers);

	static ID3D11ShaderResourceView* PSShaderResources[] = { mColorTexture.Get(), mEnvironmentMap.Get() };
	direct3DDeviceContext->PSSetShaderResources(0, ARRAYSIZE(PSShaderResources), PSShaderResources);
	direct3DDeviceContext->PSSetSamplers(0, 1, &mTrilinearSampler);

	direct3DDeviceContext->DrawIndexed(mIndexCount, 0, 0);
}

void EnvironmentMappingRenderer::Update(const DX::StepTimer& gameTime)
{
	float elapsedTime = static_cast<float>(gameTime.GetElapsedSeconds());

	if (mGamePad->IsButtonDown(GamePadButton::LeftShoulder) && mReflectionAmount < 1.0f)
	{
		mReflectionAmount += elapsedTime;
		mReflectionAmount = min(mReflectionAmount, 1.0f);
		mPixelCBufferPerObjectData.ReflectionAmount = mReflectionAmount;
	}
	if (mGamePad->IsButtonDown(GamePadButton::RightShoulder) && mReflectionAmount > 0.0f)
	{
		mReflectionAmount -= elapsedTime;
		mReflectionAmount = max(mReflectionAmount, 0.0f);
		mPixelCBufferPerObjectData.ReflectionAmount = mReflectionAmount;
	}
}

Rendering::EnvironmentMappingRenderer::EnvironmentMappingRenderer(Library::Game& game, Library::Camera& camera):
GameDemo(camera, game), mVertexCBufferPerObject(nullptr), mVertexCBufferPerObjectData(), mVertexCBufferPerFrame(nullptr), mVertexCBufferPerFrameData(),
mPixelCBufferPerObject(nullptr), mPixelCBufferPerObjectData(), mPixelCBufferPerFrame(nullptr), mPixelCBufferPerFrameData(),
 mEnvironmentMap(nullptr), mTrilinearSampler(nullptr), mReflectionAmount(1.0f)
{

}
