#pragma once

namespace ImGui
{
	// TODO:
	namespace ComfyWin32
	{
		bool Initialize(void* hwnd);
		void Shutdown();
		void NewFrame();

		// NOTE: DPI-related helpers (which run and compile without requiring 8.1 or 10, neither Windows version, neither associated SDK)
		void EnableDpiAwareness();
		float GetDpiScaleForHwnd(void* windowHandle);
		float GetDpiScaleForMonitor(void* monitorHandle);
	}

	bool ImGui_ImplWin32_Init(void* hwnd);
	void ImGui_ImplWin32_Shutdown();
	void ImGui_ImplWin32_NewFrame();

	// DPI-related helpers (which run and compile without requiring 8.1 or 10, neither Windows version, neither associated SDK)
	void  ImGui_ImplWin32_EnableDpiAwareness();
	float ImGui_ImplWin32_GetDpiScaleForHwnd(void* hwnd);       // HWND hwnd
	float ImGui_ImplWin32_GetDpiScaleForMonitor(void* monitor); // HMONITOR monitor
}
