#pragma once
#include "Types.h"
#include "Core/CoreTypes.h"
#include <d3d11.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

namespace Graphics
{
	class D3D_SwapChainRenderTarget;

	class Direct3D
	{
	public:
		bool Initialize(HWND window);
		void Dispose();

		void ResizeWindowRenderTarget(ivec2 newSize);

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

		deviceChild->SetPrivateData(WKPDID_D3DDebugObjectName, result, buffer);
	}
#else
#define D3D_SetObjectDebugName(deviceChild, format, ...) do {} while(false);
#endif
}
