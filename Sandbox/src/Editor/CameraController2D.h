#pragma once
#include "Types.h"
#include "Graphics/Camera.h"

namespace Editor
{
	class CameraController2D
	{
	public:
		float ZoomStep = 1.1f;
		float ZoomMin = 0.1f;
		float ZoomMax = 12.8f;

	public:
		void Update(OrthographicCamera& camera, vec2 relativeMouse);
		void SetUpdateCameraZoom(OrthographicCamera& camera, float newZoom, vec2 origin) const;

	private:
		bool windowHoveredOnClick[5];

		void UpdateKeyboardInput(OrthographicCamera& camera);
		void UpdateMouseInput(OrthographicCamera& camera, vec2 relativeMouse);
	};
}
