#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "IGraphicsResource.h"
#include "Core/Win32/ComfyWindows.h"
#include <d3d11.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

namespace Comfy::Render::D3D11
{
	class SwapChainRenderTarget;

	class Direct3D
	{
	public:
		bool Initialize(HWND window);
		void Dispose();

	public:
		void ResizeWindowRenderTarget(ivec2 newSize);

		void SetViewport(ivec2 size);
		void SetViewport(ivec2 position, ivec2 size);

		void SetScissorRect(ivec4 rectangle);

		// NOTE: To avoid stale refereces and invalid memory accesses during rendering
		void EnsureDeviceObjectLifetimeUntilRendering(ID3D11DeviceChild* object);
		void EndOfFrameClearStaleDeviceObjects();

	public:
		// NOTE: Raw pointers to optionally skip releasing them

		// NOTE: Manages object creation
		ID3D11Device* Device = nullptr;

		// NOTE: Manages render commands / set state
		ID3D11DeviceContext* Context = nullptr;

		IDXGISwapChain* SwapChain = nullptr;
		std::unique_ptr<SwapChainRenderTarget> WindowRenderTarget = nullptr;

	private:
		bool InternalCreateDeviceAndSwapchain(HWND window);
		bool InternalSetUpDebugInterface();

	private:
		std::vector<ID3D11DeviceChild*> objectsToBeReleased;

	private:
#if COMFY_DEBUG
		ComPtr<ID3D11Debug> debugInterface = nullptr;
		ComPtr<ID3D11InfoQueue> infoQueue = nullptr;
#endif /* COMFY_DEBUG */
	};

	// NOTE: Global instance
	extern Direct3D D3D;
}

#ifndef COMFY_D3D11_DEBUG_NAMES
#ifdef COMFY_DEBUG
#define COMFY_D3D11_DEBUG_NAMES 1
#else
#define COMFY_D3D11_DEBUG_NAMES 0
#endif /* COMFY_DEBUG */
#endif /* COMFY_D3D11_DEBUG_NAMES */

#if COMFY_D3D11_DEBUG_NAMES
inline void D3D11_SetObjectDebugName(ID3D11DeviceChild* deviceChild, const char* format, ...)
{
	char buffer[512];

	va_list arguments;
	va_start(arguments, format);
	int result = vsprintf_s(buffer, std::size(buffer), format, arguments);
	va_end(arguments);

	if (deviceChild != nullptr)
		deviceChild->SetPrivateData(WKPDID_D3DDebugObjectName, result, buffer);
}
#else
#define D3D11_SetObjectDebugName(deviceChild, format, ...) do {} while(false);
#endif /* COMFY_D3D11_DEBUG_NAMES */
