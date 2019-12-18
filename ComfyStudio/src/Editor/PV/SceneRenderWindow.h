#pragma once
#include "Editor/Core/IEditorComponent.h"
#include "Editor/Core/RenderWindowBase.h"
#include "Graphics/Direct3D/D3D_Renderer3D.h"
#include "Graphics/Auth3D/ObjSet.h"
#include "Graphics/Camera.h"
#include "Graphics/SprSet.h"
#include "ImGui/Widgets/FileViewer.h"

namespace Editor
{
	class SceneRenderWindow : public IEditorComponent, public RenderWindowBase
	{
	public:
		SceneRenderWindow(Application* parent, EditorManager* editor);
		~SceneRenderWindow();

	public:
		const char* GetGuiName() const override;
		void Initialize() override;
		void DrawGui() override;
		void PostDrawGui() override;

		void OnWindowBegin() override;
		void OnWindowEnd() override;

	private:
		void OnUpdateInput() override;
		void OnUpdate() override;
		void OnRender() override;
		void OnResize(ivec2 size) override;

	private:
		void UpdateCamera();
		void DrawComfyDebugGui();

		void LoadObjSet(const std::string& objSetPath);

	private:
		struct CameraController
		{
			float CameraSmoothness = 50.0f;
			float CameraPitch = 0.0f;
			float CameraYaw = 0.0f;
			float CameraRoll = 0.0f;

			float TargetCameraPitch = 0.0f;
			float TargetCameraYaw = 0.0f;

			float CameraSensitivity = 0.25f;
		} cameraController;

		Graphics::SceneContext context;

		Gui::FileViewer objFileViewer = { "dev_rom/objset/" };

		int objectIndex = -1;
		int materialObjIndex = 0, materialIndex = 0;
		UniquePtr<Graphics::ObjSet> objSet = nullptr, textureObjSet = nullptr;

		UniquePtr<Graphics::D3D_Renderer3D> renderer3D;
	};
}