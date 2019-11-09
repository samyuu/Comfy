#pragma once
#include "Types.h"
#include <d3d11.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

namespace Graphics
{
	class Direct3D
	{
	public:
		bool Initialize(HWND window);
		void Dispose();

		bool ResizeMainRenderTarget(ivec2 newSize);

	public:
		// NOTE: Raw pointers to optionally skip releasing them
		ID3D11Device* Device = nullptr;
		ID3D11DeviceContext* Context = nullptr;
		IDXGISwapChain* SwapChain = nullptr;
		ID3D11RenderTargetView* MainRenderTargetView = nullptr;

	private:
		bool InternalCreateDeviceAndSwapchain(HWND window);
		bool InternalCreateRenderTarget();
	};

	// NOTE: Global instance
	extern Direct3D D3D;
}
