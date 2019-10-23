#include "IconTestWindow.h"
#include "Core/Application.h"
#include "ImGui/Gui.h"

namespace DataTest
{
	IconTestWindow::IconTestWindow(Application* parent) : BaseWindow(parent)
	{
		CloseWindow();
	}

	IconTestWindow::~IconTestWindow()
	{
	}

	void IconTestWindow::DrawGui()
	{
		Gui::Text("Icon Name Filter:");
		iconFilter.Draw();

		Gui::BeginChild("IconTestColumnsChild##IconTestWindow", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysHorizontalScrollbar);
		Gui::Columns(2, "IconTestColumns");
		{
			Gui::Separator();
			Gui::Text("Name"); Gui::NextColumn();
			Gui::Text("Icon"); Gui::NextColumn();
			Gui::Separator();
			for (size_t i = 0; i < IM_ARRAYSIZE(namedFontIcons); i++)
			{
				const NamedFontIcon& fontIcon = namedFontIcons[i];

				if (!iconFilter.PassFilter(fontIcon.Name))
					continue;

				if (Gui::Selectable(fontIcon.Name, false, ImGuiSelectableFlags_AllowDoubleClick))
					GetParent()->GetHost().SetClipboardString(fontIcon.Name);
				Gui::NextColumn();

				if (Gui::Selectable(fontIcon.Value, false, ImGuiSelectableFlags_AllowDoubleClick))
					GetParent()->GetHost().SetClipboardString(fontIcon.Value);
				Gui::NextColumn();
			}
		}
		Gui::Columns(1);
		Gui::EndChild();
	}

	const char* IconTestWindow::GetGuiName() const
	{
		return u8"Icon Test";
	}

	ImGuiWindowFlags IconTestWindow::GetWindowFlags() const
	{
		return ImGuiWindowFlags_None;
	}
}