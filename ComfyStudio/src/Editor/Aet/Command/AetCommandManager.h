#pragma once
#include "AetCommand.h"
#include "Editor/Command/CommandManager.h"

// NOTE: Use messy macros to automatically handle the command type enum
#define ProcessUpdatingAetCommand(commandManager, type, ref, value) commandManager->AddOrUpdateCommand<Editor::Command::type>(Editor::Command::AetCommandType::type, ref, value)

namespace Comfy::Editor
{
	class AetCommandManager : public CommandManager<AetCommand>
	{
	public:
		template <typename TCommand, typename TRef, typename TValue>
		void AddOrUpdateCommand(Command::AetCommandType commandType, const std::shared_ptr<TRef>& ref, const TValue& value)
		{
			AetCommand* lastStackCommand = !undoStack.empty() ? undoStack.back().get() : nullptr;
			TCommand* lastCommand = (lastStackCommand != nullptr && lastStackCommand->GetType() == commandType) ? static_cast<TCommand*>(lastStackCommand) : nullptr;

			if (lastCommand != nullptr && lastCommand->ref == ref)
			{
				const std::shared_ptr<TCommand> newCommand = std::make_shared<TCommand>(ref, value);
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
