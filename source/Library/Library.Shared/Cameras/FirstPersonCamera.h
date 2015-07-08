#pragma once

#include "PerspectiveCamera.h"
#include <memory>
#include "StepTimer.h"

namespace Library
{
    class FirstPersonCamera : public PerspectiveCamera
    {
		RTTI_DECLARATIONS(FirstPersonCamera, PerspectiveCamera)
    public:
        FirstPersonCamera(Game& game);
        FirstPersonCamera(Game& game, float fieldOfView, float aspectRatio, float nearPlaneDistance, float farPlaneDistance);
        virtual ~FirstPersonCamera() = default;

		FirstPersonCamera(const FirstPersonCamera& rhs) = delete;
		FirstPersonCamera& operator=(const FirstPersonCamera& rhs) = delete;

        float& RotationRate();
        float& MovementRate();		
        
		virtual void Initialize() override;
        virtual void Update(const DX::StepTimer& gameTime) override;

        static const float DefaultRotationRate;
        static const float DefaultMovementRate;        

    protected:
		float mRotationRate;
        float mMovementRate;		
    };
}

