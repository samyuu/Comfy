#pragma once
#include "AetTool.h"
#include "Editor/Common/CameraController2D.h"

namespace Comfy::Studio::Editor
{
	class HandTool : public AetTool
	{
	public:
		HandTool();

		const char* GetIcon() const override;
		const char* GetName() const override;
		AetToolType GetType() const override;
		Input::KeyCode GetShortcutKey() const override;

		void UpdatePostDrawGui(Graphics::Transform2D* transform, vec2 dimensions) override;
		void DrawContextMenu() override;
		void UpdateCamera(Graphics::OrthographicCamera& camera, vec2 relativeMouse) override;

	private:
		CameraController2D cameraController;
	};
}
