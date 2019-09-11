#pragma once
#include "AetTool.h"
#include "Editor/Common/CameraController2D.h"

namespace Editor
{
	class HandTool : public AetTool
	{
	public:
		HandTool();

		virtual const char* GetIcon() const override;
		virtual const char* GetName() const override;
		virtual AetToolType GetType() const override;
		virtual KeyCode GetShortcutKey() const override;

		virtual void UpdatePostDrawGui(Graphics::Auth2D::Properties* properties, vec2 dimensions) override;
		virtual void DrawContextMenu() override;
		virtual void UpdateCamera(Graphics::OrthographicCamera& camera, vec2 relativeMouse) override;

	private:
		CameraController2D cameraController;
	};
}