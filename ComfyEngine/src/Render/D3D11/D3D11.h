#pragma once
#include "Types.h"
#include "D3D11Common.h"

#ifndef COMFY_D3D11_DEBUG_NAMES
#if COMFY_DEBUG
#define COMFY_D3D11_DEBUG_NAMES 1
#else
#define COMFY_D3D11_DEBUG_NAMES 0
#endif /* COMFY_DEBUG */
#endif /* COMFY_D3D11_DEBUG_NAMES */

namespace Comfy::Render
{
	// NOTE: Basically the global D3D11 application "context", though not to be confused with the "ID3D11DeviceContext"
	struct D3D11 : NonCopyable
	{
		ComPtr<ID3D11Device> Device;
		ComPtr<ID3D11DeviceContext> ImmediateContext;

		struct DXGIData
		{
			ComPtr<IDXGIDevice1> Device;
			ComPtr<IDXGIAdapter> Adapter;
			ComPtr<IDXGIFactory2> Factory;
		} DXGI;

		struct OutputWindowData
		{
			ComPtr<IDXGISwapChain> SwapChain;
			ComPtr<ID3D11Texture2D> SwapChainTexture;
			ComPtr<ID3D11RenderTargetView> SwapChainTextureView;
		} OutputWindow;

#if COMFY_DEBUG
		struct DebugData
		{
			ComPtr<ID3D11Debug> Interface;
			ComPtr<ID3D11InfoQueue> InfoQueue;
		} Debug;
#endif

		struct DeferedData
		{
			std::vector<ComPtr<ID3D11DeviceChild>> ObjectsToDelete;
		} Defered;

	public:
		D3D11() = default;
		~D3D11() { Dispose(); }

		bool Initialize(HWND windowHandle);
		void Dispose();

		// NOTE: To avoid stale refereces and invalid memory accesses during rendering
		void DeferObjectDeletion(ComPtr<ID3D11DeviceChild> object);
		void EndOfFrameDeleteDeferedObjects();

	public:
		void ResizeSwapchainIfNeeded(ivec2 newSize);
		void SetViewport(vec2 size);
		void SetViewport(vec2 position, vec2 size);
		void SetScissorRect(ivec4 rectangle);
		void WindowRenderTargetBindAndSetViewport();
		void WindowRenderTargetClearColor(vec4 clearColor);
	};

	// NOTE: Ideally this shouldn't be global and instead be explicitly passed around but changing that now would be quite the pain especially for initializers
	inline D3D11 GlobalD3D11 = {};
}

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

void D3D11_BeginDebugEvent(std::string_view name, u32 color = 0x00000000);
void D3D11_EndDebugEvent();
void D3D11_SetDebugMarker(std::string_view name, u32 color = 0x00000000);
#else
inline void D3D11_SetObjectDebugName(ID3D11DeviceChild* deviceChild, const char* format, ...) {}

inline void D3D11_BeginDebugEvent(std::string_view name, u32 color = 0x00000000) {}
inline void D3D11_EndDebugEvent() {}
inline void D3D11_SetDebugMarker(std::string_view name, u32 color = 0x00000000) {}
#endif /* COMFY_D3D11_DEBUG_NAMES */
