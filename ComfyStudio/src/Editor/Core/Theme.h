#pragma once
#include "Types.h"
#include "CoreTypes.h"

namespace Comfy::Studio::Editor
{
	enum EditorColor
	{
		EditorColor_BaseClear,
		EditorColor_DarkClear,
		EditorColor_AltRow,
		EditorColor_TreeViewSelected,
		EditorColor_TreeViewHovered,
		EditorColor_TreeViewActive,
		EditorColor_TreeViewTextHighlight,
		EditorColor_Bar,
		EditorColor_Grid,
		EditorColor_GridAlt,
		EditorColor_Selection,
		EditorColor_TimelineRowSeparator,
		EditorColor_TimelineSelection,
		EditorColor_TimelineSelectionBorder,
		EditorColor_Cursor,
		EditorColor_CursorInner,
		EditorColor_AnimatedProperty,
		EditorColor_KeyFrameProperty,
		EditorColor_KeyFrame,
		EditorColor_KeyFrameConnection,
		EditorColor_KeyFrameConnectionAlt,
		EditorColor_KeyFrameSelected,
		EditorColor_KeyFrameBorder,
		EditorColor_Count,
	};

	extern std::array<u32, EditorColor_Count> EditorColors;

	vec4 GetColorVec4(EditorColor color);
	u32 GetColor(EditorColor color);
	u32 GetColor(EditorColor color, float alpha);
	void SetColor(EditorColor color, u32 value);

	void UpdateEditorColors();
}
