#include "pch.h"
#include "PerspectiveCamera.h"

using namespace DirectX;

namespace Library
{
	RTTI_DEFINITIONS(PerspectiveCamera)

    const float PerspectiveCamera::DefaultFieldOfView = XM_PIDIV4;

    PerspectiveCamera::PerspectiveCamera(Game& game) :
		Camera(game),
        mFieldOfView(DefaultFieldOfView), mAspectRatio(game.AspectRatio())
    {
    }

    PerspectiveCamera::PerspectiveCamera(Game& game, float fieldOfView, float aspectRatio, float nearPlaneDistance, float farPlaneDistance) :
		Camera(game, nearPlaneDistance, farPlaneDistance),
        mFieldOfView(fieldOfView), mAspectRatio(aspectRatio)
    {
    }

    PerspectiveCamera::~PerspectiveCamera()
    {
    }    

    float PerspectiveCamera::AspectRatio() const
    {
        return mAspectRatio;
    }

    float PerspectiveCamera::FieldOfView() const
    {
        return mFieldOfView;
    }

    void PerspectiveCamera::UpdateProjectionMatrix()
    {
		mAspectRatio = mGame.AspectRatio();
        XMMATRIX projectionMatrix = XMMatrixPerspectiveFovRH(mFieldOfView, mAspectRatio, mNearPlaneDistance, mFarPlaneDistance);
        XMStoreFloat4x4(&mProjectionMatrix, projectionMatrix);
    }
}
