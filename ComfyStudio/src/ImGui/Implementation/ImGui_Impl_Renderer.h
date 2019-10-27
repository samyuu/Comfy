#pragma once

namespace ImGui
{
	IMGUI_IMPL_API bool ImGui_ImplOpenGL3_Init();
	IMGUI_IMPL_API void ImGui_ImplOpenGL3_Shutdown();
	IMGUI_IMPL_API void ImGui_ImplOpenGL3_NewFrame();
	IMGUI_IMPL_API void ImGui_ImplOpenGL3_RenderDrawData(ImDrawData* draw_data);

	// Called by Init/NewFrame/Shutdown
	IMGUI_IMPL_API bool ImGui_ImplOpenGL3_CreateFontsTexture();
	IMGUI_IMPL_API void ImGui_ImplOpenGL3_DestroyFontsTexture();
	IMGUI_IMPL_API bool ImGui_ImplOpenGL3_CreateDeviceObjects();
	IMGUI_IMPL_API void ImGui_ImplOpenGL3_DestroyDeviceObjects();
}
