#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "TargetTool.h"

namespace Comfy::Studio::Editor
{
	class TargetRotationTool : public TargetTool
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
		void DrawTargetButtonAngleLine(ImDrawList& drawList, const TargetProperties& properties, u32 color, f32 thickness) const;
		void DrawTargetButtonAngleArrowLine(ImDrawList& drawList, const TargetProperties& properties, u32 color, f32 thickness) const;
		void DrawTargetButtonPathCurve(ImDrawList& drawList, const TargetProperties& properties, u32 color, f32 thickness) const;

		void DrawAngleDragGuide(Chart& chart, ImDrawList& drawList);

		void UpdateMouseAngleDragInput(Chart& chart);

		void IncrementSelectedTargetAnglesBy(Undo::UndoManager& undoManager, Chart& chart, f32 increment);
		void SetSelectedTargetAnglesTo(Undo::UndoManager& undoManager, Chart& chart, f32 newAngle);

		void InvertSelectedTargetFrequencies(Undo::UndoManager& undoManager, Chart& chart);
		void InterpolateSelectedTargetAngles(Undo::UndoManager& undoManager, Chart& chart, bool clockwise);

	private:
		struct AngleDragData
		{
			vec2 StartTarget;
			vec2 StartMouse, EndMouse;
			vec2 TargetMouseDirection;
			f32 DegreesTargetAngle;

			bool Active;
			bool RoughStep, PreciseStep;

			bool UseLastTarget;
			bool MovedFarEnoughFromStart;

			i32 TargetIndex = -1;
		} angleDrag = {};
	};
}
