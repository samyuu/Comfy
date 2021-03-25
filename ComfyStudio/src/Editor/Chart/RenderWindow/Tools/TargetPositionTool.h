#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "TargetTool.h"
#include "CardinalDirection.h"

namespace Comfy::Studio::Editor
{
	class TargetPositionTool : public TargetTool
	{
	public:
		using TargetTool::TargetTool;

	public:
		void OnSelected() override;
		void OnDeselected() override;

		void PreRender(Chart& chart, Render::Renderer2D& renderer) override;
		void PostRender(Chart& chart, Render::Renderer2D& renderer) override;

		void OnContextMenuGUI(Chart& chart) override;
		void OnOverlayGUI(Chart& chart) override;

		void PreRenderGUI(Chart& chart, ImDrawList& drawList) override;
		void PostRenderGUI(Chart& chart, ImDrawList& drawList) override;

		void UpdateInput(Chart& chart) override;

		const char* GetName() const override;

	private:
		enum class FlipMode : u8 { Horizontal, Vertical, HorizontalLocal, VerticalLocal, Count };

		void DrawTickDistanceGuides(Chart& chart, ImDrawList& drawList);
		void DrawFlushThisFrameGuides(ImDrawList& drawList);
		void DrawRowDirectionGuide(Chart& chart, ImDrawList& drawList);

		void UpdateKeyboardKeyBindingsInput(Chart& chart);
		void UpdateKeyboardStepInput(Chart& chart);
		void UpdateMouseGrabInput(Chart& chart);
		void UpdateMouseRowInput(Chart& chart);

		void SetupAxisGrabGuides(const TimelineTarget& grabbedTarget);
		vec2 TrySnapGrabPositionToGuides(vec2 grabMovedPosition) const;
		void GuiTargetGrabTooltip(const Chart& chart) const;

		vec2 GetSelectedRowPerBeatDiagonalSpacing() const;

		void IncrementSelectedTargetPositionsBy(Undo::UndoManager& undoManager, Chart& chart, vec2 positionIncrement);
		void PositionSelectedTargetsInCardinalRow(Undo::UndoManager& undoManager, Chart& chart, CardinalDirection cardinal, vec2 perBeatDiagonalDirection, bool backwards);

		void FlipSelectedTargets(Undo::UndoManager& undoManager, Chart& chart, FlipMode flipMode);
		void SnapSelectedTargetPositions(Undo::UndoManager& undoManager, Chart& chart, f32 snapDistance);
		void StackSelectedTargetPositions(Undo::UndoManager& undoManager, Chart& chart);

		void PositionSelectedTargetsInRowBetweenFirstAndLastTarget(Undo::UndoManager& undoManager, Chart& chart, bool backwards);
		void InterpolateSelectedTargetPositionsLinear(Undo::UndoManager& undoManager, Chart& chart);
		void InterpolateSelectedTargetPositionsCircular(Undo::UndoManager& undoManager, Chart& chart, f32 direction);

	private:
		std::vector<TimelineTarget*> selectedTargetsBuffer;
		size_t lastFrameSelectionCount = 0;

		struct GrabData
		{
			TimelineTargetID GrabbedTargetID = {}, HoveredTargetID = {};

			vec2 MouseOnGrab;
			vec2 TargetPositionOnGrab;

			vec2 ThisPos, LastPos;
			bool ThisGridSnap, LastGridSnap;
		} grab = {};

		i32 selectedDiagonalRowLayoutIndex = 0;
		struct RowData
		{
			vec2 Start, End;
			vec2 Direction;
			f32 Angle;
			bool Active;
			bool Backwards;
			bool SteepThisFrame, SteepLastFrame;
		} row = {};

		struct GuideData
		{
			vec2 Start, End;
			u32 Color;
		};

		std::vector<GuideData> thisFrameGuides;
	};
}
