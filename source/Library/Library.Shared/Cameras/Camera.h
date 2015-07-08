#pragma once
#include "Game.h"
#include "StepTimer.h"

namespace Library
{
    class Camera : public GameComponent
    {
		RTTI_DECLARATIONS(Camera, GameComponent)
    public:
        Camera(Game& game);
		Camera(Game& game, float nearPlaneDistance, float farPlaneDistance);
        virtual ~Camera();

		Camera(const Camera& rhs) = delete;
		Camera& operator=(const Camera& rhs) = delete;

        const DirectX::XMFLOAT3& Position() const;
        const DirectX::XMFLOAT3& Direction() const;
        const DirectX::XMFLOAT3& Up() const;
        const DirectX::XMFLOAT3& Right() const;
        
        DirectX::XMVECTOR PositionVector() const;
        DirectX::XMVECTOR DirectionVector() const;
        DirectX::XMVECTOR UpVector() const;
        DirectX::XMVECTOR RightVector() const;
        
        float NearPlaneDistance() const;
        float FarPlaneDistance() const;

        DirectX::XMMATRIX ViewMatrix() const;
        DirectX::XMMATRIX ProjectionMatrix() const;
        DirectX::XMMATRIX ViewProjectionMatrix() const;

        virtual void SetPosition(FLOAT x, FLOAT y, FLOAT z);
        virtual void SetPosition(DirectX::FXMVECTOR position);
        virtual void SetPosition(const DirectX::XMFLOAT3& position);

        virtual void Reset();
        virtual void Initialize();
		virtual void Update(const DX::StepTimer& gameTime);
        virtual void UpdateViewMatrix();
        virtual void UpdateProjectionMatrix() = 0;
        virtual void ApplyRotation(DirectX::CXMMATRIX transform);
        virtual void ApplyRotation(const DirectX::XMFLOAT4X4& transform);

        static const float DefaultNearPlaneDistance;
        static const float DefaultFarPlaneDistance;

    protected:
        float mNearPlaneDistance;
        float mFarPlaneDistance;

        DirectX::XMFLOAT3 mPosition;
        DirectX::XMFLOAT3 mDirection;
        DirectX::XMFLOAT3 mUp;
        DirectX::XMFLOAT3 mRight;

        DirectX::XMFLOAT4X4 mViewMatrix;
        DirectX::XMFLOAT4X4 mProjectionMatrix;     
		Game& mGame;
		GamePadComponent* mGamepad;
    };
}

