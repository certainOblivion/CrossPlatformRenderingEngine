#pragma once

#include "pch.h"

namespace Library
{
    class Game;

    class GameComponent : public RTTI
    {
        RTTI_DECLARATIONS(GameComponent, RTTI)

    public:
        GameComponent();
        GameComponent(Game& game);
        virtual ~GameComponent() = default;

		GameComponent(const GameComponent& rhs) = delete;
		GameComponent& operator=(const GameComponent& rhs) = delete;

        Game* GetGame();
        void SetGame(Game& game);
        bool Enabled() const;
        void SetEnabled(bool enabled);

        virtual void Initialize() = 0;
		virtual void Update(const DX::StepTimer& gameTime) = 0;

    protected:
        Game* mGame;
        bool mEnabled;
    };
}
