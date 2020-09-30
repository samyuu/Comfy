#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Editor/Chart/Chart.h"
#include "ImGui/Gui.h"

namespace Comfy::Studio::Editor
{
	class TargetRenderWindow;

	// NOTE: Not part of the TargetTool classes as it doesn't perform any property modifications and requires a different more controlled interface
	class TargetBoxSelectionTool : NonCopyable
	{
	public:
		static constexpr i32 SelectionButton = 1;

	public:
		TargetBoxSelectionTool(TargetRenderWindow& renderWindow);
		~TargetBoxSelectionTool() = default;

	public:
		void UpdateInput(Chart& chart, const TimelineTick cursorTick, const TimelineTick postHitLingerDuration);
		void DrawSelection(ImDrawList& drawList);

	private:
		TargetRenderWindow& renderWindow;

		enum class ActionType : u8 { Clean, Add, Remove };
		struct SelectionData
		{
			vec2 StartMouse, EndMouse;
			vec2 StartTargetSpace, EndTargetSpace;

			ActionType Action;
			bool IsActive;
			bool IsSufficientlyLarge;
		} data = {};
	};
}
