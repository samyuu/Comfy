#pragma once
#include "Tests/TestTask.h"
#include "Tests/Game/Core/GameState.h"
#include "Tests/Game/Common/Helper.h"
#include "Tests/Game/Common/PS4MenuAetInterface.h"

namespace Comfy::Sandbox::Tests::Game
{
	class PS4GameMenu : public GameStateBase
	{
	public:
		PS4GameMenu(GameContext& context) : GameStateBase(context) {}

	public:
		void OnFocusGained(std::optional<GameStateType> previousState)
		{
			backgroundTime = TimeSpan::Zero();
			headerFooterTime = TimeSpan::Zero();

			songListTime = TimeSpan::Zero();
		}

		void OnFocusLost(std::optional<GameStateType> newState)
		{
		}

		void OnUpdateInput() override
		{
			if (Gui::IsKeyPressed(Input::KeyCode_Escape, false))
				ChangeRequest = { GameStateType::PS4MainMenu, true };

			if (Gui::IsKeyDown(Input::KeyCode_Up) && songList.EntriesToScroll == 0) ScrollSongList(-1);
			if (Gui::IsKeyDown(Input::KeyCode_Down) && songList.EntriesToScroll == 0) ScrollSongList(+1);
		}

		void OnDraw() override
		{
			DrawBackground();
			DrawSongList();
			DrawHeaderFooter();
		}

	private:
		const Aet::Layer* FindLayer(std::string_view layerName) const { return context.FindLayer(*context.AetPS4MenuMain, layerName); }
		const Aet::Video* FindVideo(std::string_view sourceName) const { return context.FindVideo(*context.AetPS4MenuMain, sourceName); }
		const Render::TexSpr FindSpr(std::string_view spriteName) const { return context.FindSpr(*context.SprPS4Menu, spriteName); }

	private:
		void DrawBackground()
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

		const Aet::Layer* GetSongListLayer()
		{
			const auto layerName = [&]()
			{
				if (songList.CurrentState == SongListState_Appear)
					return "song_list_in";
				if (songList.CurrentState == SongListState_Disappear)
					return "song_list_out";
				if (songList.CurrentState == SongListState_ScrollUp)
					return "song_list_up";
				if (songList.CurrentState == SongListState_ScrollDown)
					return "song_list_down";
				return "song_list_loop";
			}();

			return FindLayer(layerName);
		}

		SongItemPointLayerData GetSongMenuItemTransforms(SongListItem item)
		{
			const auto compItem = GetSongListLayer()->GetCompItem();

			SongItemPointLayerData result;
			result.IconSlide = compItem->FindLayer(SongListItemIconSlideLayerNames[item]).get();
			result.Achieve = compItem->FindLayer(SongListItemAchieveLayerNames[item]).get();
			result.IconDuel = compItem->FindLayer(SongListItemIconDuelLayerNames[item]).get();
			result.SongName = compItem->FindLayer(SongListItemSongNameLayerNames[item]).get();
			result.SongClear = compItem->FindLayer(SongListItemSongClearLayerNames[item]).get();
			result.SongLevel = compItem->FindLayer(SongListItemSongLevelLayerNames[item]).get();
			result.IconNew = compItem->FindLayer(SongListItemIconNewLayerNames[item]).get();
			result.IconFavorite = compItem->FindLayer(SongListItemIconFavoriteLayerNames[item]).get();
			result.Option = compItem->FindLayer(SongListItemOptionLayerNames[item]).get();
			result.IconSlide = compItem->FindLayer(SongListItemIconSlideLayerNames[item]).get();
			result.IconPack = compItem->FindLayer(SongListItemIconPackLayerNames[item]).get();
			result.Base = compItem->FindLayer(SongListItemBaseLayerNames[item]).get();
			return result;
		}

		const Aet::Layer* GetSongListBaseLayer(std::optional<SongListDifficulty> difficulty, bool selected)
		{
			const auto& namesArray = (selected ? SongListBaseSelectedLayerNames : SongListBaseLayerNames);
			const auto layerName = (!difficulty.has_value() || !InBounds(static_cast<int>(difficulty.value()), namesArray)) ? namesArray.back() : namesArray[difficulty.value()];

			return FindLayer(layerName);
		}

		void DrawSongList()
		{
			songListTime += context.Elapsed;

			UpdateSongListScroll();
			DrawSongListSongBackground();
			DrawSongListSongBases();
		}

		void UpdateSongListScroll()
		{
			//if (songList.EntriesToScroll != 0)
			//{
			//	songList.EntriesToScrollElapsed += context.Elapsed;
			//	songList.SelectedIndex += songList.EntriesToScroll;
			//	songList.EntriesToScroll = 0;
			//}

			if (songList.CurrentState == SongListState_Appear)
			{
				const auto songListLayer = GetSongListLayer();
				const auto inDuration = TimeSpan::FromFrames(songListLayer->FindMarkerFrame("ed_in").value_or(0.0f));

				if (songListTime > inDuration)
				{
					songListTime -= inDuration;
					songList.CurrentState = SongListState_Still;
				}
			}
			else if (songList.CurrentState == SongListState_ScrollUp || songList.CurrentState == SongListState_ScrollDown)
			{
				const auto songListLayer = GetSongListLayer();
				const auto scrollDuration = TimeSpan::FromFrames(songListLayer->EndFrame);
				if (songListTime > scrollDuration && songList.EntriesToScroll != 0)
				{
					songListTime -= scrollDuration;

					const auto scrollDirection = (songList.EntriesToScroll > 0) ? 1 : -1;
					songList.SelectedIndex += scrollDirection;
					songList.EntriesToScroll -= scrollDirection;

					if (songList.EntriesToScroll == 0)
						songList.CurrentState = SongListState_Still;
				}
			}
		}

		void DrawSongListSongBases()
		{
			Gui::DEBUG_NOSAVE_ONCE_PER_FRAME_WINDOW("YEP COCK", [&] 
			{
				if (auto songListLayer = GetSongListLayer(); songListLayer != nullptr)
					Gui::TextUnformatted(songListLayer->GetName().c_str());
			});

			for (int i = 0; i < SongListItem_Count; i++)
			{
				const auto loop = LoopState_Loop;
				const bool isSelected = (i == SongListItem_Center);

				const auto layerData = GetSongMenuItemTransforms(static_cast<SongListItem>(i));
				const auto entry = songList.GetEntry(i - SongListItem_Center + songList.SelectedIndex);

				if (entry == nullptr)
				{
					const auto baseLayer = GetSongListBaseLayer({}, isSelected);
					const auto transform = Aet::Util::GetTransformAt(*layerData.Base->LayerVideo, songListTime.ToFrames());
					context.Renderer.Aet().DrawLayerLooped(*baseLayer, songListTime.ToFrames(), transform.Position, transform.Opacity);
					continue;
				}

				const auto difficulty = SongListDifficulty_Extreme;

				{
					const auto baseLayer = GetSongListBaseLayer(difficulty, isSelected);
					const auto transform = Aet::Util::GetTransformAt(*layerData.Base->LayerVideo, songListTime.ToFrames());
					if (isSelected)
						context.Renderer.Aet().DrawLayerLooped(*baseLayer, LoopMarkers(*baseLayer, songListTime, loop).ToFrames(), transform.Position, transform.Opacity);
					else
						context.Renderer.Aet().DrawLayerLooped(*baseLayer, songListTime.ToFrames(), transform.Position, transform.Opacity);
				}

				{
					const auto levelStar = FindLayer(LevelStarLayerName[entry->LevelStar]);
					const auto transform = Aet::Util::GetTransformAt(*layerData.SongLevel->LayerVideo, songListTime.ToFrames());
					context.Renderer.Aet().DrawLayerLooped(*levelStar, LoopMarkers(*levelStar, songListTime, LoopState_In).ToFrames(), transform.Position, transform.Opacity);
				}

				if (entry->HasSlides)
				{
					const auto slideSpr = FindSpr(isSelected ? "ICON_SLIDE_SEL" : "ICON_SLIDE");
					const auto transform = Aet::Util::GetTransformAt(*layerData.IconSlide->LayerVideo, songListTime.ToFrames());
					context.Renderer.Aet().DrawSpr(*slideSpr.Tex, *slideSpr.Spr, transform);
				}

				{
					const auto transform = Aet::Util::GetTransformAt(*layerData.SongName->LayerVideo, songListTime.ToFrames());
					context.Renderer.Font().DrawBorder(*context.Font36, entry->SongName, transform);
				}
			}
		}

		void DrawSongListSongBackground(LoopState loop = LoopState_Loop)
		{
			const auto drawLayer = [&](std::string_view layerName)
			{
				const auto layer = FindLayer(layerName);

				const auto loopInFrame = layer->FindMarkerFrame(LoopEndMarkerNames[LoopState_In]).value_or(0.0f);
				if (loop == LoopState_Loop && backgroundTime < TimeSpan::FromFrames(loopInFrame - 1.0f))
					loop = LoopState_In;

				context.Renderer.Aet().DrawLayerLooped(*layer, LoopMarkers(*layer, backgroundTime, loop).ToFrames());
			};

			drawLayer("song_bg");
			drawLayer("song_jk");
			drawLayer("song_logo");
		}

		void DrawHeaderFooter()
		{
			headerFooterTime += context.Elapsed;
			context.Renderer.Aet().DrawLayerLooped(*FindLayer("game_header_ft"), headerFooterTime.ToFrames());
			context.Renderer.Aet().DrawLayerLooped(*FindLayer("game_footer"), headerFooterTime.ToFrames());
		}

		void ScrollSongList(int entriesToScrollBy)
		{
			if (entriesToScrollBy == 0)
				return;

			const bool scrollUp = (entriesToScrollBy > 0);
			const bool scrollDown = (entriesToScrollBy < 0);

			if (songList.EntriesToScroll == 0)
				songListTime = TimeSpan::Zero();
			
			songList.CurrentState = (scrollUp) ? SongListState_ScrollUp : SongListState_ScrollDown;
			songList.EntriesToScroll += entriesToScrollBy;
		}

	private:
		TimeSpan backgroundTime, headerFooterTime;
		TimeSpan songListTime;

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
				// Entry { u8"YEP COCK", true, LevelStar_10, },
				// Entry { u8"死にたい", true, LevelStar_09, },
				// Entry { u8"NOP", true, LevelStar_08, },
				// Entry { u8"NOP", true, LevelStar_08, },

				Entry { u8"わたしを御食べ", true, LevelStar_08, },
				Entry { u8"月飼い", true, LevelStar_08, },
				Entry { u8"魔女狩り", true, LevelStar_08, },
				Entry { u8"はいかぶり", true, LevelStar_08, },
				Entry { u8"十三番開放口", true, LevelStar_08, },
				Entry { u8"曲芸団のゆめ", true, LevelStar_08, },
				Entry { u8"まつげうさぎと踊り子の旅", true, LevelStar_08, },
				Entry { u8"ラプンツヱル", true, LevelStar_08, },
				Entry { u8"幸福なおばけ", true, LevelStar_08, },
				Entry { u8"暗やみ動物園", true, LevelStar_08, },
			};

			const Entry* GetEntry(int index) const
			{
				return IndexOrNull(index, Entries);
			}

			int SelectedIndex = 0;
			SongListState CurrentState = SongListState_Appear;

			// TimeSpan EntriesToScrollElapsed = TimeSpan::Zero();
			int EntriesToScroll = 0;

		} songList;
	};
}
