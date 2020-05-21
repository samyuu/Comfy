#include "RotateTool.h"
#include "Editor/Aet/AetIcons.h"

namespace Comfy::Studio::Editor
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

	Input::KeyCode RotateTool::GetShortcutKey() const
	{
		return Input::KeyCode_E;
	}

	void RotateTool::DrawContextMenu()
	{
		// TODO: Rotation presets (0Åã; 90Åã; 180Åã; 270Åã;")
	}
}
