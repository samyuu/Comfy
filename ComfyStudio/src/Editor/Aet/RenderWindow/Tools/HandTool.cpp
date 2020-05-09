#include "HandTool.h"
#include "Editor/Aet/AetIcons.h"

namespace Comfy::Editor
{
	HandTool::HandTool()
	{
		// NOTE: As opposed to the right mouse button used by all tools
		cameraController.MouseDragButton = 0;

		// NOTE: Because it's already handled by the common camera controller
		cameraController.AltZoomControl = false;
	}

	const char* HandTool::GetIcon() const
	{
		return ICON_FA_HAND_PAPER;
	}

	const char* HandTool::GetName() const
	{
		return "Hand Tool";
	}

	AetToolType HandTool::GetType() const
	{
		return AetToolType_Hand;
	}

	Input::KeyCode HandTool::GetShortcutKey() const
	{
		return Input::KeyCode_Q;
	}

	void HandTool::UpdatePostDrawGui(Graphics::Transform2D* transform, vec2 dimensions)
	{
	}

	void HandTool::DrawContextMenu()
	{
	}

	void HandTool::UpdateCamera(Graphics::OrthographicCamera& camera, vec2 relativeMouse)
	{
		// NOTE: Hacky way of preventing moving the camera by two camera controllers at once
		if (Gui::IsMouseDown(1))
			return;

		cameraController.Update(camera, relativeMouse);
	}
}
