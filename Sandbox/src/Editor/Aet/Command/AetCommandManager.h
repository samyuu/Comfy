#pragma once
#include "AetCommand.h"
#include "Editor/Command/CommandManager.h"

namespace Editor
{
	class AetCommandManager : public CommandManager<AetCommand>
	{
	};
}