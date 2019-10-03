#pragma once
#include "Types.h"
#include "ICommand.h"
#include "Core/CoreTypes.h"
#include <queue>

namespace Editor
{
	template <class TCommand>
	class CommandManager
	{
		static_assert(std::is_base_of<ICommand, TCommand>::value, "TCommand must inherit from ICommand");

		using CommandQueue = std::queue<RefPtr<TCommand>>;
		using CommandStack = Vector<RefPtr<TCommand>>;

	public:
		CommandManager() {};
		virtual ~CommandManager() {};

	public:
		template<class TNewCommand, class... _Types>
		void EnqueueCommand(_Types&&... _Args);

		void ExecuteClearCommandQueue();

	protected:
		void Execute(const RefPtr<TCommand>& command);

	public:
		void Undo();
		void Redo();
		void Clear();

		inline bool GetCanUndo() const { return !undoStack.empty(); };
		inline bool GetCanRedo() const { return !redoStack.empty(); };

		inline CommandStack& GetUndoStack() { return undoStack; };
		inline CommandStack& GetRedoStack() { return redoStack; };

	protected:
		// Commands to be exectued at the end of the frame.
		// This is to prevent deletion of objects mid-frame that are still being referenced by other objects.
		CommandQueue commandQueue;

		CommandStack undoStack;
		CommandStack redoStack;
	};

	template<class TCommand>
	template<class TNewCommand, class... _Types>
	inline void CommandManager<TCommand>::EnqueueCommand(_Types&&... args)
	{
		static_assert(std::is_base_of<TCommand, TNewCommand>::value, "TNewCommand must inherit from TCommand");
		commandQueue.push(MakeRef<TNewCommand>(std::forward<_Types>(args)...));
	}

	template<class TCommand>
	inline void CommandManager<TCommand>::ExecuteClearCommandQueue()
	{
		while (!commandQueue.empty())
		{
			Execute(commandQueue.front());
			commandQueue.pop();
		}
	}

	template<class TCommand>
	inline void CommandManager<TCommand>::Execute(const RefPtr<TCommand>& command)
	{
		redoStack.clear();

		command->Do();
		undoStack.push_back(command);
	}

	template<class TCommand>
	inline void CommandManager<TCommand>::Undo()
	{
		if (undoStack.empty())
			return;

		RefPtr<TCommand> undoCommand = undoStack.back();
		undoStack.erase(undoStack.begin() + undoStack.size() - 1);

		undoCommand->Undo();
		redoStack.push_back(undoCommand);
	}

	template<class TCommand>
	inline void CommandManager<TCommand>::Redo()
	{
		if (redoStack.empty())
			return;

		RefPtr<TCommand> redoCommand = redoStack.back();
		redoStack.erase(redoStack.begin() + redoStack.size() - 1);

		redoCommand->Redo();
		undoStack.push_back(redoCommand);
	}

	template<class TCommand>
	inline void CommandManager<TCommand>::Clear()
	{
		if (!commandQueue.empty())
			commandQueue = {};

		if (!undoStack.empty())
			undoStack.clear();
		
		if (!redoStack.empty())
			redoStack.clear();
	}
}