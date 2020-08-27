#include "TestTask.h"

namespace Comfy::Sandbox::Tests
{
	class Renderer2DTest : public ITestTask
	{
	public:
		Renderer2DTest()
		{
			// renderWindow.SetKeepAspectRatio(true);
			testCommand.SourceRegion = vec4(0, 0, 2048, 2048);

			renderWindow.OnRenderCallback = [&]
			{
				renderWindow.RenderTarget->Param.Resolution = camera.ProjectionSize = renderWindow.GetRenderRegion().GetSize();

				renderer.Begin(camera, *renderWindow.RenderTarget);
				{
					testCommand.TexView = (sprSet == nullptr || !InBounds(textureIndex, sprSet->TexSet.Textures)) ? nullptr : sprSet->TexSet.Textures[textureIndex].get();
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
					GuiProperty::Input("MSAA", renderWindow.RenderTarget->Param.MultiSampleCount);
				}
				Gui::End();

				if (Gui::Begin("Texture Selection") && sprSet != nullptr)
				{
					int texIndex = 0;
					for (auto& tex : sprSet->TexSet.Textures)
					{
						if (Gui::Selectable(tex->GetName().data(), (texIndex == textureIndex)))
							textureIndex = texIndex;

						if (Gui::IsItemHovered())
						{
							Gui::BeginTooltip();

							constexpr float targetSize = 512.0f;
							const auto texSize = vec2(tex->GetSize());

							const auto imageSize = (texSize.x > texSize.y) ?
								vec2(targetSize, targetSize * (texSize.y / texSize.x)) :
								vec2(targetSize * (texSize.x / texSize.y), targetSize);

							Gui::Image(*tex, imageSize, Gui::UV0_R, Gui::UV1_R);
							Gui::EndTooltip();
						}

						texIndex++;
					}
				}
				Gui::End();

				if (Gui::Begin("Test SprSet Loader"))
				{
					if (sprFileViewer.DrawGui())
					{
						const auto fileName = IO::Path::GetFileName(sprFileViewer.GetFileToOpen());
						const auto extension = IO::Path::GetExtension(fileName);

						if (IO::Path::DoesAnyPackedExtensionMatch(extension, ".bin;.spr"))
							sprSet = IO::File::Load<Graphics::SprSet>(sprFileViewer.GetFileToOpen());
					}
				}
				Gui::End();
			}
		}

	private:
		Render::Renderer2D renderer = {};
		Render::Camera2D camera = {};

		CallbackRenderWindow2D renderWindow = {};
		bool fullscreen = false;

		Gui::FileViewer sprFileViewer = { "dev_ram/sprset" };
		std::unique_ptr<Graphics::SprSet> sprSet = IO::File::Load<Graphics::SprSet>("dev_ram/sprset/spr_fnt/spr_fnt_36.bin");
		int textureIndex = 0;

		Render::RenderCommand2D testCommand;
	};
}
