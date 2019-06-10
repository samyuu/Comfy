#include "imgui_extensions.h"
#include "../Graphics/Texture.h"

namespace ImGui
{
	void AddTexture(ImDrawList* drawList, Texture* texture, ImVec2 center, ImVec2 scale, const ImVec2& uv0, const ImVec2& uv1)
	{
		float width = texture->GetWidth() * scale.x;
		float height = texture->GetHeight() * scale.y;

		center.x -= width * .5f;
		center.y -= height * .5f;
		ImVec2 bottomRight(center.x + width, center.y + height);

		drawList->AddImage(texture->GetVoidTexture(), center, bottomRight, uv0, uv1);
	}

	void ImGui::StyleComfy(ImGuiStyle* dst)
	{
		ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
		style->WindowRounding = 0.0f;
		style->FrameBorderSize = 0.0f;

		ImVec4* colors = style->Colors;
		colors[ImGuiCol_Text] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
		colors[ImGuiCol_Border] = ImVec4(0.24f, 0.24f, 0.24f, 0.21f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.36f, 0.36f, 0.36f, 0.21f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.26f, 0.26f, 1.00f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.15f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.44f, 0.44f, 0.44f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.68f, 0.68f, 0.68f, 0.57f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.70f, 0.70f, 0.70f, 0.57f);
		colors[ImGuiCol_Button] = ImVec4(0.36f, 0.36f, 0.36f, 1.00f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		colors[ImGuiCol_Separator] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.37f, 0.37f, 0.37f, 1.00f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
		colors[ImGuiCol_Tab] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
		colors[ImGuiCol_TabActive] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
		colors[ImGuiCol_DockingPreview] = ImVec4(0.46f, 0.46f, 0.46f, 1.00f);
		colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.66f, 0.66f, 0.66f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.27f, 0.37f, 0.13f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.34f, 0.47f, 0.17f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.41f, 0.56f, 0.20f, 0.99f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.27f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(0.59f, 0.59f, 0.59f, 0.98f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.83f, 0.83f, 0.83f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.99f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.99f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.50f);
	}

}