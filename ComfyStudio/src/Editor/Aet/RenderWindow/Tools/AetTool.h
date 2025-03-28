#pragma once
#include "Types.h"
#include "Render/Render.h"
#include "Undo/Undo.h"
#include "Input/Input.h"
#include "ImGui/Gui.h"
#include <functional>

namespace Comfy::Studio::Editor
{
	enum AetToolType
	{
		AetToolType_Hand,
		AetToolType_Move,
		AetToolType_Rotate,
		AetToolType_Scale,
		AetToolType_Transform,
		AetToolType_Count
	};

	class AetTool
	{
	public:
		AetTool() = default;
		virtual ~AetTool() = default;

	public:
		// TODO: Should be implement by all components
		static constexpr Input::KeyCode GridSnapModifierKey = Input::KeyCode_Ctrl;
		
		// TODO: Implement by the RectangleTool
		// NOTE: For scaling both axes at the same time
		static constexpr Input::KeyCode AxesLinkModifierKey = Input::KeyCode_Shift;

		// TODO: These should probably be set by the parent component (maybe inside the context menu (?))
		static constexpr vec2 PositionSnapPrecision = vec2(10.0f);
		static constexpr vec2 ScaleSnapPrecision = vec2(10.0f);
		static constexpr float RotationSnapPrecision = 15.0f;

		// NOTE: For keyboard based movement
		static constexpr Input::KeyCode FastNudgeModifierKey = Input::KeyCode_Shift;

		static constexpr float NudgeStepDistance = 1.0f;
		static constexpr float NudgeFastStepDistance = 10.0f;

		// NOTE: Little helper function
		template <typename T>
		static T Snap(T value, T precision)
		{
			return glm::round(value / precision) * precision;
		}

		virtual const char* GetIcon() const = 0;
		virtual const char* GetName() const = 0;
		virtual AetToolType GetType() const = 0;
		virtual Input::KeyCode GetShortcutKey() const = 0;

		// TODO: virtual void OnSelect() {};
		// TODO: virtual void OnDeselect() {};

		// NOTE: Do the input handling and draw the tool widgets
		virtual void UpdatePostDrawGui(Graphics::Transform2D* transform, vec2 dimensions) {};
		
		// NOTE: Turn the updated properties into a set of AetCommands
		virtual void ProcessCommands(Undo::UndoManager& undoManager, const std::shared_ptr<Graphics::Aet::Layer>& layer, frame_t frame, const Graphics::Transform2D& transform, const Graphics::Transform2D& previousTransform) {}
		
		// NOTE: Tool specific context menu items
		virtual void DrawContextMenu() = 0;
		
		// NOTE: Special case for the HandTool
		virtual void UpdateCamera(Render::Camera2D& camera, vec2 relativeMouse) {}

		// NOTE: So we know if we can mouse pick an object of the current composition without interfering with the tool
		// TODO: Default return false for now but should be abstract in the future
		// virtual bool MouseFocusCaptured() const = 0;
		virtual bool MouseFocusCaptured() const { return false; }

	public:
		void SetSpaceConversionFunctions(const std::function<vec2(vec2)>& worldToScreenSpace, const std::function<vec2(vec2)>& screenToWorldSpace);

	protected:
		vec2 ToScreenSpace(vec2 worldSpace) const;
		vec2 ToWorldSpace(vec2 screenSpace) const;

	protected:
		// NOTE: To prevent accidental resizing when clicking on a node
		const float mouseDragThreshold = 2.0f;

	private:
		std::function<vec2(vec2)> worldToScreenSpaceFunction;
		std::function<vec2(vec2)> screenToWorldSpaceFunction;
	};
}
