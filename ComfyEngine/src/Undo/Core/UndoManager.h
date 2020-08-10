#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "ICommand.h"

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
			static_assert(std::is_base_of<ICommand, CommandType>::value, "CommandType must inherit from ICommand");
			TryMergeOrExecute(std::make_unique<CommandType>(std::forward<Args>(args)...));
		}

		template<typename CommandType, typename... Args>
		void ExecuteEndOfFrame(Args&&... args)
		{
			static_assert(std::is_base_of<ICommand, CommandType>::value, "CommandType must inherit from ICommand");
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
		const std::vector<std::unique_ptr<ICommand>>& GetUndoStackView() const;
		const std::vector<std::unique_ptr<ICommand>>& GetRedoStackView() const;

		// TODO: Implement some kind of interface to easily disallow merges with the last added command
		//		 for when a mouse button is released or a text box lost focus etc. (?)
		void SetCommandMergingEnabled(bool value);

	private:
		void TryMergeOrExecute(std::unique_ptr<ICommand> commandToExecute);

	private:
		bool commandMergingEnabled = true;

		// NOTE: To prevent mid-frame stale pointer access errors
		std::vector<std::unique_ptr<ICommand>> endOfFrameCommands;

		std::vector<std::unique_ptr<ICommand>> undoStack, redoStack;
	};
}
