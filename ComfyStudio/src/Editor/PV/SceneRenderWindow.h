#pragma once
#include "SceneGraph.h"
#include "Editor/Core/RenderWindowBase.h"
#include "Graphics/Direct3D/D3D_Renderer3D.h"
#include "Editor/Common/CameraController3D.h"

namespace Editor
{
	class SceneRenderWindow : public RenderWindowBase
	{
	public:
		SceneRenderWindow(SceneGraph& sceneGraph, Graphics::SceneContext& context, CameraController3D& cameraController, Graphics::D3D_Renderer3D& renderer3D);
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
		Graphics::D3D_RenderTarget* GetExternalRenderTarget() override;

	private:
		bool drawCameraAxisIndicator = true;
		
		SceneGraph* sceneGraph;

		Graphics::SceneContext* context = nullptr;
		CameraController3D* cameraController = nullptr;

		Graphics::D3D_Renderer3D* renderer3D = nullptr;
	};
}