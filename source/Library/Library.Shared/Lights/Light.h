#pragma once

#include "pch.h"
#include "GameComponent.h"
#include <DirectXPackedVector.h>

namespace Library
{
	class Light : public GameComponent
	{
		RTTI_DECLARATIONS(Light, GameComponent)

	public:
		Light(Game& game);
		virtual ~Light();

		const DirectX::PackedVector::XMCOLOR& Color() const;
		DirectX::XMVECTOR ColorVector() const;
		void SetColor(FLOAT r, FLOAT g, FLOAT b, FLOAT a);
		void SetColor(const DirectX::XMFLOAT4& color);
		void SetColor(DirectX::PackedVector::XMCOLOR color);
		void SetColor(DirectX::FXMVECTOR color);

	protected:
		DirectX::PackedVector::XMCOLOR mColor;
		GamePadComponent* mGamepad;
	};
}

