#include "CameraController3D.h"
#include "ImGui/Gui.h"
#include "Input/KeyCode.h"

namespace Comfy::Editor
{
	void CameraController3D::Update(Graphics::PerspectiveCamera& camera)
	{
		if (Mode == ControlMode::None)
			return;

		const auto& io = Gui::GetIO();

		const bool fastCamera = Gui::IsKeyDown(KeyCode_Shift);
		const bool slowCamera = Gui::IsKeyDown(KeyCode_Alt);

		const float scrollStep = slowCamera ? 0.5f : (fastCamera ? 12.5f : 1.5f);
		const float cameraSpeed = (slowCamera ? 0.25f : (fastCamera ? 25.5f : 2.25f)) * io.DeltaTime;

		if (Mode == ControlMode::FirstPerson)
		{
			const float pitchRadians = glm::radians(FirstPersonData.Pitch);
			const float yawRadians = glm::radians(FirstPersonData.Yaw);

			vec3 frontDirection;
			frontDirection.x = glm::cos(yawRadians) * glm::cos(pitchRadians);
			frontDirection.y = glm::sin(pitchRadians);
			frontDirection.z = glm::sin(yawRadians) * glm::cos(pitchRadians);

			if (Gui::IsWindowFocused())
			{
				if (Gui::IsMouseDown(0))
				{
					FirstPersonData.TargetYaw += io.MouseDelta.x * Settings.MouseSensitivity;
					FirstPersonData.TargetPitch -= io.MouseDelta.y * Settings.MouseSensitivity;

					FirstPersonData.TargetPitch = glm::clamp(FirstPersonData.TargetPitch, -89.0f, +89.0f);
				}

				if (Gui::IsWindowHovered())
				{
					if (io.MouseWheel != 0.0f)
						camera.ViewPoint += frontDirection * (scrollStep * io.MouseWheel);
				}

				UpdateKeyboardInput(camera.ViewPoint, frontDirection, cameraSpeed);
			}

			if (Settings.InterpolationSmoothness > 0.0f)
			{
				FirstPersonData.Yaw = ImLerp(FirstPersonData.Yaw, FirstPersonData.TargetYaw, io.DeltaTime * Settings.InterpolationSmoothness);
				FirstPersonData.Pitch = ImLerp(FirstPersonData.Pitch, FirstPersonData.TargetPitch, io.DeltaTime * Settings.InterpolationSmoothness);
			}
			else
			{
				FirstPersonData.Yaw = FirstPersonData.TargetYaw;
				FirstPersonData.Pitch = FirstPersonData.TargetPitch;
			}

			camera.Interest = camera.ViewPoint + glm::normalize(frontDirection);
		}
		else if (Mode == ControlMode::Orbit)
		{
			if (Gui::IsWindowFocused())
			{
				vec3 frontDirection;
				frontDirection = glm::normalize(camera.Interest - camera.ViewPoint);
				frontDirection.y = 0.0f;

				if (Gui::IsMouseDown(0))
				{
					OrbitData.TargetRotation.x += io.MouseDelta.x * Settings.MouseSensitivity;
					OrbitData.TargetRotation.y += io.MouseDelta.y * Settings.MouseSensitivity;

					OrbitData.TargetRotation.y = glm::clamp(OrbitData.TargetRotation.y, -89.0f, +89.0f);
				}

				if (Gui::IsWindowHovered())
				{
					if (io.MouseWheel != 0.0f)
						OrbitData.Distance = glm::clamp(OrbitData.Distance - (scrollStep * io.MouseWheel), OrbitData.MinDistance, OrbitData.MaxDistance);
				}

				UpdateKeyboardInput(camera.Interest, frontDirection, cameraSpeed);
			}

			vec3 orbitPosition = vec3(0.0f, 0.0f, OrbitData.Distance);
			const mat4 rotationX = glm::rotate(mat4(1.0f), glm::radians(OrbitData.TargetRotation.x), vec3(0.0f, 1.0f, 0.0f));
			const mat4 rotationY = glm::rotate(mat4(1.0f), glm::radians(OrbitData.TargetRotation.y), vec3(1.0f, 0.0f, 0.0f));
			orbitPosition = vec4(orbitPosition, 1.0f) * rotationY * rotationX;

			camera.ViewPoint = camera.Interest + orbitPosition;
		}
	}

	void CameraController3D::UpdateKeyboardInput(vec3& pointToChange, const vec3& frontDirection, float cameraSpeed)
	{
		constexpr vec3 upDirection = Graphics::PerspectiveCamera::UpDirection;

		if (Gui::IsKeyDown(KeyCode_W))
			pointToChange += frontDirection * cameraSpeed;
		if (Gui::IsKeyDown(KeyCode_S))
			pointToChange -= frontDirection * cameraSpeed;
		if (Gui::IsKeyDown(KeyCode_A))
			pointToChange -= glm::normalize(glm::cross(frontDirection, upDirection)) * cameraSpeed;
		if (Gui::IsKeyDown(KeyCode_D))
			pointToChange += glm::normalize(glm::cross(frontDirection, upDirection)) * cameraSpeed;

		if (Gui::IsKeyDown(KeyCode_Space))
			pointToChange += upDirection * cameraSpeed;
		if (Gui::IsKeyDown(KeyCode_Control))
			pointToChange -= upDirection * cameraSpeed;
	}
}
