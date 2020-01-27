#pragma once
#include "AetTool.h"

namespace Editor
{
	enum ScaleNode
	{
		ScaleNode_Invalid = -1,
		ScaleNode_AxisX,
		ScaleNode_AxisY,
		ScaleNode_AxisXY,
		ScaleNode_Count
	};

	class ScaleTool : public AetTool
	{
	public:
		const char* GetIcon() const override;
		const char* GetName() const override;
		AetToolType GetType() const override;
		KeyCode GetShortcutKey() const override;

		void UpdatePostDrawGui(Graphics::Transform2D* transform, vec2 dimensions) override;
		void DrawContextMenu() override;

	private:
		const vec4 xAxisColorInner = vec4(0.45f, 0.97f, 0.12f, 1.00f);
		const vec4 xAxisColorLine = vec4(0.58f, 0.97f, 0.18f, 0.85f);

		const vec4 yAxisColorInner = vec4(0.98f, 0.09f, 0.04f, 1.00f);
		const vec4 yAxisColorLine = vec4(0.94f, 0.15f, 0.05f, 0.85f);

		const vec4 centerColor = vec4(0.73f, 0.84f, 0.83f, 1.00f);

		const int actionMouseButton = 0;

		static constexpr float NodeRadius = 4.0f, NodeCenterRadius = 6.0f;
		static constexpr float AxisLength = 80.0f;

		vec2 worldSpaceNodes[ScaleNode_Count];
		vec2 screenSpaceNodes[ScaleNode_Count];
		ScaleNode scalingNode = ScaleNode_Invalid;
		ScaleNode hoveringNode = ScaleNode_Invalid;

		bool allowAction = false;

		vec2 scaleNodeWorldPositionOnMouseDown = vec2(0.0f, 0.0f);
		vec2 mouseWorldPositionOnMouseDown = vec2(0.0f, 0.0f);
		Graphics::Transform2D transformOnMouseDown = {};
	
	private:
		vec2 GetAxisPoint(const Graphics::Transform2D& transform, ScaleNode node);
	};
}