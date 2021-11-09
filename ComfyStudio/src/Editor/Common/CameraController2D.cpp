#include "CameraController2D.h"
#include "Input/Input.h"
#include <algorithm>

namespace Comfy::Studio::Editor
{
	void CameraController2D::Update(Render::Camera2D& camera, vec2 relativeMouse)
	{
		for (ImGuiMouseButton i = 0; i < ImGuiMouseButton_COUNT; i++)
		{
			if (Gui::IsMouseClicked(i))
				windowHoveredOnClick[i] = Gui::IsWindowHovered();
		}

		if (updateKeyboardControls)
			UpdateKeyboardInput(camera);

		if (updateMouseControls)
			UpdateMouseInput(camera, relativeMouse);
	}

	void CameraController2D::SetUpdateCameraZoom(Render::Camera2D& camera, float newZoom, vec2 origin)
	{
		const vec2 worldSpace = camera.ScreenToWorldSpace(origin);

		camera.Zoom = Clamp(newZoom, ZoomMin, ZoomMax);
		camera.UpdateMatrices();

		const vec2 postWorldSpace = camera.ScreenToWorldSpace(origin);

		camera.Position -= (postWorldSpace - worldSpace) * vec2(camera.Zoom);
		camera.Position = glm::round(camera.Position);
	}

	void CameraController2D::UpdateKeyboardInput(Render::Camera2D& camera)
	{
		if (!Gui::IsWindowFocused())
			return;

		constexpr float step = 25.0f;
		if (Gui::IsKeyPressed(Input::KeyCode_W, true))
			camera.Position.y -= step;
		if (Gui::IsKeyPressed(Input::KeyCode_S, true))
			camera.Position.y += step;
		if (Gui::IsKeyPressed(Input::KeyCode_A, true))
			camera.Position.x -= step;
		if (Gui::IsKeyPressed(Input::KeyCode_D, true))
			camera.Position.x += step;

		if (Gui::IsKeyPressed(Input::KeyCode_Escape, true))
		{
			camera.Position = vec2(0.0f);
			camera.Zoom = 1.0f;
		}
	}

	void CameraController2D::UpdateMouseInput(Render::Camera2D& camera, vec2 relativeMouse)
	{
		const auto& io = Gui::GetIO();

		if (windowHoveredOnClick[MouseDragButton] && Gui::IsMouseDown(MouseDragButton))
		{
			camera.Position -= vec2(io.MouseDelta.x, io.MouseDelta.y);
			Gui::SetMouseCursor(ImGuiMouseCursor_Hand);
		}

		if (AltZoomControl && io.KeyAlt && io.MouseWheel != 0.0f)
		{
			if (Gui::IsWindowHovered())
			{
				const float newZoom = camera.Zoom * ((io.MouseWheel > 0) ? ZoomStep : (1.0f / ZoomStep));
				SetUpdateCameraZoom(camera, newZoom, relativeMouse);
			}
		}
	}
}
