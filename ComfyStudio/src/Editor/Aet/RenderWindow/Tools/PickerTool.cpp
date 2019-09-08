#include "PickerTool.h"
#include "Editor/Aet/AetIcons.h"

namespace Editor
{
	const char* PickerTool::GetIcon() const
	{
		return ICON_FA_MOUSE_POINTER;
	}

	const char* PickerTool::GetName() const
	{
		return "Picker Tool";
	}

	AetToolType PickerTool::GetType() const
	{
		return AetToolType_Picker;
	}

	KeyCode PickerTool::GetShortcutKey() const
	{
		return KeyCode_V;
	}

	void PickerTool::DrawContextMenu()
	{
		// Center Origin (?); advanced context menu with origin / position corner/center selection buttons
		Gui::MenuItem("TODO: List AetObjs of the current layer here");
	}
}