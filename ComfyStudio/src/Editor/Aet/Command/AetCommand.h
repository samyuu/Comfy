#pragma once
#include "Editor/Command/ICommand.h"

namespace Comfy::Studio::Editor
{
	namespace Command
	{
		enum class AetCommandType;
	}

	class AetCommand : public ICommand
	{
	public:
		// NOTE: Used for comparing the last command with the pending one to perform an update instead
		virtual Command::AetCommandType GetType() = 0;
	};
}
