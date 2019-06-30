#pragma once
#include "Types.h"
#include <array>

typedef unsigned int ImU32;

namespace Editor
{
	enum EditorColor
	{
		EditorColor_BaseClear,
		EditorColor_DarkClear,
		EditorColor_Bar,
		EditorColor_Grid,
		EditorColor_GridAlt,
		EditorColor_Selection,
		EditorColor_InfoColumn,
		EditorColor_TempoMapBg,
		EditorColor_TimelineBg,
		EditorColor_TimelineRowSeparator,
		EditorColor_Cursor,
		EditorColor_CursorInner,
		EditorColor_TextHighlight,
		EditorColor_KeyFrame,
		EditorColor_Max,
	};

	extern std::array<ImU32, EditorColor_Max> EditorColors;

	ImU32 GetColor(EditorColor color);
	vec4 GetColorVec4(EditorColor color);
	ImU32 GetColor(EditorColor color, float alpha);
	void SetColor(EditorColor color, ImU32 value);

	void UpdateEditorColors();
}