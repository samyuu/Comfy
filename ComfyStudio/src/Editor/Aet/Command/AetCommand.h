#pragma once
#include "Editor/Command/ICommand.h"

namespace Editor
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

		// NOTE: Used for comparing if two commands modifier the same property of an object
		virtual const void* GetDataIdentifier() { return nullptr; }
	};
}