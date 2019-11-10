#pragma once

IMGUI_IMPL_API bool ImGui_ImplDX11_Init();
IMGUI_IMPL_API void ImGui_ImplDX11_Shutdown();
IMGUI_IMPL_API void ImGui_ImplDX11_NewFrame();
IMGUI_IMPL_API void ImGui_ImplDX11_RenderDrawData(ImDrawData* draw_data);

// Use if you want to reset your rendering device without losing ImGui state.
IMGUI_IMPL_API void ImGui_ImplDX11_InvalidateDeviceObjects();
IMGUI_IMPL_API bool ImGui_ImplDX11_CreateDeviceObjects();
