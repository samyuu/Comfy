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
		static constexpr vec4 redColor = vec4(0.91f, 0.17f, 0.05f, 0.85f);
		static constexpr vec4 redPreColor = vec4(0.99f, 0.29f, 0.12f, 0.85f);
		static constexpr vec4 yellowColor = vec4(.83f, .76f, .29f, 0.85f);
		static constexpr vec4 whiteColor = vec4(1.0f, 1.0f, 1.0f, 0.85f);

		enum class GrabMode
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

		bool allowAction = false;

		void MoveBoxCorner(BoxNode scalingNode, TransformBox& box, vec2 position, float rotation) const;

	private:
		void DragPositionTooltip(const vec2& position);
		void DragScaleTooltip(const vec2& scale, const vec2& dimensions);
	
	private:
		TransformBox BoxWorldToScreenSpace(const TransformBox& box) const;
	};
}