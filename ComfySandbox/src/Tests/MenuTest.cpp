#include "TestTask.h"
#include "Game/Core/GameStateManager.h"

namespace Comfy::Sandbox::Tests
{
	class MenuTest : public ITestTask
	{
	public:
		COMFY_REGISTER_TEST_TASK(MenuTest);

		MenuTest()
		{
			renderWindow.OnRenderCallback = [&]
			{
				renderWindow.RenderTarget->Param.Resolution = camera.ProjectionSize = renderWindow.GetRenderRegion().GetSize();
				camera.CenterAndZoomToFit(context.VirtualResolution);

				context.Renderer.Begin(camera, *renderWindow.RenderTarget);
				{
					if (context.AetPS4MenuMain != nullptr && context.SprPS4Menu != nullptr)
						gameStateManager->Tick();
				}
				context.Renderer.End();
			};
		}

		void Update() override
		{
			const auto stopwatchElapsed = stopwatch.Restart();
			context.Elapsed = (UseFixedMenuTimeStep ? TimeSpan::FromFrames(1.0f, FixedMenuTimeStepFPS) : stopwatchElapsed) * MenuTimeFactor;

			if (StepSingleFrame)
			{
				context.Elapsed = TimeSpan::FromFrames(1.0f, FixedMenuTimeStepFPS);
				StepSingleFrame = false;
			}

			context.VirtualResolution = (context.AetPS4MenuMain != nullptr) ? context.AetPS4MenuMain->Resolution : ivec2(1920, 1080);

			if (Gui::IsKeyPressed(Input::KeyCode_F11, false))
				fullscreen ^= true;

			if (Gui::IsKeyPressed(Input::KeyCode_F5))
				gameStateManager = std::make_unique<Game::GameStateManager>(context);

			if (fullscreen)
			{
				renderWindow.BeginEndGuiFullScreen("Menu Test##FullSceen");
			}
			else
			{
				renderWindow.BeginEndGui("Menu Test##Windowed");

				if (Gui::Begin("Debug Control"))
				{
					if (Gui::Button("Reset State Manager", vec2(Gui::GetContentRegionAvailWidth(), 0.0f)))
						gameStateManager = std::make_unique<Game::GameStateManager>(context);

					Gui::Separator();
					Gui::SliderFloat("Time Factor", &MenuTimeFactor, 0.0f, 4.0f);
					Gui::Separator();
					Gui::Checkbox("Use Fixed Time Step", &UseFixedMenuTimeStep);
					Gui::SliderFloat("Fixed Time Step", &FixedMenuTimeStepFPS, 1.0f, 300.0f, "%.3f FPS");
					if (Gui::Button("Step Single Frame", vec2(Gui::GetContentRegionAvailWidth(), 0.0f)))
						StepSingleFrame = true;

					Gui::Separator();
					gameStateManager->DebugGui();
					Gui::End();
				}
			}
		}

	private:
		bool fullscreen = false;
		CallbackRenderWindow2D renderWindow = {};

		Render::Camera2D camera = {};

		float MenuTimeFactor = 1.0f;

		bool UseFixedMenuTimeStep = false;
		frame_t FixedMenuTimeStepFPS = 60.0f;
		bool StepSingleFrame = false;

		Stopwatch stopwatch;
		Game::GameContext context;

		std::unique_ptr<Game::GameStateManager> gameStateManager = std::make_unique<Game::GameStateManager>(context);
	};
}
