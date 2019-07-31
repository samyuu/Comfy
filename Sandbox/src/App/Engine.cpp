#include "Engine.h"
#include "TestTasks.h"

namespace App
{
	EngineRenderWindow::EngineRenderWindow()
	{
		renderer.Initialize();

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

	void EngineRenderWindow::OnRender()
	{
		renderTarget.Bind();
		{
			GLCall(glViewport(0, 0, static_cast<GLint>(renderTarget.GetWidth()), static_cast<GLint>(renderTarget.GetHeight())));

			vec4 backgroundColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
			GLCall(glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, backgroundColor.w));
			GLCall(glClear(GL_COLOR_BUFFER_BIT));

			camera.UpdateMatrices();

			renderer.Begin(camera);
			for (auto& task : tasks)
				task->Render(renderer);
			renderer.End();
		}
		renderTarget.UnBind();
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