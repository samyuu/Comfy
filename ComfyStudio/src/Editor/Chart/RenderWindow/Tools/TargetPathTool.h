#pragma once
#include "Types.h"
#include "TargetTool.h"

namespace Comfy::Studio::Editor
{
	class TargetPathTool : public TargetTool
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
		void DrawTargetAngleGuides(Chart& chart, ImDrawList& drawList);
		void DrawAngleDragGuide(Chart& chart, ImDrawList& drawList);

		void UpdateKeyboardKeyBindingsInput(Chart& chart);
		void UpdateMouseAngleScrollInput(Chart& chart);
		void UpdateMouseAngleDragInput(Chart& chart);

		void IncrementSelectedTargetAnglesBy(Undo::UndoManager& undoManager, Chart& chart, f32 increment);
		void SetSelectedTargetAnglesTo(Undo::UndoManager& undoManager, Chart& chart, f32 newAngle);

		void InvertSelectedTargetFrequencies(Undo::UndoManager& undoManager, Chart& chart);
		void InterpolateSelectedTargetAngles(Undo::UndoManager& undoManager, Chart& chart, bool clockwise);
		void InterpolateSelectedTargetDistances(Undo::UndoManager& undoManager, Chart& chart);

		void ApplySelectedTargetAngleIncrements(Undo::UndoManager& undoManager, Chart& chart, f32 direction, bool backwards);

	private:
		struct AngleScrollData
		{
			Stopwatch LastScroll;
			bool Active;
			i32 TargetIndex = -1;
		} angleScroll = {};

		struct AngleDragData
		{
			vec2 StartTargetPosition;
			vec2 StartMouse, EndMouse;
			vec2 TargetMouseDirection;
			f32 DegreesTargetAngle;

			bool Active;
			bool RoughStep, PreciseStep;

			bool UseLastTarget;
			bool MovedFarEnoughFromStart;

			i32 TargetIndex = -1;
		} angleDrag = {};

		struct AngleIncrementData
		{
			f32 IncrementPerBeat = 2.0f;
			f32 IncrementPerBeatDiagonal = 10.0f;
			f32 FixedStepIncrementPerTarget = 2.0f;
			bool UseFixedStepIncrement = false;
			bool ApplyToChainSlides = false;

			bool operator==(const AngleIncrementData& other) const;
			bool operator!=(const AngleIncrementData& other) const;
		} angleIncrement = {};
	};
}
