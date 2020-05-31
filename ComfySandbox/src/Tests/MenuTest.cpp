#include "TestTask.h"

namespace Comfy::Sandbox::Tests
{
	using namespace Graphics;

	struct MenuContext
	{
		TimeSpan Elapsed = TimeSpan::Zero();
		Render::Renderer2D Renderer = {};

		std::unique_ptr<SprSet> SprPS4Menu = IO::File::Load<SprSet>("dev_ram/sprset/spr_ps4/spr_ps4_menu.bin");

		std::unique_ptr<Aet::AetSet> PS4Menu = IO::File::Load<Aet::AetSet>("dev_ram/aetset/aet_ps4/aet_ps4_menu.bin");
		std::shared_ptr<Aet::Scene> PS4MenuMain = PS4Menu->GetScenes().front();

		std::array<std::unique_ptr<SprSet>*, 1> AllSprSets
		{
			&SprPS4Menu,
		};

		std::unique_ptr<Graphics::FontMap> FontMap = []
		{
			auto fontMap = IO::File::Load<Graphics::FontMap>("dev_ram/font/fontmap/fontmap.bin");
			if (fontMap != nullptr)
			{
				auto sprSet36 = IO::File::Load<Graphics::SprSet>("dev_ram/sprset/spr_fnt/spr_fnt_36.bin");

				const auto font36 = std::find_if(fontMap->Fonts.begin(), fontMap->Fonts.end(), [&](const auto& font) { return (font.GetFontSize() == ivec2(36)); });
				if (font36 != fontMap->Fonts.end() && sprSet36 != nullptr)
					font36->Texture = sprSet36->TexSet->Textures[0];
			}
			return fontMap;
		}();

		const Graphics::BitmapFont* Font36 = (FontMap == nullptr) ? nullptr : FontMap->FindFont(ivec2(36));
	};

	namespace
	{
		// constexpr frame_t ToFrame(TimeSpan time, frame_t frameRate = 60.0f) { return static_cast<frame_t>(time.TotalSeconds() / frameRate); }

		enum LoopState { LoopState_In, LoopState_Loop, LoopState_Out, LoopState_Count };

		static constexpr std::array<std::string_view, LoopState_Count> LoopStartMarkerNames
		{
			"st_in",
			"st_lp",
			"st_out",
		};

		static constexpr std::array<std::string_view, LoopState_Count> LoopEndMarkerNames
		{
			"ed_in",
			"ed_lp",
			"ed_out",
		};

		frame_t LoopFrames(frame_t inputFrame, frame_t loopStart, frame_t loopEnd)
		{
			return std::fmod(inputFrame, (loopEnd - loopStart) - 1.0f) + loopStart;
		}

		TimeSpan LoopMarkers(const Aet::Layer& layer, TimeSpan inputTime, std::string_view startMarker, std::string_view endMarker)
		{
			const auto loopStart = layer.FindMarkerFrame(startMarker).value_or(0.0f);
			const auto loopEnd = std::max(layer.FindMarkerFrame(endMarker).value_or(loopStart), loopStart);

			const auto inputFrame = inputTime.ToFrames();
			const auto loopedFrame = LoopFrames(inputFrame, loopStart, loopEnd);

			return TimeSpan::FromFrames(loopedFrame);
		}

		TimeSpan LoopMarkersOnce(const Aet::Layer& layer, TimeSpan inputTime, std::string_view startMarker, std::string_view endMarker)
		{
			return LoopMarkers(layer, inputTime, startMarker, endMarker);

			const auto loopStart = layer.FindMarkerFrame(startMarker).value_or(0.0f);
			const auto loopEnd = std::max(layer.FindMarkerFrame(endMarker).value_or(loopStart), loopStart);

			const auto inputFrame = std::min(inputTime.ToFrames(), (loopEnd - loopStart));
			const auto loopedFrame = LoopFrames(inputFrame, loopStart, loopEnd);

			return TimeSpan::FromFrames(loopedFrame);
		}

		TimeSpan LoopMarkers(const Aet::Layer& layer, TimeSpan inputTime, LoopState loop)
		{
			return LoopMarkers(layer, inputTime, LoopStartMarkerNames[loop], LoopEndMarkerNames[loop]);
		}

		class MenuTransition
		{
		public:
			struct DurationData
			{
				TimeSpan FadeIn;
				TimeSpan FadeLoop;
				TimeSpan FadeOut;
			};

		public:
			MenuTransition(DurationData duration) : duration(duration) {}
			~MenuTransition() = default;

		public:
			void Tick(Render::Renderer2D& renderer, TimeSpan elapsed, vec2 size)
			{
#if 0
				Gui::DEBUG_NOSAVE_WINDOW(__FUNCTION__"(): Debug", [&]
				{
					Gui::Text(__FUNCTION__"():");
					Gui::Text("CurrentState: FadeState::%s", std::array { "None", "In", "Loop", "Out", "Done" }[static_cast<size_t>(currentState)]);
					Gui::Text("ElapsedFadeTime: %s", elapsedFadeTime.ToString().c_str());
				});
#endif

				if (currentState == FadeState::None || currentState == FadeState::Done)
					return;

				auto checkAdvance = [&](auto stateToCheck, auto nextState, auto duration) { if (currentState == stateToCheck && elapsedFadeTime > duration) { AdvanceFadeState(nextState); } };
				checkAdvance(FadeState::In, FadeState::Loop, duration.FadeIn);
				checkAdvance(FadeState::Loop, FadeState::Out, duration.FadeLoop);
				checkAdvance(FadeState::Out, FadeState::Done, duration.FadeOut);

				const auto fadeAlpha = [&]
				{
					if (currentState == FadeState::In)
						return std::clamp(static_cast<f32>(elapsedFadeTime.TotalSeconds() / duration.FadeIn.TotalSeconds()), 0.0f, 1.0f);
					if (currentState == FadeState::Loop)
						return 1.0f;
					if (currentState == FadeState::Out)
						return std::clamp(1.0f - static_cast<f32>(elapsedFadeTime.TotalSeconds() / duration.FadeOut.TotalSeconds()), 0.0f, 1.0f);
					return 0.0f;
				}();

				elapsedFadeTime += elapsed;

				if (currentState != FadeState::Done)
					renderer.Draw(Render::RenderCommand2D(vec2(0.0f, 0.0f), size, vec4(fadeBaseColor, fadeAlpha)));
			}

			void StartFadeOnce(bool& transitionStarted) { if (!transitionStarted) { AdvanceFadeState(FadeState::In); transitionStarted = true; } }
			// void Reset() { AdvanceFadeState(FadeState::None); }
			bool ResetIfDone() { if (currentState != FadeState::None && currentState != FadeState::In) { /*AdvanceFadeState(FadeState::None);*/ return true; } return false; }

			bool IsFading() const { return currentState != FadeState::None; }
			//bool IsFadingIn() const { return currentState == FadeState::In; }
			//bool IsFadingOut() const { return (currentState == FadeState::Out) || (currentState == FadeState::Done); }

		private:
			enum class FadeState { None, In, Loop, Out, Done };

			void AdvanceFadeState(FadeState newState)
			{
				currentState = newState;
				elapsedFadeTime = TimeSpan::Zero();
			}

		private:
			vec3 fadeBaseColor = vec3(0.0f, 0.0f, 0.0f);
			DurationData duration;
			FadeState currentState = FadeState::None;
			TimeSpan elapsedFadeTime = TimeSpan::Zero();
		};
	}

	class PS4Menu
	{
	public:
		PS4Menu(MenuContext& context) : context(context) {}
		~PS4Menu() = default;

	public:
		void Tick()
		{
			InternalTick();
		}

	private:
		enum MenuSubState
		{
			MenuSubState_Game,
			MenuSubState_Custom,
			MenuSubState_PV,
			MenuSubState_MyRoom,
			MenuSubState_Store,
			MenuSubState_Options,

			MenuSubState_Count
		};

		static constexpr std::array<std::string_view, MenuSubState_Count> mainMenuPlateLayerNames
		{
			"menu_plate_game",
			"menu_plate_custom",
			"menu_plate_pv",
			"menu_plate_myroom",
			"menu_plate_store",
			"menu_plate_option",
		};

		static constexpr std::array<std::string_view, MenuSubState_Count> mainMenuPlateSelectLayerNames
		{
			"menu_plate_game_sel_t",
			"menu_plate_custom_sel_t",
			"menu_plate_pv_sel_t",
			"menu_plate_myroom_sel_t",
			"menu_plate_store_sel_t",
			"menu_plate_option_sel_t",
		};

		static constexpr std::array<std::string_view, MenuSubState_Count> mainMenuTextLayerNames
		{
			"menu_txt_game",
			"menu_txt_custom",
			"menu_txt_pv",
			"menu_txt_myroom",
			"menu_txt_store",
			"menu_txt_option",
		};

		static constexpr std::array<std::string_view, MenuSubState_Count> mainMenuCharaLayerNames
		{
			"menu_chara_game",
			"menu_chara_custom",
			"menu_chara_pv",
			"menu_chara_myroom",
			"menu_chara_store",
			"menu_chara_option",
		};

		static constexpr std::array<std::string_view, MenuSubState_Count> mainMenuItemPointLayerNames
		{
			"p_menu_list01_c",
			"p_menu_list02_c",
			"p_menu_list03_c",
			"p_menu_list04_c",
			"p_menu_list05_c",
			"p_menu_list06_c",
		};

		enum SongListDifficulty
		{
			SongListDifficulty_Easy,
			SongListDifficulty_Normal,
			SongListDifficulty_Hard,
			SongListDifficulty_Extreme,
			SongListDifficulty_ExtraExtreme,
			SongListDifficulty_Count
		};

		enum SongListItem
		{
			SongListItem_00,
			SongListItem_01,
			SongListItem_02,
			SongListItem_03,
			SongListItem_04,
			SongListItem_05,
			SongListItem_06,
			SongListItem_07,
			SongListItem_08,
			SongListItem_09,
			SongListItem_10,
			SongListItem_11,
			SongListItem_12,
			SongListItem_Count,

			SongListItem_Selected = SongListItem_05,
		};

		enum SongListScroll
		{
			SongListScroll_Up,
			SongListScroll_Still,
			SongListScroll_Down,
			SongListScroll_Count
		};

		/*
		static constexpr std::array<std::string_view, SongListItem_Count> songListBaseLayerNames
		{
		};
		*/

	private:
		const Aet::Layer* FindLayer(std::string_view layerName)
		{
			return context.PS4MenuMain->FindLayer(layerName).get();
		}

		const Aet::Video* FindVideo(std::string_view sourceName)
		{
			for (const auto& video : context.PS4MenuMain->Videos)
			{
				if (!video->Sources.empty() && video->Sources.front().Name == sourceName)
					return video.get();
			}

			return nullptr;
		}

		const Render::TexSpr FindSpr(std::string_view spriteName)
		{
			for (const auto& spr : context.SprPS4Menu->Sprites)
			{
				if (spr.Name == spriteName)
					return { context.SprPS4Menu->TexSet->Textures[spr.TextureIndex].get(), &spr };
			}

			return { nullptr, nullptr };
		}

	private:
		void InternalTick()
		{
			if (isMainMenu)
				MainMenuTick();
			else
				SubMenuTick();

			menuTransition.Tick(context.Renderer, context.Elapsed, context.PS4MenuMain->Resolution);
		}

		void MainMenuTick()
		{
			DrawMainMenuBackground();
			DrawMainMenuItemsAndText();
			DrawMainMenuHeaderFooter();

			if (isMainMenuFadingIn)
			{
				const auto fadeInEndFrame = FindLayer(mainMenuTextLayerNames[mainMenuSelectedtem])->FindMarkerFrame(LoopEndMarkerNames[LoopState_In]).value_or(0.0f);

				mainMenuTime += context.Elapsed;
				mainMenuSelectTime += context.Elapsed;

				if (mainMenuTime.ToFrames() > fadeInEndFrame - 1.0f)
					isMainMenuFadingIn = false;
			}
			else if (isMainMenuFadingOut)
			{
				mainMenuFadeOutTime += context.Elapsed;

				const auto layer = FindLayer(mainMenuPlateSelectLayerNames[mainMenuSelectedtem]);
				const auto fadeOutStartFrame = layer->FindMarkerFrame("st_sp").value_or(0.0f);
				const auto fadeOutEndFrame = layer->FindMarkerFrame("ed_sp").value_or(0.0f);
				const auto fadeOutDuration = (fadeOutEndFrame - fadeOutStartFrame);

				if (mainMenuFadeOutTime.ToFrames() > fadeOutDuration - 1.0f)
				{
					menuTransition.StartFadeOnce(mainMenuTransitionStarted);

					if (menuTransition.ResetIfDone())
					{
						currentSubState = mainMenuSelectedtem;
						isMainMenu = false;
					}
				}
			}
			else
			{
				MainMenuItemInput();
				mainMenuSelectTime += context.Elapsed;
			}

			mainMenuTime += context.Elapsed;
		}

		void SubMenuTick()
		{
			switch (currentSubState)
			{
			case MenuSubState_Game:
				GameMenuTick();
				break;
			case MenuSubState_Custom:
				CustomMenuTick();
				break;
			case MenuSubState_PV:
				PVMenuTick();
				break;
			case MenuSubState_MyRoom:
				MyRoomMenuTick();
				break;
			case MenuSubState_Store:
				StoreMenuTick();
				break;
			case MenuSubState_Options:
				OptionsMenuTick();
				break;
			}
		}

	private:
		void GameMenuTick()
		{
			DrawGameMenuBackground();
			DrawGameMenuSongList();
			DrawGameMenuHeaderFooter();
		}

		void DrawGameMenuBackground()
		{
			backgroundTime += context.Elapsed;
			context.Renderer.Aet().DrawLayerLooped(*FindLayer("menu_bg_t__f"), backgroundTime.ToFrames());
		}

		struct SongItemPointLayerData
		{
			const Aet::Layer* IconSlide;
			const Aet::Layer* Achieve;
			const Aet::Layer* IconDuel;
			const Aet::Layer* SongName;
			const Aet::Layer* SongClear;
			const Aet::Layer* SongLevel;
			const Aet::Layer* IconNew;
			const Aet::Layer* IconFavorite;
			const Aet::Layer* Option;
			const Aet::Layer* IconPack;
			const Aet::Layer* Base;
		};

		SongItemPointLayerData GetSongMenuItemTransforms(SongListItem item, LoopState loop)
		{
			const auto layerName = std::array { "song_list_in", "song_list_loop", "song_list_out" }[loop];
			const auto layer = FindLayer(layerName);
			const auto compItem = layer->GetCompItem();

			char pointLayerNameBuffer[64];
			auto findPointLayer = [&](const char* formatString)
			{
				sprintf_s(pointLayerNameBuffer, formatString, static_cast<int>(item));
				return compItem->FindLayer(pointLayerNameBuffer).get();
			};

			SongItemPointLayerData result;
			result.IconSlide = findPointLayer("p_icon_slide%02d_c");
			result.Achieve = findPointLayer("p_achieve_%02d");
			result.IconDuel = findPointLayer("p_icon_duet%02d_c");
			result.SongName = findPointLayer("p_song_name%02d_lt");
			result.SongClear = findPointLayer("p_song_clear%02d_c");
			result.SongLevel = findPointLayer("p_song_level%02d_c");
			result.IconNew = findPointLayer("p_icon_new%02d_c");
			result.IconFavorite = findPointLayer("p_icon_fav%02d_c");
			result.Option = findPointLayer("p_option_%02d");
			result.IconPack = findPointLayer("p_icon_pack%02d_c");
			result.Base = findPointLayer("p_song_list_base%02d_c");
			return result;
		}

		const Aet::Layer* GetSongListBaseLayer(std::optional<SongListDifficulty> difficulty, bool selected)
		{
			char nameBuffer[64];
			sprintf_s(nameBuffer, "song_list_base_%s%s",
				!difficulty.has_value() ? "blank" : std::array { "easy", "normal", "hard", "extreme", "extra_extreme" }[difficulty.value()],
				(selected && difficulty.has_value()) ? "_sel" : "");

			return FindLayer(nameBuffer);
		}

		void DrawGameMenuSongList()
		{
			gameMenuListTime += context.Elapsed;

			DrawGameMenuSongListSongBackground();

			for (int i = 0; i < SongListItem_Count; i++)
			{
				const auto loop = LoopState_Loop;
				const bool isSelected = (i == SongListItem_Selected);

				const auto difficulty = std::optional<SongListDifficulty> { SongListDifficulty_Extreme };

				const auto layerData = GetSongMenuItemTransforms(static_cast<SongListItem>(i), loop);

				{
					const auto baseLayer = GetSongListBaseLayer(difficulty, isSelected);
					const auto transform = Aet::Util::GetTransformAt(*layerData.Base->LayerVideo, gameMenuListTime.ToFrames());
					if (isSelected && difficulty.has_value())
						context.Renderer.Aet().DrawLayerLooped(*baseLayer, LoopMarkers(*baseLayer, gameMenuListTime, loop).ToFrames(), transform.Position, transform.Opacity);
					else
						context.Renderer.Aet().DrawLayerLooped(*baseLayer, gameMenuListTime.ToFrames(), transform.Position, transform.Opacity);
				}

				{
					const auto levelStar = FindLayer("level_star_10");
					const auto transform = Aet::Util::GetTransformAt(*layerData.SongLevel->LayerVideo, gameMenuListTime.ToFrames());
					context.Renderer.Aet().DrawLayerLooped(*levelStar, LoopMarkers(*levelStar, gameMenuListTime, LoopState_In).ToFrames(), transform.Position, transform.Opacity);
				}

				{
					const auto slideSpr = FindSpr(isSelected ? "ICON_SLIDE_SEL" : "ICON_SLIDE");
					const auto transform = Aet::Util::GetTransformAt(*layerData.IconSlide->LayerVideo, gameMenuListTime.ToFrames());
					context.Renderer.Aet().DrawSpr(*slideSpr.Tex, *slideSpr.Spr, transform);
				}

				{
					const auto transform = Aet::Util::GetTransformAt(*layerData.SongName->LayerVideo, gameMenuListTime.ToFrames());
					context.Renderer.Font().DrawBorder(*context.Font36, u8"YEP COCK@Ž€‚É‚½‚¢", transform);
				}
			}
		}

		void DrawGameMenuSongListSongBackground(LoopState loop = LoopState_Loop)
		{
			const auto drawLayer = [&](std::string_view layerName)
			{
				const auto layer = FindLayer(layerName);

				const auto loopInFrame = layer->FindMarkerFrame(LoopEndMarkerNames[LoopState_In]).value_or(0.0f);
				if (loop == LoopState_Loop && gameMenuListTime < TimeSpan::FromFrames(loopInFrame - 1.0f))
					loop = LoopState_In;

				context.Renderer.Aet().DrawLayerLooped(*layer, LoopMarkers(*layer, gameMenuListTime, loop).ToFrames());
			};

			drawLayer("song_bg");
			drawLayer("song_jk");
			drawLayer("song_logo");
		}

		void DrawGameMenuHeaderFooter()
		{
			headerFooterTime += context.Elapsed;
			context.Renderer.Aet().DrawLayerLooped(*FindLayer("game_header_ft"), headerFooterTime.ToFrames());
			context.Renderer.Aet().DrawLayerLooped(*FindLayer("game_footer"), headerFooterTime.ToFrames());
		}

	private:
		void CustomMenuTick()
		{
			DrawCustomMenuHeaderFooter();
		}

		void DrawCustomMenuHeaderFooter()
		{
			headerFooterTime += context.Elapsed;
			context.Renderer.Aet().DrawLayerLooped(*FindLayer("menu_header_ft"), headerFooterTime.ToFrames());
			context.Renderer.Aet().DrawLayerLooped(*FindLayer("menu_footer"), headerFooterTime.ToFrames());
		}

	private:
		void PVMenuTick()
		{
			DrawPVMenuHeaderFooter();
		}

		void DrawPVMenuHeaderFooter()
		{
			headerFooterTime += context.Elapsed;
			context.Renderer.Aet().DrawLayerLooped(*FindLayer("menu_header_ft"), headerFooterTime.ToFrames());
			context.Renderer.Aet().DrawLayerLooped(*FindLayer("menu_footer"), headerFooterTime.ToFrames());
		}

	private:
		void MyRoomMenuTick()
		{
			DrawMyRoomMenuHeaderFooter();
		}

		void DrawMyRoomMenuHeaderFooter()
		{
			headerFooterTime += context.Elapsed;
			context.Renderer.Aet().DrawLayerLooped(*FindLayer("menu_header_ft"), headerFooterTime.ToFrames());
			context.Renderer.Aet().DrawLayerLooped(*FindLayer("menu_footer"), headerFooterTime.ToFrames());
		}

	private:
		void StoreMenuTick()
		{
			DrawStoreMenuHeaderFooter();
		}

		void DrawStoreMenuHeaderFooter()
		{
			headerFooterTime += context.Elapsed;
			context.Renderer.Aet().DrawLayerLooped(*FindLayer("menu_header_ft"), headerFooterTime.ToFrames());
			context.Renderer.Aet().DrawLayerLooped(*FindLayer("menu_footer"), headerFooterTime.ToFrames());
		}

	private:
		void OptionsMenuTick()
		{
			DrawOptionsMenuHeaderFooter();
		}

		void DrawOptionsMenuHeaderFooter()
		{
			headerFooterTime += context.Elapsed;
			context.Renderer.Aet().DrawLayerLooped(*FindLayer("menu_header_ft"), headerFooterTime.ToFrames());
			context.Renderer.Aet().DrawLayerLooped(*FindLayer("menu_footer"), headerFooterTime.ToFrames());
		}

	private:
		void DrawMainMenuBackground()
		{
			backgroundTime += context.Elapsed;
			context.Renderer.Aet().DrawLayerLooped(*FindLayer("menu_bg_t__f"), backgroundTime.ToFrames());
		}

		void DrawMainMenuHeaderFooter()
		{
			headerFooterTime += context.Elapsed;
			context.Renderer.Aet().DrawLayerLooped(*FindLayer("menu_header_ft"), headerFooterTime.ToFrames());
			context.Renderer.Aet().DrawLayerLooped(*FindLayer("menu_footer"), headerFooterTime.ToFrames());
		}

		void MainMenuItemInput()
		{
			if (!Gui::IsWindowFocused())
				return;

			if (Gui::IsKeyPressed(Input::KeyCode_Up))
			{
				if ((mainMenuSelectedtem = static_cast<MenuSubState>(mainMenuSelectedtem - 1)) < MenuSubState_Game)
					mainMenuSelectedtem = MenuSubState_Options;
				mainMenuSelectTime = TimeSpan::Zero();
			}
			else if (Gui::IsKeyPressed(Input::KeyCode_Down))
			{
				if ((mainMenuSelectedtem = static_cast<MenuSubState>(mainMenuSelectedtem + 1)) >= MenuSubState_Count)
					mainMenuSelectedtem = MenuSubState_Game;
				mainMenuSelectTime = TimeSpan::Zero();
			}

			if (Gui::IsKeyPressed(Input::KeyCode_Enter))
				isMainMenuFadingOut = true;
		}

		void DrawMenuDeco(LoopState loop)
		{
			const auto layer = FindLayer("menu_deco_t__f");
			context.Renderer.Aet().DrawLayer(*layer, LoopMarkers(*layer, mainMenuTime, loop).ToFrames());
		}

		std::pair<vec2, float> GetMainMenuItemPositionAndOpacity(MenuSubState item, TimeSpan time)
		{
			auto transform = Aet::Util::GetTransformAt(*FindLayer("menu_list_in02")->GetCompItem()->FindLayer(mainMenuItemPointLayerNames[item])->LayerVideo, time.ToFrames());
			return std::make_pair(transform.Position, transform.Opacity);
		}

		void DrawMainMenuItem(MenuSubState item, LoopState loop)
		{
			const auto[position, opacity] = GetMainMenuItemPositionAndOpacity(item, mainMenuTime);
			const auto layer = FindLayer(mainMenuPlateLayerNames[item]);
			context.Renderer.Aet().DrawLayer(*layer, LoopMarkers(*layer, mainMenuTime, loop).ToFrames(), position, opacity);
		}

		void DrawMainMenuItemSelected(MenuSubState item, LoopState loop)
		{
			const auto[position, opacity] = GetMainMenuItemPositionAndOpacity(item, mainMenuTime);
			const auto layer = FindLayer(mainMenuPlateSelectLayerNames[item]);

			// BUG:
			if (loop == LoopState_Loop && mainMenuSelectTime < TimeSpan::FromFrames(layer->FindMarkerFrame(LoopEndMarkerNames[LoopState_In]).value_or(0.0f) - 1.0f))
				loop = LoopState_In;

			if (isMainMenuFadingOut)
			{
				context.Renderer.Aet().DrawLayer(*layer, LoopMarkersOnce(*layer, mainMenuFadeOutTime, "st_sp", "ed_sp").ToFrames(), position, opacity);
			}
			else
			{
				context.Renderer.Aet().DrawLayer(*layer, LoopMarkers(*layer, mainMenuSelectTime, loop).ToFrames(), position, opacity);
			}
		}

		void DrawMainMenuText(MenuSubState item, LoopState loop)
		{
			const auto layer = FindLayer(mainMenuTextLayerNames[item]);

			if (loop == LoopState_Loop && mainMenuSelectTime < TimeSpan::FromFrames(layer->FindMarkerFrame(LoopEndMarkerNames[LoopState_In]).value_or(0.0f) - 1.0f))
				loop = LoopState_In;

			context.Renderer.Aet().DrawLayer(*layer, LoopMarkers(*layer, (mainMenuSelectTime), loop).ToFrames());
		}

		void DrawMainMenuChara(MenuSubState item, LoopState loop)
		{
			const auto layer = FindLayer(mainMenuCharaLayerNames[item]);

			const auto loopInFrame = layer->FindMarkerFrame(LoopEndMarkerNames[LoopState_In]).value_or(0.0f);
			// const auto loopOutFrame = layer->FindMarkerFrame(LoopEndMarkerNames[LoopState_Out]).value_or(0.0f);

			if (loop == LoopState_Loop && mainMenuSelectTime < TimeSpan::FromFrames(loopInFrame - 1.0f))
				loop = LoopState_In;

			context.Renderer.Aet().DrawLayer(*layer, LoopMarkers(*layer, (mainMenuSelectTime), loop).ToFrames());
		}

		void DrawMainMenuItemsAndText()
		{
			const auto loop = (isMainMenuFadingIn) ? LoopState_In : LoopState_Loop;

			DrawMenuDeco(loop);

			for (MenuSubState i = MenuSubState_Game; i < MenuSubState_Count; i = static_cast<MenuSubState>(i + 1))
				if (i != mainMenuSelectedtem) DrawMainMenuItem(i, loop);

			DrawMainMenuItemSelected(mainMenuSelectedtem, loop);
			DrawMainMenuChara(mainMenuSelectedtem, loop);
			DrawMainMenuText(mainMenuSelectedtem, loop);
		}

	private:
		MenuContext& context;

	private:
		MenuSubState currentSubState = MenuSubState_Count;

		MenuTransition menuTransition = { MenuTransition::DurationData { TimeSpan::FromSeconds(0.1f), TimeSpan::FromSeconds(0.25f), TimeSpan::FromSeconds(0.1f) } };

		bool isMainMenu = true;
		bool isMainMenuFadingIn = true;
		bool isMainMenuFadingOut = false;

		TimeSpan backgroundTime = TimeSpan::Zero();
		TimeSpan headerFooterTime = TimeSpan::Zero();

		TimeSpan mainMenuTime = TimeSpan::Zero();
		TimeSpan mainMenuSelectTime = TimeSpan::Zero();
		TimeSpan mainMenuFadeOutTime = TimeSpan::Zero();
		bool mainMenuTransitionStarted = false;

		MenuSubState mainMenuSelectedtem = MenuSubState_Game;

		TimeSpan gameMenuListTime = TimeSpan::Zero();
		size_t gameMenuSelectedSongIndex = 0;

		struct SongEntry
		{
			std::string SongName;
		};

		std::vector<SongEntry> gameMenuSongEntries;
	};

	class MenuTest : public ITestTask
	{
	public:
		COMFY_REGISTER_TEST_TASK(MenuTest);

		MenuTest()
		{
			renderWindow.OnRenderCallback = [&]
			{
				renderWindow.RenderTarget->Param.Resolution = camera.ProjectionSize = renderWindow.GetRenderRegion().GetSize();
				camera.CenterAndZoomToFit(ivec2(1920, 1080));

				context.Renderer.Begin(camera, *renderWindow.RenderTarget);
				{
					ps4Menu->Tick();
				}
				context.Renderer.End();
			};

			context.Renderer.Aet().SetSprGetter([&](const Aet::VideoSource& source)
			{
				for (auto& sprSet : context.AllSprSets)
				{
					if (auto result = Render::SprSetNameStringSprGetter(source, sprSet->get()); result.Tex != nullptr && result.Spr != nullptr)
						return result;
				}

				return Render::NullSprGetter(source);
			});
		}

		void Update() override
		{
			context.Elapsed = (stopwatch.Restart() * MenuTimeFactor);

			if (Gui::IsKeyPressed(Input::KeyCode_F11, false))
				fullscreen ^= true;

			if (Gui::IsKeyPressed(Input::KeyCode_Escape))
				ps4Menu = std::make_unique<PS4Menu>(context);

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
						ps4Menu = std::make_unique<PS4Menu>(context);

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
		MenuContext context;

		std::unique_ptr<PS4Menu> ps4Menu = std::make_unique<PS4Menu>(context);
	};
}
