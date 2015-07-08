#include "pch.h"
#include "GameDemo.h"
#include <DirectXMath.h>
#include "Game.h"

using namespace Library;
using namespace DirectX;

GameDemo::GameDemo(Camera& camera, Game& game):
mCamera(camera), mWorldMatrix(MatrixHelper::Identity), mIndexCount(0), mGame(game)
{

}

void Library::GameDemo::Initialize(ID3D11Device2* device, ID3D11DeviceContext* deviceContext)
{
	mGamePad = (GamePadComponent*)mGame.Services().GetService(GamePadComponent::TypeIdClass());
}
