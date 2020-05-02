#include "Engine.h"
#include "TestTasks.h"

namespace Comfy::App
{
	EngineRenderWindow::EngineRenderWindow()
	{
		renderer = std::make_unique<Graphics::D3D11::Renderer2D>();
		aetRenderer = std::make_unique<Graphics::Aet::AetRenderer>(renderer.get());

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
		owningRenderTarget->BindSetViewport();
		{
			const vec4 backgroundColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
			owningRenderTarget->Clear(backgroundColor);

			camera.UpdateMatrices();

			renderer->Begin(camera);
			for (auto& task : tasks)
				task->Render(renderer.get(), aetRenderer.get());
			renderer->End();
		}
		owningRenderTarget->UnBind();
	}

	void EngineRenderWindow::PostDrawGui()
	{
		for (auto& task : tasks)
			task->PostDrawGui();
	}

	void EngineRenderWindow::OnResize(ivec2 size)
	{
		RenderWindowBase::OnResize(size);
		camera.ProjectionSize = vec2(size);
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
