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
		static constexpr ivec2 DefaultStartupWindowPosition = ivec2(0, 0);
		static constexpr ivec2 DefaultStartupWindowSize = ivec2(1280, 720);
		static constexpr ivec2 WindowSizeRestraints = ivec2(640, 360);

		static constexpr std::string_view UnnamedWindowName = "Application Host Window";
		static constexpr std::string_view ComfyWindowClassName = "ComfyWindowClass";

	public:
		struct ConstructionParam
		{
			std::optional<void*> IconHandle;

			struct WindowStateData
			{
				std::optional<std::string_view> Title;
				std::optional<ivec4> RestoreRegion;
				std::optional<ivec2> Position;
				std::optional<ivec2> Size;
				std::optional<bool> IsFullscreen;
				std::optional<bool> IsMaximized;
			} StartupWindowState;
		};

		ApplicationHost(const ConstructionParam& param);
		~ApplicationHost();

	public:
		void EnterProgramLoop(const std::function<void()> updateFunction);

		// NOTE: Should be called after the MainMenuBar but before any other gui windows
		void GuiMainDockspace(bool hasMenuBar);

		void Exit();

	public:
		std::string_view GetWindowTitle() const;
		void SetWindowTitle(std::string_view value);

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

		static HICON GetComfyWindowIcon();

	private:
		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}
