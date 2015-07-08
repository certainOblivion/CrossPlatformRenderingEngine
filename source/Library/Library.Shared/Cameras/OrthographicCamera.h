#pragma once

#include "Camera.h"

namespace Library
{
    class OrthographicCamera : public Camera
    {
		RTTI_DECLARATIONS(OrthographicCamera, Camera)

    public:
        OrthographicCamera(Game& game);
		OrthographicCamera(Game& game, float viewWidth, float viewHeight, float nearPlaneDistance, float farPlaneDistance);
        virtual ~OrthographicCamera();

		OrthographicCamera(const OrthographicCamera& rhs) = delete;
		OrthographicCamera& operator=(const OrthographicCamera& rhs) = delete;

		float ViewWidth() const;
		void SetViewWidth(float viewWidth);

		float ViewHeight() const;
		void SetViewHeight(float viewHeight);

        virtual void UpdateProjectionMatrix() override;

		static const float DefaultViewWidth;
		static const float DefaultViewHeight;

    protected:
		float mViewWidth;
		float mViewHeight;
    };
}
