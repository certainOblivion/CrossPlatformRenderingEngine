#include "pch.h"
#include "Light.h"
#include "ColorHelper.h"
#include "Game.h"

using namespace DirectX;
using namespace DirectX::PackedVector;

namespace Library
{
	RTTI_DEFINITIONS(Light)

	Light::Light(Game& game)
		: GameComponent(game), mColor(reinterpret_cast<const float*>(&ColorHelper::White))
	{
		mGamepad = (GamePadComponent*)game.Services().GetService(GamePadComponent::TypeIdClass());
	}

	Light::~Light()
	{
	}

	const XMCOLOR& Light::Color() const
	{
		return mColor;
	}

	XMVECTOR Light::ColorVector() const
	{
		return XMLoadColor(&mColor);
	}

	void Light::SetColor(FLOAT r, FLOAT g, FLOAT b, FLOAT a)
	{
		SetColor(XMCOLOR(r, g, b, a));
	}

	void Light::SetColor(const XMFLOAT4& color)
	{
		SetColor(XMCOLOR(color.x, color.y, color.z, color.w));
	}

	void Light::SetColor(XMCOLOR color)
	{
		mColor = color;
	}

	void Light::SetColor(FXMVECTOR color)
	{
		XMStoreColor(&mColor, color);
	}
}