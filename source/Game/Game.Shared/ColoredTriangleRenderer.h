#pragma once
namespace Library
{
	class Camera;
	class Game;
}
namespace Rendering
{
	class ColoredTriangleRenderer : public Library::GameDemo
	{
	public:
		ColoredTriangleRenderer(Library::Game& game, Library::Camera& camera);
		virtual ~ColoredTriangleRenderer();

		virtual void Initialize(ID3D11Device2* device, ID3D11DeviceContext* deviceContext) override;
		virtual void Draw(ID3D11DeviceContext* direct3DDeviceContext) override;
		virtual void Update(const DX::StepTimer& gameTime) override;

		ColoredTriangleRenderer& operator=(const ColoredTriangleRenderer& rhs) = delete;
		ColoredTriangleRenderer(const ColoredTriangleRenderer& rhs) = delete;

	private:
		bool mIsLoadingComplete = false;
	};
}

