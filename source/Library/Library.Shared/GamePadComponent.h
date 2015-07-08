#pragma once

#include "GameComponent.h"
#include <GamePad.h>
#include <memory>

namespace Library
{
	enum class GamePadButton
	{
		A,
		B,
		X,
		Y,
		LeftStick,
		RightStick,
		LeftShoulder,
		RightShoulder,
		Back,
		Start,
		DPadUp,
		DPadDown,
		DPadLeft,
		DPadRight
	};

	class GamePadComponent : public GameComponent
	{
		RTTI_DECLARATIONS(GamePadComponent, GameComponent)

	public:
		static DirectX::GamePad* GamePad();
		
		GamePadComponent(Game& game, int player =0);

		int Player() const;
		const DirectX::GamePad::State& CurrentState() const;
		const DirectX::GamePad::State& LastState() const;

		virtual void Initialize() override;
		virtual void Update(const DX::StepTimer& gameTime) override;

		bool IsButtonUp(GamePadButton button) const;
		bool IsButtonDown(GamePadButton button) const;
		bool WasButtonUp(GamePadButton button) const;
		bool WasButtonDown(GamePadButton button) const;
		bool WasButtonPressedThisFrame(GamePadButton button) const;
		bool WasButtonReleasedThisFrame(GamePadButton button) const;
		bool IsButtonHeldDown(GamePadButton button) const;

	private:
		GamePadComponent(const GamePadComponent& rhs) = delete;
		static std::unique_ptr<DirectX::GamePad> sGamePad;

		bool GetButtonState(DirectX::GamePad::State state, GamePadButton button) const;

		int mPlayer;
		DirectX::GamePad::State mCurrentState;
		DirectX::GamePad::State mLastState;		
	};
}