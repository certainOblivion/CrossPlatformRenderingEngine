#include "pch.h"
#include "Game.h"

#include "ColoredTriangleRenderer.h"
#if (WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
#include "Common/DirectXHelper.h"  
#elif (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
#include "Utility.h"
#endif // (WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)

#include <vector>
#include "Cameras/Camera.h"
#include "Game.h"

using namespace std;
using namespace Library;
using namespace DirectX;
using namespace Rendering;

ColoredTriangleRenderer::ColoredTriangleRenderer(Game& game, Camera& camera)
	:GameDemo(camera,game)
{
}


ColoredTriangleRenderer::~ColoredTriangleRenderer()
{
}

void ColoredTriangleRenderer::Initialize(ID3D11Device2* d3dDevice, ID3D11DeviceContext* deviceContext)
{
#if (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
	string compiledVertexShader = Utility::LoadBinaryFile("ColorTriangleVS.cso");
	DX::ThrowIfFailed(d3dDevice->CreateVertexShader(&compiledVertexShader[0], compiledVertexShader.size(), nullptr, mVertexShader.ReleaseAndGetAddressOf()), "ID3D11DEVICE::CreateVertexShader() failed.");

	string compiledPixelShader = Utility::LoadBinaryFile("ColorTrianglePS.cso");
	DX::ThrowIfFailed(d3dDevice->CreatePixelShader(&compiledPixelShader[0], compiledPixelShader.size(), nullptr, mPixelShader.ReleaseAndGetAddressOf()), "ID3D11DEVICE::CreatePixelShader() failed.");

	D3D11_INPUT_ELEMENT_DESC inputElementDescription[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	DX::ThrowIfFailed(d3dDevice->CreateInputLayout(inputElementDescription, ARRAYSIZE(inputElementDescription), &compiledVertexShader[0],
		compiledVertexShader.size(), mInputLayout.ReleaseAndGetAddressOf()), "ID3D11DEVICE::CreateInputLayout() failed.");

	#endif // (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)

#if (WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP)
	// Load shaders asynchronously.
	auto loadVSTask = DX::ReadDataAsync(L"ColorTriangleVS.cso");
	auto loadPSTask = DX::ReadDataAsync(L"ColorTrianglePS.cso");

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createVSTask = loadVSTask.then([this, d3dDevice](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			d3dDevice->CreateVertexShader(
			&fileData[0],
			fileData.size(),
			nullptr,
			&mVertexShader
			)
			);

		static const D3D11_INPUT_ELEMENT_DESC inputElementDescription[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		DX::ThrowIfFailed(
			d3dDevice->CreateInputLayout(
			inputElementDescription,
			ARRAYSIZE(inputElementDescription),
			&fileData[0],
			fileData.size(),
			&mInputLayout
			)
			);
	});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createPSTask = loadPSTask.then([this, d3dDevice](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			d3dDevice->CreatePixelShader(
			&fileData[0],
			fileData.size(),
			nullptr,
			&mPixelShader
			)
			);
	});

#endif // (WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP)
	VertexPositionColor vertices[] =
	{
		VertexPositionColor(XMFLOAT4(-0.5f, -0.5f, 0.0f, 1.0f), XMFLOAT4(1, 0, 0, 1)),
		VertexPositionColor(XMFLOAT4(0.0f, 0.5f, 0.0f, 1.0f), XMFLOAT4(0, 1, 0, 1)),
		VertexPositionColor(XMFLOAT4(0.5f, -0.5f, 0.0f, 1.0f), XMFLOAT4(0, 0, 1, 1))
	};

	D3D11_BUFFER_DESC vertexBufferDesc = { 0 };
	vertexBufferDesc.ByteWidth = sizeof(VertexPositionColor) * ARRAYSIZE(vertices);
	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertexSubResourceData = { 0 };
	vertexSubResourceData.pSysMem = vertices;
	DX::ThrowIfFailed(d3dDevice->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, mVertexBuffer.ReleaseAndGetAddressOf()), "ID3D11DEVICE::CreateBuffer() failed.");

}

void ColoredTriangleRenderer::Draw(ID3D11DeviceContext* m_d3dContext)
{
	m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_d3dContext->IASetInputLayout(mInputLayout.Get());

	UINT stride = sizeof(VertexPositionColor);
	UINT offset = 0;

	m_d3dContext->IASetVertexBuffers(0, 1, mVertexBuffer.GetAddressOf(), &stride, &offset);

	m_d3dContext->VSSetShader(mVertexShader.Get(), nullptr, 0);
	m_d3dContext->PSSetShader(mPixelShader.Get(), nullptr, 0);

	m_d3dContext->Draw(3, 0);
}

void Rendering::ColoredTriangleRenderer::Update(const DX::StepTimer& gameTime)
{
}
