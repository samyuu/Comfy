#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "IGraphicsResource.h"
#include "Core/Win32/ComfyWindows.h"
#include <d3d11.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

namespace Comfy::Graphics
{
	class D3D_SwapChainRenderTarget;

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
		UniquePtr<D3D_SwapChainRenderTarget> WindowRenderTarget = nullptr;

	private:
		bool InternalCreateDeviceAndSwapchain(HWND window);
		bool InternalSetUpDebugInterface();

	private:
		std::vector<ID3D11DeviceChild*> objectsToBeReleased;

	private:
#if COMFY_DEBUG
		ComPtr<ID3D11Debug> debugInterface = nullptr;
		ComPtr<ID3D11InfoQueue> infoQueue = nullptr;
#endif
	};

	// NOTE: Global instance
	extern Direct3D D3D;

#if COMFY_DEBUG
	inline void D3D_SetObjectDebugName(ID3D11DeviceChild* deviceChild, const char* format, ...)
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
#define D3D_SetObjectDebugName(deviceChild, format, ...) do {} while(false);
#endif
}
