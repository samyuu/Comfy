#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Time/TimeSpan.h"
#include "Core/Win32/ComfyWindows.h"
#include <functional>
#include <optional>

namespace Comfy
{
	// NOTE: Manage window and general IO platform agnostically
	class ApplicationHost
	{
	public:
		static constexpr ivec2 StartupWindowPosition = ivec2(0, 0);
		static constexpr ivec2 StartupWindowSize = ivec2(1280, 720);
		static constexpr ivec2 WindowSizeRestraints = ivec2(640, 360);

		static constexpr const char* ComfyWindowClassName = "ComfyWindowClass";
		static constexpr const char* ComfyStudioWindowTitle = "Comfy Studio";
		static constexpr const char* MainDockSpaceID = "MainDockSpace";

	public:
		ApplicationHost();
		~ApplicationHost() = default;

		// NOTE: Explicit Initialize() / Dispose() methods beaucse they could potentially fail
		bool Initialize();
		void EnterProgramLoop(const std::function<void()> updateFunction);
		void Exit();
		void Dispose();

	public:
		bool GetIsFullscreen() const;
		void SetIsFullscreen(bool value);
		void ToggleFullscreen();

		bool GetIsMaximized() const;
		void SetIsMaximized(bool value);

		void SetSwapInterval(int interval);

		bool HasFocusBeenGained() const;
		bool HasFocusBeenLost() const;

		bool IsWindowFocused() const;

		bool GetMainLoopPowerSleep() const;
		void SetMainLoopPowerSleep(bool value);

		ivec2 GetWindowPosition() const;
		void SetWindowPosition(ivec2 value);

		ivec2 GetWindowSize() const;
		void SetWindowSize(ivec2 value);

		ivec4 GetWindowRestoreRegion();
		void SetWindowRestoreRegion(ivec4 value);

		void SetDefaultPositionWindow(bool value);
		void SetDefaultResizeWindow(bool value);

		void* GetWindowHandle() const;

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
		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}
