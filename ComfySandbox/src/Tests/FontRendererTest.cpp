#include "TestTask.h"
#include "Misc/ImageHelper.h"
#include "Time/TimeUtilities.h"
#include <future>

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

	class TextOfflineRenderer
	{
	public:
		void RenderTakeScreenshot(const Graphics::BitmapFont& font, std::string_view text, std::string filePath, bool border = true)
		{
			constexpr float padding = 16.0f;
			const auto measuredSize = renderer.Font().Measure(font, text) + vec2(padding);

			camera.ProjectionSize = measuredSize;

			renderTarget->Param.Resolution = measuredSize;
			renderTarget->Param.ClearColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);

			renderer.Begin(camera, *renderTarget);
			{
				renderer.Font().SetBlendMode(Graphics::AetBlendMode::Unknown);

				if (border)
					renderer.Font().DrawBorder(font, text, Graphics::Transform2D(vec2(padding / 2.0f)));
				else
					renderer.Font().Draw(font, text, Graphics::Transform2D(vec2(padding / 2.0f)));
			}
			renderer.End();

			auto pixelData = renderTarget->TakeScreenshot();

#if 1
			Util::WriteImage(filePath, renderTarget->Param.Resolution, pixelData.get());
#else
			for (const auto& future : saveImageFutures)
				future.wait();
			saveImageFutures.clear();

			saveImageFutures.push_back(std::async(std::launch::async, [this, prefix = std::string(fileNamePrefix), data = std::move(pixelData)]
				{
					const auto filePath = IO::Path::Combine("dev_ram/ss/font/offline_render", IO::Path::ChangeExtension(prefix + FormatFileNameDateTimeNow(), ".png"));
					Util::WritePNG(filePath, renderTarget->Param.Resolution, data.get());
				}));
#endif
		}

	private:
		std::vector<std::future<void>> saveImageFutures;

		Render::OrthographicCamera camera;

		Render::Renderer2D renderer;
		std::unique_ptr<Render::RenderTarget2D> renderTarget = Render::Renderer2D::CreateRenderTarget();
	};

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
							if (render.VisualizeBounds)
							{
								auto command = Render::RenderCommand2D(nullptr, render.Transform.Origin, render.Transform.Position, render.Transform.Rotation, render.Transform.Scale, vec4(0.0f, 0.0f, measuredTextSize), Graphics::AetBlendMode::Normal, 1.0f);
								command.SetColor(vec4(0.15f, 0.15f, 0.15f, 0.5f));
								renderer.Draw(command);
							}

							if (render.VisualizeOrigin)
							{
								auto command = Render::RenderCommand2D(render.Transform.Position, vec2(6.0f), vec4(0.35f, 0.79f, 0.69f, 0.75f));
								command.Origin = vec2(command.SourceRegion.z, command.SourceRegion.w) * 0.5f;
								command.Rotation = render.Transform.Rotation;
								renderer.Draw(command);
							}

							renderer.Font().SetBlendMode(render.BlendMode);

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

				if (Gui::Begin("Offline Render Test", nullptr, ImGuiWindowFlags_NoSavedSettings))
				{
					static TextOfflineRenderer offlineRenderer;
					static char textBuffer[255] = "YEP COCK";

					const auto selecedFont = GetSelectedFont();
					if (selecedFont == nullptr)
						return;

					Gui::InputText("Text", textBuffer, sizeof(textBuffer));
					if (Gui::Button("YEP COCK!"))
						offlineRenderer.RenderTakeScreenshot(*selecedFont, textBuffer, ("dev_ram/ss/font/offline_render/" + FormatFileNameDateTimeNow() + ".png"));
					Gui::Separator();

					static char songNameBuffer[255] = u8"SLoWMoTIoN", creatorBuffer[255] = u8"作詞・作曲：ピノキオピー";
					Gui::InputText("Song Name", songNameBuffer, sizeof(songNameBuffer));
					Gui::InputText("Creator", creatorBuffer, sizeof(creatorBuffer));

					if (Gui::Button("Render!"))
					{
						offlineRenderer.RenderTakeScreenshot(*selecedFont, songNameBuffer, ("dev_ram/ss/font/offline_render/song_" + FormatFileNameDateTimeNow() + ".png"));
						offlineRenderer.RenderTakeScreenshot(*selecedFont, creatorBuffer, ("dev_ram/ss/font/offline_render/creator_" + FormatFileNameDateTimeNow() + ".png"));
					}

					Gui::Separator();
					if (Gui::Button("Render All X Titles"))
					{
						struct SongInfo
						{
							i32 ID, SubIndex;
							std::string_view Title;
							std::array<std::string_view, 3> Info;
						};

						static constexpr std::array allPVs =
						{
							SongInfo { 801, -1, u8"Strangers", { u8"作詞・作曲：Heavenz" } },
							SongInfo { 802, -1, u8"愛Dee", { u8"作詞：Cotori・Mitchie M", u8"作曲：Mitchie M" } },
							SongInfo { 803, -1, u8"ストリーミングハート", { u8"作詞・作曲：DECO*27", u8"編曲：DECO*27・kous" } },
							SongInfo { 804, -1, u8"バビロン", { u8"作詞・作曲：トーマ" } },
							SongInfo { 805, -1, u8"ロストワンの号哭", { u8"作詞・作曲：Neru" } },
							SongInfo { 806, -1, u8"すろぉもぉしょん", { u8"作詞・作曲：ピノキオピー" } },
							SongInfo { 807, -1, u8"ウミユリ海底譚", { u8"作詞・作曲：n-buna" } },
							SongInfo { 808, -1, u8"恋愛裁判", { u8"作詞・作曲：40mP" } },
							SongInfo { 809, -1, u8"愛の詩", { u8"作詞・作曲：ラマーズP" } },
							SongInfo { 810, -1, u8"ハジメテノオト", { u8"作詞・作曲：malo" } },
							SongInfo { 811, -1, u8"LOL -lots of laugh-", { u8"作詞：エンドケイプ", u8"作曲：KeN" } },
							SongInfo { 812, -1, u8"ツギハギスタッカート", { u8"作詞・作曲：とあ" } },
							SongInfo { 813, -1, u8"クノイチでも恋がしたい", { u8"作詞・作曲：みきとP" } },
							SongInfo { 814, -1, u8"Calc.", { u8"作詞・作曲：ジミーサムP" } },
							SongInfo { 815, -1, u8"紅一葉", { u8"作詞・作曲：黒うさ" } },
							SongInfo { 816, -1, u8"聖槍爆裂ボーイ", { u8"作詞・作曲：れるりり・もじゃ" } },
							SongInfo { 817, -1, u8"卑怯戦隊うろたんだー", { u8"作詞・作曲：シンP" } },
							SongInfo { 818, -1, u8"Mrs.Pumpkinの滑稽な夢", { u8"作詞・作曲：ハチ" } },
							SongInfo { 819, -1, u8"独りんぼエンヴィー", { u8"作詞・作曲：koyori" } },
							SongInfo { 820, -1, u8"ラズベリー＊モンスター", { u8"作詞：Gom&shito", u8"作曲：Gom", u8"編曲：HoneyWorks" } },
							SongInfo { 821, -1, u8"脳内革命ガール", { u8"作詞・作曲：MARETU" } },
							SongInfo { 822, -1, u8"Amazing Dolce", { u8"作詞・作曲：ひとしずくP", u8"編曲：やま△" } },
							SongInfo { 823, -1, u8"罪の名前", { u8"作詞・作曲：ryo (supercell)" } },
							SongInfo { 824, -1, u8"Satisfaction", { u8"作詞・作曲：kz" } },
							SongInfo { 831, -1, u8"Sharing The World", { u8"作詞・作曲：BIGHEAD", u8"編曲：BIGHEAD" } },
							SongInfo { 832, -1, u8"Hand in Hand", { u8"作詞・作曲：kz" } },

							// キュート・メドレー ～アイドル サウンズ～
							SongInfo { 825, 0, u8"イージーデンス", { u8"作詞：ЯIRE", u8"作曲：Mitchie M" } },
							SongInfo { 825, 1, u8"FREELY TOMORROW", { u8"作詞：Mitchie M/ЯIRE", u8"作曲：Mitchie M" } },
							SongInfo { 825, 2, u8"ビバハピ", { u8"作詞・作曲：Mitchie M" } },
							SongInfo { 825, 3, u8"アゲアゲアゲイン", { u8"作詞・作曲：Mitchie M" } },
							SongInfo { 825, 3, u8"アイドルを咲かせ", { u8"作詞・作曲：Mitchie M" } },

							// 始まりのメドレー 〜プライマリーカラーズ〜
							SongInfo { 826, 0, u8"恋スルVOC@LOID", { u8"作詞・作曲：OSTER project" } },
							SongInfo { 826, 1, u8"Dreaming Leaf -ユメミルコトノハ-", { u8"作詞・作曲：OSTER project" } },
							SongInfo { 826, 2, u8"フキゲンワルツ", { u8"作詞・作曲：OSTER project" } },
							SongInfo { 826, 3, u8"ミラクルペイント", { u8"作詞・作曲：OSTER project" } },

							// クール・メドレー 〜サイバーロックジャム〜
							SongInfo { 827, 0, u8"アンハッピーリフレイン", { u8"作詞・作曲：wowaka" } },
							SongInfo { 827, 1, u8"マイリスダメー！", { u8"作詞・作曲：ライブP" } },
							SongInfo { 827, 2, u8"天樂", { u8"作詞・作曲：ゆうゆ" } },
							SongInfo { 827, 3, u8"Palette", { u8"作詞・作曲：ゆよゆっぺ" } },
							SongInfo { 827, 4, u8"このふざけた素晴らしき世界は、僕の為にある", { u8"作詞・作曲：n.k" } },


							// ビューティ・メドレー 〜Glossy Mixture〜
							SongInfo { 828, 0, u8"Dependence Intension", { u8"作詞：NaturaLe", u8"作詞：Treow"} },
							SongInfo { 828, 1, u8"Sweet Devil", { u8"作詞：q*Left", u8"作詞：八王子P"} },
							SongInfo { 828, 2, u8"Nebula", { u8"作詞・作曲：Tripshots"} },
							SongInfo { 828, 3, u8"Chaining Intention", { u8"作詞：NaturaLe", u8"作詞：Treow"} },

							// カオスメドレー 〜ギガリミックス〜
							SongInfo { 829, 0, u8"ぴんこすてぃっくLuv", { u8"作詞：れをる", u8"作曲：ギガＰ" } },
							SongInfo { 829, 1, u8"ギガンティックO.T.N", { u8"作詞：れをる", u8"作曲：ギガＰ" } },
							SongInfo { 829, 2, u8"おこちゃま戦争", { u8"作詞：れをる", u8"作曲：ギガＰ" } },
							SongInfo { 829, 3, u8"いーあるふぁんくらぶ", { u8"作詞・作曲：みことＰ" } },

							// 終極のメドレー ～超絶技巧暴走組曲～
							SongInfo { 830, 0, u8"初音ミクの消失", { u8"作詞・作曲：cosMo@暴走P" } },
							SongInfo { 830, 1, u8"裏表ラバーズ", { u8"作詞・作曲：wowaka" } },
							SongInfo { 830, 2, u8"Sadistic.Music∞Factory", { u8"作詞・作曲：cosMo@暴走P" } },
							SongInfo { 830, 3, u8"二次元ドリームフィーバー", { u8"作詞・作曲：PolyphonicBranch" } },
							SongInfo { 830, 4, u8"初音ミクの激唱", { u8"作詞・作曲：Storyteller" } },
						};

						for (const auto& pv : allPVs)
						{
							const auto indexString = (pv.SubIndex >= 0) ? std::string("_" + std::to_string(pv.SubIndex)) : "";

							offlineRenderer.RenderTakeScreenshot(*selecedFont, pv.Title, "dev_ram/ss/font/offline_render/pv_tit/pv_" + std::to_string(pv.ID) + indexString + "_song.png");

							for (size_t i = 0; i < pv.Info.size() && !pv.Info[i].empty(); i++)
								offlineRenderer.RenderTakeScreenshot(*selecedFont, pv.Info[i], "dev_ram/ss/font/offline_render/pv_tit/pv_" + std::to_string(pv.ID) + indexString + "_txt" + std::to_string(i) + ".png");
						}
					}

					Gui::End();
				}

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

					GuiProperty::Checkbox("Checkerboard Background", render.CheckerboardBackground);
					GuiProperty::Checkbox("Visualize Origin", render.VisualizeOrigin);
					GuiProperty::Checkbox("Visualize Bounds", render.VisualizeBounds);

					GuiProperty::ColorEdit("Clear Color", renderWindow.RenderTarget->Param.ClearColor);
					GuiProperty::Combo("Blend Mode", render.BlendMode, Graphics::AetBlendModeNames);
					if (Gui::Button("Take Screenshot", vec2(Gui::GetContentRegionAvailWidth(), 0.0f)))
					{
						auto pixelData = renderWindow.RenderTarget->TakeScreenshot();

						if (saveScreenshotFuture.valid())
							saveScreenshotFuture.wait();

						saveScreenshotFuture = std::async(std::launch::async, [this, data = std::move(pixelData)]
							{
								const auto filePath = IO::Path::Combine("dev_ram/ss/font", IO::Path::ChangeExtension("scene_" + FormatFileNameDateTimeNow(), ".png"));
								Util::WriteImage(filePath, renderWindow.RenderTarget->Param.Resolution, data.get());
							});
					}
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

		std::future<void> saveScreenshotFuture;

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

			bool NoCameraZoom = true;
			vec2 CameraZoomTargetSize = { 1920.0f, 1080.0f };

			bool CheckerboardBackground = true;
			bool VisualizeOrigin = true;
			bool VisualizeBounds = true;

			Graphics::AetBlendMode BlendMode = Graphics::AetBlendMode::Normal;

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
