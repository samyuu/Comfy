#include "ApplicationHost.h"
#include "Render/D3D11/Direct3D.h"
#include "Render/D3D11/Texture/RenderTarget.h"
#include "Audio/Audio.h"
#include "Input/Input.h"
#include "Core/Logger.h"
#include "Core/Win32/ComfyWindows.h"
#include "IO/File.h"
#include "IO/Shell.h"
#include "System/Profiling/Profiler.h"
#include "System/ComfyData.h"
#include "Misc/StringUtil.h"
#include "ImGui/GuiRenderer.h"
#include <shellapi.h>

namespace Comfy
{
	HMODULE GlobalModuleHandle = NULL;
	HICON GlobalIconHandle = NULL;

	struct ApplicationHost::Impl
	{
	public:
		Impl(ApplicationHost& parent) : Parent(parent)
		{
		}

		~Impl() = default;

	public:
		ApplicationHost& Parent;
		bool FailedToInitialize = false;

		struct WindowData
		{
			std::string Title;

			HWND Handle = nullptr;
			bool IsRunning = false;
			bool IsFullscreen = false;
			bool IsFullscreenRequested = false;
			bool IsMaximized = false;

			// NOTE: All positions and sizes are always stored in client space
			ivec2 Position = DefaultStartupWindowPosition;
			ivec2 Size = DefaultStartupWindowSize;

			ivec4 RestoreRegion = { DefaultStartupWindowPosition, DefaultStartupWindowSize };

			bool UseDefaultPosition = false;
			bool UseDefaultSize = false;

			ivec2 PreFullScreenPosition = DefaultStartupWindowPosition;
			ivec2 PreFullScreenSize = DefaultStartupWindowSize;

			bool Focused = true, LastFocused = false;
			bool FocusLostThisFrame = false, FocusGainedThisFrame = false;

			bool AnyFocused = true, AnyLastFocused = false;
			bool AllFocusLostThisFrame = false, AnyFocusGainedThisFrame = false;

			vec4 ClearColor = vec4(0.16f, 0.16f, 0.16f, 0.0f);

		} Window;

		struct CallbackData
		{
			std::function<bool(HWND, UINT, WPARAM, LPARAM)> WindowProc = {};
			std::function<void(ivec2 size)> WindowResize = {};
			std::function<ApplicationHostCloseResponse()> WindowClosing = {};
			std::function<void()> WindowDestroy = {};
			std::function<void()> UpdateFunction = {};
		} Callback;

		struct TimingData
		{
			int SwapInterval = 1;
			TimeSpan ElapsedTime = TimeSpan::FromSeconds(0.0f);
			TimeSpan CurrentTime, LastTime;
			u64 ElapsedFrames = 0;

			bool MainLoopLowPowerSleep = false;
			TimeSpan PowerSleepDuration = TimeSpan::FromMilliseconds(5.0);
		} Timing;

		struct FileDropData
		{
			std::vector<std::string> DroppedFiles;
			std::vector<wchar_t> Buffer;

			bool FilesDroppedThisFrame = false, FilesDropped = false;
			bool FilesLastDropped = false, FileDropDispatched = false;

			bool ResolveFileLinks = true;
		} FileDrop;

		struct InputData
		{
			ivec2 MousePosition = ivec2(0, 0), LastMousePosition = ivec2(0, 0);
			ivec2 MouseDelta = ivec2(0, 0);

			float MouseWheel = 0.0f, LastMouseWheel = 0.0f;
			bool MouseScrolledUp = false, MouseScrolledDown = false;
		} Input;

		Gui::GuiRenderer GuiRenderer = { Parent };

	public:
		bool Initialize()
		{
			GlobalModuleHandle = ::GetModuleHandleW(nullptr);
			TimeSpan::InitializeClock();

			if (!InternalCreateWindow())
				return false;

			Input::InitializeDirectInput(GlobalModuleHandle);

			if (!CheckConnectedDevices())
				return false;

			Audio::AudioEngine::CreateInstance();

			if (!GuiRenderer.Initialize())
				return false;

			return true;
		}

		void EnterProgramLoop(std::function<void()> updateFunction)
		{
			Callback.UpdateFunction = std::move(updateFunction);
			Window.IsRunning = true;

			MSG message = {};
			while (Window.IsRunning && message.message != WM_QUIT)
			{
				if (::PeekMessageW(&message, NULL, 0, 0, PM_REMOVE))
				{
					::TranslateMessage(&message);
					::DispatchMessageW(&message);
					continue;
				}

				ProgramLoopTick();
			}
		}

		void ProgramLoopTick()
		{
			// NOTE: Delay fullscreen requests until start of the (next) frame to avoid messing up any mid-farme rendering
			UpdateFullscreenRequests();

			PreUpdateFrameChangeState();
			PreUpdatePollInput();
			{
				System::Profiler::Get().StartFrame();
				UserUpdateTick();
				System::Profiler::Get().EndFrame();

				Timing.MainLoopLowPowerSleep = !Window.AnyFocused;
				if (Timing.MainLoopLowPowerSleep)
				{
					// NOTE: Arbitrary sleep to drastically reduce power usage. This could really use a better solution for final release builds
					::Sleep(static_cast<u32>(Timing.PowerSleepDuration.TotalMilliseconds()));
				}

				Render::D3D11::D3D.SwapChain->Present(Timing.SwapInterval, 0);
			}
			PostUpdateElapsedTime();
		}

		void Exit()
		{
			Window.IsRunning = false;
			::PostQuitMessage(EXIT_SUCCESS);
		}

		void Dispose()
		{
			GuiRenderer.Dispose();

			Input::Keyboard::DeleteInstance();
			Input::DualShock4::DeleteInstance();

			if (Audio::AudioEngine::InstanceValid())
				Audio::AudioEngine::GetInstance().StopCloseStream();
			Audio::AudioEngine::DeleteInstance();

			Render::D3D11::D3D.Dispose();
			DisposeWindow();
		}

	public:
		ivec4 ClientToWindowArea(ivec2 clientPosition, ivec2 clientSize)
		{
			RECT inClientOutWindowRect;
			inClientOutWindowRect.left = clientPosition.x;
			inClientOutWindowRect.top = clientPosition.y;
			inClientOutWindowRect.right = clientPosition.x + clientSize.x;
			inClientOutWindowRect.bottom = clientPosition.y + clientSize.y;

			if (!::AdjustWindowRect(&inClientOutWindowRect, WS_OVERLAPPEDWINDOW, false))
				return ivec4(clientPosition, clientSize);

			const ivec2 windowPosition =
			{
				inClientOutWindowRect.left,
				inClientOutWindowRect.top,
			};

			const ivec2 windowSize =
			{
				inClientOutWindowRect.right - inClientOutWindowRect.left,
				inClientOutWindowRect.bottom - inClientOutWindowRect.top,
			};

			return ivec4(windowPosition, windowSize);
		}

		ivec4 ClientToWindowArea(ivec4 clientArea)
		{
			return ClientToWindowArea(ivec2(clientArea.x, clientArea.y), ivec2(clientArea.z, clientArea.w));
		}

		bool InternalCreateWindow()
		{
			const auto windowClassName = UTF8::WideArg(ApplicationHost::ComfyWindowClassName);
			const auto windowTitle = UTF8::WideArg(Window.Title);

			WNDCLASSEXW windowClass = {};
			windowClass.cbSize = sizeof(WNDCLASSEXW);
			windowClass.style = (CS_HREDRAW | CS_VREDRAW);
			windowClass.lpfnWndProc = &StaticProcessWindowMessage;
			windowClass.cbClsExtra = 0;
			windowClass.cbWndExtra = 0;
			windowClass.hInstance = GlobalModuleHandle;
			windowClass.hIcon = ApplicationHost::GetComfyWindowIcon();
			windowClass.hCursor = ::LoadCursorA(NULL, IDC_ARROW);
			windowClass.hbrBackground = NULL;
			windowClass.lpszMenuName = NULL;
			windowClass.lpszClassName = windowClassName.c_str();
			windowClass.hIconSm = ApplicationHost::GetComfyWindowIcon();

			if (!::RegisterClassExW(&windowClass))
				return false;

			auto windowStartupRegion = ClientToWindowArea(Window.Position, Window.Size);
			windowStartupRegion.y = std::max(windowStartupRegion.y, 0);

			Window.Handle = ::CreateWindowExW(
				NULL,
				windowClassName.c_str(),
				windowTitle.c_str(),
				WS_OVERLAPPEDWINDOW,
				Window.UseDefaultPosition ? CW_USEDEFAULT : windowStartupRegion.x,
				Window.UseDefaultPosition ? CW_USEDEFAULT : windowStartupRegion.y,
				Window.UseDefaultSize ? CW_USEDEFAULT : windowStartupRegion.z,
				Window.UseDefaultSize ? CW_USEDEFAULT : windowStartupRegion.w,
				NULL,
				NULL,
				GlobalModuleHandle,
				this);

			if (Window.Handle == NULL)
				return false;

			const bool maximizedOnStartup = Window.IsMaximized;

			// NOTE: Set window placement first to retrieve the maximized window size and set the restore position
			if (maximizedOnStartup)
			{
				const auto windowRestoreRegion = ClientToWindowArea(Window.RestoreRegion);

				WINDOWPLACEMENT windowPlacement;
				windowPlacement.length = sizeof(WINDOWPLACEMENT);
				windowPlacement.flags = 0;
				windowPlacement.showCmd = SW_MAXIMIZE;
				windowPlacement.ptMinPosition = { windowStartupRegion.x, windowStartupRegion.y };
				windowPlacement.ptMaxPosition = { windowStartupRegion.x, windowStartupRegion.y };
				windowPlacement.rcNormalPosition = { windowRestoreRegion.x, windowRestoreRegion.y, windowRestoreRegion.x + windowRestoreRegion.z, windowRestoreRegion.y + windowRestoreRegion.w };
				::SetWindowPlacement(Window.Handle, &windowPlacement);
			}

			if (!Render::D3D11::D3D.Initialize(Window.Handle))
			{
				Render::D3D11::D3D.Dispose();
				return false;
			}

			if (!maximizedOnStartup)
			{
				Window.RestoreRegion = ivec4(Window.Position, Window.Size);
				::ShowWindow(Window.Handle, SW_SHOWNORMAL);
			}

			::UpdateWindow(Window.Handle);

			// NOTE: Should this ever be disabled again (?)
			::DragAcceptFiles(Window.Handle, true);

			return true;
		}

		void InternalApplyWindowPositionAndSize()
		{
			if (Window.Handle != nullptr)
			{
				const auto newWindowRegion = ClientToWindowArea(Window.Position, Window.Size);
				::MoveWindow(Window.Handle, newWindowRegion.x, newWindowRegion.y, newWindowRegion.z, newWindowRegion.w, true);
			}
		}

	public:
		void InternalMouseMoveCallback(ivec2 position)
		{
		}

		void InternalMouseScrollCallback(float offset)
		{
			Input.MouseWheel += offset;
		}

		void InternalWindowMoveCallback(ivec2 position)
		{
			Window.Position = position;
		}

		void InternalWindowResizeCallback(bool minimized, bool maximized, ivec2 size)
		{
			Window.IsMaximized = maximized;
			Window.Size = size;

			if (minimized)
			{
				Window.RestoreRegion = ivec4(Window.Position, Window.Size);
			}
			else
			{
				if (Callback.WindowResize)
					Callback.WindowResize(Window.Size);

				if (Render::D3D11::D3D.WindowRenderTarget != nullptr)
					Render::D3D11::D3D.ResizeWindowRenderTarget(size);

				// HACK: Far from perfect and might cause issues in the future but fixes the ugly freeze frame while resizing for now
				if (Window.IsRunning && Callback.UpdateFunction && Render::D3D11::D3D.SwapChain != nullptr)
					ProgramLoopTick();
			}
		}

		void InternalWindowDropCallback(const HDROP dropHandle)
		{
			const auto droppedFileCount = ::DragQueryFileW(dropHandle, 0xFFFFFFFF, nullptr, 0u);

			FileDrop.FileDropDispatched = false;
			FileDrop.FilesDropped = true;

			FileDrop.DroppedFiles.clear();
			FileDrop.DroppedFiles.reserve(droppedFileCount);

			for (UINT i = 0; i < droppedFileCount; i++)
			{
				const auto requiredBufferSize = ::DragQueryFileW(dropHandle, i, nullptr, 0u);
				if (requiredBufferSize == 0)
					continue;

				FileDrop.Buffer.resize(requiredBufferSize + 1);

				const auto success = ::DragQueryFileW(dropHandle, i, FileDrop.Buffer.data(), static_cast<UINT>(FileDrop.Buffer.size()));
				const auto bufferView = std::wstring_view(FileDrop.Buffer.data(), requiredBufferSize);

				if (success != 0)
				{
					auto path = UTF8::Narrow(bufferView);
					if (FileDrop.ResolveFileLinks && IO::Shell::IsFileLink(path))
						path = IO::Shell::ResolveFileLink(path);
					FileDrop.DroppedFiles.push_back(std::move(path));
				}

				FileDrop.Buffer.clear();
			}

			::DragFinish(dropHandle);
		}

		void InternalWindowPaintCallback()
		{
			PAINTSTRUCT paint;
			HDC hdc = ::BeginPaint(Window.Handle, &paint);
			{
				auto windowFrameBrush = reinterpret_cast<HBRUSH>(/*COLOR_WINDOWFRAME*/6);
				::FillRect(hdc, &paint.rcPaint, windowFrameBrush);
			}
			::EndPaint(Window.Handle, &paint);
		}

		void InternalWindowFocusCallback(bool focused)
		{
			Window.Focused = focused;
		}

		ApplicationHostCloseResponse InternalWindowClosingCallback()
		{
			return (Callback.WindowClosing) ? Callback.WindowClosing() : ApplicationHostCloseResponse::Exit;
		}

		void InternalWindowDestroyCallback()
		{
			if (Callback.WindowDestroy)
				Callback.WindowDestroy();

			Exit();
		}

		bool CheckConnectedDevices()
		{
			if (!Input::Keyboard::GetInstanceInitialized() && Input::Keyboard::TryInitializeInstance())
			{
				Logger::LogLine(__FUNCTION__"(): Keyboard connected and initialized");
			}

			if (!Input::DualShock4::GetInstanceInitialized() && Input::DualShock4::TryInitializeInstance())
			{
				Logger::LogLine(__FUNCTION__"(): DualShock4 connected and initialized");
			}

			return true;
		}

	public:
		void UpdateFullscreenRequests()
		{
			if (Window.IsFullscreen == Window.IsFullscreenRequested)
				return;

			Window.IsFullscreen = Window.IsFullscreenRequested;

			if (Window.IsFullscreen)
			{
				Window.PreFullScreenPosition = Window.Position;
				Window.PreFullScreenSize = Window.Size;

				Render::D3D11::D3D.SwapChain->SetFullscreenState(true, nullptr);
			}
			else
			{
				Render::D3D11::D3D.SwapChain->SetFullscreenState(false, nullptr);

				Window.Position = Window.PreFullScreenPosition;
				Window.Size = Window.PreFullScreenSize;
				InternalApplyWindowPositionAndSize();
			}
		}

		void PreUpdateFrameChangeState()
		{
			FileDrop.FilesDroppedThisFrame = FileDrop.FilesDropped && !FileDrop.FilesLastDropped;
			FileDrop.FilesLastDropped = FileDrop.FilesDropped;
			FileDrop.FilesDropped = false;

			Window.AnyFocused = (Window.Focused || GuiRenderer.IsAnyViewportFocused());

			if (Timing.ElapsedFrames > 2)
			{
				Window.FocusLostThisFrame = Window.LastFocused && !Window.Focused;
				Window.FocusGainedThisFrame = !Window.LastFocused && Window.Focused;

				Window.AllFocusLostThisFrame = Window.AnyLastFocused && !Window.AnyFocused;
				Window.AnyFocusGainedThisFrame = !Window.AnyLastFocused && Window.AnyFocused;
			}

			Window.LastFocused = Window.Focused;
			Window.AnyLastFocused = Window.AnyFocused;
		}

		void GuiMainDockSpace(bool hasMenuBar)
		{
			Gui::PushStyleVar(ImGuiStyleVar_WindowPadding, vec2(0.0f, 0.0f));
			Gui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			Gui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

			const ImGuiViewport* viewport = Gui::GetMainViewport();
			Gui::SetNextWindowPos(viewport->Pos);
			Gui::SetNextWindowSize(viewport->Size);
			Gui::SetNextWindowViewport(viewport->ID);

			constexpr ImGuiWindowFlags dockspaceWindowFlags =
				ImGuiWindowFlags_NoDocking |
				ImGuiWindowFlags_NoTitleBar |
				ImGuiWindowFlags_NoCollapse |
				ImGuiWindowFlags_NoResize |
				ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoBringToFrontOnFocus |
				ImGuiWindowFlags_NoNavFocus |
				ImGuiWindowFlags_NoBackground;

			const ImGuiWindowFlags menuBarFlag = (hasMenuBar) ? ImGuiWindowFlags_MenuBar : ImGuiWindowFlags_None;

			Gui::Begin(Gui::GuiRenderer::MainDockSpaceID, nullptr, dockspaceWindowFlags | menuBarFlag);
			Gui::DockSpace(Gui::GetID(Gui::GuiRenderer::MainDockSpaceID), vec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
			Gui::End();

			Gui::PopStyleVar(3);

			/* // NOTE: An exclusive window can then be created on top using
				const ImGuiViewport* viewport = Gui::GetMainViewport();
				Gui::SetNextWindowPos(viewport->Pos);
				Gui::SetNextWindowSize(viewport->Size);
				Gui::SetNextWindowViewport(viewport->ID);

				constexpr ImGuiWindowFlags fullscreenFlags =
					ImGuiWindowFlags_NoDocking |
					ImGuiWindowFlags_NoTitleBar |
					ImGuiWindowFlags_NoCollapse |
					ImGuiWindowFlags_NoResize |
					ImGuiWindowFlags_NoMove |
					ImGuiWindowFlags_NoNavFocus |
					ImGuiWindowFlags_NoSavedSettings;
			*/
		}

		void UserUpdateTick()
		{
			GuiRenderer.BeginFrame();
			{
				Callback.UpdateFunction();
				Render::D3D11::D3D.WindowRenderTarget->BindSetViewport();
				Render::D3D11::D3D.WindowRenderTarget->Clear(Window.ClearColor);
			}
			GuiRenderer.EndFrame();
			Render::D3D11::D3D.EndOfFrameClearStaleDeviceObjects();
		}

		void PostUpdateElapsedTime()
		{
			Timing.LastTime = Timing.CurrentTime;
			Timing.CurrentTime = TimeSpan::GetTimeNow();
			Timing.ElapsedTime = (Timing.CurrentTime - Timing.LastTime);
			Timing.ElapsedFrames++;
		}

		void PreUpdatePollInput()
		{
			POINT cursorPosition;
			::GetCursorPos(&cursorPosition);
			::ScreenToClient(Window.Handle, &cursorPosition);

			Input.LastMousePosition = Input.MousePosition;
			Input.MousePosition = ivec2(cursorPosition.x, cursorPosition.y);

			Input.MouseDelta = Input.MousePosition - Input.LastMousePosition;

			Input.MouseScrolledUp = Input.LastMouseWheel < Input.MouseWheel;
			Input.MouseScrolledDown = Input.LastMouseWheel > Input.MouseWheel;
			Input.LastMouseWheel = Input.MouseWheel;

			if (Input::Keyboard::GetInstanceInitialized())
				Input::Keyboard::GetInstance()->PollInput();

			if (Input::DualShock4::GetInstanceInitialized())
			{
				if (!Input::DualShock4::GetInstance()->PollInput())
				{
					Input::DualShock4::DeleteInstance();
					Logger::LogLine(__FUNCTION__"(): DualShock4 connection lost");
				}
			}
		}

		void DisposeWindow()
		{
			// TODO: Is this really necessary or should windows dispose of this itself?
			if (Window.Handle != nullptr)
			{
				::DestroyWindow(Window.Handle);
				Window.Handle = nullptr;
			}

			::UnregisterClassW(UTF8::WideArg(ApplicationHost::ComfyWindowClassName).c_str(), GlobalModuleHandle);
		}

	public:
		LRESULT ProcessWindowMessage(const UINT message, const WPARAM wParam, const LPARAM lParam)
		{
			if (Callback.WindowProc && Callback.WindowProc(Window.Handle, message, wParam, lParam))
				return 0;

			switch (message)
			{

			case WM_SIZE:
			{
				auto shortParams = reinterpret_cast<const short*>(&lParam);
				const ivec2 size = { shortParams[0], shortParams[1] };

				const bool minimized = (wParam == SIZE_MINIMIZED);
				const bool maximized = (wParam == SIZE_MAXIMIZED);

				InternalWindowResizeCallback(minimized, maximized, size);
				return 0;
			}

			case WM_MOVE:
			{
				auto shortParams = reinterpret_cast<const short*>(&lParam);
				const ivec2 position = { shortParams[0], shortParams[1] };

				InternalWindowMoveCallback(position);
				return 0;
			}

			case WM_MOUSEMOVE:
			{
				auto shortParams = reinterpret_cast<const short*>(&lParam);
				const ivec2 position = { shortParams[0], shortParams[1] };

				InternalMouseMoveCallback(position);
				return 0;
			}

			case WM_MOUSEWHEEL:
			{
				InternalMouseScrollCallback(static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / static_cast<float>(WHEEL_DELTA));
				return 0;
			}

			case WM_DROPFILES:
			{
				InternalWindowDropCallback(reinterpret_cast<HDROP>(wParam));
				return 0;
			}

			case WM_GETMINMAXINFO:
			{
				MINMAXINFO* minMaxInfo = reinterpret_cast<MINMAXINFO*>(lParam);
				if (WindowSizeRestraints.x > 0)
					minMaxInfo->ptMinTrackSize.x = WindowSizeRestraints.x;
				if (WindowSizeRestraints.y > 0)
					minMaxInfo->ptMinTrackSize.y = WindowSizeRestraints.y;
				return 0;
			}

			case WM_DEVICECHANGE:
			{
				CheckConnectedDevices();
				return 0;
			}

			case WM_SYSCOMMAND:
			{
				// NOTE: Disable ALT application menu
				if (auto lowOrderBits = (wParam & 0xFFF0); lowOrderBits == SC_KEYMENU)
					return 0;

				break;
			}

			case WM_PAINT:
			{
				InternalWindowPaintCallback();
				return 0;
			}

			case WM_SETFOCUS:
			{
				InternalWindowFocusCallback(true);
				return 0;
			}

			case WM_KILLFOCUS:
			{
				InternalWindowFocusCallback(false);
				return 0;
			}

			case WM_CLOSE:
			{
				const auto response = InternalWindowClosingCallback();
				if (response == ApplicationHostCloseResponse::SupressExit)
					return 0;
				break;
			}

			case WM_DESTROY:
			{
				InternalWindowDestroyCallback();
				return 0;
			}

			default:
				break;
			}

			return ::DefWindowProcW(Window.Handle, message, wParam, lParam);
		}

		static LRESULT StaticProcessWindowMessage(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
		{
			Impl* receiver = nullptr;

			if (message == WM_NCCREATE)
			{
				LPCREATESTRUCT createStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
				receiver = reinterpret_cast<Impl*>(createStruct->lpCreateParams);

				receiver->Window.Handle = windowHandle;
				::SetWindowLongPtrW(windowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(receiver));
			}
			else
			{
				receiver = reinterpret_cast<Impl*>(::GetWindowLongPtrW(windowHandle, GWLP_USERDATA));
			}

			if (receiver != nullptr)
				return receiver->ProcessWindowMessage(message, wParam, lParam);

			return ::DefWindowProcW(windowHandle, message, wParam, lParam);
		}
	};

	ApplicationHost::ApplicationHost(const ConstructionParam& param) : impl(std::make_unique<Impl>(*this))
	{
		auto setIfHasValue = [](auto& valueToBeSet, const auto& optional) { if (optional.has_value()) { valueToBeSet = optional.value(); } };

		if (param.IconHandle.has_value())
			GlobalIconHandle = static_cast<HICON>(param.IconHandle.value());

		impl->Window.Title = param.StartupWindowState.Title.value_or(UnnamedWindowName);
		setIfHasValue(impl->Window.RestoreRegion, param.StartupWindowState.RestoreRegion);
		setIfHasValue(impl->Window.Position, param.StartupWindowState.Position);
		setIfHasValue(impl->Window.Size, param.StartupWindowState.Size);
		setIfHasValue(impl->Window.IsFullscreen, param.StartupWindowState.IsFullscreen);
		setIfHasValue(impl->Window.IsMaximized, param.StartupWindowState.IsMaximized);

		impl->Window.UseDefaultPosition = !param.StartupWindowState.Position.has_value();
		impl->Window.UseDefaultSize = !param.StartupWindowState.Size.has_value();

		impl->FailedToInitialize = !(impl->Initialize());
	}

	ApplicationHost::~ApplicationHost()
	{
		impl->Dispose();
	}

	void ApplicationHost::EnterProgramLoop(std::function<void()> updateFunction)
	{
		if (impl->FailedToInitialize)
			return;

		impl->EnterProgramLoop(std::move(updateFunction));
	}

	void ApplicationHost::GuiMainDockspace(bool hasMenuBar)
	{
		impl->GuiMainDockSpace(hasMenuBar);
	}

	void ApplicationHost::Exit()
	{
		impl->Exit();
	}

	std::string_view ApplicationHost::GetWindowTitle() const
	{
		return impl->Window.Title;
	}

	void ApplicationHost::SetWindowTitle(std::string_view value)
	{
		impl->Window.Title = value;

		if (impl->Window.Handle != nullptr)
			::SetWindowTextW(impl->Window.Handle, UTF8::WideArg(value).c_str());
	}

	bool ApplicationHost::GetIsFullscreen() const
	{
		return impl->Window.IsFullscreen;
	}

	void ApplicationHost::SetIsFullscreen(bool value)
	{
		impl->Window.IsFullscreenRequested = value;
	}

	void ApplicationHost::ToggleFullscreen()
	{
		SetIsFullscreen(!impl->Window.IsFullscreen);
	}

	bool ApplicationHost::GetIsMaximized() const
	{
		return impl->Window.IsMaximized;
	}

	void ApplicationHost::SetIsMaximized(bool value)
	{
		impl->Window.IsMaximized = value;

		if (impl->Window.Handle != nullptr)
			::ShowWindow(impl->Window.Handle, impl->Window.IsMaximized ? SW_MAXIMIZE : SW_RESTORE);
	}

	int ApplicationHost::GetSwapInterval() const
	{
		return impl->Timing.SwapInterval;
	}

	void ApplicationHost::SetSwapInterval(int interval)
	{
		impl->Timing.SwapInterval = interval;
	}

	bool ApplicationHost::IsWindowFocused(bool mainWindowOnly) const
	{
		return mainWindowOnly ? impl->Window.Focused : impl->Window.AnyFocused;
	}

	bool ApplicationHost::HasFocusBeenGained(bool mainWindowOnly) const
	{
		return mainWindowOnly ? impl->Window.FocusGainedThisFrame : impl->Window.AnyFocusGainedThisFrame;
	}

	bool ApplicationHost::HasFocusBeenLost(bool mainWindowOnly) const
	{
		return mainWindowOnly ? impl->Window.FocusLostThisFrame : impl->Window.AllFocusLostThisFrame;
	}

	bool ApplicationHost::GetMainLoopPowerSleep() const
	{
		return impl->Timing.MainLoopLowPowerSleep;
	}

	void ApplicationHost::SetMainLoopPowerSleep(bool value)
	{
		impl->Timing.MainLoopLowPowerSleep = value;
	}

	ivec2 ApplicationHost::GetWindowPosition() const
	{
		return impl->Window.Position;
	}

	void ApplicationHost::SetWindowPosition(ivec2 value)
	{
		if (impl->Window.Position == value)
			return;

		impl->Window.Position = value;
		impl->InternalApplyWindowPositionAndSize();
	}

	ivec2 ApplicationHost::GetWindowSize() const
	{
		return impl->Window.Size;
	}

	void ApplicationHost::SetWindowSize(ivec2 value)
	{
		if (impl->Window.Size == value)
			return;

		impl->Window.Size = value;
		impl->InternalApplyWindowPositionAndSize();
	}

	ivec4 ApplicationHost::GetWindowRestoreRegion()
	{
		return impl->Window.RestoreRegion;
	}

	void ApplicationHost::SetWindowRestoreRegion(ivec4 value)
	{
		impl->Window.RestoreRegion = value;
	}

	void* ApplicationHost::GetWindowHandle() const
	{
		return impl->Window.Handle;
	}

	void ApplicationHost::RegisterWindowProcCallback(std::function<bool(HWND, UINT, WPARAM, LPARAM)> onWindowProc)
	{
		impl->Callback.WindowProc = std::move(onWindowProc);
	}

	void ApplicationHost::RegisterWindowResizeCallback(std::function<void(ivec2 size)> onWindowResize)
	{
		impl->Callback.WindowResize = std::move(onWindowResize);
	}

	void ApplicationHost::RegisterWindowClosingCallback(std::function<ApplicationHostCloseResponse()> onClosing)
	{
		impl->Callback.WindowClosing = std::move(onClosing);
	}

	void ApplicationHost::RegisterWindowDestoyCallback(std::function<void()> onDestroy)
	{
		impl->Callback.WindowDestroy = std::move(onDestroy);
	}

	bool ApplicationHost::GetDispatchFileDrop()
	{
		return impl->FileDrop.FilesDroppedThisFrame && !impl->FileDrop.FileDropDispatched;
	}

	void ApplicationHost::SetFileDropDispatched(bool value)
	{
		impl->FileDrop.FileDropDispatched = value;
	}

	const std::vector<std::string>& ApplicationHost::GetDroppedFiles() const
	{
		return impl->FileDrop.DroppedFiles;
	}

	HICON ApplicationHost::GetComfyWindowIcon()
	{
		return GlobalIconHandle;
	}
}
