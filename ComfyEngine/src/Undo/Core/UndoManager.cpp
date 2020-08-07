#include "UndoManager.h"
#include "Core/Hacks.h"

namespace Comfy::Undo
{
	namespace
	{
		template <typename VectorType>
		auto VectorPop(VectorType& vectorToPop)
		{
			auto popped = std::move(vectorToPop.back());
			vectorToPop.erase(vectorToPop.end() - 1);

			return popped;
		}
	}

	UndoManager::UndoManager()
	{
		constexpr auto reasonableInitialCapacity = 64;

		endOfFrameCommands.reserve(reasonableInitialCapacity);
		undoStack.reserve(reasonableInitialCapacity);
		redoStack.reserve(reasonableInitialCapacity);
	}

	void UndoManager::FlushExecuteEndOfFrameCommands()
	{
		if (endOfFrameCommands.empty())
			return;

		for (auto& commandToExecute : endOfFrameCommands)
			TryMergeOrExecute(std::move(commandToExecute));

		endOfFrameCommands.clear();
	}

	void UndoManager::TryMergeOrExecute(std::unique_ptr<ICommand> commandToExecute)
	{
		redoStack.clear();
		if (undoStack.empty() || !commandMergingEnabled)
		{
			undoStack.emplace_back(std::move(commandToExecute))->Redo();
		}
		else
		{
			const auto& lastCommand = undoStack.back();
			const auto typeMatch = Hacks::CompareVirtualFunctionTablePointers(*commandToExecute, *lastCommand);

			if (!typeMatch || lastCommand->TryMerge(*commandToExecute) == MergeResult::FailedToMerge)
				undoStack.emplace_back(std::move(commandToExecute))->Redo();
		}
	}

	void UndoManager::Undo()
	{
		if (!undoStack.empty())
			redoStack.emplace_back(VectorPop(undoStack))->Undo();
	}

	void UndoManager::Redo()
	{
		if (!redoStack.empty())
			undoStack.emplace_back(VectorPop(redoStack))->Redo();
	}

	void UndoManager::Clear()
	{
		if (!endOfFrameCommands.empty())
			endOfFrameCommands.clear();

		if (!undoStack.empty())
			undoStack.clear();

		if (!redoStack.empty())
			redoStack.clear();
	}

	bool UndoManager::CanUndo() const
	{
		return !undoStack.empty();
	}

	bool UndoManager::CanRedo() const
	{
		return !redoStack.empty();
	}

	void UndoManager::SetCommandMergingEnabled(bool value)
	{
		commandMergingEnabled = value;
	}
}
