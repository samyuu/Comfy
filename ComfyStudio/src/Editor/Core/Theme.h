#pragma once
#include "Types.h"
#include "CoreTypes.h"

namespace Comfy::Editor
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

	extern std::array<uint32_t, EditorColor_Count> EditorColors;

	vec4 GetColorVec4(EditorColor color);
	uint32_t GetColor(EditorColor color);
	uint32_t GetColor(EditorColor color, float alpha);
	void SetColor(EditorColor color, uint32_t value);

	void UpdateEditorColors();
}