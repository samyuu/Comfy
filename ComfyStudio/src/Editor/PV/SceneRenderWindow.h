#pragma once
#include "SceneGraph.h"
#include "Editor/Core/RenderWindowBase.h"
#include "Editor/Common/CameraController3D.h"
#include "Graphics/GPU/GPURenderers.h"

namespace Comfy::Studio::Editor
{
	class SceneRenderWindow : public RenderWindowBase
	{
	public:
		SceneRenderWindow(SceneGraph& sceneGraph, Graphics::SceneViewport& viewport, Graphics::SceneParameters& scene, CameraController3D& cameraController, Graphics::GPU_Renderer3D& renderer3D);
		~SceneRenderWindow() = default;

	public:
		void DrawGui();
		void PostDrawGui() override;
		
	public:
		// NOTE: Publicly accessible to allow for potential offline renders
		void RenderScene();

	private:
		void OnUpdateInput() override;
		void OnUpdate() override;
		void OnRender() override;
		void OnResize(ivec2 size) override;
		Graphics::GPU_RenderTarget* GetExternalRenderTarget() override;

	private:
		bool drawCameraAxisIndicator = true;
		
		SceneGraph* sceneGraph;

		Graphics::SceneViewport* viewport = nullptr;
		Graphics::SceneParameters* scene = nullptr;

		CameraController3D* cameraController = nullptr;

		Graphics::GPU_Renderer3D* renderer3D = nullptr;
	};
}
