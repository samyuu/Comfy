#include "Engine.h"
#include "TestTasks.h"

namespace App
{
	EngineRenderWindow::EngineRenderWindow()
	{
		renderer = MakeUnique<Auth2D::Renderer2D>();
		renderer->Initialize();
		aetRenderer = MakeUnique<Auth2D::AetRenderer>(renderer.get());

		StartTask<TaskPs4Menu>();
	}

	EngineRenderWindow::~EngineRenderWindow()
	{
	}

	void EngineRenderWindow::OnUpdateInput()
	{
	}

	void EngineRenderWindow::OnUpdate()
	{
		cameraController.Update(camera, GetRelativeMouse());
		
		for (auto& task : tasks)
			task->Update();
	}

	void EngineRenderWindow::OnDrawGui()
	{
		for (auto& task : tasks)
			task->PreDrawGui();
	}

	void EngineRenderWindow::OnRender()
	{
		renderTarget.Bind();
		{
			RenderCommand::SetViewport(renderTarget.GetSize());

			vec4 backgroundColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
			RenderCommand::SetClearColor(backgroundColor);
			RenderCommand::Clear(ClearTarget_ColorBuffer);

			camera.UpdateMatrices();

			renderer->Begin(camera);
			for (auto& task : tasks)
				task->Render(renderer.get(), aetRenderer.get());
			renderer->End();
		}
		renderTarget.UnBind();
	}

	void EngineRenderWindow::PostDrawGui()
	{
		for (auto& task : tasks)
			task->PostDrawGui();
	}

	void EngineRenderWindow::OnResize(int width, int height)
	{
		RenderWindowBase::OnResize(width, height);
		camera.ProjectionSize = vec2(width, height);
	}

	Engine::Engine()
	{
		engineWindow.Initialize();
	}

	Engine::~Engine()
	{
	}

	void Engine::Tick()
	{
		engineWindow.DrawGui();
	}
}