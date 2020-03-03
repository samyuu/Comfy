#include "MoveTool.h"
#include "Editor/Aet/AetIcons.h"

namespace Comfy::Editor
{
	const char* MoveTool::GetIcon() const
	{
		return ICON_FA_ARROWS_ALT;
	}

	const char* MoveTool::GetName() const
	{
		return "Move Tool";
	}

	AetToolType MoveTool::GetType() const
	{
		return AetToolType_Move;
	}

	KeyCode MoveTool::GetShortcutKey() const
	{
		return KeyCode_W;
	}

	void MoveTool::DrawContextMenu()
	{
		// TODO: Center Origin (?); advanced context menu with origin / position corner/center selection buttons
	}
}
