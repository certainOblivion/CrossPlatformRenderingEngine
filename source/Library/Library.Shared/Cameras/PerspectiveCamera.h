#pragma once

#include "Camera.h"
#include "StepTimer.h"

namespace Library
{
    class PerspectiveCamera : public Camera
    {
		RTTI_DECLARATIONS(PerspectiveCamera, Camera)

    public:
		PerspectiveCamera(Game& game);
		PerspectiveCamera(Game& game, float fieldOfView, float aspectRatio, float nearPlaneDistance, float farPlaneDistance);
        virtual ~PerspectiveCamera();

		PerspectiveCamera(const PerspectiveCamera& rhs) = delete;
		PerspectiveCamera& operator=(const PerspectiveCamera& rhs) = delete;
       
        float AspectRatio() const;
        float FieldOfView() const;

		virtual void UpdateProjectionMatrix() override;
        
        static const float DefaultFieldOfView;

    protected:
        float mFieldOfView;
        float mAspectRatio;
    };
}

