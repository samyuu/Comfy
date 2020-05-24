#pragma once
#include "SceneGraph.h"
#include "Window/RenderWindow.h"
#include "Editor/Common/CameraController3D.h"

namespace Comfy::Studio::Editor
{
	class SceneRenderWindow : public RenderWindow
	{
	public:
		SceneRenderWindow(SceneGraph& sceneGraph, Render::PerspectiveCamera& camera, Render::Renderer3D& renderer, Render::SceneParam3D& sceneParam, CameraController3D& cameraController);
		~SceneRenderWindow() = default;

	public:
		ImTextureID GetTextureID() const override;

		// NOTE: Publicly accessible to allow for potential offline renders
		void RenderScene();

		Render::RenderTarget3D* GetRenderTarget();

	protected:
		ImGuiWindowFlags GetRenderTextureChildWindowFlags() const;
		void OnFirstFrame() override;
		void PreRenderTextureGui() override;
		void PostRenderTextureGui() override;
		void OnResize(ivec2 newSize) override;
		void OnRender() override;

	private:
		void UpdateInputRayTest();

	private:
		bool drawCameraAxisIndicator = true;

		std::unique_ptr<Render::RenderTarget3D> renderTarget = nullptr;

		SceneGraph& sceneGraph;

		Render::PerspectiveCamera& camera;
		Render::Renderer3D& renderer;
		Render::SceneParam3D& sceneParam;

		CameraController3D& cameraController;
	};
}
