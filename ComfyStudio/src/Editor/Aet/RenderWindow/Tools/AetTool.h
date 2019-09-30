#pragma once
#include "Graphics/Camera.h"
#include "Graphics/Auth2D/AetMgr.h"
#include "Editor/Aet/Command/AetCommandManager.h"
#include "Input/KeyCode.h"
#include "ImGui/Gui.h"
#include <functional>

namespace Editor
{
	using namespace FileSystem;

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
		// TODO: Should be implement by all components
		static constexpr KeyCode GridSnapModifierKey = KeyCode_Left_Control;
		
		// TODO: Implement by the RectangleTool
		// NOTE: For scaling both axes at the same time
		static constexpr KeyCode AxesLinkModifierKey = KeyCode_Left_Shift;

		// TODO: These should probably be set by the parent component (maybe inside the context menu (?))
		static constexpr vec2 PositionSnapPrecision = vec2(16.0f);
		static constexpr vec2 ScaleSnapPrecision = vec2(10.0f);
		static constexpr float RotationSnapPrecision = 15.0f;

		// NOTE: Little helper function
		template <typename T>
		static inline T Snap(T value, T precision)
		{
			return glm::round(value / precision) * precision;
		}

		virtual const char* GetIcon() const = 0;
		virtual const char* GetName() const = 0;
		virtual AetToolType GetType() const = 0;
		virtual KeyCode GetShortcutKey() const = 0;

		// TODO: virtual void OnSelect() {};
		// TODO: virtual void OnDeselect() {};

		// NOTE: Do the input handling and draw the tool widgets
		virtual void UpdatePostDrawGui(Graphics::Auth2D::Properties* properties, vec2 dimensions) {};
		
		// NOTE: Turn the updated properties into a set of AetCommands
		virtual void ProcessCommands(AetCommandManager* commandManager, const RefPtr<AetObj>& aetObj, float frame, const Graphics::Auth2D::Properties& properties, const Graphics::Auth2D::Properties& previousProperties) {};
		
		// NOTE: Tool specific context menu items
		virtual void DrawContextMenu() = 0;
		
		// NOTE: Special case for the HandTool
		virtual void UpdateCamera(Graphics::OrthographicCamera& camera, vec2 relativeMouse) {};

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