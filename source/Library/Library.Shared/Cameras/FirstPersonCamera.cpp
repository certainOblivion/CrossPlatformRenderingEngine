#include "pch.h"
#include "FirstPersonCamera.h"

using namespace DirectX;
using namespace DX;

namespace Library
{
	RTTI_DEFINITIONS(FirstPersonCamera)

    const float FirstPersonCamera::DefaultRotationRate = XMConvertToRadians(100.0f);
    const float FirstPersonCamera::DefaultMovementRate = 100.0f;

    FirstPersonCamera::FirstPersonCamera(Game& game) :
		PerspectiveCamera(game),
		mRotationRate(DefaultRotationRate), mMovementRate(DefaultMovementRate)
    {
    }

    FirstPersonCamera::FirstPersonCamera(Game& game, float fieldOfView, float aspectRatio, float nearPlaneDistance, float farPlaneDistance) :
		PerspectiveCamera(game, fieldOfView, aspectRatio, nearPlaneDistance, farPlaneDistance),
		mRotationRate(DefaultRotationRate), mMovementRate(DefaultMovementRate)
    {
    }

    float& FirstPersonCamera::RotationRate()
    {
        return mRotationRate;
    }

    float& FirstPersonCamera::MovementRate()
    {
        return mMovementRate;
    }

	void FirstPersonCamera::Initialize()
	{
		Camera::Initialize();
	}

    void FirstPersonCamera::Update(const StepTimer& gameTime)
    {
		if (mGamepad!=nullptr)
		{
			auto gamePadState = mGamepad->CurrentState();
			if (gamePadState.IsConnected())
			{
				XMFLOAT2 movementAmount;
				XMFLOAT2 rotationAmount;

				movementAmount.x = gamePadState.thumbSticks.leftX;
				movementAmount.y = gamePadState.thumbSticks.leftY;

				rotationAmount.x = -gamePadState.thumbSticks.rightX;
				rotationAmount.y = gamePadState.thumbSticks.rightY;

				float elapsedTime = (float)gameTime.GetElapsedSeconds();
				XMVECTOR rotationVector = XMLoadFloat2(&rotationAmount) * mRotationRate * elapsedTime;
				XMVECTOR right = XMLoadFloat3(&mRight);

				XMMATRIX pitchMatrix = XMMatrixRotationAxis(right, XMVectorGetY(rotationVector));
				XMMATRIX yawMatrix = XMMatrixRotationY(XMVectorGetX(rotationVector));

				ApplyRotation(XMMatrixMultiply(pitchMatrix, yawMatrix));

				XMVECTOR position = XMLoadFloat3(&mPosition);
				XMVECTOR movement = XMLoadFloat2(&movementAmount) * mMovementRate * elapsedTime;

				XMVECTOR strafe = right * XMVectorGetX(movement);
				position += strafe;

				XMVECTOR forward = XMLoadFloat3(&mDirection) * XMVectorGetY(movement);
				position += forward;

				XMStoreFloat3(&mPosition, position);
			}
		}
        Camera::Update(gameTime);
    }
}
