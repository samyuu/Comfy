#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "TargetTool.h"

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
		void DrawRowDirectionGuide(Chart& chart, ImDrawList& drawList);

		void UpdateKeyboardKeyBindingsInput(Chart& chart);
		void UpdateKeyboardStepInput(Chart& chart);
		void UpdateMouseGrabInput(Chart& chart);
		void UpdateMouseRowInput(Chart& chart);

		void IncrementSelectedTargetPositionsBy(Undo::UndoManager& undoManager, Chart& chart, vec2 positionIncrement);
		void ArrangeSelectedTargetsInRow(Undo::UndoManager& undoManager, Chart& chart, vec2 rowDirection, bool useStairDistance, bool backwards);

		void FlipSelectedTargets(Undo::UndoManager& undoManager, Chart& chart, FlipMode flipMode);

		void PositionSelectedTargetInRowAutoDirection(Undo::UndoManager& undoManager, Chart& chart, bool backwards);
		void InterpolateSelectedTargetPositionsLinear(Undo::UndoManager& undoManager, Chart& chart);
		void InterpolateSelectedTargetPositionsCircular(Undo::UndoManager& undoManager, Chart& chart, f32 direction);

		i32 GetSelectedTargetIndex(const Chart& chart, const TimelineTarget* selectedTarget) const;

	private:
		std::vector<TimelineTarget*> selectedTargetsBuffer;
		size_t lastFrameSelectionCount = 0;

		bool drawDistanceGuides = true;

		struct GrabData
		{
			i32 GrabbedTargetIndex = -1, HoveredTargetIndex = -1;

			vec2 MouseOnGrab;
			vec2 TargetPositionOnGrab;

			vec2 ThisPos, LastPos;
			bool ThisGridSnap, LastGridSnap;
		} grab = {};

		struct RowData
		{
			vec2 Start, End;
			vec2 Direction;
			f32 Angle;
			bool Active;
			bool Backwards;
			bool SteepThisFrame, SteepLastFrame;
		} row = {};
	};
}
