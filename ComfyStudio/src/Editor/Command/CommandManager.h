#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "ICommand.h"
#include <queue>

namespace Comfy::Studio::Editor
{
	template <typename TCommand>
	class CommandManager
	{
		static_assert(std::is_base_of<ICommand, TCommand>::value, "TCommand must inherit from ICommand");

		using CommandQueue = std::queue<std::shared_ptr<TCommand>>;
		using CommandStack = std::vector<std::shared_ptr<TCommand>>;

	public:
		CommandManager() = default;
		virtual ~CommandManager() = default;

	public:
		template<class TNewCommand, class... Types>
		void EnqueueCommand(Types&&... _Args);

		void ExecuteClearCommandQueue();

	protected:
		void Execute(const std::shared_ptr<TCommand>& command);

	public:
		void Undo();
		void Redo();
		void Clear();

		inline bool GetCanUndo() const { return !undoStack.empty(); }
		inline bool GetCanRedo() const { return !redoStack.empty(); }

		inline CommandStack& GetUndoStack() { return undoStack; }
		inline CommandStack& GetRedoStack() { return redoStack; }

	protected:
		// Commands to be exectued at the end of the frame.
		// This is to prevent deletion of layers mid-frame that are still being referenced by other layers.
		CommandQueue commandQueue;

		CommandStack undoStack;
		CommandStack redoStack;
	};

	template<class TCommand>
	template<class TNewCommand, class... Types>
	void CommandManager<TCommand>::EnqueueCommand(Types&&... args)
	{
		static_assert(std::is_base_of<TCommand, TNewCommand>::value, "TNewCommand must inherit from TCommand");
		commandQueue.push(std::make_shared<TNewCommand>(std::forward<Types>(args)...));
	}

	template<class TCommand>
	void CommandManager<TCommand>::ExecuteClearCommandQueue()
	{
		while (!commandQueue.empty())
		{
			Execute(commandQueue.front());
			commandQueue.pop();
		}
	}

	template<class TCommand>
	void CommandManager<TCommand>::Execute(const std::shared_ptr<TCommand>& command)
	{
		redoStack.clear();

		command->Do();
		undoStack.push_back(command);
	}

	template<class TCommand>
	void CommandManager<TCommand>::Undo()
	{
		if (undoStack.empty())
			return;

		std::shared_ptr<TCommand> undoCommand = undoStack.back();
		undoStack.erase(undoStack.begin() + undoStack.size() - 1);

		undoCommand->Undo();
		redoStack.push_back(undoCommand);
	}

	template<class TCommand>
	void CommandManager<TCommand>::Redo()
	{
		if (redoStack.empty())
			return;

		std::shared_ptr<TCommand> redoCommand = redoStack.back();
		redoStack.erase(redoStack.begin() + redoStack.size() - 1);

		redoCommand->Redo();
		undoStack.push_back(redoCommand);
	}

	template<class TCommand>
	void CommandManager<TCommand>::Clear()
	{
		if (!commandQueue.empty())
			commandQueue = {};

		if (!undoStack.empty())
			undoStack.clear();
		
		if (!redoStack.empty())
			redoStack.clear();
	}
}
