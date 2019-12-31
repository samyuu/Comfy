#pragma once
#include "Types.h"
#include "Core/CoreTypes.h"
#include "Core/TimeSpan.h"
#include "Core/Win32/ComfyWindows.h"
#include <functional>
#include <optional>

// NOTE: Manage window and general IO platform agnostically
class ApplicationHost
{
public:
	static constexpr ivec2 StartupWindowPosition = ivec2(0, 0);
	static constexpr ivec2 StartupWindowSize = ivec2(1280, 720);
	static constexpr ivec2 WindowSizeRestraints = ivec2(640, 360);

	static constexpr const char* ComfyWindowClassName = "ComfyWindowClass";
	static constexpr const char* ComfyStudioWindowTitle = "Comfy Studio";

public:
	ApplicationHost();
	~ApplicationHost();

	bool Initialize();
	void EnterProgramLoop(const std::function<void()> updateFunction);
	void Exit();
	void Dispose();

public:
	bool GetIsFullscreen() const;
	void SetIsFullscreen(bool value);
	void ToggleFullscreen();

	void SetSwapInterval(int interval);

	bool HasFocusBeenGained() const;
	bool HasFocusBeenLost() const;

	ivec2 GetWindowPosition() const;
	void SetWindowPosition(ivec2 value);

	ivec2 GetWindowSize() const;
	void SetWindowSize(ivec2 value);

	inline HWND GetWindow() const { return windowHandle; };

public:
	void RegisterWindowProcCallback(const std::function<bool(HWND, UINT, WPARAM, LPARAM)> onWindowProc);
	void RegisterWindowResizeCallback(const std::function<void(ivec2 size)> onWindowResize);
	void RegisterWindowClosingCallback(const std::function<void()> onClosing);

public:
	bool GetDispatchFileDrop();
	void SetFileDropDispatched(bool value = true);
	const std::vector<std::string>& GetDroppedFiles() const;

	static void LoadComfyWindowIcon();
	static HICON GetComfyWindowIcon();

private:
	// NOTE: Initialization
	bool InternalCreateWindow();

	// NOTE: Helpers
	void InternalSnycMoveWindow();

	// NOTE: Callbacks
	void InternalMouseMoveCallback(ivec2 position);
	void InternalMouseScrollCallback(float offset);
	void InternalWindowMoveCallback(ivec2 position);
	void InternalWindowResizeCallback(ivec2 size);
	void InternalWindowDropCallback(size_t count, const char* paths[]);
	void InternalWindowPaintCallback();
	void InternalWindowFocusCallback(bool focused);
	void InternalWindowClosingCallback();
	void InternalCheckConnectedDevices();

	void InternalPreUpdateTick();
	void InternalPostUpdateTick();
	void InternalPreUpdatePollInput();

	void InternalDisposeWindow();

private:
	LRESULT InternalProcessWindowMessage(const UINT message, const WPARAM wParam, const LPARAM lParam);

private:
	// NOTE: Window management
	HWND windowHandle = nullptr;
	bool isRunning = false;
	bool isFullscreen = false;

	ivec2 windowPosition = StartupWindowPosition;
	ivec2 windowSize = StartupWindowSize;

	ivec2 preFullScreenWindowPosition = StartupWindowPosition;
	ivec2 preFullScreenWindowSize = StartupWindowSize;
	const bool mainLoopLowPowerSleep = false;

	bool windowFocused = true, lastWindowFocused = false;
	bool focusLostThisFrame = false, focusGainedThisFrame = false;

	// NOTE: Callbacks
	std::optional<std::function<bool(HWND, UINT, WPARAM, LPARAM)>> windowProcCallback = {};
	std::optional<std::function<void(ivec2 size)>> windowResizeCallback = {};
	std::optional<std::function<void()>> windowClosingCallback = {};

	// NOTE: Program timing
	int swapInterval = 1;
	TimeSpan elapsedTime = 0.0f;
	TimeSpan currentTime, lastTime;
	uint64_t elapsedFrames = 0;

	const TimeSpan powerSleepDuration = TimeSpan::FromMilliseconds(10.0);

	// NOTE: File drop dispatching
	std::vector<std::string> droppedFiles;

	bool filesDroppedThisFrame = false, filesDropped = false;
	bool filesLastDropped = false, fileDropDispatched = false;

	// NOTE: Input management
	ivec2 mousePosition = ivec2(0, 0), lastMousePosition = ivec2(0, 0);
	ivec2 mouseDelta = ivec2(0, 0);

	float mouseWheel = 0.0f, lastMouseWheel = 0.0f;
	bool mouseScrolledUp = false, mouseScrolledDown = false;

private:
	static LRESULT ProcessWindowMessage(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);
};
