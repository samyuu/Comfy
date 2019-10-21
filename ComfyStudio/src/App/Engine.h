#pragma once
#include "Task.h"
#include "Editor/Core/RenderWindowBase.h"
#include "Editor/Common/CameraController2D.h"
#include "Graphics/Camera.h"
#include "Graphics/Auth2D/Renderer2D.h"
#include "Graphics/Auth2D/AetRenderer.h"
#include "ImGui/Gui.h"

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
			static_assert(std::is_base_of<Task, T>::value, "T must inherit from Task");

			tasks.push_back(MakeRef<T>());
			tasks.back()->Initialize();
		};

	protected:
		void OnResize(ivec2 size) override;

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
		Engine(const Engine&) = delete;
		Engine& operator= (const Engine&) = delete;
		~Engine();

		void Tick();

	protected:
		EngineRenderWindow engineWindow;
	};
}