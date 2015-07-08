#pragma once

namespace Library
{
    class Game;

    class RenderStateHelper
    {
    public:
        RenderStateHelper(Game& game);
        ~RenderStateHelper() = default;

		RenderStateHelper(const RenderStateHelper& rhs) = delete;
		RenderStateHelper& operator=(const RenderStateHelper& rhs) = delete;

        static void ResetAll(ID3D11DeviceContext* deviceContext);

        ID3D11RasterizerState* RasterizerState();
        ID3D11BlendState* BlendState();
        ID3D11DepthStencilState* DepthStencilState();

        void SaveRasterizerState();
        void RestoreRasterizerState() const;

        void SaveBlendState();
        void RestoreBlendState() const;

        void SaveDepthStencilState();
        void RestoreDepthStencilState() const;

        void SaveAll();
        void RestoreAll() const;

    private:
        Game& mGame;

        Microsoft::WRL::ComPtr<ID3D11RasterizerState> mRasterizerState;
		Microsoft::WRL::ComPtr<ID3D11BlendState> mBlendState;
        std::unique_ptr<FLOAT> mBlendFactor;
        UINT mSampleMask;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> mDepthStencilState;
        UINT mStencilRef;
    };
}
