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
		void Update(Graphics::OrthographicCamera& camera, vec2 relativeMouse);
		void SetUpdateCameraZoom(Graphics::OrthographicCamera& camera, float newZoom, vec2 origin);

	private:
		bool windowHoveredOnClick[5];

		void UpdateKeyboardInput(Graphics::OrthographicCamera& camera);
		void UpdateMouseInput(Graphics::OrthographicCamera& camera, vec2 relativeMouse);
	};
}
