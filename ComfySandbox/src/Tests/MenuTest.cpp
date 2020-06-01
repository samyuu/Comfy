#include "TestTask.h"
#include "Game/GameContext.h"
#include "Game/PS4Menu.h"

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
				camera.CenterAndZoomToFit((context.AetPS4MenuMain != nullptr) ? context.AetPS4MenuMain->Resolution : ivec2(1920, 1080));

				context.Renderer.Begin(camera, *renderWindow.RenderTarget);
				{
					if (context.AetPS4MenuMain != nullptr && context.SprPS4Menu != nullptr)
						ps4Menu->Tick();
				}
				context.Renderer.End();
			};
		}

		void Update() override
		{
			context.Elapsed = (stopwatch.Restart() * MenuTimeFactor);

			if (Gui::IsKeyPressed(Input::KeyCode_F11, false))
				fullscreen ^= true;

			if (Gui::IsKeyPressed(Input::KeyCode_Escape))
				ps4Menu = std::make_unique<Game::PS4Menu>(context);

			if (fullscreen)
			{
				renderWindow.BeginEndGuiFullScreen("Menu Test##FullSceen");
			}
			else
			{
				renderWindow.BeginEndGui("Menu Test##Windowed");

				if (Gui::Begin("Menu Test Control"))
				{
					if (Gui::Button("Reset", vec2(Gui::GetContentRegionAvailWidth(), 0.0f)))
						ps4Menu = std::make_unique<Game::PS4Menu>(context);

					Gui::SliderFloat("Time Factor", &MenuTimeFactor, 0.0f, 4.0f);

					Gui::End();
				}
			}
		}

	private:
		bool fullscreen = false;
		CallbackRenderWindow2D renderWindow = {};

		Render::OrthographicCamera camera = {};

		float MenuTimeFactor = 1.0f;
		Stopwatch stopwatch;
		Game::GameContext context;

		std::unique_ptr<Game::PS4Menu> ps4Menu = std::make_unique<Game::PS4Menu>(context);
	};
}
