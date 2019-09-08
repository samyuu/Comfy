#pragma once
#include "AetTool.h"
#include "TransformBox.h"

namespace Editor
{
	class TransformTool : public AetTool
	{
	public:
		virtual const char* GetIcon() const override;
		virtual const char* GetName() const override;
		virtual AetToolType GetType() const override;
		virtual KeyCode GetShortcutKey() const override;

		virtual void UpdatePostDrawGui(Graphics::Auth2D::Properties* properties, vec2 dimensions) override;
		virtual void DrawContextMenu() override;

	private:
		enum GrabMode
		{
			None, Move, Scale, Rotate
		};

		vec2 scaleNodeWorldPositionOnMouseDown;
		vec2 mouseWorldPositionOnMouseDown;
		TransformBox screenSpaceBox, worldSpaceBox;
		Graphics::Auth2D::Properties propertiesOnMouseDown;

		GrabMode mode = GrabMode::None;
		int scalingNode, hoveringNode;

		void MoveBoxCorner(TransformBox& box, vec2 position, float rotation);

	private:
		void DragPositionTooltip(const vec2& position);
		void DragScaleTooltip(const vec2& scale, const vec2& dimensions);
	
	private:
		TransformBox BoxWorldToScreenSpace(const TransformBox& box) const;
	};
}