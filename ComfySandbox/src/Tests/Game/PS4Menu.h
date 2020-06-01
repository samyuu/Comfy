#pragma once
#include "Tests/TestTask.h"
#include "GameContext.h"
#include "Helper.h"
#include "PS4MenuAetInterface.h"

namespace Comfy::Sandbox::Tests::Game
{
	class PS4Menu
	{
	public:
		PS4Menu(GameContext& context) : context(context)
		{
#if 0 // DEBUG: YEP
			isMainMenu = false;
			currentSubState = MenuSubState_Game;
#endif /* DEBUG */
		}
		~PS4Menu() = default;

	public:
		void Tick()
		{
			InternalTick();
		}

	private:
		const Aet::Layer* FindLayer(std::string_view layerName)
		{
			return context.AetPS4MenuMain->FindLayer(layerName).get();
		}

		const Aet::Video* FindVideo(std::string_view sourceName)
		{
			for (const auto& video : context.AetPS4MenuMain->Videos)
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

			menuTransition.Tick(context.Renderer, context.Elapsed, context.AetPS4MenuMain->Resolution);
		}

		void MainMenuTick()
		{
			DrawMainMenuBackground();
			DrawMainMenuItemsAndText();
			DrawMainMenuHeaderFooter();

			if (isMainMenuFadingIn)
			{
				const auto fadeInEndFrame = FindLayer(MainMenuTextLayerNames[mainMenuSelectedtem])->FindMarkerFrame(LoopEndMarkerNames[LoopState_In]).value_or(0.0f);

				mainMenuTime += context.Elapsed;
				mainMenuSelectTime += context.Elapsed;

				if (mainMenuTime.ToFrames() > fadeInEndFrame - 1.0f)
					isMainMenuFadingIn = false;
			}
			else if (isMainMenuFadingOut)
			{
				mainMenuFadeOutTime += context.Elapsed;

				const auto layer = FindLayer(MainMenuPlateSelectLayerNames[mainMenuSelectedtem]);
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
			const auto& namesArray = (selected ? SongListBaseSelectedLayerNames : SongListBaseLayerNames);
			const auto layerName = (!difficulty.has_value() || !InBounds(static_cast<int>(difficulty.value()), namesArray)) ? namesArray.back() : namesArray[difficulty.value()];

			return FindLayer(layerName);
		}

		void DrawGameMenuSongList()
		{
			gameMenuListTime += context.Elapsed;

			DrawGameMenuSongListSongBackground();
			DrawGameMenuSongListSongBases();
		}

		void DrawGameMenuSongListSongBases()
		{
			for (int i = 0; i < SongListItem_Count; i++)
			{
				const auto loop = LoopState_Loop;
				const bool isSelected = (i == SongListItem_Selected);

				const auto layerData = GetSongMenuItemTransforms(static_cast<SongListItem>(i), loop);
				const auto entry = gameMenuSongList.GetEntry(i - SongListItem_Selected);

				if (entry == nullptr)
				{
					const auto baseLayer = GetSongListBaseLayer({}, isSelected);
					const auto transform = Aet::Util::GetTransformAt(*layerData.Base->LayerVideo, gameMenuListTime.ToFrames());
					context.Renderer.Aet().DrawLayerLooped(*baseLayer, gameMenuListTime.ToFrames(), transform.Position, transform.Opacity);
					continue;
				}

				const auto difficulty = SongListDifficulty_Extreme;

				{
					const auto baseLayer = GetSongListBaseLayer(difficulty, isSelected);
					const auto transform = Aet::Util::GetTransformAt(*layerData.Base->LayerVideo, gameMenuListTime.ToFrames());
					if (isSelected)
						context.Renderer.Aet().DrawLayerLooped(*baseLayer, LoopMarkers(*baseLayer, gameMenuListTime, loop).ToFrames(), transform.Position, transform.Opacity);
					else
						context.Renderer.Aet().DrawLayerLooped(*baseLayer, gameMenuListTime.ToFrames(), transform.Position, transform.Opacity);
				}

				{
					const auto levelStar = FindLayer(LevelStarLayerName[entry->LevelStar]);
					const auto transform = Aet::Util::GetTransformAt(*layerData.SongLevel->LayerVideo, gameMenuListTime.ToFrames());
					context.Renderer.Aet().DrawLayerLooped(*levelStar, LoopMarkers(*levelStar, gameMenuListTime, LoopState_In).ToFrames(), transform.Position, transform.Opacity);
				}

				if (entry->HasSlides)
				{
					const auto slideSpr = FindSpr(isSelected ? "ICON_SLIDE_SEL" : "ICON_SLIDE");
					const auto transform = Aet::Util::GetTransformAt(*layerData.IconSlide->LayerVideo, gameMenuListTime.ToFrames());
					context.Renderer.Aet().DrawSpr(*slideSpr.Tex, *slideSpr.Spr, transform);
				}

				{
					const auto transform = Aet::Util::GetTransformAt(*layerData.SongName->LayerVideo, gameMenuListTime.ToFrames());
					context.Renderer.Font().DrawBorder(*context.Font36, entry->SongName, transform);
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
			auto transform = Aet::Util::GetTransformAt(*FindLayer("menu_list_in02")->GetCompItem()->FindLayer(MainMenuItemPointLayerNames[item])->LayerVideo, time.ToFrames());
			return std::make_pair(transform.Position, transform.Opacity);
		}

		void DrawMainMenuItem(MenuSubState item, LoopState loop)
		{
			const auto[position, opacity] = GetMainMenuItemPositionAndOpacity(item, mainMenuTime);
			const auto layer = FindLayer(MainMenuPlateLayerNames[item]);
			context.Renderer.Aet().DrawLayer(*layer, LoopMarkers(*layer, mainMenuTime, loop).ToFrames(), position, opacity);
		}

		void DrawMainMenuItemSelected(MenuSubState item, LoopState loop)
		{
			const auto[position, opacity] = GetMainMenuItemPositionAndOpacity(item, mainMenuTime);
			const auto layer = FindLayer(MainMenuPlateSelectLayerNames[item]);

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
			const auto layer = FindLayer(MainMenuTextLayerNames[item]);

			if (loop == LoopState_Loop && mainMenuSelectTime < TimeSpan::FromFrames(layer->FindMarkerFrame(LoopEndMarkerNames[LoopState_In]).value_or(0.0f) - 1.0f))
				loop = LoopState_In;

			context.Renderer.Aet().DrawLayer(*layer, LoopMarkers(*layer, (mainMenuSelectTime), loop).ToFrames());
		}

		void DrawMainMenuChara(MenuSubState item, LoopState loop)
		{
			const auto layer = FindLayer(MainMenuCharaLayerNames[item]);

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
		GameContext& context;

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

		struct SongList
		{
			struct Entry
			{
				std::string SongName;
				bool HasSlides;
				LevelStar LevelStar;
			};

			std::vector<Entry> Entries =
			{
				Entry { u8"YEP COCK", true, LevelStar_10, },
				Entry { u8"Ž€‚É‚½‚¢", true, LevelStar_09, },
				Entry { u8"NOP", true, LevelStar_08, },
			};

			const Entry* GetEntry(int index) const
			{
				return IndexOrNull(index, Entries);
			}

			int SelectedIndex = 0;
			SongListState CurrentScroll = SongListState_Appear;

		} gameMenuSongList;

	};
}
