#include "GameState.h"
#include "Tests/Game/States/DebugGameStates.h"
#include "Tests/Game/States/PS4MainMenu.h"
#include "Tests/Game/States/PS4GameMenu.h"

namespace Comfy::Sandbox::Tests::Game
{
	std::unique_ptr<GameStateBase> GameStateBase::CreateByType(GameStateType type, GameContext& context)
	{
		switch (type)
		{
		case GameStateType::PS4MainMenu:
			return std::make_unique<PS4MainMenu>(context);
		case GameStateType::PS4GameMenu:
			return std::make_unique<PS4GameMenu>(context);

		case GameStateType::DEBUG_TextMenuSelect:
			return std::make_unique<DEBUG_TextMenuSelect>(context);
		case GameStateType::DEBUG_TextGameMenu:
			return std::make_unique<DEBUG_TextGameMenu>(context);
		case GameStateType::DEBUG_TextCustomMenu:
			return std::make_unique<DEBUG_TextCustomMenu>(context);

		default:
			assert(false);
			__fallthrough;

		case GameStateType::Count:
			return nullptr;
		}

		return std::make_unique<DEBUG_TextMenuSelect>(context);
	}
}
