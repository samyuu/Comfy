#pragma once

IMGUI_IMPL_API bool ImGui_ImplWin32_Init(void* hwnd);
IMGUI_IMPL_API void ImGui_ImplWin32_Shutdown();
IMGUI_IMPL_API void ImGui_ImplWin32_NewFrame();

// DPI-related helpers (which run and compile without requiring 8.1 or 10, neither Windows version, neither associated SDK)
IMGUI_IMPL_API void  ImGui_ImplWin32_EnableDpiAwareness();
IMGUI_IMPL_API float ImGui_ImplWin32_GetDpiScaleForHwnd(void* hwnd);       // HWND hwnd
IMGUI_IMPL_API float ImGui_ImplWin32_GetDpiScaleForMonitor(void* monitor); // HMONITOR monitor
