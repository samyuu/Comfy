#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Tests/Game/Core/GameState.h"

namespace Comfy::Sandbox::Tests::Game
{
	/*
		TEST MENU SELECT:

		- [1] CHANGE TO TEST GAME MENU
		- [2] CHANGE TO TEST GAME MENU (FADE)
		- [3] CHANGE TO TEST CUSTOM MENU
		- [4] CHANGE TO TEST CUSTOM MENU (FADE)
	*/

	/*
		TEST GAME MENU:

		- [1] CHANGE TO TEST MENU SELECT
		- [2] CHANGE TO TEST MENU SELECT (FADE)
	*/

	/*
		TEST CUSTOM MENU:

		- [1] CHANGE TO TEST MENU SELECT
		- [2] CHANGE TO TEST MENU SELECT (FADE)
	*/

	static constexpr float TestMenuItemBaseHeight = 80.0f;
	static constexpr float TestMenuItemHeightSpacing = 42.0f;

	static constexpr std::array<vec2, 5> TestMenuItemTextPositions =
	{
		vec2(36.0f, 0.0f),
		vec2(0.0f, TestMenuItemBaseHeight + TestMenuItemHeightSpacing * 0),
		vec2(0.0f, TestMenuItemBaseHeight + TestMenuItemHeightSpacing * 1),
		vec2(0.0f, TestMenuItemBaseHeight + TestMenuItemHeightSpacing * 2),
		vec2(0.0f, TestMenuItemBaseHeight + TestMenuItemHeightSpacing * 3),
	};

	class DEBUG_TextGameMenu : public GameStateBase
	{
	public:
		DEBUG_TextGameMenu(GameContext& context) : GameStateBase(context) {}

	public:
		void OnFocusGained(std::optional<GameStateType> previousState)
		{
		}

		void OnFocusLost(std::optional<GameStateType> newState)
		{
		}

		void OnUpdateInput() override
		{
			if (Gui::IsKeyPressed(Input::KeyCode_1, false)) { ChangeRequest = { GameStateType::DEBUG_TextMenuSelect, false }; }
			if (Gui::IsKeyPressed(Input::KeyCode_2, false)) { ChangeRequest = { GameStateType::DEBUG_TextMenuSelect, true }; }
		}

		void OnDraw() override
		{
			context.Renderer.Font().DrawShadow(*context.Font36, "TEST GAME MENU:", Transform2D(TestMenuItemTextPositions[0]));
			context.Renderer.Font().DrawShadow(*context.Font36, "- [1] CHANGE TO TEST MENU SELECT", Transform2D(TestMenuItemTextPositions[1]));
			context.Renderer.Font().DrawShadow(*context.Font36, "- [2] CHANGE TO TEST MENU SELECT (FADE)", Transform2D(TestMenuItemTextPositions[2]));
		}
	};

	class DEBUG_TextCustomMenu : public GameStateBase
	{
	public:
		DEBUG_TextCustomMenu(GameContext& context) : GameStateBase(context) {}

	public:
		void OnFocusGained(std::optional<GameStateType> previousState)
		{
		}
		
		void OnFocusLost(std::optional<GameStateType> newState)
		{
		}

		void OnUpdateInput() override
		{
			if (Gui::IsKeyPressed(Input::KeyCode_1, false)) { ChangeRequest = { GameStateType::DEBUG_TextMenuSelect, false }; }
			if (Gui::IsKeyPressed(Input::KeyCode_2, false)) { ChangeRequest = { GameStateType::DEBUG_TextMenuSelect, true }; }
		}

		void OnDraw() override
		{
			context.Renderer.Font().DrawShadow(*context.Font36, "TEST CUSTOM MENU:", Transform2D(TestMenuItemTextPositions[0]));
			context.Renderer.Font().DrawShadow(*context.Font36, "- [1] CHANGE TO TEST MENU SELECT", Transform2D(TestMenuItemTextPositions[1]));
			context.Renderer.Font().DrawShadow(*context.Font36, "- [2] CHANGE TO TEST MENU SELECT (FADE)", Transform2D(TestMenuItemTextPositions[2]));
		}
	};

	class DEBUG_TextMenuSelect : public GameStateBase
	{
	public:
		DEBUG_TextMenuSelect(GameContext& context) : GameStateBase(context) {}

	public:
		void OnFocusGained(std::optional<GameStateType> previousState)
		{
		}

		void OnFocusLost(std::optional<GameStateType> newState)
		{
		}

		void OnUpdateInput() override
		{
			if (Gui::IsKeyPressed(Input::KeyCode_1, false)) { ChangeRequest = { GameStateType::DEBUG_TextGameMenu, false }; }
			if (Gui::IsKeyPressed(Input::KeyCode_2, false)) { ChangeRequest = { GameStateType::DEBUG_TextGameMenu, true }; }
			if (Gui::IsKeyPressed(Input::KeyCode_3, false)) { ChangeRequest = { GameStateType::DEBUG_TextCustomMenu, false }; }
			if (Gui::IsKeyPressed(Input::KeyCode_4, false)) { ChangeRequest = { GameStateType::DEBUG_TextCustomMenu, true }; }
		}

		void OnDraw() override
		{
			context.Renderer.Font().DrawShadow(*context.Font36, "TEST MENU SELECT:", Transform2D(TestMenuItemTextPositions[0]));
			context.Renderer.Font().DrawShadow(*context.Font36, "- [1] CHANGE TO TEST GAME MENU", Transform2D(TestMenuItemTextPositions[1]));
			context.Renderer.Font().DrawShadow(*context.Font36, "- [2] CHANGE TO TEST GAME MENU (FADE)", Transform2D(TestMenuItemTextPositions[2]));
			context.Renderer.Font().DrawShadow(*context.Font36, "- [3] CHANGE TO TEST CUSTOM MENU", Transform2D(TestMenuItemTextPositions[3]));
			context.Renderer.Font().DrawShadow(*context.Font36, "- [4] CHANGE TO TEST CUSTOM MENU (FADE)", Transform2D(TestMenuItemTextPositions[4]));
		}
	};
}
