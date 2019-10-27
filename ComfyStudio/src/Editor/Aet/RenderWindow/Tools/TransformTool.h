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

		// TODO: Rename to 'UpdateProperties' (?)
		virtual void UpdatePostDrawGui(Graphics::Properties* properties, vec2 dimensions) override;
		virtual void ProcessCommands(AetCommandManager* commandManager, const RefPtr<Graphics::AetObj>& aetObj, float frame, const Graphics::Properties& properties, const Graphics::Properties& previousProperties) override;

		virtual void DrawContextMenu() override;
		virtual bool MouseFocusCaptured() const override;

	private:
		static constexpr vec4 redColor = vec4(0.91f, 0.17f, 0.05f, 0.85f);
		static constexpr vec4 redPreColor = vec4(0.99f, 0.29f, 0.12f, 0.85f);
		static constexpr vec4 yellowColor = vec4(.83f, .76f, .29f, 0.85f);
		static constexpr vec4 whiteColor = vec4(1.0f, 1.0f, 1.0f, 0.85f);

		enum class GrabMode
		{
			None, Move, Scale, Rotate
		};

		static constexpr int actionMouseButton = 0;

		vec2 scaleNodeWorldPositionOnMouseDown;
		vec2 mouseWorldPositionOnMouseDown;
		TransformBox screenSpaceBox, worldSpaceBox;
		Graphics::Properties propertiesOnMouseDown;

		GrabMode mode = GrabMode::None;
		BoxNode scalingNode = BoxNode_Invalid, hoveringNode = BoxNode_Invalid;

		bool boxHovered = false;
		bool allowAction = false;

		void UpdateKeyboardMoveInput(Graphics::Properties* properties);
		void MoveBoxCorner(BoxNode scalingNode, TransformBox& box, vec2 position, float rotation) const;

	private:
		void DragPositionTooltip(const vec2& position);
		void DragScaleTooltip(const vec2& scale, const vec2& dimensions);
	
	private:
		TransformBox BoxWorldToScreenSpace(const TransformBox& box) const;
	};
}