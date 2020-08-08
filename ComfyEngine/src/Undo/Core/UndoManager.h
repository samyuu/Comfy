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
		template<class CommandType, class... Types>
		void AddToEndOfFrameExecutionList(Types&&... args)
		{
			static_assert(std::is_base_of<ICommand, CommandType>::value, "CommandType must inherit from ICommand");
			endOfFrameCommands.emplace_back(std::make_unique<CommandType>(std::forward<Types>(args)...));
		}

		void FlushExecuteEndOfFrameCommands();

	public:
		void Undo();
		void Redo();
		void Clear();

		bool CanUndo() const;
		bool CanRedo() const;

		const std::vector<std::unique_ptr<ICommand>> GetUndoStack() const;
		const std::vector<std::unique_ptr<ICommand>> GetRedoStack() const;

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
