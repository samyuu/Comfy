#include "Direct3D.h"
#include "D3D_RenderTarget.h"
#include "Core/CoreTypes.h"
#include "Core/Logger.h"

namespace Graphics
{
	namespace
	{
		template <class T>
		inline void DisposeReleaseSetNullIfNotNull(T*& resourcePointerReference)
		{
			if (resourcePointerReference == nullptr)
				return;

			resourcePointerReference->Release();
			resourcePointerReference = nullptr;
		}
	}

	// NOTE: Global instance
	Direct3D D3D;

	bool Direct3D::Initialize(HWND window)
	{
		if (!InternalCreateDeviceAndSwapchain(window))
			return false;

		WindowRenderTarget = MakeUnique<D3D_SwapChainRenderTarget>(SwapChain);

		return true;
	}

	void Direct3D::Dispose()
	{
		DisposeReleaseSetNullIfNotNull(Device);
		DisposeReleaseSetNullIfNotNull(Context);
		DisposeReleaseSetNullIfNotNull(SwapChain);
		WindowRenderTarget = nullptr;
	}

	void Direct3D::ResizeWindowRenderTarget(ivec2 newSize)
	{
		WindowRenderTarget->Resize(newSize);
	}

	bool Direct3D::InternalCreateDeviceAndSwapchain(HWND window)
	{
		DXGI_SWAP_CHAIN_DESC swapChainDescription = {};
		swapChainDescription.BufferCount = 2;
		swapChainDescription.BufferDesc.Width = 0;
		swapChainDescription.BufferDesc.Height = 0;
		swapChainDescription.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDescription.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDescription.BufferDesc.RefreshRate.Denominator = 0;
		swapChainDescription.Flags = 0;
		swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDescription.OutputWindow = window;
		swapChainDescription.SampleDesc.Count = 1;
		swapChainDescription.SampleDesc.Quality = 0;
		swapChainDescription.Windowed = true;
		swapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

		UINT deviceFlags = 0;
#if COMFY_DEBUG
		deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		D3D_FEATURE_LEVEL featureLevel;

		std::array<D3D_FEATURE_LEVEL, 3> featureLevels =
		{
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
		};

		IDXGIAdapter* adapter = nullptr;
		HMODULE software = NULL;

		const HRESULT deviceSwapChainResult = D3D11CreateDeviceAndSwapChain(
			adapter,
			D3D_DRIVER_TYPE_HARDWARE,
			software,
			deviceFlags,
			featureLevels.data(),
			static_cast<UINT>(featureLevels.size()),
			D3D11_SDK_VERSION,
			&swapChainDescription,
			&SwapChain,
			&Device,
			&featureLevel,
			&Context);

		if (FAILED(deviceSwapChainResult))
		{
			Logger::LogErrorLine(__FUNCTION__"(): Unable to create device and swap chain. Error: 0x%X", deviceSwapChainResult);
			return false;
		}

		return true;
	}
}