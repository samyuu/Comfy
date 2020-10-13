#include "Direct3D.h"
#include "Texture/RenderTarget.h"
#include "Core/Logger.h"
#include "Misc/UTF8.h"
#include "System/Library/LibraryLoader.h"

#if COMFY_D3D11_DEBUG_NAMES
typedef INT(WINAPI D3DPERF_BEGINEVENT)(DWORD col, LPCWSTR wszName);
typedef INT(WINAPI D3DPERF_ENDEVENT)(void);
typedef VOID(WINAPI D3DPERF_SETMARKER)(DWORD col, LPCWSTR wszName);
typedef DWORD(WINAPI D3DPERF_GETSTATUS)(void);

namespace
{
	struct GlobalD3D9Data
	{
		// NOTE: Yes this still works with d3d11
		Comfy::System::LibraryLoader Library = { "d3d9.dll" };

		// NOTE: Alternatively the "ID3DUserDefinedAnnotation" interface provides similar functionality 
		//		 but requires d3d11_1 and doesn't allow setting event and marker colors
		D3DPERF_BEGINEVENT* D3DPERF_BeginEvent = nullptr;
		D3DPERF_ENDEVENT* D3DPERF_EndEvent = nullptr;
		D3DPERF_SETMARKER* D3DPERF_SetMarker = nullptr;
		D3DPERF_GETSTATUS* D3DPERF_GetStatus = nullptr;

		inline void LoadImport()
		{
			if (!Library.Load())
				return;

			D3DPERF_BeginEvent = Library.GetFunctionAddress<D3DPERF_BEGINEVENT>("D3DPERF_BeginEvent");
			D3DPERF_EndEvent = Library.GetFunctionAddress<D3DPERF_ENDEVENT>("D3DPERF_EndEvent");
			D3DPERF_SetMarker = Library.GetFunctionAddress<D3DPERF_SETMARKER>("D3DPERF_SetMarker");
			D3DPERF_GetStatus = Library.GetFunctionAddress<D3DPERF_GETSTATUS>("D3DPERF_GetStatus");
		}

	} GlobalD3D9;
}
#endif /* COMFY_D3D11_DEBUG_NAMES */

namespace Comfy::Render::D3D11
{
	namespace
	{
		template <typename T>
		void DisposeReleaseSetNullIfNotNull(T*& resourcePointerReference)
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
#if COMFY_D3D11_DEBUG_NAMES
		GlobalD3D9.LoadImport();
		if (GlobalD3D9.D3DPERF_GetStatus != nullptr)
			runningUnderGraphicsDebugger = GlobalD3D9.D3DPERF_GetStatus();
#endif /* COMFY_D3D11_DEBUG_NAMES */

		if (!InternalCreateDeviceAndSwapchain(window))
			return false;

		WindowRenderTarget = std::make_unique<SwapChainRenderTarget>(SwapChain);

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

	void Direct3D::SetViewport(ivec2 size)
	{
		SetViewport(ivec2(0, 0), size);
	}

	void Direct3D::SetViewport(ivec2 position, ivec2 size)
	{
		D3D11_VIEWPORT viewport;
		viewport.TopLeftX = static_cast<float>(position.x);
		viewport.TopLeftY = static_cast<float>(position.y);
		viewport.Width = static_cast<float>(size.x);
		viewport.Height = static_cast<float>(size.y);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		D3D.Context->RSSetViewports(1, &viewport);
	}

	void Direct3D::SetScissorRect(ivec4 rectangle)
	{
		D3D11_RECT scissorRect = { rectangle.x, rectangle.y, rectangle.z, rectangle.w };
		D3D.Context->RSSetScissorRects(1, &scissorRect);
	}

	void Direct3D::EnsureDeviceObjectLifetimeUntilRendering(ID3D11DeviceChild* object)
	{
		if (object == nullptr)
			return;

		object->AddRef();
		objectsToBeReleased.push_back(object);
	}

	void Direct3D::EndOfFrameClearStaleDeviceObjects()
	{
		if (objectsToBeReleased.empty())
			return;

		for (auto object : objectsToBeReleased)
			object->Release();

		objectsToBeReleased.clear();
	}

#if COMFY_D3D11_DEBUG_NAMES
	void Direct3D::BeginDebugEvent(std::string_view name, u32 color)
	{
		if (runningUnderGraphicsDebugger && GlobalD3D9.D3DPERF_BeginEvent != nullptr)
			GlobalD3D9.D3DPERF_BeginEvent(static_cast<DWORD>(color), UTF8::WideArg(name).c_str());
	}

	void Direct3D::EndDebugEvent()
	{
		if (runningUnderGraphicsDebugger && GlobalD3D9.D3DPERF_EndEvent != nullptr)
			GlobalD3D9.D3DPERF_EndEvent();
	}

	void Direct3D::SetDebugMarker(std::string_view name, u32 color)
	{
		if (runningUnderGraphicsDebugger && GlobalD3D9.D3DPERF_SetMarker != nullptr)
			GlobalD3D9.D3DPERF_SetMarker(static_cast<DWORD>(color), UTF8::WideArg(name).c_str());
	}
#else
	void Direct3D::BeginDebugEvent(std::string_view name, u32 color)
	{
	}

	void Direct3D::EndDebugEvent()
	{
	}

	void Direct3D::SetDebugMarker(std::string_view name, u32 color)
	{
	}
#endif /* COMFY_D3D11_DEBUG_NAMES */

	bool Direct3D::InternalCreateDeviceAndSwapchain(HWND window)
	{
		DXGI_SWAP_CHAIN_DESC swapChainDescription = {};
		swapChainDescription.BufferCount = 2;
		swapChainDescription.BufferDesc.Width = 0;
		swapChainDescription.BufferDesc.Height = 0;
		swapChainDescription.BufferDesc.Format = RenderTargetLDRFormatRGBA;
		swapChainDescription.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDescription.BufferDesc.RefreshRate.Denominator = 0;
		swapChainDescription.Flags = 0;
		swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDescription.OutputWindow = window;
		swapChainDescription.SampleDesc.Count = 1;
		swapChainDescription.SampleDesc.Quality = 0;
		swapChainDescription.Windowed = true;
		swapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

		D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_HARDWARE;
#if 0
		// DEBUG: Sanity checks
		// driverType = D3D_DRIVER_TYPE_REFERENCE;
		driverType = D3D_DRIVER_TYPE_WARP;
#endif

		UINT deviceFlags = D3D11_CREATE_DEVICE_SINGLETHREADED;

#if COMFY_DEBUG
		deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		D3D_FEATURE_LEVEL outFeatureLevel;
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
			driverType,
			software,
			deviceFlags,
			featureLevels.data(),
			static_cast<UINT>(featureLevels.size()),
			D3D11_SDK_VERSION,
			&swapChainDescription,
			&SwapChain,
			&Device,
			&outFeatureLevel,
			&Context);

		if (FAILED(deviceSwapChainResult))
		{
			Logger::LogErrorLine(__FUNCTION__"(): Unable to create device and swap chain. Error: 0x%X", deviceSwapChainResult);
			return false;
		}

		if (!InternalSetUpDebugInterface())
			return false;

		return true;
	}

	bool Direct3D::InternalSetUpDebugInterface()
	{
#if COMFY_DEBUG
		Device->QueryInterface(IID_PPV_ARGS(&debugInterface));
		debugInterface->QueryInterface(IID_PPV_ARGS(&infoQueue));

		D3D11_MESSAGE_ID hide[] =
		{
			// NOTE: Because the Renderer2D is fine with using the default null value for unused textures / samplers
			D3D11_MESSAGE_ID_DEVICE_DRAW_SAMPLER_NOT_SET,

			// NOTE: Not sure about this one, multiple resources with the same debug name shouldn't be a problem (?)
			D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,

			// NOTE: Because only rendering to the depth buffer is intended behavior during the shadow mapping render pass
			D3D11_MESSAGE_ID_DEVICE_DRAW_RENDERTARGETVIEW_NOT_SET,
		};

		D3D11_INFO_QUEUE_FILTER filter = {};
		filter.DenyList.NumIDs = static_cast<UINT>(std::size(hide));
		filter.DenyList.pIDList = hide;

		infoQueue->AddStorageFilterEntries(&filter);
#endif

		return true;
	}
}
