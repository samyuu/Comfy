#include "ApplicationHost.h"
#include "Graphics/OpenGL/OpenGLLoader.h"
#include "Input/DirectInput/DualShock4.h"
#include "Input/Keyboard.h"
#include "Core/TimeSpan.h"
#include "Core/Logger.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include "Core/Win32/ComfyWindows.h"
#include "../res/resource.h"
#include <glfw/glfw3.h>
#include <glfw/glfw3native.h>

namespace
{
	ApplicationHost* GlobalCallbackInstance;
	
	HMODULE GlobalModuleHandle = NULL;
	HICON GlobalIconHandle = NULL;

	void GlfwErrorCallback(int error, const char* description)
	{
		Logger::LogErrorLine(__FUNCTION__"(): [GLFW Error: 0x%X] %s", error, description);
	}

	GLFWmonitor* GetActiveOrCurrentMonitor(GLFWwindow* window)
	{
		GLFWmonitor* monitor = glfwGetWindowMonitor(window);

		if (monitor != nullptr)
			return monitor;

		int windowXPosition, windowYPosition;
		glfwGetWindowPos(window, &windowXPosition, &windowYPosition);

		int windowWidth, windowHeight;
		glfwGetWindowSize(window, &windowWidth, &windowHeight);

		int monitorCount;
		GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);

		int bestoverlap = 0;
		for (int i = 0; i < monitorCount; i++)
		{
			const GLFWvidmode* videoMode = glfwGetVideoMode(monitors[i]);

			int monitorX, monitorY;
			glfwGetMonitorPos(monitors[i], &monitorX, &monitorY);

			int overlap =
				glm::max(0, glm::min(static_cast<int>(windowXPosition + windowWidth), monitorX + videoMode->width) - glm::max(windowXPosition, monitorX)) *
				glm::max(0, glm::min(static_cast<int>(windowYPosition + windowHeight), monitorY + videoMode->height) - glm::max(windowYPosition, monitorY));

			if (bestoverlap < overlap)
			{
				bestoverlap = overlap;
				monitor = monitors[i];
			}
		}

		return monitor;
	}
}

ApplicationHost::ApplicationHost()
{
}

ApplicationHost::~ApplicationHost()
{
}

bool ApplicationHost::Initialize()
{
	glfwSetErrorCallback(&GlfwErrorCallback);

	if (const bool glfwInitResult = glfwInit(); !glfwInitResult)
		return false;

	GlobalModuleHandle = ::GetModuleHandleA(nullptr);
	if (!InternalCreateWindow())
		return false;

	glfwMakeContextCurrent(window);
	Graphics::OpenGLLoader::LoadFunctions(reinterpret_cast<OpenGLFunctionLoader*>(glfwGetProcAddress));

	InternalWindowRegisterCallbacks();

	InitializeDirectInput(GlobalModuleHandle);
	InternalCheckConnectedDevices();

	return true;
}

void ApplicationHost::EnterProgramLoop(const std::function<void()> updateFunction)
{
	while (!glfwWindowShouldClose(window))
	{
		InternalPreUpdateTick();
		{
			updateFunction();

			if (!windowFocused)
			{
				// NOTE: Arbitrary sleep to drastically reduce power usage. This could really use a better solution for final release builds
				Sleep(static_cast<uint32_t>(powerSleepDuration.TotalMilliseconds()));
			}

			glfwSwapBuffers(window);
			glfwPollEvents();
		}
		InternalPostUpdateTick();
	}
}

void ApplicationHost::Exit()
{
	glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void ApplicationHost::Dispose()
{
	Keyboard::DeleteInstance();
	DualShock4::DeleteInstance();

	glfwDestroyWindow(window);
	glfwTerminate();
}

bool ApplicationHost::IsFullscreen() const
{
	return glfwGetWindowMonitor(window) != nullptr;
}

void ApplicationHost::SetFullscreen(bool value)
{
	if (IsFullscreen() == value)
		return;

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
}

void ApplicationHost::ToggleFullscreen()
{
	const bool fullscreenInverse = !IsFullscreen();
	SetFullscreen(fullscreenInverse);
}

void ApplicationHost::SetSwapInterval(int interval)
{
	glfwSwapInterval(interval);
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

void ApplicationHost::SetClipboardString(const char* value) const
{
	glfwSetClipboardString(window, value);
}

std::string ApplicationHost::GetClipboardString() const
{
	return glfwGetClipboardString(window);
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

void ApplicationHost::SetComfyWindowIcon(GLFWwindow* window)
{
	assert(GlobalIconHandle != NULL);

	const HWND windowHandle = glfwGetWin32Window(window);
	::SendMessageA(windowHandle, WM_SETICON, ICON_SMALL, (LPARAM)GlobalIconHandle);
	::SendMessageA(windowHandle, WM_SETICON, ICON_BIG, (LPARAM)GlobalIconHandle);
}

bool ApplicationHost::InternalCreateWindow()
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(windowSize.x, windowSize.y, ComfyStudioWindowTitle, nullptr, nullptr);

	if (window == nullptr)
		return false;

	glfwSetWindowSizeLimits(window, WindowSizeRestraints.x, WindowSizeRestraints.y, GLFW_DONT_CARE, GLFW_DONT_CARE);
	glfwGetWindowPos(window, &windowPosition.x, &windowPosition.y);

	ApplicationHost::LoadComfyWindowIcon();
	ApplicationHost::SetComfyWindowIcon(window);

	return true;
}

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
		if (Keyboard::TryInitializeInstance(window))
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
	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);

	lastMousePosition = mousePosition;
	mousePosition = ivec2(static_cast<int>(mouseX), static_cast<int>(mouseY));

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

ApplicationHost* ApplicationHost::DecodeWindowUserDataPointer(GLFWwindow* window)
{
	return static_cast<ApplicationHost*>(glfwGetWindowUserPointer(window));
}
