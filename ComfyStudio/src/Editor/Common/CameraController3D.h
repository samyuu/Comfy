#pragma once
#include "Types.h"
#include "Render/Render.h"

namespace Comfy::Studio::Editor
{
	class CameraController3D
	{
	public:
		enum class ControlMode : i32
		{
			// NOTE: Disable
			None,
			// NOTE: First person style
			FirstPerson,
			// NOTE: Rotate around point of interest
			Orbit,

			Count,
		};

		static constexpr std::array<const char*, EnumCount<ControlMode>()> ControlModeNames =
		{
			"None",
			"First Person",
			"Orbit",
		};

		ControlMode Mode = ControlMode::Orbit;

		struct Settings
		{
			float InterpolationSmoothness = 0.0f;
			float MouseSensitivity = 0.25f;
			bool OrbitMouseScrollDistance = true;
		} Settings;

		struct FirstPersonData
		{
			float Yaw = 0.0f;
			float Pitch = 0.0f;

			float TargetYaw = 0.0f;
			float TargetPitch = 0.0f;
		} FirstPersonData;

		struct OrbitData
		{
			const float MinDistance = 0.1f;
			const float MaxDistance = 1000.0f;

			float Distance = 5.0f;
			vec2 TargetRotation = vec2(0.0f);
		} OrbitData;

		struct Visualization
		{
			bool VisualizeInterest = false;

			Graphics::Sphere InterestSphere = { vec3(0.0f), 0.3f };
			vec4 InterestSphereColor = vec4(0.66f, 1.00f, 0.32f, 0.25f);
			std::unique_ptr<Graphics::Obj> InterestSphereObj = nullptr;
		} Visualization;

	public:
		void Update(Render::Camera3D& camera);

		void FitOrbitAroundSphere(Render::Camera3D& camera, const Graphics::Sphere& sphere);
		void SetControlModePreserveOrientation(Render::Camera3D& camera, ControlMode newMode);

	private:
		void UpdateKeyboardInput(vec3& pointToChange, const vec3& frontDirection, float cameraSpeed);
	};
}
