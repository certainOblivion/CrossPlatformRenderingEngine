#include "pch.h"
#include "GameComponent.h"

namespace Library
{
    RTTI_DEFINITIONS(GameComponent)

    GameComponent::GameComponent()
        : mGame(nullptr), mEnabled(true)
    {
    }

    GameComponent::GameComponent(Game& game)
        : mGame(&game), mEnabled(true)
    {
    }

    Game* GameComponent::GetGame()
    {
        return mGame;
    }

    void GameComponent::SetGame(Game& game)
    {
        mGame = &game;
    }

    bool GameComponent::Enabled() const
    {
        return mEnabled;
    }

    void GameComponent::SetEnabled(bool enabled)
    {
        mEnabled = enabled;
    }

}
