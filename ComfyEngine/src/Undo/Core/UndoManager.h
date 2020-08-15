#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Command.h"

namespace Comfy::Undo
{
	class UndoManager : NonCopyable
	{
	public:
		UndoManager();
		virtual ~UndoManager() = default;

	public:
		template<typename CommandType, typename... Args>
		void Execute(Args&&... args)
		{
			static_assert(std::is_base_of_v<Command, CommandType>, "CommandType must inherit from Undo::Command");
			TryMergeOrExecute(std::make_unique<CommandType>(std::forward<Args>(args)...));
		}

		template<typename CommandType, typename... Args>
		void ExecuteEndOfFrame(Args&&... args)
		{
			static_assert(std::is_base_of_v<Command, CommandType>, "CommandType must inherit from Undo::Command");
			endOfFrameCommands.emplace_back(std::make_unique<CommandType>(std::forward<Args>(args)...));
		}

		void FlushExecuteEndOfFrameCommands();

	public:
		void Undo(size_t count = 1);
		void Redo(size_t count = 1);

		void ClearAll();

		// NOTE: Specifically for visualization, attempting to undo / redo an empty stack is a no-op
		bool CanUndo() const;
		bool CanRedo() const;

		// NOTE: Specifically for command history visualization
		const std::vector<std::unique_ptr<Command>>& GetUndoStackView() const;
		const std::vector<std::unique_ptr<Command>>& GetRedoStackView() const;

		// TODO: Implement some kind of interface to easily disallow merges with the last added command
		//		 for when a mouse button is released or a text box lost focus etc. (?)

	private:
		void TryMergeOrExecute(std::unique_ptr<Command> commandToExecute);
		bool CommandsAreOfSameType(const Command& commandA, const Command& commandB) const;

	private:
		// NOTE: To prevent mid-frame stale pointer access errors
		std::vector<std::unique_ptr<Command>> endOfFrameCommands;

		std::vector<std::unique_ptr<Command>> undoStack, redoStack;
	};
}
