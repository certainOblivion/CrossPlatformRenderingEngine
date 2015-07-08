#pragma once

#include "GameComponent.h"

namespace Library
{
    class Camera;

    class DrawableGameComponent : public GameComponent
    {
        RTTI_DECLARATIONS(DrawableGameComponent, GameComponent)

    public:
        DrawableGameComponent();
        DrawableGameComponent(Game& game);
        DrawableGameComponent(Game& game, Camera& camera);
        virtual ~DrawableGameComponent() = default;

		DrawableGameComponent(const DrawableGameComponent& rhs) = delete;
		DrawableGameComponent& operator=(const DrawableGameComponent& rhs) = delete;

        bool Visible() const;
        void SetVisible(bool visible);

        Camera* GetCamera();
        void SetCamera(Camera* camera);

        virtual void Draw() = 0;
		virtual void Update(const DX::StepTimer& gameTime) = 0;

    protected:
        bool mVisible;
        Camera* mCamera;
    };
}