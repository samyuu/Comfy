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

	static constexpr std::string_view GameStateTypeNameInvalid = "Invalid";

	static constexpr std::array<std::string_view, EnumCount<GameStateType>()> GameStateTypeNames
	{
		"PS4MainMenu",
		"PS4GameMenu",
		"DEBUG_TextMenuSelect",
		"DEBUG_TextGameMenu",
		"DEBUG_TextCustomMenu",
	};

	constexpr std::string_view GetGameStateTypeName(GameStateType gameStateType)
	{
		return IndexOr(static_cast<size_t>(gameStateType), GameStateTypeNames, GameStateTypeNameInvalid);
	}

	constexpr std::string_view GetGameStateTypeName(std::optional<GameStateType> gameStateType)
	{
		return GetGameStateTypeName(gameStateType.value_or(GameStateType::Count));
	}

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
