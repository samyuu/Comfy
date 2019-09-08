#include "CameraController2D.h"
#include "ImGui/Gui.h"
#include "Input/KeyCode.h"
#include <algorithm>

namespace Editor
{
	void CameraController2D::Update(Graphics::OrthographicCamera& camera, vec2 relativeMouse)
	{
		for (int i = 0; i < IM_ARRAYSIZE(windowHoveredOnClick); i++)
		{
			if (Gui::IsMouseClicked(i))
				windowHoveredOnClick[i] = Gui::IsWindowHovered();
		}

		if (Gui::IsWindowFocused())
		{
			UpdateKeyboardInput(camera);
			UpdateMouseInput(camera, relativeMouse);
		}
	}

	void CameraController2D::SetUpdateCameraZoom(Graphics::OrthographicCamera& camera, float newZoom, vec2 origin)
	{
		vec2 worldSpace = camera.ScreenToWorldSpace(origin);

		camera.Zoom = std::clamp(newZoom, ZoomMin, ZoomMax);
		camera.UpdateMatrices();

		vec2 postWorldSpace = camera.ScreenToWorldSpace(origin);

		camera.Position -= (postWorldSpace - worldSpace) * vec2(camera.Zoom);
		camera.Position = glm::round(camera.Position);
	}

	void CameraController2D::UpdateKeyboardInput(Graphics::OrthographicCamera& camera)
	{
		// TODO: Either remove or add as an optional bool field

		constexpr float step = 25.0f;
		if (Gui::IsKeyPressed(KeyCode_W, true))
			camera.Position.y -= step;
		if (Gui::IsKeyPressed(KeyCode_S, true))
			camera.Position.y += step;
		if (Gui::IsKeyPressed(KeyCode_A, true))
			camera.Position.x -= step;
		if (Gui::IsKeyPressed(KeyCode_D, true))
			camera.Position.x += step;

		if (Gui::IsKeyPressed(KeyCode_Escape, true))
		{
			camera.Position = vec2(0.0f);
			camera.Zoom = 1.0f;
		}
	}

	void CameraController2D::UpdateMouseInput(Graphics::OrthographicCamera& camera, vec2 relativeMouse)
	{
		ImGuiIO& io = Gui::GetIO();

		if (windowHoveredOnClick[1] && Gui::IsMouseDown(1))
		{
			camera.Position -= vec2(io.MouseDelta.x, io.MouseDelta.y);
			Gui::SetMouseCursor(ImGuiMouseCursor_Hand);
		}

		if (io.KeyAlt && io.MouseWheel != 0.0f)
		{
			float newZoom = camera.Zoom * ((io.MouseWheel > 0) ? ZoomStep : (1.0f / ZoomStep));
			SetUpdateCameraZoom(camera, newZoom, relativeMouse);
		}
	}
}