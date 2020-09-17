#include "UndoManager.h"
#include "Core/Hacks.h"

namespace Comfy::Undo
{
	namespace
	{
		template <typename T>
		auto VectorPop(std::vector<T>& vectorToPop)
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

	void UndoManager::DisallowMergeForLastCommand()
	{
		// NOTE: Set to 1 instead of incrementing in case this gets called multiple times per frame
		numberOfCommandsToDisallowMergesFor = 1;
	}

	TimeSpan UndoManager::GetCommandMergeTimeThreshold() const
	{
		return commandMergeTimeThreshold;
	}

	void UndoManager::SetCommandMergeTimeThreshold(TimeSpan value)
	{
		commandMergeTimeThreshold = value;
	}

	void UndoManager::TryMergeOrExecute(std::unique_ptr<Command> commandToExecute)
	{
		assert(commandToExecute != nullptr);
		hasPendingChanges = true;

		redoStack.clear();
		auto* lastCommand = (!undoStack.empty() ? undoStack.back().get() : nullptr);

		const bool mergeDisallowedByType = (lastCommand == nullptr || !CommandsAreOfSameType(*commandToExecute, *lastCommand));

		// NOTE: This is a bit hacky because it introduces a somewhat unpredictable outside variable of time
		//		 but if so required by the host application it can be disabled by setting the threshold to zero
		//		 and it automatically takes care of merging possibly unrelated commands without adding additional code to all call sites
		const auto timeSinceLastCommand = lastExecutedCommandStopwatch.Restart();
		const bool mergeDisallowedByTime = (commandMergeTimeThreshold > TimeSpan::Zero()) && (timeSinceLastCommand > commandMergeTimeThreshold);

		const bool mergeDisallowedByCounter = (numberOfCommandsToDisallowMergesFor > 0);
		if (numberOfCommandsToDisallowMergesFor > 0)
			numberOfCommandsToDisallowMergesFor--;

		if (!mergeDisallowedByType && !mergeDisallowedByTime && !mergeDisallowedByCounter)
		{
			const auto mergeResult = lastCommand->TryMerge(*commandToExecute);

			if (mergeResult == MergeResult::Failed)
				undoStack.emplace_back(std::move(commandToExecute))->Redo();
			else if (mergeResult == MergeResult::ValueUpdated)
				lastCommand->Redo();
			else
				assert(false);
		}
		else
		{
			undoStack.emplace_back(std::move(commandToExecute))->Redo();
		}
	}

	bool UndoManager::CommandsAreOfSameType(const Command& commandA, const Command& commandB) const
	{
		return Hacks::CompareVirtualFunctionTablePointers(commandA, commandB);
	}

	void UndoManager::Undo(size_t count)
	{
		for (size_t i = 0; i < count; i++)
		{
			if (undoStack.empty())
				break;

			hasPendingChanges = true;
			redoStack.emplace_back(VectorPop(undoStack))->Undo();
		}
	}

	void UndoManager::Redo(size_t count)
	{
		for (size_t i = 0; i < count; i++)
		{
			if (redoStack.empty())
				break;

			hasPendingChanges = true;
			undoStack.emplace_back(VectorPop(redoStack))->Redo();
		}
	}

	void UndoManager::ClearAll()
	{
		hasPendingChanges = false;

		if (!endOfFrameCommands.empty())
			endOfFrameCommands.clear();

		if (!undoStack.empty())
			undoStack.clear();

		if (!redoStack.empty())
			redoStack.clear();
	}

	void UndoManager::SetChangesWereMade()
	{
		hasPendingChanges = true;
	}

	void UndoManager::ClearPendingChangesFlag()
	{
		hasPendingChanges = false;
	}

	bool UndoManager::GetHasPendingChanged() const
	{
		return hasPendingChanges;
	}

	bool UndoManager::CanUndo() const
	{
		return !undoStack.empty();
	}

	bool UndoManager::CanRedo() const
	{
		return !redoStack.empty();
	}

	const std::vector<std::unique_ptr<Command>>& UndoManager::GetUndoStackView() const
	{
		return undoStack;
	}

	const std::vector<std::unique_ptr<Command>>& UndoManager::GetRedoStackView() const
	{
		return redoStack;
	}
}
