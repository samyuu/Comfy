#include "IconTestWindow.h"
#include "Core/ComfyStudioApplication.h"
#include "ImGui/Gui.h"

namespace Comfy::Studio::DataTest
{
	IconTestWindow::IconTestWindow(ComfyStudioApplication& parent) : BaseWindow(parent)
	{
		Close();
	}

	const char* IconTestWindow::GetName() const
	{
		return "Icon Test";
	}

	ImGuiWindowFlags IconTestWindow::GetFlags() const
	{
		return ImGuiWindowFlags_None;
	}

	void IconTestWindow::Gui()
	{
		Gui::Text("Icon Name Filter:");
		iconFilter.Draw();

		Gui::BeginChild("IconTestColumnsChild##IconTestWindow", vec2(0.0f, 0.0f), false, ImGuiWindowFlags_AlwaysHorizontalScrollbar);
		Gui::Columns(2, "IconTestColumns");
		{
			Gui::Separator();
			Gui::Text("Name"); Gui::NextColumn();
			Gui::Text("Icon"); Gui::NextColumn();
			Gui::Separator();
			for (size_t i = 0; i < std::size(namedFontIcons); i++)
			{
				const NamedFontIcon& fontIcon = namedFontIcons[i];

				if (!iconFilter.PassFilter(fontIcon.Name))
					continue;

				if (Gui::Selectable(fontIcon.Name, false, ImGuiSelectableFlags_AllowDoubleClick))
					Gui::SetClipboardText(fontIcon.Name);
				Gui::NextColumn();

				if (Gui::Selectable(fontIcon.Value, false, ImGuiSelectableFlags_AllowDoubleClick))
					Gui::SetClipboardText(fontIcon.Value);
				Gui::NextColumn();
			}
		}
		Gui::Columns(1);
		Gui::EndChild();
	}
}
