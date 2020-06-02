#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Tests/Game/Core/GameContext.h"

namespace Comfy::Sandbox::Tests::Game
{
	enum class GameStateType
	{
		// PS4Adv,
		PS4MainMenu,
		PS4GameMenu,
		DEBUG_TextMenuSelect,
		DEBUG_TextGameMenu,
		DEBUG_TextCustomMenu,
		Count
	};

	static constexpr std::array<std::string_view, static_cast<size_t>(GameStateType::Count)> GameStateTypeNames
	{
		"PS4MainMenu",
		"PS4GameMenu",
		"DEBUG_TextMenuSelect",
		"DEBUG_TextGameMenu",
		"DEBUG_TextCustomMenu",
	};

	struct GameStateChangeRequest
	{
		GameStateType NewState;
		bool Fade = true;
	};

	class GameStateBase
	{
	public:
		GameStateBase(GameContext& context) : context(context) {}
		virtual ~GameStateBase() = default;

	public:
		virtual void OnFocusGained(std::optional<GameStateType> previousState) = 0;
		virtual void OnFocusLost(std::optional<GameStateType> newState) = 0;

		virtual void OnUpdateInput() = 0;
		virtual void OnDraw() = 0;

	public:
		static std::unique_ptr<GameStateBase> CreateByType(GameStateType type, GameContext& context);

	public:
		std::optional<GameStateChangeRequest> ChangeRequest = {};

	protected:
		GameContext& context;
	};
}
