#pragma once
#include "SceneGraph.h"
#include "Window/RenderWindow.h"
#include "Editor/Common/CameraController3D.h"
#include <optional>

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

		struct RayPickResult
		{
			ObjectEntity* Entity;
			const Graphics::Mesh* Mesh;
			const Graphics::SubMesh* SubMesh;
			float Distance;
		};

		RayPickResult RayPickSceneRay(vec3 viewPoint, vec3 ray, float nearPlane) const;
		RayPickResult RayPickSceneMouse(vec2 relativeMousePosition) const;

	public:
		i64 GetLastFocusedFrameCount() const;
		bool GetRequestsDuplication() const;

		std::optional<RayPickResult> GetRayPickRequest() const;

	protected:
		ImGuiWindowFlags GetRenderTextureChildWindowFlags() const;
		void PreBeginWindow() override;
		void OnFirstFrame() override;
		void PreRenderTextureGui() override;
		void PostRenderTextureGui() override;
		void OnResize(ivec2 newSize) override;
		void OnRender() override;

	private:
		void UpdateInputRayTest();

	private:
		bool drawCameraAxisIndicator = true;
		
		bool hasInputFocus = false;
		bool requestsDuplication = false;
		i64 lastFocusedFrameCount = 0;

		std::optional<RayPickResult> rayPickRequest = {};

		std::unique_ptr<Render::RenderTarget3D> renderTarget = nullptr;

		SceneGraph& sceneGraph;

		Render::PerspectiveCamera& camera;
		Render::Renderer3D& renderer;
		Render::SceneParam3D& sceneParam;

		CameraController3D& cameraController;
	};
}
