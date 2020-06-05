#include "ApplicationHost.h"
#include "Render/D3D11/Direct3D.h"
#include "Render/D3D11/Texture/RenderTarget.h"
#include "Audio/Audio.h"
#include "Input/Input.h"
#include "Core/Logger.h"
#include "Core/Win32/ComfyWindows.h"
#include "IO/File.h"
#include "System/Profiling/Profiler.h"
#include "System/ComfyData.h"
#include "Misc/StringUtil.h"
#include "ImGui/GuiRenderer.h"

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
			bool IsMaximized = false;

			ivec2 Position = DefaultStartupWindowPosition;
			ivec2 Size = DefaultStartupWindowSize;

			ivec4 RestoreRegion = { DefaultStartupWindowPosition, DefaultStartupWindowSize };

			bool UseDefaultPosition = false;
			bool UseDefaultSize = false;

			ivec2 PreFullScreenPosition = DefaultStartupWindowPosition;
			ivec2 PreFullScreenSize = DefaultStartupWindowSize;

			bool Focused = true, LastFocused = false;
			bool FocusLostThisFrame = false, FocusGainedThisFrame = false;

			vec4 ClearColor = vec4(0.16f, 0.16f, 0.16f, 0.0f);

		} Window;

		struct CallbackData
		{
			std::function<bool(HWND, UINT, WPARAM, LPARAM)> WindowProc = {};
			std::function<void(ivec2 size)> WindowResize = {};
			std::function<void()> WindowClosing = {};
			std::function<void()> UpdateFunction = {};
		} Callback;

		struct TimingData
		{
			int SwapInterval = 1;
			TimeSpan ElapsedTime = TimeSpan::FromSeconds(0.0f);
			TimeSpan CurrentTime, LastTime;
			u64 ElapsedFrames = 0;

			bool MainLoopLowPowerSleep = false;
			TimeSpan PowerSleepDuration = TimeSpan::FromMilliseconds(10.0);
		} Timing;

		struct FileDropData
		{
			std::vector<std::string> DroppedFiles;

			bool FilesDroppedThisFrame = false, FilesDropped = false;
			bool FilesLastDropped = false, FileDropDispatched = false;
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

			Audio::Engine::CreateInstance();

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
			PreUpdateFrameChangeState();
			PreUpdatePollInput();
			{
				System::Profiler::Get().StartFrame();
				UserUpdateTick();
				System::Profiler::Get().EndFrame();

				Timing.MainLoopLowPowerSleep = (!Window.Focused && !GuiRenderer.IsAnyViewportFocused());
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

			if (Audio::Engine::InstanceValid())
				Audio::Engine::GetInstance().StopStream();
			Audio::Engine::DeleteInstance();

			Render::D3D11::D3D.Dispose();
			DisposeWindow();
		}

	public:
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

			Window.Handle = ::CreateWindowExW(
				NULL,
				windowClassName.c_str(),
				windowTitle.c_str(),
				WS_OVERLAPPEDWINDOW,
				Window.UseDefaultPosition ? CW_USEDEFAULT : Window.Position.x,
				Window.UseDefaultPosition ? CW_USEDEFAULT : Window.Position.y,
				Window.UseDefaultSize ? CW_USEDEFAULT : Window.Size.x,
				Window.UseDefaultSize ? CW_USEDEFAULT : Window.Size.y,
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
				WINDOWPLACEMENT windowPlacement;
				windowPlacement.length = sizeof(WINDOWPLACEMENT);
				windowPlacement.flags = 0;
				windowPlacement.showCmd = SW_MAXIMIZE;
				windowPlacement.ptMinPosition = { Window.Position.x, Window.Position.y };
				windowPlacement.ptMaxPosition = { Window.Position.x, Window.Position.y };
				windowPlacement.rcNormalPosition = { Window.RestoreRegion.x, Window.RestoreRegion.y, Window.RestoreRegion.x + Window.RestoreRegion.z, Window.RestoreRegion.y + Window.RestoreRegion.w };

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
				::ShowWindow(Window.Handle, SW_SHOWDEFAULT);
			}

			::UpdateWindow(Window.Handle);
			return true;
		}

		void InternalSnycMoveWindow()
		{
			if (Window.Handle != nullptr)
				::MoveWindow(Window.Handle, Window.Position.x, Window.Position.y, Window.Size.x, Window.Size.y, true);
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

		void InternalWindowDropCallback(size_t count, const char* paths[])
		{
			FileDrop.FileDropDispatched = false;
			FileDrop.FilesDropped = true;

			FileDrop.DroppedFiles.clear();
			FileDrop.DroppedFiles.reserve(count);

			for (size_t i = 0; i < count; i++)
				FileDrop.DroppedFiles.emplace_back(paths[i]);
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

		void InternalWindowClosingCallback()
		{
			if (Callback.WindowClosing)
				Callback.WindowClosing();
		}

		bool CheckConnectedDevices()
		{
			if (!Input::Keyboard::GetInstanceInitialized())
			{
				if (Input::Keyboard::TryInitializeInstance())
				{
					Logger::LogLine(__FUNCTION__"(): Keyboard connected and initialized");
				}
			}

			if (!Input::DualShock4::GetInstanceInitialized())
			{
				if (Input::DualShock4::TryInitializeInstance())
				{
					Logger::LogLine(__FUNCTION__"(): DualShock4 connected and initialized");
				}
			}

			return true;
		}

	public:
		void PreUpdateFrameChangeState()
		{
			FileDrop.FilesDroppedThisFrame = FileDrop.FilesDropped && !FileDrop.FilesLastDropped;
			FileDrop.FilesLastDropped = FileDrop.FilesDropped;
			FileDrop.FilesDropped = false;

			if (Timing.ElapsedFrames > 2)
			{
				Window.FocusLostThisFrame = Window.LastFocused && !Window.Focused;
				Window.FocusGainedThisFrame = !Window.LastFocused && Window.Focused;
			}
			Window.LastFocused = Window.Focused;
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

			// TODO: case WM_DROPFILES:

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
				InternalWindowClosingCallback();
				break;
			}

			case WM_DESTROY:
			{
				Exit();
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
		if (impl->Window.IsFullscreen == value)
			return;

		if (value)
		{
			impl->Window.PreFullScreenPosition = impl->Window.Position;
			impl->Window.PreFullScreenSize = impl->Window.Size;

			// TODO: Enter fullscreen
		}
		else
		{
			// TODO: Exit fullscreen and restore window position / size
		}
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

	void ApplicationHost::SetSwapInterval(int interval)
	{
		impl->Timing.SwapInterval = interval;
	}

	bool ApplicationHost::HasFocusBeenGained() const
	{
		return impl->Window.FocusGainedThisFrame;
	}

	bool ApplicationHost::HasFocusBeenLost() const
	{
		return impl->Window.FocusLostThisFrame;
	}

	bool ApplicationHost::IsWindowFocused() const
	{
		return impl->Window.Focused;
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
		impl->InternalSnycMoveWindow();
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
		impl->InternalSnycMoveWindow();
	}

	ivec4 ApplicationHost::GetWindowRestoreRegion()
	{
		return impl->Window.RestoreRegion;
	}

	void ApplicationHost::SetWindowRestoreRegion(ivec4 value)
	{
		impl->Window.RestoreRegion = value;
	}

	void ApplicationHost::SetDefaultPositionWindow(bool value)
	{
		impl->Window.UseDefaultPosition = value;
	}

	void ApplicationHost::SetDefaultResizeWindow(bool value)
	{
		impl->Window.UseDefaultSize = value;
	}

	void* ApplicationHost::GetWindowHandle() const
	{
		return impl->Window.Handle;
	}

	void ApplicationHost::RegisterWindowProcCallback(const std::function<bool(HWND, UINT, WPARAM, LPARAM)> onWindowProc)
	{
		impl->Callback.WindowProc = onWindowProc;
	}

	void ApplicationHost::RegisterWindowResizeCallback(const std::function<void(ivec2 size)> onWindowResize)
	{
		impl->Callback.WindowResize = onWindowResize;
	}

	void ApplicationHost::RegisterWindowClosingCallback(const std::function<void()> onClosing)
	{
		impl->Callback.WindowClosing = onClosing;
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
