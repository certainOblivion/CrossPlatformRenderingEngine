#include "pch.h"
#include "TexturedSphereRenderer.h"
#include <DDSTextureLoader.h>
#include "Utility.h"  


using namespace Library;
using namespace DirectX;
using namespace Rendering;

TexturedSphereRenderer::TexturedSphereRenderer(Library::Game& game, Camera& camera)
	:GameDemo(camera, game), mCBufferPerObject()
{
}

void TexturedSphereRenderer::Initialize(ID3D11Device2* device, ID3D11DeviceContext* deviceContext)
{
	GameDemo::Initialize(device,deviceContext);

	// Load a compiled vertex shader
	std::vector<char> compiledVertexShader;
	Utility::LoadBinaryFile(L"Content\\Shaders\\TexturedModelVS.cso", compiledVertexShader);
	DX::ThrowIfFailed(device->CreateVertexShader(&compiledVertexShader[0], compiledVertexShader.size(), nullptr, mVertexShader.GetAddressOf()), "ID3D11Device::CreatedVertexShader() failed.");

	// Load a compiled pixel shader
	std::vector<char> compiledPixelShader;
	Utility::LoadBinaryFile(L"Content\\Shaders\\TexturedModelPS.cso", compiledPixelShader);
	DX::ThrowIfFailed(device->CreatePixelShader(&compiledPixelShader[0], compiledPixelShader.size(), nullptr, mPixelShader.GetAddressOf()), "ID3D11Device::CreatedPixelShader() failed.");

	// Create an input layout
	D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	DX::ThrowIfFailed(device->CreateInputLayout(inputElementDescriptions, ARRAYSIZE(inputElementDescriptions), &compiledVertexShader[0], compiledVertexShader.size(), mInputLayout.GetAddressOf()), "ID3D11Device::CreateInputLayout() failed.");

	// Load the model
	std::unique_ptr<Model> model = std::make_unique<Model>("Content\\Models\\texturedSphere.bin", true);

	// Create vertex and index buffers for the model
	Mesh* mesh = model->Meshes().at(0);
	CreateVertexBuffer(device, *mesh, mVertexBuffer.GetAddressOf());
	mesh->CreateIndexBuffer(mIndexBuffer.GetAddressOf(), device);
	mIndexCount = (UINT)(mesh->Indices().size());

	// Create a constant buffer
	D3D11_BUFFER_DESC constantBufferDesc;
	ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));
	constantBufferDesc.ByteWidth = sizeof(CBufferPerObject);
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	DX::ThrowIfFailed(device->CreateBuffer(&constantBufferDesc, nullptr, mConstantBuffer.GetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

	// Load a texture
	std::wstring textureName = L"Content\\Textures\\EarthComposite.dds";
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, deviceContext, textureName.c_str(), nullptr, mColorTexture.GetAddressOf()), "CreateWICTextureFromFile() failed.");

	// Create a texture sampler
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	DX::ThrowIfFailed(device->CreateSamplerState(&samplerDesc, mColorSampler.GetAddressOf()), "ID3D11Device::CreateSamplerState() failed.");
}

void TexturedSphereRenderer::Draw(ID3D11DeviceContext* direct3DDeviceContext)
{
	direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	direct3DDeviceContext->IASetInputLayout(mInputLayout.Get());

	UINT stride = sizeof(VertexPositionTexture);
	UINT offset = 0;
	direct3DDeviceContext->IASetVertexBuffers(0, 1, mVertexBuffer.GetAddressOf(), &stride, &offset);
	direct3DDeviceContext->IASetIndexBuffer(mIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	direct3DDeviceContext->VSSetShader(mVertexShader.Get(), nullptr, 0);
	direct3DDeviceContext->PSSetShader(mPixelShader.Get(), nullptr, 0);
	
	XMMATRIX worldMatrix = XMLoadFloat4x4(&mWorldMatrix);
	XMMATRIX wvp = worldMatrix * mCamera.ViewProjectionMatrix();
	wvp = XMMatrixTranspose(wvp);
	XMStoreFloat4x4(&mCBufferPerObject.WorldViewProjection, wvp);

	direct3DDeviceContext->UpdateSubresource(mConstantBuffer.Get(), 0, nullptr, &mCBufferPerObject, 0, 0);
	direct3DDeviceContext->VSSetConstantBuffers(0, 1, mConstantBuffer.GetAddressOf());

	direct3DDeviceContext->PSSetShaderResources(0, 1, mColorTexture.GetAddressOf());
	direct3DDeviceContext->PSSetSamplers(0, 1, mColorSampler.GetAddressOf());

	direct3DDeviceContext->DrawIndexed(mIndexCount, 0, 0);
}

void TexturedSphereRenderer::Update(const DX::StepTimer& gameTime)
{

}

void TexturedSphereRenderer::CreateVertexBuffer(ID3D11Device* device, const Mesh& mesh, ID3D11Buffer** vertexBuffer) const
{
	const std::vector<XMFLOAT3>& sourceVertices = mesh.Vertices();

	std::vector<VertexPositionTexture> vertices;
	vertices.reserve(sourceVertices.size());

	std::vector<XMFLOAT3>* textureCoordinates = mesh.TextureCoordinates().at(0);
	assert(textureCoordinates->size() == sourceVertices.size());

	for (UINT i = 0; i < sourceVertices.size(); i++)
	{
		XMFLOAT3 position = sourceVertices.at(i);
		XMFLOAT3 uv = textureCoordinates->at(i);
		vertices.push_back(VertexPositionTexture(XMFLOAT4(position.x, position.y, position.z, 1.0f), XMFLOAT2(uv.x, uv.y)));
	}

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
	vertexBufferDesc.ByteWidth = (UINT)(sizeof(VertexPositionTexture) * vertices.size());
	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertexSubResourceData;
	ZeroMemory(&vertexSubResourceData, sizeof(vertexSubResourceData));
	vertexSubResourceData.pSysMem = &vertices[0];
	if (FAILED(device->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, vertexBuffer)))
	{
		throw std::exception("ID3D11Device::CreateBuffer() failed.");
	}
}

