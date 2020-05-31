#include "TestTask.h"

namespace Comfy::Sandbox::Tests
{
	static constexpr std::string_view FontTestText = u8R"(Test Lyrics:
「魔女狩り」

編曲、ミックス：塚越雄一朗
作曲、作詞、歌唱：浮森かや子

彼女は今日も種をまく

愛しい人が居りました
彼の人も今は墓の下
わたしを置いて墓の下
まみえることもかなわない

人も世界も色を変え　わたしだけが変わらない
愚かで夢見がちな儘　ふるびた恋慕も捨てられず

迫り来る足音　木炭と錆の匂い
変わらぬ彼女を裁く為　世界が揺れる

花煙る春　草木繋る夏　甘く実る秋　森が眠る冬
世界は急ぎ足　歩めない彼女
人々は叫んだ　“あいつは魔女だ！”

肌に喰い込む鎖
神様の炎に焼かれても
あなたのもとへいけないのですか

罵声の響く広場　燃えさかる広場
我先と火を注ぐ　時代の操り人形
甲高い叫びが　晴れた空に消える
火種がなくなれば　“もう誰も居ない”

二度三度　まばたく　うつろな睫毛の羽音
四度五度　きしんだ　歪な骨の律動
柔らかな灰を退け　白く伸びた腕
彼女の声が鳴いた　“わたしは生きている”

彼女は今日も種をまく
愛しい季節を見送って
変わらぬ彼女の物語
いついつまでも　続くでしょう)";

	class FontRendererTest : public ITestTask
	{
	public:
		COMFY_REGISTER_TEST_TASK(FontRendererTest);

		FontRendererTest()
		{
			renderWindow.OnRenderCallback = [&]
			{
				renderWindow.RenderTarget->Param.Resolution = camera.ProjectionSize = renderWindow.GetRenderRegion().GetSize();

				if (render.NoCameraZoom)
				{
					camera.Zoom = 1.0f;
					camera.Position = {};
				}
				else
				{
					camera.CenterAndZoomToFit(render.CameraZoomTargetSize);
				}

				renderer.Begin(camera, *renderWindow.RenderTarget);
				{
					if (render.CheckerboardBackground)
					{
						constexpr auto color = vec4(0.15f, 0.15f, 0.15f, 1.0f), colorAlt = vec4(0.32f, 0.32f, 0.32f, 1.0f);
						constexpr auto position = vec2(0.0f, 0.0f);
						const auto size = (render.NoCameraZoom) ? vec2(renderWindow.RenderTarget->Param.Resolution) : render.CameraZoomTargetSize;
						const auto precision = renderer.GetCamera().Zoom / 5.0f;

						renderer.Draw(Render::RenderCommand2D(position, size, color));
						renderer.DrawRectCheckerboard(position, vec2(1.0f), vec2(0.0f), 0.0f, size, colorAlt, precision);
					}

					if (const auto selectedFont = GetSelectedFont(); selectedFont != nullptr)
					{
						selectedFont->Texture = GetSelectedTexture();

						const auto textToRender = std::string_view(render.TextBuffer.data());

						const auto measuredTextSize = renderer.Font().Measure(*selectedFont, textToRender);
						const auto positionOffset = vec2(render.RightAlign ? measuredTextSize.x : 0.0f, render.BottomAlign ? measuredTextSize.y : 0.0f);

						render.Transform.Position -= positionOffset;
						{
							if (render.Border)
								renderer.Font().DrawBorder(*selectedFont, textToRender, render.Transform, render.Color);
							else if (render.Shadow)
								renderer.Font().DrawShadow(*selectedFont, textToRender, render.Transform, render.Color, render.ShadowColor, render.ShadowOffset);
							else
								renderer.Font().Draw(*selectedFont, textToRender, render.Transform, render.Color);
						}
						render.Transform.Position += positionOffset;
					}
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
				renderWindow.BeginEndGuiFullScreen("RenderWindow2D Font Test##FullSceen");
			}
			else
			{
				renderWindow.BeginEndGui("RenderWindow2D Font Test");

				if (Gui::Begin("Font Control Test"))
				{
					const auto columns = GuiPropertyRAII::PropertyValueColumns();

					GuiProperty::PropertyLabelValueFunc("Text", [&]
					{
						GuiPropertyRAII::ItemWidth width(-1.0f);
						return Gui::InputTextMultiline(GuiProperty::Detail::DummyLabel, render.TextBuffer.data(), render.TextBufferSize - 1, {}, ImGuiInputTextFlags_AllowTabInput);
					});

					GuiProperty::Input("Origin", render.Transform.Origin);
					GuiProperty::Input("Position", render.Transform.Position);
					GuiProperty::Input("Rotation", render.Transform.Rotation);
					GuiProperty::Input("Scale", render.Transform.Scale, 0.01f);
					GuiProperty::Input("Opacity", render.Transform.Opacity, 0.01f, vec2(0.0f, 1.0f));

					GuiProperty::Checkbox("Border", render.Border);
					GuiProperty::Checkbox("Shadow", render.Shadow);
					GuiProperty::ColorEdit("Color", render.Color);
					GuiProperty::ColorEdit("Shadow Color", render.ShadowColor);
					GuiProperty::Input("Shadow Offset", render.ShadowOffset);

					GuiProperty::Checkbox("Right Align", render.RightAlign);
					GuiProperty::Checkbox("Bottom Align", render.BottomAlign);

					GuiProperty::Checkbox("No Camera Zoom", render.NoCameraZoom);
					GuiProperty::Input("Camera Zoom Target Size", render.CameraZoomTargetSize);
				}
				Gui::End();

				if (Gui::Begin("Test SprSet Loader"))
				{
					if (sprFileViewer.DrawGui() && IO::Path::GetExtension(sprFileViewer.GetFileToOpen()) == ".bin")
						sprSet = IO::File::Load<Graphics::SprSet>(sprFileViewer.GetFileToOpen());
				}
				Gui::End();

				if (Gui::Begin("Test FontMap Loader"))
				{
					if (fontMapFileViewer.DrawGui() && IO::Path::GetExtension(fontMapFileViewer.GetFileToOpen()) == ".bin")
						fontMap = IO::File::Load<Graphics::FontMap>(fontMapFileViewer.GetFileToOpen());
				}
				Gui::End();

				if (Gui::Begin("Test Texture Selection"))
				{
					if (sprSet != nullptr)
					{
						const auto& textures = sprSet->TexSet->Textures;
						char nameBuffer[128];
						selected.TextureHoverIndex = -1;

						for (int i = 0; i < static_cast<int>(textures.size()); i++)
						{
							const auto sprite = FindIfOrNull(sprSet->Sprites, [&](const auto& sprite) { return sprite.TextureIndex == i; });
							sprintf_s(nameBuffer, "%s / %s", textures[i]->GetName().data(), (sprite == nullptr) ? "<None>" : sprite->Name.c_str());

							if (Gui::Selectable(nameBuffer, (selected.TextureIndex == i)))
								selected.TextureIndex = i;

							if (Gui::IsItemHovered())
							{
								selected.TextureHoverIndex = i;

								Gui::BeginTooltip();
								constexpr float targetSize = 512.0f;
								const auto texSize = vec2(textures[i]->GetSize());

								const auto imageSize = (texSize.x > texSize.y) ?
									vec2(targetSize, targetSize * (texSize.y / texSize.x)) :
									vec2(targetSize * (texSize.x / texSize.y), targetSize);

								Gui::Image(*textures[i], imageSize, Gui::UV0_R, Gui::UV1_R);
								Gui::EndTooltip();
							}
						}
					}
					else
					{
						Gui::TextDisabled("<Invalid SprSet>");
					}

					Gui::End();
				}

				if (Gui::Begin("Test Font Selection"))
				{
					if (fontMap != nullptr)
					{
						selected.FontHoverIndex = -1;
						char nameBuffer[128];
						for (int i = 0; i < static_cast<int>(fontMap->Fonts.size()); i++)
						{
							const auto& font = fontMap->Fonts[i];
							const auto fontSize = font.GetFontSize(), glyphSize = font.GetGlyphSize();

							sprintf_s(nameBuffer, "[%d] Font %dx%d %dx%d", i, fontSize.x, fontSize.y, glyphSize.x, glyphSize.y);

							if (Gui::Selectable(nameBuffer, (selected.FontIndex == i)))
								selected.FontIndex = i;

							if (Gui::IsItemHovered())
								selected.FontHoverIndex = i;
						}
					}
					else
					{
						Gui::TextDisabled("<Invalid FontMap>");
					}
				}
				Gui::End();
			}
		}

	private:
		std::shared_ptr<Graphics::Tex> GetSelectedTexture() const
		{
			if (sprSet == nullptr)
				return nullptr;

			if (InBounds(selected.TextureHoverIndex, sprSet->TexSet->Textures))
				return sprSet->TexSet->Textures[selected.TextureHoverIndex];
			else
				return InBounds(selected.TextureIndex, sprSet->TexSet->Textures) ? sprSet->TexSet->Textures[selected.TextureIndex] : nullptr;
		}

		Graphics::BitmapFont* GetSelectedFont() const
		{
			if (fontMap == nullptr)
				return nullptr;

			if (InBounds(selected.FontHoverIndex, fontMap->Fonts))
				return &fontMap->Fonts[selected.FontHoverIndex];
			else
				return IndexOrNull(selected.FontIndex, fontMap->Fonts);
		}

	private:
		Render::Renderer2D renderer = {};
		Render::OrthographicCamera camera = {};

		CallbackRenderWindow2D renderWindow = {};
		bool fullscreen = false;

		Gui::FileViewer sprFileViewer = { "dev_ram/sprset/spr_fnt/" };
		Gui::FileViewer fontMapFileViewer = { "dev_ram/font/fontmap/" };

		std::unique_ptr<Graphics::SprSet> sprSet = IO::File::Load<Graphics::SprSet>("dev_ram/sprset/spr_fnt/spr_fnt_36.bin");
		std::unique_ptr<Graphics::FontMap> fontMap = IO::File::Load<Graphics::FontMap>("dev_ram/font/fontmap/fontmap.bin");

		struct RenderData
		{
			static constexpr size_t TextBufferSize = (8192);
			std::array<char, TextBufferSize> TextBuffer = []
			{
				constexpr auto testSize = FontTestText.size();
				static_assert(TextBufferSize > FontTestText.size());

				std::array<char, TextBufferSize> result = {};
				std::memcpy(result.data(), FontTestText.data(), FontTestText.size());
				return result;
			}();

			const char Null = '\0';

			Graphics::Transform2D Transform = { vec2(0.0f, 0.0f) };

			bool Border = true;
			bool Shadow = false;

			vec3 Color = Render::FontRenderer::DefaultColor;

			vec4 ShadowColor = Render::FontRenderer::DefaultShadowColor;
			vec2 ShadowOffset = Render::FontRenderer::DefaultShadowOffset;

			bool RightAlign = false;
			bool BottomAlign = false;
			bool CheckerboardBackground = true;

			bool NoCameraZoom = true;
			vec2 CameraZoomTargetSize = { 1920.0f, 1080.0f };

		} render;

		struct SelectedData
		{
			int TextureIndex = 0;
			int FontIndex = 3;

			int TextureHoverIndex = -1;
			int FontHoverIndex = -1;
		} selected;
	};
}
