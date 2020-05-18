#include "TestTask.h"

namespace Comfy::Sandbox::Tests
{
	class Renderer2DTest : public ITestTask
	{
	public:
		COMFY_REGISTER_TEST_TASK(Renderer2DTest);

		Renderer2DTest()
		{
			renderWindow.SetKeepAspectRatio(true);
			testCommand.SourceRegion = vec4(0, 0, 2048, 2048);

			renderWindow.OnRenderDebugFunc = [&]
			{
				camera.ProjectionSize = renderWindow.GetRenderRegion().GetSize();

				renderer.Begin(camera);
				{
					testCommand.Texture = (sprSet == nullptr || !InBounds(textureIndex, sprSet->TexSet->Textures)) ? nullptr : sprSet->TexSet->Textures[textureIndex].get();
					renderer.Draw(testCommand);
				}
				renderer.End();
			};
		}

		void Update() override
		{
			if (Gui::IsKeyPressed(Input::KeyCode_F11))
				fullscreen ^= true;

			if (fullscreen)
			{
				renderWindow.BeginEndGuiFullScreen("RenderWindow2D Test##FullSceen");
			}
			else
			{
				renderWindow.BeginEndGui("RenderWindow2D Test");

				if (Gui::Begin("RenderCommand2D Test"))
				{
					auto columns = GuiPropertyRAII::PropertyValueColumns();
					GuiProperty::Input("Texture Index", textureIndex);
					GuiProperty::Input("Origin", testCommand.Origin);
					GuiProperty::Input("Position", testCommand.Position);
					GuiProperty::Input("Rotation", testCommand.Rotation);
					GuiProperty::Input("Scale", testCommand.Scale, 0.05f);
					GuiProperty::Input("SourceRegion", testCommand.SourceRegion);
					GuiProperty::ColorEdit("CornerColors[0]", testCommand.CornerColors[0]);
					GuiProperty::ColorEdit("CornerColors[1]", testCommand.CornerColors[1]);
					GuiProperty::ColorEdit("CornerColors[2]", testCommand.CornerColors[2]);
					GuiProperty::ColorEdit("CornerColors[3]", testCommand.CornerColors[3]);
					GuiProperty::Checkbox("Draw Text Border", testCommand.DrawTextBorder);
				}
				Gui::End();

				if (Gui::Begin("Test SprSet Loader"))
				{
					if (sprFileViewer.DrawGui() && IO::Path::GetExtension(sprFileViewer.GetFileToOpen()) == ".bin")
						sprSet = IO::File::Load<Graphics::SprSet>(sprFileViewer.GetFileToOpen());
				}
				Gui::End();
			}
		}

	private:
		Render::Renderer2D renderer = {};

		Render::OrthographicCamera camera = {};

		Comfy::RenderWindow2D renderWindow = {};
		bool fullscreen = false;

		Gui::FileViewer sprFileViewer = { "dev_ram/sprset" };
		std::unique_ptr<Graphics::SprSet> sprSet = IO::File::Load<Graphics::SprSet>("dev_ram/sprset/spr_fnt/spr_fnt_36.bin");
		int textureIndex = 0;

		Render::RenderCommand2D testCommand;
	};
}
