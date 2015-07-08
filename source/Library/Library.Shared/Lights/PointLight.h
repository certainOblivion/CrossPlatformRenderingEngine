#pragma once

#include "pch.h"
#include "Light.h"

namespace Library
{
	class PointLight : public Light
	{
		RTTI_DECLARATIONS(PointLight, Light)

	public:
		PointLight(Game& game);
		virtual ~PointLight();

		DirectX::XMFLOAT3& Position();
		DirectX::XMVECTOR PositionVector() const;
		FLOAT Radius() const;

		virtual void SetPosition(FLOAT x, FLOAT y, FLOAT z);
		virtual void SetPosition(DirectX::FXMVECTOR position);
		virtual void SetPosition(const DirectX::XMFLOAT3& position);
		virtual void SetRadius(float value);

		static const float DefaultRadius;

		virtual void Initialize();
		virtual void Update(const DX::StepTimer& gameTime);

	protected:
		DirectX::XMFLOAT3 mPosition;
		float mRadius;
	};
}

