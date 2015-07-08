#include "pch.h"
#include "RenderStateHelper.h"
#include "Game.h"

namespace Library
{
    RenderStateHelper::RenderStateHelper(Game& game)
        : mGame(game), mRasterizerState(nullptr), mBlendState(nullptr), mBlendFactor(new FLOAT[4]), mSampleMask(UINT_MAX), mDepthStencilState(nullptr), mStencilRef(UINT_MAX)
    {
    }

    void RenderStateHelper::ResetAll(ID3D11DeviceContext* deviceContext)
    {
        deviceContext->RSSetState(nullptr);
        deviceContext->OMSetBlendState(nullptr, nullptr, UINT_MAX);
        deviceContext->OMSetDepthStencilState(nullptr, UINT_MAX);
    }

	ID3D11RasterizerState* RenderStateHelper::RasterizerState()
	{
		return mRasterizerState.Get();
	}

    ID3D11BlendState* RenderStateHelper::BlendState()
	{
		return mBlendState.Get();
	}

    ID3D11DepthStencilState* RenderStateHelper::DepthStencilState()
	{
		return mDepthStencilState.Get();
	}

    void RenderStateHelper::SaveRasterizerState()
    {
        mRasterizerState = nullptr;
        mGame.GetD3DDeviceContext()->RSGetState(mRasterizerState.GetAddressOf());
    }

    void RenderStateHelper::RestoreRasterizerState() const
    {
		mGame.GetD3DDeviceContext()->RSSetState(mRasterizerState.Get());
    }

    void RenderStateHelper::SaveBlendState()
    {
        mBlendState = nullptr;
		mGame.GetD3DDeviceContext()->OMGetBlendState(mBlendState.GetAddressOf(), mBlendFactor.get(), &mSampleMask);
    }

    void RenderStateHelper::RestoreBlendState() const
    {
		mGame.GetD3DDeviceContext()->OMSetBlendState(mBlendState.Get(), mBlendFactor.get(), mSampleMask);
    }

    void RenderStateHelper::SaveDepthStencilState()
    {
        mDepthStencilState = nullptr;
		mGame.GetD3DDeviceContext()->OMGetDepthStencilState(mDepthStencilState.GetAddressOf(), &mStencilRef);
    }

    void RenderStateHelper::RestoreDepthStencilState() const
    {
		mGame.GetD3DDeviceContext()->OMSetDepthStencilState(mDepthStencilState.Get(), mStencilRef);
    }

    void RenderStateHelper::SaveAll()
    {
        SaveRasterizerState();
        SaveBlendState();
        SaveDepthStencilState();
    }

    void RenderStateHelper::RestoreAll() const
    {
        RestoreRasterizerState();
        RestoreBlendState();
        RestoreDepthStencilState();
    }
}
