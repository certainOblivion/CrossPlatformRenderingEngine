#pragma once

#include "pch.h"
#include "Light.h"

namespace Library
{
	class DirectionalLight : public Light
	{
		RTTI_DECLARATIONS(DirectionalLight, Light)

	public:
		DirectionalLight(Game& game);
		virtual ~DirectionalLight();

		const DirectX::XMFLOAT3& Direction() const;
		const DirectX::XMFLOAT3& Up() const;
		const DirectX::XMFLOAT3& Right() const;

		DirectX::XMVECTOR DirectionVector() const;
		DirectX::XMVECTOR UpVector() const;
		DirectX::XMVECTOR RightVector() const;

		void ApplyRotation(DirectX::CXMMATRIX transform);
        void ApplyRotation(const DirectX::XMFLOAT4X4& transform);

		virtual void Initialize();
		virtual void Update(const DX::StepTimer& gameTime);

	protected:
		DirectX::XMFLOAT3 mDirection;
		DirectX::XMFLOAT3 mUp;
		DirectX::XMFLOAT3 mRight;
	};
}

