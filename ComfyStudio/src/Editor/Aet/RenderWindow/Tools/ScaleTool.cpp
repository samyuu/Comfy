#include "ScaleTool.h"
#include "Editor/Aet/AetIcons.h"

namespace Editor
{
	const char* ScaleTool::GetIcon() const
	{
		return ICON_FA_COMPRESS_ARROWS_ALT;
	}

	const char* ScaleTool::GetName() const
	{
		return "Scale Tool";
	}

	AetToolType ScaleTool::GetType() const
	{
		return AetToolType_Scale;
	}

	KeyCode ScaleTool::GetShortcutKey() const
	{
		return KeyCode_R;
	}

	void ScaleTool::DrawContextMenu()
	{
		// TODO: Scale to fixed values
	}
}