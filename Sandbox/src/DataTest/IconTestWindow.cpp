#include "IconTestWindow.h"
#include "Application.h"
#include "ImGui/imgui_extensions.h"
#include <glfw/glfw3.h>

IconTestWindow::IconTestWindow(Application* parent) : BaseWindow(parent)
{
	*GetIsGuiOpenPtr() = false;
}

IconTestWindow::~IconTestWindow()
{
}

void IconTestWindow::DrawGui()
{
	ImGui::Text("Icon Name Filter:");
	iconFilter.Draw();

	ImGui::BeginChild("IconTestColumnsChild##IconTestWindow", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysHorizontalScrollbar);
	ImGui::Columns(2, "IconTestColumns");
	{
		ImGui::Separator();
		ImGui::Text("Name"); ImGui::NextColumn();
		ImGui::Text("Icon"); ImGui::NextColumn();
		ImGui::Separator();
		for (size_t i = 0; i < IM_ARRAYSIZE(namedFontIcons); i++)
		{
			NamedFontIcon& fontIcon = namedFontIcons[i];

			if (!iconFilter.PassFilter(fontIcon.Name))
				continue;

			if (ImGui::Selectable(fontIcon.Name, false, ImGuiSelectableFlags_AllowDoubleClick))
				glfwSetClipboardString(GetParent()->GetWindow(), fontIcon.Name);
			ImGui::NextColumn();

			if (ImGui::Selectable(fontIcon.Value, false, ImGuiSelectableFlags_AllowDoubleClick))
				glfwSetClipboardString(GetParent()->GetWindow(), fontIcon.Value);
			ImGui::NextColumn();
		}
	}
	ImGui::Columns(1);
	ImGui::EndChild();
}

const char* IconTestWindow::GetGuiName() const
{
	return u8"Icon Test";
}

ImGuiWindowFlags IconTestWindow::GetWindowFlags() const
{
	return ImGuiWindowFlags_None;
}
