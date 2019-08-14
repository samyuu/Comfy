#pragma once
#include "Task.h"
#include "Editor/RenderWindowBase.h"
#include "Editor/CameraController2D.h"
#include "Graphics/Camera.h"
#include "Graphics/Auth2D/Renderer2D.h"
#include "Graphics/Auth2D/AetRenderer.h"
#include "ImGui/imgui_extensions.h"

namespace App
{
	class EngineRenderWindow : public Editor::RenderWindowBase
	{
	public:
		EngineRenderWindow();
		EngineRenderWindow(const EngineRenderWindow& other) = delete;
		~EngineRenderWindow();

		void OnUpdateInput() override;
		void OnUpdate() override;
		void OnDrawGui() override;
		void OnRender() override;
		void PostDrawGui() override;

		template <class T> void StartTask() 
		{
			static_assert(std::is_convertible<T*, Task*>::value);

			tasks.push_back(MakeRefPtr<T>());
			tasks.back()->Initialize();
		};

	protected:
		void OnResize(int width, int height) override;

	protected:
		UniquePtr<Graphics::Auth2D::Renderer2D> renderer;
		UniquePtr<Graphics::Auth2D::AetRenderer> aetRenderer;

		Graphics::OrthographicCamera camera;
		Editor::CameraController2D cameraController;

		std::vector<RefPtr<Task>> tasks;

		ImGuiWindow* guiWindow;
	};

	class Engine
	{
	public:
		Engine();
		Engine(const Engine& other) = delete;
		~Engine();

		void Tick();

	protected:
		EngineRenderWindow engineWindow;
	};
}