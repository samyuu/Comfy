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
		}

		void OnFocusLost(std::optional<GameStateType> newState)
		{
		}

		void OnUpdateInput() override
		{
		}

		void OnDraw() override
		{
			MainMenuTick();
		}

	private:
		const Aet::Layer* FindLayer(std::string_view layerName) const { return context.FindLayer(*context.AetPS4MenuMain, layerName); }
		const Aet::Video* FindVideo(std::string_view sourceName) const { return context.FindVideo(*context.AetPS4MenuMain, sourceName); }
		const Render::TexSpr FindSpr(std::string_view spriteName) const { return context.FindSpr(*context.SprPS4Menu, spriteName); }

	private:
		void MainMenuTick()
		{
			DrawMainMenuBackground();
			DrawMainMenuItemsAndText();
			DrawMainMenuHeaderFooter();

			if (isFadingIn)
			{
				const auto fadeInEndFrame = FindLayer(MainMenuTextLayerNames[mainMenuSelectedItem])->FindMarkerFrame(LoopEndMarkerNames[LoopState_In]).value_or(0.0f);

				mainMenuTime += context.Elapsed;
				mainMenuSelectTime += context.Elapsed;

				if (mainMenuTime.ToFrames() > fadeInEndFrame - 1.0f)
					isFadingIn = false;
			}
			else if (isFadingOut)
			{
				mainMenuFadeOutTime += context.Elapsed;

				const auto layer = FindLayer(MainMenuPlateSelectLayerNames[mainMenuSelectedItem]);
				const auto fadeOutStartFrame = layer->FindMarkerFrame("st_sp").value_or(0.0f);
				const auto fadeOutEndFrame = layer->FindMarkerFrame("ed_sp").value_or(0.0f);
				const auto fadeOutDuration = (fadeOutEndFrame - fadeOutStartFrame);

				if (mainMenuFadeOutTime.ToFrames() > fadeOutDuration - 1.0f)
				{
					switch (mainMenuSelectedItem)
					{
					case MenuSubState_Game:
						ChangeRequest = { GameStateType::PS4GameMenu };
						break;
					case MenuSubState_Custom:
						break;
					case MenuSubState_PV:
						break;
					case MenuSubState_MyRoom:
						break;
					case MenuSubState_Store:
						break;
					case MenuSubState_Options:
						break;
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
				if ((mainMenuSelectedItem = static_cast<MenuSubState>(mainMenuSelectedItem - 1)) < MenuSubState_Game)
					mainMenuSelectedItem = MenuSubState_Options;
				mainMenuSelectTime = TimeSpan::Zero();
			}
			else if (Gui::IsKeyPressed(Input::KeyCode_Down))
			{
				if ((mainMenuSelectedItem = static_cast<MenuSubState>(mainMenuSelectedItem + 1)) >= MenuSubState_Count)
					mainMenuSelectedItem = MenuSubState_Game;
				mainMenuSelectTime = TimeSpan::Zero();
			}

			if (Gui::IsKeyPressed(Input::KeyCode_Enter))
				isFadingOut = true;
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

			if (isFadingOut)
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
			const auto loop = (isFadingIn) ? LoopState_In : LoopState_Loop;

			DrawMenuDeco(loop);

			for (MenuSubState i = MenuSubState_Game; i < MenuSubState_Count; i = static_cast<MenuSubState>(i + 1))
				if (i != mainMenuSelectedItem) DrawMainMenuItem(i, loop);

			DrawMainMenuItemSelected(mainMenuSelectedItem, loop);
			DrawMainMenuChara(mainMenuSelectedItem, loop);
			DrawMainMenuText(mainMenuSelectedItem, loop);
		}

	private:
		bool isFadingIn = true;
		bool isFadingOut = false;

		TimeSpan backgroundTime = TimeSpan::Zero();
		TimeSpan headerFooterTime = TimeSpan::Zero();

		TimeSpan mainMenuTime = TimeSpan::Zero();
		TimeSpan mainMenuSelectTime = TimeSpan::Zero();
		TimeSpan mainMenuFadeOutTime = TimeSpan::Zero();
		bool mainMenuTransitionStarted = false;

		MenuSubState mainMenuSelectedItem = MenuSubState_Game;
	};
}
