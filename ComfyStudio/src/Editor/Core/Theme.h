#pragma once
#include "Types.h"
#include "Core/CoreTypes.h"

typedef unsigned int ImU32;

namespace Editor
{
	enum EditorColor
	{
		EditorColor_BaseClear,
		EditorColor_DarkClear,
		EditorColor_AltRow,
		EditorColor_Bar,
		EditorColor_Grid,
		EditorColor_GridAlt,
		EditorColor_Selection,
		EditorColor_InfoColumn,
		EditorColor_TempoMapBg,
		EditorColor_TimelineBg,
		EditorColor_TimelineRowSeparator,
		EditorColor_TimelineSelection,
		EditorColor_TimelineSelectionBorder,
		EditorColor_Cursor,
		EditorColor_CursorInner,
		EditorColor_TextHighlight,
		EditorColor_AnimatedProperty,
		EditorColor_KeyFrameProperty,
		EditorColor_KeyFrame,
		EditorColor_KeyFrameConnection,
		EditorColor_KeyFrameConnectionAlt,
		EditorColor_KeyFrameSelected,
		EditorColor_KeyFrameBorder,
		EditorColor_Count,
	};

	extern Array<ImU32, EditorColor_Count> EditorColors;

	vec4 GetColorVec4(EditorColor color);
	ImU32 GetColor(EditorColor color);
	ImU32 GetColor(EditorColor color, float alpha);
	void SetColor(EditorColor color, ImU32 value);

	void UpdateEditorColors();
}