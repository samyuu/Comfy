#include "RotationTool.h"
#include "Editor/Aet/AetIcons.h"

namespace Editor
{
	const char* RotationTool::GetIcon() const
	{
		return ICON_FA_SYNC;
	}

	const char* RotationTool::GetName() const
	{
		return "Rotation Tool";
	}

	AetToolType RotationTool::GetType() const
	{
		return AetToolType_Rotate;
	}

	KeyCode RotationTool::GetShortcutKey() const
	{
		return KeyCode_R;
	}

	void RotationTool::DrawContextMenu()
	{
		Gui::MenuItem("TODO: Rotation presets (0Åã; 90Åã; 180Åã; 270Åã;");
	}
}