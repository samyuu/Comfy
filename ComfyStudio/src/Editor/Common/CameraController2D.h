#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Render/Render.h"

namespace Comfy::Studio::Editor
{
	class CameraController2D
	{
	public:
		void Update(Render::OrthographicCamera& camera, vec2 relativeMouse);
		void SetUpdateCameraZoom(Render::OrthographicCamera& camera, float newZoom, vec2 origin);

	public:
		float ZoomStep = 1.1f;
		float ZoomMin = 0.1f;
		float ZoomMax = 12.8f;

		int MouseDragButton = 1;
		bool AltZoomControl = true;

	private:
		void UpdateKeyboardInput(Render::OrthographicCamera& camera);
		void UpdateMouseInput(Render::OrthographicCamera& camera, vec2 relativeMouse);

	private:
		bool updateKeyboardControls = false;
		bool updateMouseControls = true;
		std::array<bool, 5> windowHoveredOnClick = {};
	};
}
