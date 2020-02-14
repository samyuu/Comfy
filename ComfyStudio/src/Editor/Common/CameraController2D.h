#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Graphics/Camera.h"

namespace Editor
{
	class CameraController2D
	{
	public:
		float ZoomStep = 1.1f;
		float ZoomMin = 0.1f;
		float ZoomMax = 12.8f;

		int MouseDragButton = 1;
		bool AltZoomControl = true;

	public:
		void Update(Graphics::OrthographicCamera& camera, vec2 relativeMouse);
		void SetUpdateCameraZoom(Graphics::OrthographicCamera& camera, float newZoom, vec2 origin);

	private:
		bool updateKeyboardControls = false;
		bool updateMouseControls = true;
		std::array<bool, 5> windowHoveredOnClick;

		void UpdateKeyboardInput(Graphics::OrthographicCamera& camera);
		void UpdateMouseInput(Graphics::OrthographicCamera& camera, vec2 relativeMouse);
	};
}
