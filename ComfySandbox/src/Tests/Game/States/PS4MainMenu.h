#pragma once
#include "Tests/TestTask.h"
#include "Tests/Game/Core/GameState.h"
#include "Tests/Game/Common/Helper.h"
#include "Tests/Game/Common/PS4MenuAetInterface.h"

namespace Comfy::Sandbox::Tests::Game
{
	class PS4MainMenu : public GameStateBase
	{
	public:
		PS4MainMenu(GameContext& context) : GameStateBase(context) {}

	public:
		void OnFocusGained(std::optional<GameStateType> previousState)
		{
			isAppearing = true;
			isDisappearing = false;

			backgroundTime = TimeSpan::Zero();
			headerFooterTime = TimeSpan::Zero();

			itemTime = TimeSpan::Zero();
			itemSelectTime = TimeSpan::Zero();
			fadeOutTime = TimeSpan::Zero();
		}

		void OnFocusLost(std::optional<GameStateType> newState)
		{
		}

		void OnUpdateInput() override
		{
			if (!isAppearing && !isDisappearing)
				UpdateItemSelectionInput();
		}

		void OnDraw() override
		{
			DrawMainMenu();
		}

	private:
		const Aet::Layer* FindLayer(std::string_view layerName) const { return context.FindLayer(*context.AetPS4MenuMain, layerName); }
		const Aet::Video* FindVideo(std::string_view sourceName) const { return context.FindVideo(*context.AetPS4MenuMain, sourceName); }
		const Render::TexSpr FindSpr(std::string_view spriteName) const { return context.FindSpr(*context.SprPS4Menu, spriteName); }

	private:
		void DrawMainMenu()
		{
			DrawMainMenuBackground();
			DrawMainMenuItemsAndText();
			DrawMainMenuHeaderFooter();

			if (isAppearing)
			{
				const auto fadeInEndFrame = FindLayer(MainMenuTextLayerNames[selectedItem])->FindMarkerFrame(LoopEndMarkerNames[LoopState_In]).value_or(0.0f);

				itemTime += context.Elapsed;
				itemSelectTime += context.Elapsed;

				if (itemTime.ToFrames() > fadeInEndFrame - 1.0f)
					isAppearing = false;
			}
			else if (isDisappearing)
			{
				fadeOutTime += context.Elapsed;

				const auto layer = FindLayer(MainMenuPlateSelectLayerNames[selectedItem]);
				const auto fadeOutStartFrame = layer->FindMarkerFrame("st_sp").value_or(0.0f);
				const auto fadeOutEndFrame = layer->FindMarkerFrame("ed_sp").value_or(0.0f);
				const auto fadeOutDuration = (fadeOutEndFrame - fadeOutStartFrame);

				if (fadeOutTime.ToFrames() > fadeOutDuration - 1.0f)
				{
					switch (selectedItem)
					{
					case MenuSubState_Game:
						ChangeRequest = { GameStateType::PS4GameMenu };
						break;
					case MenuSubState_Custom:
						ChangeRequest = { GameStateType::Count };
						break;
					case MenuSubState_PV:
						ChangeRequest = { GameStateType::Count };
						break;
					case MenuSubState_MyRoom:
						ChangeRequest = { GameStateType::Count };
						break;
					case MenuSubState_Store:
						ChangeRequest = { GameStateType::Count };
						break;
					case MenuSubState_Options:
						ChangeRequest = { GameStateType::Count };
						break;
					}
				}
			}
			else
			{
				itemSelectTime += context.Elapsed;
			}

			itemTime += context.Elapsed;
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

		void UpdateItemSelectionInput()
		{
			if (Gui::IsKeyPressed(Input::KeyCode_Up))
			{
				if ((selectedItem = static_cast<MenuSubState>(selectedItem - 1)) < MenuSubState_Game)
					selectedItem = MenuSubState_Options;
				itemSelectTime = TimeSpan::Zero();
			}
			else if (Gui::IsKeyPressed(Input::KeyCode_Down))
			{
				if ((selectedItem = static_cast<MenuSubState>(selectedItem + 1)) >= MenuSubState_Count)
					selectedItem = MenuSubState_Game;
				itemSelectTime = TimeSpan::Zero();
			}

			if (Gui::IsKeyPressed(Input::KeyCode_Enter))
				isDisappearing = true;
		}

		void DrawMenuDeco(LoopState loop)
		{
			const auto layer = FindLayer("menu_deco_t__f");
			context.Renderer.Aet().DrawLayer(*layer, LoopMarkers(*layer, itemTime, loop).ToFrames());
		}

		std::pair<vec2, float> GetMainMenuItemPositionAndOpacity(MenuSubState item, TimeSpan time)
		{
			auto transform = Aet::Util::GetTransformAt(*FindLayer("menu_list_in02")->GetCompItem()->FindLayer(MainMenuItemPointLayerNames[item])->LayerVideo, time.ToFrames());
			return std::make_pair(transform.Position, transform.Opacity);
		}

		void DrawMainMenuItem(MenuSubState item, LoopState loop)
		{
			const auto[position, opacity] = GetMainMenuItemPositionAndOpacity(item, itemTime);
			const auto layer = FindLayer(MainMenuPlateLayerNames[item]);
			context.Renderer.Aet().DrawLayer(*layer, LoopMarkers(*layer, itemTime, loop).ToFrames(), position, opacity);
		}

		void DrawMainMenuItemSelected(MenuSubState item, LoopState loop)
		{
			const auto[position, opacity] = GetMainMenuItemPositionAndOpacity(item, itemTime);
			const auto layer = FindLayer(MainMenuPlateSelectLayerNames[item]);

			// BUG:
			if (loop == LoopState_Loop && itemSelectTime < TimeSpan::FromFrames(layer->FindMarkerFrame(LoopEndMarkerNames[LoopState_In]).value_or(0.0f) - 1.0f))
				loop = LoopState_In;

			if (isDisappearing)
			{
				// BUG: This should only be played once
				context.Renderer.Aet().DrawLayer(*layer, ClampMarkers(*layer, fadeOutTime, "st_sp", "ed_sp").ToFrames(), position, opacity);
			}
			else
			{
				context.Renderer.Aet().DrawLayer(*layer, LoopMarkers(*layer, itemSelectTime, loop).ToFrames(), position, opacity);
			}
		}

		void DrawMainMenuText(MenuSubState item, LoopState loop)
		{
			const auto layer = FindLayer(MainMenuTextLayerNames[item]);

			if (loop == LoopState_Loop && itemSelectTime < TimeSpan::FromFrames(layer->FindMarkerFrame(LoopEndMarkerNames[LoopState_In]).value_or(0.0f) - 1.0f))
				loop = LoopState_In;

			context.Renderer.Aet().DrawLayer(*layer, LoopMarkers(*layer, (itemSelectTime), loop).ToFrames());
		}

		void DrawMainMenuChara(MenuSubState item, LoopState loop)
		{
			const auto layer = FindLayer(MainMenuCharaLayerNames[item]);

			const auto loopInFrame = layer->FindMarkerFrame(LoopEndMarkerNames[LoopState_In]).value_or(0.0f);
			// const auto loopOutFrame = layer->FindMarkerFrame(LoopEndMarkerNames[LoopState_Out]).value_or(0.0f);

			if (loop == LoopState_Loop && itemSelectTime < TimeSpan::FromFrames(loopInFrame - 1.0f))
				loop = LoopState_In;

			context.Renderer.Aet().DrawLayer(*layer, LoopMarkers(*layer, (itemSelectTime), loop).ToFrames());
		}

		void DrawMainMenuItemsAndText()
		{
			const auto loop = (isAppearing) ? LoopState_In : LoopState_Loop;

			DrawMenuDeco(loop);

			for (MenuSubState i = MenuSubState_Game; i < MenuSubState_Count; i = static_cast<MenuSubState>(i + 1))
				if (i != selectedItem) DrawMainMenuItem(i, loop);

			DrawMainMenuItemSelected(selectedItem, loop);
			DrawMainMenuChara(selectedItem, loop);
			DrawMainMenuText(selectedItem, loop);
		}

	private:
		bool isAppearing, isDisappearing;

		TimeSpan backgroundTime, headerFooterTime;
		TimeSpan itemTime, itemSelectTime;
		TimeSpan fadeOutTime;

		MenuSubState selectedItem = MenuSubState_Game;
	};
}
