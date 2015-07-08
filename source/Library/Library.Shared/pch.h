#pragma once

#include "targetver.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <d3d11_2.h>
#include <algorithm>
#include <exception>
#include <memory>
#include <functional>
#include <d2d1_2.h>
#include <fstream>
#include "StepTimer.h"
#include "Model.h"
#include "Mesh.h"
#include "ModelMaterial.h"
#include "VectorHelper.h"
#include "MatrixHelper.h"
#include <cstdint>
#include "RTTI.h"
#include "GameComponent.h"
#include "GamePadComponent.h"
#include "GameDemo.h"
#include "ServiceContainer.h"
#include "Skybox.h"
#include "VertexDeclarations.h"
#include "BlendStates.h"
#include "RenderStateHelper.h"


namespace DX
{
	inline void ThrowIfFailed(HRESULT hr, std::string msg = std::string(""))
	{
		if (FAILED(hr))
		{
			// Set a breakpoint on this line to catch DirectX API errors
			throw std::exception(msg.c_str());
		}
	}
}

#define DeleteObject(object) if((object) != nullptr) { delete object; object = nullptr; }
#define DeleteObjects(objects) if((objects) != nullptr) { delete[] objects; objects = nullptr; }
#define ReleaseObject(object) if((object) != nullptr) { object->Release(); object = nullptr; }


#if !defined(WINAPI_FAMILY)|| (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
#include <WinSDKVer.h>
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif // _WIN32_WINNT
#include <SDKDDKVer.h>
#define NOMINMAX
#endif

#if (WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
#include <wrl.h>
#include <d2d1effects_1.h>
#include <dwrite_2.h>
#include <wincodec.h>
#include <concrt.h>
#endif
