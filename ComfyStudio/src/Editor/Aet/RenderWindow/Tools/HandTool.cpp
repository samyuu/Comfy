#include "HandTool.h"
#include "Editor/Aet/AetIcons.h"

namespace Editor
{
	const char* HandTool::GetIcon() const
	{
		return ICON_FA_HAND_PAPER;
	}
	
	const char* HandTool::GetName() const
	{
		return "Hand Tool";
	}
	
	AetToolType HandTool::GetType() const
	{
		return AetToolType_Hand;
	}
	
	KeyCode HandTool::GetShortcutKey() const
	{
		return KeyCode_H;
	}

	void HandTool::DrawContextMenu()
	{
		Gui::MenuItem(__FUNCTION__"(): Test");
	}
}