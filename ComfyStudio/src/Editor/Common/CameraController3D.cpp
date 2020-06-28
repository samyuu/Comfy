#include "CameraController3D.h"
#include "ImGui/Gui.h"
#include "Input/Input.h"

namespace Comfy::Studio::Editor
{
	constexpr float YawDegreesMin = -89.0f;
	constexpr float YawDegreesMax = +89.0f;

	void CameraController3D::Update(Render::PerspectiveCamera& camera)
	{
		if (Mode == ControlMode::None)
			return;

		const auto& io = Gui::GetIO();

		const bool fastCamera = Gui::IsKeyDown(Input::KeyCode_Shift);
		const bool slowCamera = Gui::IsKeyDown(Input::KeyCode_Alt);

		const float scrollStep = slowCamera ? 0.5f : (fastCamera ? 12.5f : 1.5f);
		const float cameraSpeed = (slowCamera ? 0.25f : (fastCamera ? 25.5f : 2.25f)) * io.DeltaTime;

		if (Mode == ControlMode::FirstPerson)
		{
			const mat4 rotationY = glm::rotate(mat4(1.0f), glm::radians(FirstPersonData.Pitch), vec3(1.0f, 0.0f, 0.0f));
			const mat4 rotationX = glm::rotate(mat4(1.0f), glm::radians(FirstPersonData.Yaw), vec3(0.0f, 1.0f, 0.0f));
			const vec3 frontDirection = glm::normalize(vec4(0.0f, 0.0f, -1.0f, 1.0f) * rotationY * rotationX);

			if (Gui::IsWindowFocused())
			{
				if (Gui::IsMouseDown(0))
				{
					FirstPersonData.TargetYaw += io.MouseDelta.x * Settings.MouseSensitivity;
					FirstPersonData.TargetPitch += io.MouseDelta.y * Settings.MouseSensitivity;
					FirstPersonData.TargetPitch = glm::clamp(FirstPersonData.TargetPitch, YawDegreesMin, YawDegreesMax);
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

			camera.Interest = (camera.ViewPoint + frontDirection);
		}
		else if (Mode == ControlMode::Orbit)
		{
			if (Gui::IsWindowFocused())
			{
				auto frontDirection = (camera.Interest - camera.ViewPoint);
				frontDirection = glm::normalize(vec3(frontDirection.x, 0.0f, frontDirection.z));

				if (Gui::IsMouseDown(0))
				{
					OrbitData.TargetRotation.x += io.MouseDelta.x * Settings.MouseSensitivity;
					OrbitData.TargetRotation.y += io.MouseDelta.y * Settings.MouseSensitivity;
					OrbitData.TargetRotation.y = glm::clamp(OrbitData.TargetRotation.y, YawDegreesMin, YawDegreesMax);
				}

				if (Settings.OrbitMouseScrollDistance && Gui::IsWindowHovered())
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

	void CameraController3D::FitOrbitAroundSphere(Render::PerspectiveCamera& camera, const Graphics::Sphere& sphere)
	{
		const float cameraView = 2.0f * glm::tan(glm::radians(camera.FieldOfView) * 0.5f);
		const float distance = (sphere.Radius / cameraView) + sphere.Radius;

		camera.Interest = camera.ViewPoint = sphere.Center;
		OrbitData.Distance = distance;

		SetControlModePreserveOrientation(camera, ControlMode::Orbit);
	}

	void CameraController3D::SetControlModePreserveOrientation(Render::PerspectiveCamera& camera, ControlMode newMode)
	{
		const auto oldMode = Mode;
		Mode = newMode;

		if (newMode == ControlMode::None || newMode == oldMode)
			return;

		const auto cameraDirection = glm::quatLookAt(glm::normalize(camera.Interest - camera.ViewPoint), camera.UpDirection);

		// BUG: Applying these directly isn't always correct in case of above 90 degrees pitch etc. but it's better than nothing for now
		const float cameraYaw = glm::yaw(cameraDirection);
		const float cameraPitch = glm::pitch(cameraDirection);

		switch (newMode)
		{
		case ControlMode::FirstPerson:
		{
			FirstPersonData.Pitch = FirstPersonData.TargetPitch = glm::degrees(-cameraPitch);
			FirstPersonData.Yaw = FirstPersonData.TargetYaw = glm::degrees(-cameraYaw);
		}
		break;

		case ControlMode::Orbit:
		{
			OrbitData.Distance = glm::distance(camera.Interest, camera.ViewPoint);
			OrbitData.TargetRotation = glm::degrees(vec2(-glm::yaw(cameraDirection), -glm::pitch(cameraDirection)));
		}
		break;

		default:
			assert(false);
		}
	}

	void CameraController3D::UpdateKeyboardInput(vec3& pointToChange, const vec3& frontDirection, float cameraSpeed)
	{
		constexpr vec3 upDirection = Render::PerspectiveCamera::UpDirection;

		if (Gui::IsKeyDown(Input::KeyCode_W))
			pointToChange += frontDirection * cameraSpeed;
		if (Gui::IsKeyDown(Input::KeyCode_S))
			pointToChange -= frontDirection * cameraSpeed;
		if (Gui::IsKeyDown(Input::KeyCode_A))
			pointToChange -= glm::normalize(glm::cross(frontDirection, upDirection)) * cameraSpeed;
		if (Gui::IsKeyDown(Input::KeyCode_D))
			pointToChange += glm::normalize(glm::cross(frontDirection, upDirection)) * cameraSpeed;

		if (Gui::IsKeyDown(Input::KeyCode_Space))
			pointToChange += upDirection * cameraSpeed;
		if (Gui::IsKeyDown(Input::KeyCode_Control))
			pointToChange -= upDirection * cameraSpeed;
	}
}
