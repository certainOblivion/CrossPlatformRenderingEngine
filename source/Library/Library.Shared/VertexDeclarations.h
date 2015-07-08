#pragma once

#include "pch.h"

namespace Library
{
	struct VertexPosition
    {
        DirectX::XMFLOAT4 Position;

        VertexPosition() { }

		VertexPosition(const DirectX::XMFLOAT4& position)
            : Position(position) { }
	};

	struct VertexPositionSize
	{
		DirectX::XMFLOAT4 Position;
		DirectX::XMFLOAT2 Size;

		VertexPositionSize() { }

		VertexPositionSize(const DirectX::XMFLOAT4& position, const DirectX::XMFLOAT2& size)
			: Position(position), Size(size) { }
	};

	struct VertexPositionNormal
    {
		DirectX::XMFLOAT4 Position;
		DirectX::XMFLOAT3 Normal;

        VertexPositionNormal() { }

		VertexPositionNormal(const DirectX::XMFLOAT4& position, const DirectX::XMFLOAT3& normal)
            : Position(position), Normal(normal) { }
    };

	struct VertexPositionTextureNormal
	{
		DirectX::XMFLOAT4 Position;
		DirectX::XMFLOAT2 TextureCoordinates;
		DirectX::XMFLOAT3 Normal;

		VertexPositionTextureNormal() { }

		VertexPositionTextureNormal(const DirectX::XMFLOAT4& position, const DirectX::XMFLOAT2& textureCoordinates, const DirectX::XMFLOAT3& normal)
			: Position(position), TextureCoordinates(textureCoordinates), Normal(normal) { }
	};

	struct VertexPositionTextureNormalTangent
	{
		DirectX::XMFLOAT4 Position;
		DirectX::XMFLOAT2 TextureCoordinates;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT3 Tangent;

		VertexPositionTextureNormalTangent() { }

		VertexPositionTextureNormalTangent(const DirectX::XMFLOAT4& position, const DirectX::XMFLOAT2& textureCoordinates, const DirectX::XMFLOAT3& normal, const DirectX::XMFLOAT3& tangent)
			: Position(position), TextureCoordinates(textureCoordinates), Normal(normal), Tangent(tangent) { }
	};
}
