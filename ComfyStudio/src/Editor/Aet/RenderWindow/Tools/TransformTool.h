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

		const int actionMouseButton = 0;

		vec2 scaleNodeWorldPositionOnMouseDown;
		vec2 mouseWorldPositionOnMouseDown;
		TransformBox screenSpaceBox, worldSpaceBox;
		Graphics::Auth2D::Properties propertiesOnMouseDown;

		GrabMode mode = GrabMode::None;
		BoxNode scalingNode, hoveringNode;

		// NOTE: To prevent accidental resizing when clicking on a node
		const float mouseDragThreshold = 2.0f;
		bool allowAction = false;

		void MoveBoxCorner(BoxNode scalingNode, TransformBox& box, vec2 position, float rotation) const;

	private:
		void DragPositionTooltip(const vec2& position);
		void DragScaleTooltip(const vec2& scale, const vec2& dimensions);
	
	private:
		TransformBox BoxWorldToScreenSpace(const TransformBox& box) const;
	};
}