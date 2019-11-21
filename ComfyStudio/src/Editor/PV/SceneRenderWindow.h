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
		bool GetShouldCreateDepthRenderTarget() const override { return true; };

		void OnUpdateInput() override;
		void OnUpdate() override;
		void OnRender() override;
		void OnResize(ivec2 size) override;

	private:
		void UpdateCamera();
		void DrawComfyDebugGui();

		void LoadObjSet(const std::string& objSetPath);

	private:
		struct SceneCameraData
		{
			float CameraSmoothness = 50.0f;
			float CameraPitch = 0.0f;
			float CameraYaw = -90.0f; 
			float CameraRoll = 0.0f;

			float TargetCameraPitch = 0.0f;
			float TargetCameraYaw = -90.0f;

			float CameraSensitivity = 0.25f;
			
			Graphics::PerspectiveCamera Camera;
		} sceneData;

		struct PostProcessData
		{
			float Saturation = 2.2f;
			float Brightness = 0.45455f;
			// Graphics::D3D_RenderTarget postProcessingRenderTarget = { RenderWindowBase::RenderTargetDefaultSize };
		} postProcessData;

		Gui::FileViewer objFileViewer = { "dev_rom/objset/" };

		int objectIndex = -1;
		UniquePtr<Graphics::ObjSet> objSet = nullptr;

		UniquePtr<Graphics::D3D_Renderer3D> renderer3D;
	};
}