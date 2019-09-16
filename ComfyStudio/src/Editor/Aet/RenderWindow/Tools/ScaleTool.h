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
		virtual const char* GetIcon() const override;
		virtual const char* GetName() const override;
		virtual AetToolType GetType() const override;
		virtual KeyCode GetShortcutKey() const override;

		virtual void UpdatePostDrawGui(Graphics::Auth2D::Properties* properties, vec2 dimensions) override;
		virtual void DrawContextMenu() override;

	private:
		static constexpr vec4 xAxisColorInner = vec4(0.45f, 0.97f, 0.12f, 1.00f);
		static constexpr vec4 xAxisColorLine = vec4(0.58f, 0.97f, 0.18f, 0.85f);

		static constexpr vec4 yAxisColorInner = vec4(0.98f, 0.09f, 0.04f, 1.00f);
		static constexpr vec4 yAxisColorLine = vec4(0.94f, 0.15f, 0.05f, 0.85f);

		static constexpr vec4 centerColor = vec4(0.73f, 0.84f, 0.83f, 1.00f);

		const int actionMouseButton = 0;

		static constexpr float NodeRadius = 4.0f;
		static constexpr float NodeCenterRadius = 6.0f;
		static constexpr float AxisLength = 80.0f;

		vec2 worldSpaceNodes[ScaleNode_Count];
		vec2 screenSpaceNodes[ScaleNode_Count];
		ScaleNode scalingNode = ScaleNode_Invalid;
		ScaleNode hoveringNode = ScaleNode_Invalid;

		bool allowAction = false;

		vec2 scaleNodeWorldPositionOnMouseDown;
		vec2 mouseWorldPositionOnMouseDown;
		Graphics::Auth2D::Properties propertiesOnMouseDown;
	
	private:
		vec2 GetAxisPoint(const Graphics::Auth2D::Properties& properties, ScaleNode node);
	};
}