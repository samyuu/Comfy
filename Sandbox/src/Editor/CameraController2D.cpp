#include "CameraController2D.h"
#include "ImGui/imgui.h"
#include "Input/KeyCode.h"
#include <algorithm>

namespace Editor
{
	void CameraController2D::Update(OrthographicCamera& camera, vec2 relativeMouse)
	{
		for (int i = 0; i < IM_ARRAYSIZE(windowHoveredOnClick); i++)
		{
			if (ImGui::IsMouseClicked(i))
				windowHoveredOnClick[i] = ImGui::IsWindowHovered();
		}

		if (ImGui::IsWindowFocused())
		{
			UpdateKeyboardInput(camera);
			UpdateMouseInput(camera, relativeMouse);
		}
	}

	void CameraController2D::SetUpdateCameraZoom(OrthographicCamera& camera, float newZoom, vec2 origin) const
	{
		vec2 worldSpace = camera.ScreenToWorldSpace(origin);
		
		camera.Zoom = std::clamp(newZoom, ZoomMin, ZoomMax);
		camera.UpdateMatrices();
		
		vec2 postWorldSpace = camera.ScreenToWorldSpace(origin);

		camera.Position -= (postWorldSpace - worldSpace) * vec2(camera.Zoom);
	}

	void CameraController2D::UpdateKeyboardInput(OrthographicCamera& camera)
	{
		constexpr float step = 10.0f;
		if (ImGui::IsKeyPressed(KeyCode_W, true))
			camera.Position.y -= step;
		if (ImGui::IsKeyPressed(KeyCode_S, true))
			camera.Position.y += step;
		if (ImGui::IsKeyPressed(KeyCode_A, true))
			camera.Position.x -= step;
		if (ImGui::IsKeyPressed(KeyCode_D, true))
			camera.Position.x += step;

		if (ImGui::IsKeyPressed(KeyCode_Escape, true))
		{
			camera.Position = vec2(0.0f);
			camera.Zoom = 1.0f;
		}
	}

	void CameraController2D::UpdateMouseInput(OrthographicCamera& camera, vec2 relativeMouse)
	{
		ImGuiIO& io = ImGui::GetIO();

		if (windowHoveredOnClick[1] && ImGui::IsMouseDown(1))
		{
			camera.Position -= vec2(io.MouseDelta.x, io.MouseDelta.y);
			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
		}

		if (io.KeyAlt && io.MouseWheel != 0.0f)
		{
			float newZoom = camera.Zoom * ((io.MouseWheel > 0) ? ZoomStep : (1.0f / ZoomStep));
			SetUpdateCameraZoom(camera, newZoom, relativeMouse);
		}
	}
}