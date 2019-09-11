#pragma once
#include "Graphics/Camera.h"
#include "Graphics/Auth2D/AetMgr.h"
#include "Input/KeyCode.h"
#include "ImGui/Gui.h"
#include <functional>

namespace Editor
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
		virtual const char* GetIcon() const = 0;
		virtual const char* GetName() const = 0;
		virtual AetToolType GetType() const = 0;
		virtual KeyCode GetShortcutKey() const = 0;

		virtual void UpdatePostDrawGui(Graphics::Auth2D::Properties* properties, vec2 dimensions) {};
		virtual void DrawContextMenu() = 0;
		virtual void UpdateCamera(Graphics::OrthographicCamera& camera, vec2 relativeMouse) {};

	public:
		void SetSpaceConversionFunctions(const std::function<vec2(vec2)>& worldToScreenSpace, const std::function<vec2(vec2)>& screenToWorldSpace);

	protected:
		vec2 ToScreenSpace(vec2 worldSpace) const;
		vec2 ToWorldSpace(vec2 screenSpace) const;

	private:
		std::function<vec2(vec2)> worldToScreenSpaceFunction;
		std::function<vec2(vec2)> screenToWorldSpaceFunction;
	};
}