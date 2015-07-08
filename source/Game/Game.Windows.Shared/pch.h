#pragma once

#include <wrl.h>
#include <wrl/client.h>
#include <d3d11_2.h>
#include <d2d1_2.h>
#include <d2d1effects_1.h>
#include <dwrite_2.h>
#include <wincodec.h>
#include <DirectXColors.h>
#include <DirectXMath.h>
#include <memory>
#include <agile.h>
#include <concrt.h>
#include <collection.h>
#include <ppltasks.h>
#include "Common/DirectXHelper.h"
#include "Game.h"
#include <cstdint>

inline Concurrency::task<std::vector<byte>> ReadDataAsync(const std::wstring& filename)
{
	using namespace Windows::Storage;
	using namespace Concurrency;

	auto folder = Windows::ApplicationModel::Package::Current->InstalledLocation;

	return create_task(folder->GetFileAsync(Platform::StringReference(filename.c_str()))).then([](StorageFile^ file)
	{
		return FileIO::ReadBufferAsync(file);
	}).then([](Streams::IBuffer^ fileBuffer) -> std::vector < byte >
	{
		std::vector<byte> returnBuffer;
		returnBuffer.resize(fileBuffer->Length);
		Streams::DataReader::FromBuffer(fileBuffer)->ReadBytes(Platform::ArrayReference<byte>(returnBuffer.data(), fileBuffer->Length));
		return returnBuffer;
	});
}

// Converts a length in device-independent pixels (DIPs) to a length in physical pixels.
inline float ConvertDipsToPixels(float dips, float dpi)
{
	static const float dipsPerInch = 96.0f;
	return floorf(dips * dpi / dipsPerInch + 0.5f); // Round to nearest integer.
}

#if defined(_DEBUG)
// Check for SDK Layer support.
inline bool SdkLayersAvailable()
{
	HRESULT hr = D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_NULL,       // There is no need to create a real hardware device.
		0,
		D3D11_CREATE_DEVICE_DEBUG,  // Check for the SDK layers.
		nullptr,                    // Any feature level will do.
		0,
		D3D11_SDK_VERSION,          // Always set this to D3D11_SDK_VERSION for Windows Store apps.
		nullptr,                    // No need to keep the D3D device reference.
		nullptr,                    // No need to know the feature level.
		nullptr                     // No need to keep the D3D device context reference.
		);

	return SUCCEEDED(hr);
}
#endif

