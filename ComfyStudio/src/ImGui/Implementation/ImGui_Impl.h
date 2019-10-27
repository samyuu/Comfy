#pragma once
#include "ImGui/Implementation/Imgui_Impl_Renderer.h"

namespace ImGui
{
	IMGUI_IMPL_API bool ImGui_ImplGlfw_InitForOpenGL(struct GLFWwindow* window, bool install_callbacks);
	IMGUI_IMPL_API void ImGui_ImplGlfw_Shutdown();
	IMGUI_IMPL_API void ImGui_ImplGlfw_NewFrame();

	// InitXXX function with 'install_callbacks=true': install GLFW callbacks. They will call user's previously installed callbacks, if any.
	// InitXXX function with 'install_callbacks=false': do not install GLFW callbacks. You will need to call them yourself from your own GLFW callbacks.
	IMGUI_IMPL_API void ImGui_ImplGlfw_MouseButtonCallback(struct GLFWwindow* window, int button, int action, int mods);
	IMGUI_IMPL_API void ImGui_ImplGlfw_ScrollCallback(struct GLFWwindow* window, double xoffset, double yoffset);
	IMGUI_IMPL_API void ImGui_ImplGlfw_KeyCallback(struct GLFWwindow* window, int key, int scancode, int action, int mods);
	IMGUI_IMPL_API void ImGui_ImplGlfw_CharCallback(struct GLFWwindow* window, unsigned int c);
}