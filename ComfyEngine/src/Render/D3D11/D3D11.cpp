#include "D3D11.h"
#include "Core/Logger.h"
#include "Misc/UTF8.h"
#include "System/Library/LibraryLoader.h"

#if COMFY_D3D11_DEBUG_NAMES
typedef INT(WINAPI D3DPERF_BEGINEVENT)(DWORD col, LPCWSTR wszName);
typedef INT(WINAPI D3DPERF_ENDEVENT)(void);
typedef VOID(WINAPI D3DPERF_SETMARKER)(DWORD col, LPCWSTR wszName);
typedef DWORD(WINAPI D3DPERF_GETSTATUS)(void);

namespace Comfy::Render
{
	struct GlobalD3D9Data
	{
		// NOTE: Yes this still works with d3d11
		Comfy::System::LibraryLoader Library { "d3d9.dll" };

		// NOTE: Alternatively the "ID3DUserDefinedAnnotation" interface provides similar functionality 
		//		 but requires d3d11_1 and doesn't allow setting event and marker colors
		D3DPERF_BEGINEVENT* D3DPERF_BeginEvent = nullptr;
		D3DPERF_ENDEVENT* D3DPERF_EndEvent = nullptr;
		D3DPERF_SETMARKER* D3DPERF_SetMarker = nullptr;
		D3DPERF_GETSTATUS* D3DPERF_GetStatus = nullptr;

		bool RunningUnderGraphicsDebugger = false;

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

namespace Comfy::Render
{
	bool D3D11::Initialize(HWND windowHandle)
	{
#if COMFY_D3D11_DEBUG_NAMES
		GlobalD3D9.LoadImport();
		if (GlobalD3D9.D3DPERF_GetStatus != nullptr)
			GlobalD3D9.RunningUnderGraphicsDebugger = GlobalD3D9.D3DPERF_GetStatus();
#endif /* COMFY_D3D11_DEBUG_NAMES */

		HRESULT hr = {};
		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferDesc.Width = 0;
		swapChainDesc.BufferDesc.Height = 0;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 2;
		swapChainDesc.OutputWindow = windowHandle;
		swapChainDesc.Windowed = true;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.Flags = 0;

		D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_HARDWARE;
#if 0 // DEBUG: Sanity checks
		driverType = D3D_DRIVER_TYPE_REFERENCE;
#elif 0 // DEBUG: Sanity checks... but with a bit more speed
		driverType = D3D_DRIVER_TYPE_WARP;
#endif

		UINT deviceFlags = {};
#if 0 // NOTE: Single threaded no longer works if used together with multithreaded video decoding
		deviceFlags |= D3D11_CREATE_DEVICE_SINGLETHREADED;
#endif
#if 0 // NOTE: Settings the video support flag makes it so the device creation fails if unsupported which definitely isn't desired here
		deviceFlags |= D3D11_CREATE_DEVICE_VIDEO_SUPPORT;
#endif
#if COMFY_DEBUG
		deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		D3D_FEATURE_LEVEL outFeatureLevel = {};
		constexpr D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
		};

		hr = ::D3D11CreateDeviceAndSwapChain(
			nullptr,
			driverType,
			NULL,
			deviceFlags,
			featureLevels,
			ARRAYSIZE(featureLevels),
			D3D11_SDK_VERSION,
			&swapChainDesc,
			&OutputWindow.SwapChain,
			&Device,
			&outFeatureLevel,
			&ImmediateContext);

		if (FAILED(hr))
		{
			Logger::LogErrorLine(__FUNCTION__"(): Unable to create device and swap chain. Error: 0x%X", hr);
			return false;
		}

#if COMFY_DEBUG
		{
			hr = Device->QueryInterface(__uuidof(Debug.Interface), &Debug.Interface);
			hr = Device->QueryInterface(__uuidof(Debug.InfoQueue), &Debug.InfoQueue);

			if (Debug.InfoQueue != nullptr)
			{
				D3D11_MESSAGE_ID hide[] =
				{
					// TODO: SWAP CHAIN BS

					// NOTE: Because the Renderer2D is fine with using the default null value for unused textures / samplers
					D3D11_MESSAGE_ID_DEVICE_DRAW_SAMPLER_NOT_SET,

					// NOTE: Not sure about this one, multiple resources with the same debug name shouldn't be a problem (?)
					D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,

					// NOTE: Because only rendering to the depth buffer is intended behavior during the shadow mapping render pass
					D3D11_MESSAGE_ID_DEVICE_DRAW_RENDERTARGETVIEW_NOT_SET,

					/*
						D3D11 WARNING: Using ID3D11Debug::ReportLiveDeviceObjects with D3D11_RLDO_DETAIL will help drill into object lifetimes.
						Objects with Refcount=0 and IntRef=0 will be eventually destroyed through typical Immediate Context usage.
						However, if the application requires these objects to be destroyed sooner, ClearState followed by Flush on the Immediate Context will realize their destruction.
						[ STATE_CREATION WARNING #422: LIVE_OBJECT_SUMMARY]
					*/ D3D11_MESSAGE_ID_LIVE_OBJECT_SUMMARY,
				};

				D3D11_INFO_QUEUE_FILTER filter = {};
				filter.DenyList.NumIDs = static_cast<UINT>(std::size(hide));
				filter.DenyList.pIDList = hide;
				hr = Debug.InfoQueue->AddStorageFilterEntries(&filter);

#if 0
				hr = Debug.InfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
				hr = Debug.InfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, true);
#endif
#if 0 // BUG: Unhandled exception in KernelBase.dll when used with IMFMediaEngine..?
				hr = Debug.InfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif
			}
			else
			{
				assert(!"ID3D11Debug not supported?");
			}
		}
#endif

		hr = Device->QueryInterface(__uuidof(DXGI.Device), &DXGI.Device);
		hr = DXGI.Device->GetAdapter(&DXGI.Adapter);
		hr = DXGI.Adapter->GetParent(__uuidof(DXGI.Factory), &DXGI.Factory);
		hr = DXGI.Factory->MakeWindowAssociation(windowHandle, DXGI_MWA_NO_ALT_ENTER);

		hr = OutputWindow.SwapChain->GetBuffer(0, __uuidof(OutputWindow.SwapChainTexture), &OutputWindow.SwapChainTexture);
		hr = Device->CreateRenderTargetView(OutputWindow.SwapChainTexture.Get(), nullptr, &OutputWindow.SwapChainTextureView);

		return true;
	}

	void D3D11::Dispose()
	{
#ifdef COMFY_DEBUG
		if (Debug.InfoQueue != nullptr)
		{
			Debug.InfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, false);
			Debug.InfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, false);
			Debug.InfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, false);
		}

		if (ImmediateContext != nullptr)
		{
			ImmediateContext->ClearState();
			ImmediateContext->Flush();
		}

		auto debugCopyBeforeRelease = Debug.Interface;
		Device = nullptr;
		ImmediateContext = nullptr;
		DXGI = {};
		OutputWindow = {};
		Debug = {};

		if (debugCopyBeforeRelease != nullptr)
		{
			::OutputDebugStringA(
				"--- --- --- --- --- --- --- --- ---\n"
				"> ReportLiveDeviceObjects():\n"
				"--- --- --- --- --- --- --- --- ---\n");
			debugCopyBeforeRelease->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY | D3D11_RLDO_DETAIL | D3D11_RLDO_IGNORE_INTERNAL);
			::OutputDebugStringA(
				"--- --- --- --- --- --- --- --- ---\n");
		}
#else
		Device = nullptr;
		ImmediateContext = nullptr;
		DXGI = {};
		OutputWindow = {};
#endif
	}

	void D3D11::DeferObjectDeletion(ComPtr<ID3D11DeviceChild> object)
	{
		if (object != nullptr)
			Defered.ObjectsToDelete.push_back(std::move(object));
	}

	void D3D11::EndOfFrameDeleteDeferedObjects()
	{
		Defered.ObjectsToDelete.clear();
	}

	void D3D11::ResizeSwapchainIfNeeded(ivec2 newSize)
	{
		D3D11Helper::HandleSwapChainResizeIfNeeded(Device.Get(), OutputWindow.SwapChain.Get(), OutputWindow.SwapChainTexture, OutputWindow.SwapChainTextureView, newSize);
	}

	void D3D11::SetViewport(vec2 size)
	{
		SetViewport({ 0.0f, 0.0f }, size);
	}

	void D3D11::SetViewport(vec2 position, vec2 size)
	{
		D3D11_VIEWPORT viewport;
		viewport.TopLeftX = position.x;
		viewport.TopLeftY = position.y;
		viewport.Width = size.x;
		viewport.Height = size.y;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		ImmediateContext->RSSetViewports(1, &viewport);
	}

	void D3D11::SetScissorRect(ivec4 rectangle)
	{
		D3D11_RECT scissorRect = { rectangle.x, rectangle.y, rectangle.z, rectangle.w };
		ImmediateContext->RSSetScissorRects(1, &scissorRect);
	}

	void D3D11::WindowRenderTargetBindAndSetViewport()
	{
		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		OutputWindow.SwapChain->GetDesc(&swapChainDesc);

		ImmediateContext->OMSetRenderTargets(1, PtrArg<ID3D11RenderTargetView*>(OutputWindow.SwapChainTextureView.Get()), nullptr);
		SetViewport({ 0.0f, 0.0f }, { static_cast<f32>(swapChainDesc.BufferDesc.Width), static_cast<f32>(swapChainDesc.BufferDesc.Height) });
	}

	void D3D11::WindowRenderTargetClearColor(vec4 clearColor)
	{
		ImmediateContext->ClearRenderTargetView(OutputWindow.SwapChainTextureView.Get(), glm::value_ptr(clearColor));
	}
}

#if COMFY_D3D11_DEBUG_NAMES
void D3D11_BeginDebugEvent(std::string_view name, u32 color)
{
	using namespace Comfy;
	using namespace Comfy::Render;
	if (GlobalD3D9.RunningUnderGraphicsDebugger && GlobalD3D9.D3DPERF_BeginEvent != nullptr)
		GlobalD3D9.D3DPERF_BeginEvent(static_cast<DWORD>(color), UTF8::WideArg(name).c_str());
}

void D3D11_EndDebugEvent()
{
	using namespace Comfy::Render;
	if (GlobalD3D9.RunningUnderGraphicsDebugger && GlobalD3D9.D3DPERF_EndEvent != nullptr)
		GlobalD3D9.D3DPERF_EndEvent();
}

void D3D11_SetDebugMarker(std::string_view name, u32 color)
{
	using namespace Comfy;
	using namespace Comfy::Render;
	if (GlobalD3D9.RunningUnderGraphicsDebugger && GlobalD3D9.D3DPERF_SetMarker != nullptr)
		GlobalD3D9.D3DPERF_SetMarker(static_cast<DWORD>(color), UTF8::WideArg(name).c_str());
}
#endif
