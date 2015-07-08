#include "pch.h"
#include "GamePadComponent.h"

using namespace DirectX;

namespace Library
{
	RTTI_DEFINITIONS(GamePadComponent)

	std::unique_ptr<DirectX::GamePad> GamePadComponent::sGamePad(new DirectX::GamePad);

	DirectX::GamePad* GamePadComponent::GamePad()
	{
		return sGamePad.get();
	}

	GamePadComponent::GamePadComponent(Game& game, int player) :
		GameComponent(game), mPlayer(player)
	{
	}

	int GamePadComponent::Player() const
	{
		return mPlayer;
	}

	const GamePad::State& GamePadComponent::CurrentState() const
	{
		return mCurrentState;
	}

	const GamePad::State& GamePadComponent::LastState() const
	{
		return mLastState;
	}

	void GamePadComponent::Initialize()
	{
		mCurrentState = sGamePad->GetState(mPlayer);
		mLastState = mCurrentState;
	}

	void GamePadComponent::Update(const DX::StepTimer& gameTime)
	{
		mLastState = mCurrentState;
		mCurrentState = sGamePad->GetState(mPlayer);
	}

	bool GamePadComponent::IsButtonUp(GamePadButton button) const
	{
		return GetButtonState(mCurrentState, button) == false;
	}

	bool GamePadComponent::IsButtonDown(GamePadButton button) const
	{
		return GetButtonState(mCurrentState, button);
	}

	bool GamePadComponent::WasButtonUp(GamePadButton button) const
	{
		return GetButtonState(mLastState, button) == false;
	}

	bool GamePadComponent::WasButtonDown(GamePadButton button) const
	{
		return GetButtonState(mLastState, button);
	}

	bool GamePadComponent::WasButtonPressedThisFrame(GamePadButton button) const
	{
		return (IsButtonDown(button) && WasButtonUp(button));
	}

	bool GamePadComponent::WasButtonReleasedThisFrame(GamePadButton button) const
	{
		return (IsButtonUp(button) && WasButtonDown(button));
	}

	bool GamePadComponent::IsButtonHeldDown(GamePadButton button) const
	{
		return (IsButtonDown(button) && WasButtonDown(button));
	}

	bool GamePadComponent::GetButtonState(DirectX::GamePad::State state, GamePadButton button) const
	{
		switch (button)
		{
			case GamePadButton::A:
				return state.buttons.a;

			case GamePadButton::B:
				return state.buttons.b;

			case GamePadButton::X:
				return state.buttons.x;

			case GamePadButton::Y:
				return state.buttons.y;

			case GamePadButton::LeftStick:
				return state.buttons.leftStick;

			case GamePadButton::RightStick:
				return state.buttons.rightStick;

			case GamePadButton::LeftShoulder:
				return state.buttons.leftShoulder;

			case GamePadButton::RightShoulder:
				return state.buttons.rightShoulder;

			case GamePadButton::Back:
				return state.buttons.back;

			case GamePadButton::Start:
				return state.buttons.start;

			case GamePadButton::DPadUp:
				return state.dpad.up;

			case GamePadButton::DPadDown:
				return state.dpad.down;

			case GamePadButton::DPadLeft:
				return state.dpad.left;

			case GamePadButton::DPadRight:
				return state.dpad.right;

			default:
				throw std::exception("Invalid GamePadButton.");
		}
	}
}