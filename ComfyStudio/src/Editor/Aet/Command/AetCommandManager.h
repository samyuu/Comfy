#pragma once
#include "AetCommand.h"
#include "Editor/Command/CommandManager.h"

// NOTE: Use messy macros to automatically handle the command type enum
#define ProcessUpdatingAetCommand(commandManager, type, ref, value) commandManager->AddOrUpdateCommand<Editor::Command::type>(Editor::Command::AetCommandType::type, ref, value)

namespace Editor
{
	class AetCommandManager : public CommandManager<AetCommand>
	{
	public:
		template <typename TCommand, typename TRef, typename TValue>
		inline void AddOrUpdateCommand(Command::AetCommandType commandType, const RefPtr<TRef>& ref, const TValue& value)
		{
			AetCommand* lastStackCommand = !undoStack.empty() ? undoStack.back().get() : nullptr;
			TCommand* lastCommand = (lastStackCommand != nullptr && lastStackCommand->GetType() == commandType) ? static_cast<TCommand*>(lastStackCommand) : nullptr;

			if (lastCommand != nullptr && lastCommand->ref == ref)
			{
				const RefPtr<TCommand> newCommand = MakeRef<TCommand>(ref, value);
				if (lastCommand->CanUpdate(newCommand.get()))
				{
					lastCommand->Update(value);
				}
				else
				{
					commandQueue.push(newCommand);
				}
			}
			else
			{
				EnqueueCommand<TCommand>(ref, value);
			}
		}
	};
}