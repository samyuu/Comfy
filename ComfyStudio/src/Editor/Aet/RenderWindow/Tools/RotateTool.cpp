#include "RotateTool.h"
#include "Editor/Aet/AetIcons.h"

namespace Editor
{
	const char* RotateTool::GetIcon() const
	{
		return ICON_FA_SYNC;
	}

	const char* RotateTool::GetName() const
	{
		return "Rotate Tool";
	}

	AetToolType RotateTool::GetType() const
	{
		return AetToolType_Rotate;
	}

	KeyCode RotateTool::GetShortcutKey() const
	{
		return KeyCode_E;
	}

	void RotateTool::DrawContextMenu()
	{
		// TODO: Rotation presets (0Åã; 90Åã; 180Åã; 270Åã;")
	}
}