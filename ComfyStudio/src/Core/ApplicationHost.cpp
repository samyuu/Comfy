#include "ApplicationHost.h"
#include "Graphics/Direct3D/Direct3D.h"
#include "Input/DirectInput/DualShock4.h"
#include "Input/Keyboard.h"
#include "Core/Logger.h"
#include "Core/Win32/ComfyWindows.h"
#include "Misc/StringHelper.h"
#include "../res/resource.h"

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

	InitializeDirectInput(GlobalModuleHandle);
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

			if (!windowFocused)
			{
				// NOTE: Arbitrary sleep to drastically reduce power usage. This could really use a better solution for final release builds
				::Sleep(static_cast<uint32_t>(powerSleepDuration.TotalMilliseconds()));
			}

			Graphics::D3D.SwapChain->Present(swapInterval, 0);
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
	Keyboard::DeleteInstance();
	DualShock4::DeleteInstance();

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

ivec2 ApplicationHost::GetWindowPosition() const
{
	return windowPosition;
}

ivec2 ApplicationHost::GetWindowSize() const
{
	return windowSize;
}

void ApplicationHost::SetClipboardString(const std::string_view value) const
{
	GetClipboardString();

	const std::wstring_view wideString = Utf8ToUtf16(value);

	::OpenClipboard(windowHandle);
	{
		HGLOBAL dataHandle = ::GlobalAlloc(GMEM_MOVEABLE, wideString.size());

		memcpy(::GlobalLock(dataHandle), wideString.data(), wideString.size() * sizeof(wchar_t));
		::GlobalUnlock(dataHandle);

		::EmptyClipboard();
		::SetClipboardData(CF_UNICODETEXT, dataHandle);
	}
	::CloseClipboard();
}

std::string ApplicationHost::GetClipboardString() const
{
	::OpenClipboard(windowHandle);
	HANDLE dataHandle = ::GetClipboardData(CF_UNICODETEXT);
	const std::wstring wideString = std::wstring(reinterpret_cast<wchar_t*>(::GlobalLock(dataHandle)));

	::GlobalUnlock(dataHandle);
	::CloseClipboard();

	return Utf16ToUtf8(wideString);
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

void ApplicationHost::SetComfyWindowIcon(HWND windowHandle)
{
	assert(GlobalIconHandle != NULL);

	const LPARAM iconArgument = reinterpret_cast<LPARAM>(GlobalIconHandle);
	::SendMessageA(windowHandle, WM_SETICON, ICON_SMALL, iconArgument);
	::SendMessageA(windowHandle, WM_SETICON, ICON_BIG, iconArgument);
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
	windowClass.hIcon = GlobalIconHandle;
	windowClass.hCursor = ::LoadCursorA(NULL, IDC_ARROW);
	windowClass.hbrBackground = NULL;
	windowClass.lpszMenuName = NULL;
	windowClass.lpszClassName = ComfyWindowClassName;
	windowClass.hIconSm = GlobalIconHandle;

	if (!::RegisterClassExA(&windowClass))
		return false;

	windowHandle = ::CreateWindowExA(
		NULL,
		ComfyWindowClassName,
		ComfyStudioWindowTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		StartupWindowSize.x,
		StartupWindowSize.y,
		NULL,
		NULL,
		GlobalModuleHandle,
		this);

	if (windowHandle == NULL)
		return false;

	if (!Graphics::D3D.Initialize(windowHandle))
	{
		Graphics::D3D.Dispose();
		return false;
	}

	::ShowWindow(windowHandle, SW_SHOWDEFAULT);
	::UpdateWindow(windowHandle);

	return true;
}

#if 0
bool ApplicationHost::InternalWindowRegisterCallbacks()
{
	GlobalCallbackInstance = this;
	glfwSetWindowUserPointer(window, this);

	glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height)
	{
		DecodeWindowUserDataPointer(window)->InternalWindowResizeCallback(ivec2(width, height));
	});

	glfwSetWindowPosCallback(window, [](GLFWwindow* window, int x, int y)
	{
		DecodeWindowUserDataPointer(window)->InternalWindowMoveCallback(ivec2(x, y));
	});

	glfwSetCursorPosCallback(window, [](GLFWwindow* window, double x, double y)
	{
		DecodeWindowUserDataPointer(window)->InternalMouseMoveCallback(ivec2(static_cast<int>(x), static_cast<int>(y)));
	});

	glfwSetScrollCallback(window, [](GLFWwindow* window, double xOffset, double yOffset)
	{
		DecodeWindowUserDataPointer(window)->InternalMouseScrollCallback(static_cast<float>(yOffset));
	});

	glfwSetDropCallback(window, [](GLFWwindow* window, int count, const char* paths[])
	{
		DecodeWindowUserDataPointer(window)->InternalWindowDropCallback(count, paths);
	});

	glfwSetWindowFocusCallback(window, [](GLFWwindow* window, int focused)
	{
		DecodeWindowUserDataPointer(window)->InternalWindowFocusCallback(focused);
	});

	glfwSetWindowCloseCallback(window, [](GLFWwindow* window)
	{
		DecodeWindowUserDataPointer(window)->InternalWindowClosingCallback();
	});

	glfwSetJoystickCallback([](int id, int event)
	{
		if (event == GLFW_CONNECTED || event == GLFW_DISCONNECTED)
			GlobalCallbackInstance->InternalCheckConnectedDevices();
	});

	return true;
}
#endif

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

void ApplicationHost::InternalWindowResizeCallback(ivec2 size)
{
	windowSize = size;

	if (windowResizeCallback.has_value())
		windowResizeCallback.value()(windowSize);
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
	if (!Keyboard::GetInstanceInitialized())
	{
		if (Keyboard::TryInitializeInstance())
		{
			// Logger::LogLine(__FUNCTION__"(): Keyboard connected and initialized");
		}
	}

	if (!DualShock4::GetInstanceInitialized())
	{
		if (DualShock4::TryInitializeInstance())
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

	if (Keyboard::GetInstanceInitialized())
		Keyboard::GetInstance()->PollInput();

	if (DualShock4::GetInstanceInitialized())
	{
		if (!DualShock4::GetInstance()->PollInput())
		{
			DualShock4::DeleteInstance();
			Logger::LogLine(__FUNCTION__"(): DualShock4 connection lost");
		}
	}
}

void ApplicationHost::InternalDisposeWindow()
{
	// TODO: Is this really necessary or should windows dispose of this itself?

	Graphics::D3D.Dispose();

	if (windowHandle != nullptr)
	{
		::DestroyWindow(windowHandle);
		windowHandle = nullptr;
	}

	::UnregisterClassA(ComfyWindowClassName, GlobalModuleHandle);
}

LRESULT ApplicationHost::InternalProcessWindowMessage(const UINT message, const WPARAM parameter, const LPARAM userData)
{
	if (windowProcCallback.has_value() && windowProcCallback.value()(windowHandle, message, parameter, userData))
		return 0;

	switch (message)
	{

	case WM_SIZE:
	{
		const ivec2 size = { LOWORD(parameter), HIWORD(parameter) };
		if (parameter != SIZE_MINIMIZED)
			InternalWindowResizeCallback(size);
		return 0;
	}

	case WM_MOVE:
	{
		const ivec2 position = { LOWORD(parameter), HIWORD(parameter) };
		InternalWindowMoveCallback(position);
		return 0;
	}

	case WM_MOUSEMOVE:
	{
		const ivec2 position = { LOWORD(parameter), HIWORD(parameter) };
		InternalMouseMoveCallback(position);
		return 0;
	}

	case WM_MOUSEWHEEL:
	{
		InternalMouseScrollCallback(static_cast<float>(GET_WHEEL_DELTA_WPARAM(parameter)) / static_cast<float>(WHEEL_DELTA));
		return 0;
	}

	// TODO: case WM_DROPFILES:

	case WM_DEVICECHANGE:
	{
		InternalCheckConnectedDevices();
		return 0;
	}

	case WM_SYSCOMMAND:
	{
		// NOTE: Disable ALT application menu
		if (auto lowOrderBits = (parameter & 0xFFF0); lowOrderBits == SC_KEYMENU)
			return 0;

		break;
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

	return ::DefWindowProcA(windowHandle, message, parameter, userData);
}

LRESULT ApplicationHost::ProcessWindowMessage(HWND windowHandle, UINT message, WPARAM parameter, LPARAM userData)
{
	ApplicationHost* receiver = nullptr;

	if (message == WM_NCCREATE)
	{
		LPCREATESTRUCT createStruct = reinterpret_cast<LPCREATESTRUCT>(userData);
		receiver = reinterpret_cast<ApplicationHost*>(createStruct->lpCreateParams);

		receiver->windowHandle = windowHandle;
		::SetWindowLongPtrA(windowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(receiver));
	}
	else
	{
		receiver = reinterpret_cast<ApplicationHost*>(::GetWindowLongPtrA(windowHandle, GWLP_USERDATA));
	}

	if (receiver != nullptr)
		return receiver->InternalProcessWindowMessage(message, parameter, userData);

	return ::DefWindowProcA(windowHandle, message, parameter, userData);
}
