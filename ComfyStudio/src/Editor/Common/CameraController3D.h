#pragma once
#include "Graphics/Camera.h"
#include "Graphics/Auth3D/ObjSet.h"

namespace Comfy::Editor
{
	class CameraController3D
	{
	public:
		enum class ControlMode : int32_t
		{
			// NOTE: Disable
			None,
			// NOTE: First person style
			FirstPerson,
			// NOTE: Rotate around point of interest
			Orbit,
		};

		ControlMode Mode = ControlMode::Orbit;

		struct Settings
		{
			float InterpolationSmoothness = 0.0f;
			float MouseSensitivity = 0.25f;
		} Settings;

		struct FirstPersonData
		{
			float Pitch = 0.0f;
			float Yaw = 0.0f;
			float Roll = 0.0f;

			float TargetPitch = 0.0f;
			float TargetYaw = 0.0f;
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
			UniquePtr<Graphics::Obj> InterestSphereObj = nullptr;
		} Visualization;

	public:
		void Update(Graphics::PerspectiveCamera& camera);
	
	private:
		void UpdateKeyboardInput(vec3& pointToChange, const vec3& frontDirection, float cameraSpeed);
	};
}
