#include "ApplicationHost.h"
#include "Graphics/D3D11/Direct3D.h"
#include "Input/DirectInput/DualShock4.h"
#include "Input/Keyboard.h"
#include "Core/Logger.h"
#include "Core/Win32/ComfyWindows.h"
#include "Misc/StringHelper.h"
#include "../res/resource.h"

namespace Comfy
{
	namespace
	{
		HMODULE GlobalModuleHandle = NULL;
		HICON GlobalIconHandle = NULL;
	}

	ApplicationHost::ApplicationHost()
	{
	}

	ApplicationHost::~ApplicationHost()
	{
	}

	bool ApplicationHost::Initialize()
	{
		GlobalModuleHandle = ::GetModuleHandleA(nullptr);
		if (!InternalCreateWindow())
			return false;

		Input::InitializeDirectInput(GlobalModuleHandle);
		InternalCheckConnectedDevices();

		return true;
	}

	void ApplicationHost::EnterProgramLoop(const std::function<void()> updateFunction)
	{
		isRunning = true;

		MSG message = {};
		while (isRunning && message.message != WM_QUIT)
		{
			if (::PeekMessageA(&message, NULL, 0, 0, PM_REMOVE))
			{
				::TranslateMessage(&message);
				::DispatchMessageA(&message);
				continue;
			}

			InternalPreUpdateTick();
			{
				updateFunction();

				if (mainLoopLowPowerSleep)
				{
					// NOTE: Arbitrary sleep to drastically reduce power usage. This could really use a better solution for final release builds
					::Sleep(static_cast<u32>(powerSleepDuration.TotalMilliseconds()));
				}

				Graphics::D3D11::D3D.SwapChain->Present(swapInterval, 0);
			}
			InternalPostUpdateTick();
		}
	}

	void ApplicationHost::Exit()
	{
		isRunning = false;
		::PostQuitMessage(EXIT_SUCCESS);
	}

	void ApplicationHost::Dispose()
	{
		Input::Keyboard::DeleteInstance();
		Input::DualShock4::DeleteInstance();

		InternalDisposeWindow();
	}

	bool ApplicationHost::GetIsFullscreen() const
	{
		return isFullscreen;
	}

	void ApplicationHost::SetIsFullscreen(bool value)
	{
		if (GetIsFullscreen() == value)
			return;

		// TODO:
#if 0
		if (value)
		{
			preFullScreenWindowPosition = windowPosition;
			preFullScreenWindowSize = windowSize;

			GLFWmonitor* monitor = GetActiveOrCurrentMonitor(window);
			const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);

			glfwSetWindowMonitor(window, monitor, 0, 0, videoMode->width, videoMode->height, videoMode->refreshRate);
		}
		else
		{
			// NOTE: Restore last window size and position
			glfwSetWindowMonitor(window, nullptr,
				preFullScreenWindowPosition.x,
				preFullScreenWindowPosition.y,
				preFullScreenWindowSize.x,
				preFullScreenWindowSize.y,
				GLFW_DONT_CARE);
		}
#endif
	}

	void ApplicationHost::ToggleFullscreen()
	{
		const bool fullscreenInverse = !GetIsFullscreen();
		SetIsFullscreen(fullscreenInverse);
	}

	bool ApplicationHost::GetIsMaximized() const
	{
		return isMaximized;
	}

	void ApplicationHost::SetIsMaximized(bool value)
	{
		isMaximized = value;

		if (windowHandle != nullptr)
			::ShowWindow(windowHandle, isMaximized ? SW_MAXIMIZE : SW_RESTORE);
	}

	void ApplicationHost::SetSwapInterval(int interval)
	{
		swapInterval = interval;
	}

	bool ApplicationHost::HasFocusBeenGained() const
	{
		return focusGainedThisFrame;
	}

	bool ApplicationHost::HasFocusBeenLost() const
	{
		return focusLostThisFrame;
	}

	bool ApplicationHost::IsWindowFocused() const
	{
		return windowFocused;
	}

	bool ApplicationHost::GetMainLoopPowerSleep() const
	{
		return mainLoopLowPowerSleep;
	}

	void ApplicationHost::SetMainLoopPowerSleep(bool value)
	{
		mainLoopLowPowerSleep = value;
	}

	ivec2 ApplicationHost::GetWindowPosition() const
	{
		return windowPosition;
	}

	void ApplicationHost::SetWindowPosition(ivec2 value)
	{
		if (windowPosition == value)
			return;

		windowPosition = value;
		InternalSnycMoveWindow();
	}

	ivec2 ApplicationHost::GetWindowSize() const
	{
		return windowSize;
	}

	void ApplicationHost::SetWindowSize(ivec2 value)
	{
		if (windowSize == value)
			return;

		windowSize = value;
		InternalSnycMoveWindow();
	}

	ivec4 ApplicationHost::GetWindowRestoreRegion()
	{
		return windowRestoreRegion;
	}

	void ApplicationHost::SetWindowRestoreRegion(ivec4 value)
	{
		assert(windowHandle == nullptr);
		windowRestoreRegion = value;
	}

	void ApplicationHost::SetDefaultPositionWindow(bool value)
	{
		defaultPositionWindow = value;
	}

	void ApplicationHost::SetDefaultResizeWindow(bool value)
	{
		defaultResizeWindow = value;
	}

	void ApplicationHost::RegisterWindowProcCallback(const std::function<bool(HWND, UINT, WPARAM, LPARAM)> onWindowProc)
	{
		windowProcCallback = onWindowProc;
	}

	void ApplicationHost::RegisterWindowResizeCallback(const std::function<void(ivec2 size)> onWindowResize)
	{
		windowResizeCallback = onWindowResize;
	}

	void ApplicationHost::RegisterWindowClosingCallback(const std::function<void()> onClosing)
	{
		windowClosingCallback = onClosing;
	}

	bool ApplicationHost::GetDispatchFileDrop()
	{
		return filesDroppedThisFrame && !fileDropDispatched;
	}

	void ApplicationHost::SetFileDropDispatched(bool value)
	{
		fileDropDispatched = value;
	}

	const std::vector<std::string>& ApplicationHost::GetDroppedFiles() const
	{
		return droppedFiles;
	}

	void ApplicationHost::LoadComfyWindowIcon()
	{
		assert(GlobalIconHandle == NULL);
		GlobalIconHandle = ::LoadIconA(GlobalModuleHandle, MAKEINTRESOURCEA(COMFY_ICON));
	}

	HICON ApplicationHost::GetComfyWindowIcon()
	{
		return GlobalIconHandle;
	}

	bool ApplicationHost::InternalCreateWindow()
	{
		ApplicationHost::LoadComfyWindowIcon();

		WNDCLASSEX windowClass = {};
		windowClass.cbSize = sizeof(WNDCLASSEX);
		windowClass.style = CS_HREDRAW | CS_VREDRAW;
		windowClass.lpfnWndProc = &ProcessWindowMessage;
		windowClass.cbClsExtra = 0;
		windowClass.cbWndExtra = 0;
		windowClass.hInstance = GlobalModuleHandle;
		windowClass.hIcon = GetComfyWindowIcon();
		windowClass.hCursor = ::LoadCursorA(NULL, IDC_ARROW);
		windowClass.hbrBackground = NULL;
		windowClass.lpszMenuName = NULL;
		windowClass.lpszClassName = ComfyWindowClassName;
		windowClass.hIconSm = GetComfyWindowIcon();

		if (!::RegisterClassExA(&windowClass))
			return false;

		windowHandle = ::CreateWindowExA(
			NULL,
			ComfyWindowClassName,
			ComfyStudioWindowTitle,
			WS_OVERLAPPEDWINDOW,
			defaultPositionWindow ? CW_USEDEFAULT : windowPosition.x,
			defaultPositionWindow ? CW_USEDEFAULT : windowPosition.y,
			defaultResizeWindow ? CW_USEDEFAULT : windowSize.x,
			defaultResizeWindow ? CW_USEDEFAULT : windowSize.y,
			NULL,
			NULL,
			GlobalModuleHandle,
			this);

		if (windowHandle == NULL)
			return false;

		bool maximizedOnStartup = isMaximized;

		// NOTE: Set window placement first to retrieve the maximized window size and set the restore position
		if (maximizedOnStartup)
		{
			WINDOWPLACEMENT windowPlacement;
			windowPlacement.length = sizeof(WINDOWPLACEMENT);
			windowPlacement.flags = 0;
			windowPlacement.showCmd = SW_MAXIMIZE;
			windowPlacement.ptMinPosition = { windowPosition.x, windowPosition.y };
			windowPlacement.ptMaxPosition = { windowPosition.x, windowPosition.y };
			windowPlacement.rcNormalPosition = { windowRestoreRegion.x, windowRestoreRegion.y, windowRestoreRegion.x + windowRestoreRegion.z, windowRestoreRegion.y + windowRestoreRegion.w };

			::SetWindowPlacement(windowHandle, &windowPlacement);
		}

		if (!Graphics::D3D11::D3D.Initialize(windowHandle))
		{
			Graphics::D3D11::D3D.Dispose();
			return false;
		}

		if (!maximizedOnStartup)
		{
			windowRestoreRegion = ivec4(windowPosition, windowSize);
			::ShowWindow(windowHandle, SW_SHOWDEFAULT);
		}

		::UpdateWindow(windowHandle);

		return true;
	}

	void ApplicationHost::InternalSnycMoveWindow()
	{
		if (windowHandle != nullptr)
		{
			::MoveWindow(windowHandle, windowPosition.x, windowPosition.y, windowSize.x, windowSize.y, true);
		}
	}

	void ApplicationHost::InternalMouseMoveCallback(ivec2 position)
	{

	}

	void ApplicationHost::InternalMouseScrollCallback(float offset)
	{
		mouseWheel += offset;
	}

	void ApplicationHost::InternalWindowMoveCallback(ivec2 position)
	{
		windowPosition = position;
	}

	void ApplicationHost::InternalWindowResizeCallback(bool minimized, bool maximized, ivec2 size)
	{
		isMaximized = maximized;
		windowSize = size;

		if (minimized)
		{
			windowRestoreRegion = ivec4(windowPosition, windowSize);
		}
		else
		{
			if (windowResizeCallback.has_value())
				windowResizeCallback.value()(windowSize);
		}
	}

	void ApplicationHost::InternalWindowDropCallback(size_t count, const char* paths[])
	{
		fileDropDispatched = false;
		filesDropped = true;

		droppedFiles.clear();
		droppedFiles.reserve(count);

		for (size_t i = 0; i < count; i++)
			droppedFiles.emplace_back(paths[i]);
	}

	void ApplicationHost::InternalWindowPaintCallback()
	{
		PAINTSTRUCT paint;
		HDC hdc = ::BeginPaint(windowHandle, &paint);
		{
			auto windowFrameBrush = reinterpret_cast<HBRUSH>(/*COLOR_WINDOWFRAME*/6);
			::FillRect(hdc, &paint.rcPaint, windowFrameBrush);
		}
		::EndPaint(windowHandle, &paint);
	}

	void ApplicationHost::InternalWindowFocusCallback(bool focused)
	{
		windowFocused = focused;
	}

	void ApplicationHost::InternalWindowClosingCallback()
	{
		if (windowClosingCallback.has_value())
			windowClosingCallback.value()();
	}

	void ApplicationHost::InternalCheckConnectedDevices()
	{
		if (!Input::Keyboard::GetInstanceInitialized())
		{
			if (Input::Keyboard::TryInitializeInstance())
			{
				// Logger::LogLine(__FUNCTION__"(): Keyboard connected and initialized");
			}
		}

		if (!Input::DualShock4::GetInstanceInitialized())
		{
			if (Input::DualShock4::TryInitializeInstance())
			{
				// Logger::LogLine(__FUNCTION__"(): DualShock4 connected and initialized");
			}
		}
	}

	void ApplicationHost::InternalPreUpdateTick()
	{
		filesDroppedThisFrame = filesDropped && !filesLastDropped;
		filesLastDropped = filesDropped;
		filesDropped = false;

		if (elapsedFrames > 2)
		{
			focusLostThisFrame = lastWindowFocused && !windowFocused;
			focusGainedThisFrame = !lastWindowFocused && windowFocused;
		}
		lastWindowFocused = windowFocused;

		InternalPreUpdatePollInput();
	}

	void ApplicationHost::InternalPostUpdateTick()
	{
		lastTime = currentTime;
		currentTime = TimeSpan::GetTimeNow();
		elapsedTime = (currentTime - lastTime);
		elapsedFrames++;
	}

	void ApplicationHost::InternalPreUpdatePollInput()
	{
		POINT cursorPosition;
		::GetCursorPos(&cursorPosition);
		::ScreenToClient(windowHandle, &cursorPosition);

		lastMousePosition = mousePosition;
		mousePosition = ivec2(cursorPosition.x, cursorPosition.y);

		mouseDelta = mousePosition - lastMousePosition;

		mouseScrolledUp = lastMouseWheel < mouseWheel;
		mouseScrolledDown = lastMouseWheel > mouseWheel;
		lastMouseWheel = mouseWheel;

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

	void ApplicationHost::InternalDisposeWindow()
	{
		// TODO: Is this really necessary or should windows dispose of this itself?

		Graphics::D3D11::D3D.Dispose();

		if (windowHandle != nullptr)
		{
			::DestroyWindow(windowHandle);
			windowHandle = nullptr;
		}

		::UnregisterClassA(ComfyWindowClassName, GlobalModuleHandle);
	}

	LRESULT ApplicationHost::InternalProcessWindowMessage(const UINT message, const WPARAM wParam, const LPARAM lParam)
	{
		if (windowProcCallback.has_value() && windowProcCallback.value()(windowHandle, message, wParam, lParam))
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
			InternalCheckConnectedDevices();
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

		return ::DefWindowProcA(windowHandle, message, wParam, lParam);
	}

	LRESULT ApplicationHost::ProcessWindowMessage(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
	{
		ApplicationHost* receiver = nullptr;

		if (message == WM_NCCREATE)
		{
			LPCREATESTRUCT createStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
			receiver = reinterpret_cast<ApplicationHost*>(createStruct->lpCreateParams);

			receiver->windowHandle = windowHandle;
			::SetWindowLongPtrA(windowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(receiver));
		}
		else
		{
			receiver = reinterpret_cast<ApplicationHost*>(::GetWindowLongPtrA(windowHandle, GWLP_USERDATA));
		}

		if (receiver != nullptr)
			return receiver->InternalProcessWindowMessage(message, wParam, lParam);

		return ::DefWindowProcA(windowHandle, message, wParam, lParam);
	}
}
