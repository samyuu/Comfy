#pragma once
#include "Types.h"
#include "Undo/Undo.h"
#include "Chart.h"

namespace Comfy::Studio::Editor
{
	class ChangeSongName : public Undo::Command
	{
	public:
		ChangeSongName(Chart& chart, std::string value)
			: chart(chart), newValue(std::move(value)), oldValue(chart.SongName)
		{
		}

	public:
		void Undo() override
		{
			chart.SongName = oldValue;
		}

		void Redo() override
		{
			chart.SongName = newValue;
		}

		Undo::MergeResult TryMerge(Command& commandToMerge) override
		{
			auto* other = static_cast<decltype(this)>(&commandToMerge);
			if (&other->chart != &chart)
				return Undo::MergeResult::Failed;

			newValue = std::move(other->newValue);
			return Undo::MergeResult::ValueUpdated;
		}

		std::string_view GetName() const override { return "Change Song Name"; }

	private:
		Chart& chart;
		std::string newValue, oldValue;
	};

	class ChangeStartOffset : public Undo::Command
	{
	public:
		ChangeStartOffset(Chart& chart, TimeSpan value)
			: chart(chart), newValue(value), oldValue(chart.StartOffset)
		{
		}

	public:
		void Undo() override
		{
			chart.StartOffset = oldValue;
		}

		void Redo() override
		{
			chart.StartOffset = newValue;
		}

		Undo::MergeResult TryMerge(Command& commandToMerge) override
		{
			auto* other = static_cast<decltype(this)>(&commandToMerge);
			if (&other->chart != &chart)
				return Undo::MergeResult::Failed;

			newValue = other->newValue;
			return Undo::MergeResult::ValueUpdated;
		}

		std::string_view GetName() const override { return "Change Offset"; }

	private:
		Chart& chart;
		TimeSpan newValue, oldValue;
	};

	class ChangeSongDuration : public Undo::Command
	{
	public:
		ChangeSongDuration(Chart& chart, TimeSpan value)
			: chart(chart), newValue(value), oldValue(chart.Duration)
		{
		}

	public:
		void Undo() override
		{
			chart.Duration = oldValue;
		}

		void Redo() override
		{
			chart.Duration = newValue;
		}

		Undo::MergeResult TryMerge(Command& commandToMerge) override
		{
			auto* other = static_cast<decltype(this)>(&commandToMerge);
			if (&other->chart != &chart)
				return Undo::MergeResult::Failed;

			newValue = other->newValue;
			return Undo::MergeResult::ValueUpdated;
		}

		std::string_view GetName() const override { return "Change Duration"; }

	private:
		Chart& chart;
		TimeSpan newValue, oldValue;
	};

	class AddTarget : public Undo::Command
	{
	public:
		AddTarget(Chart& chart, TimelineTick tick, ButtonType type)
			: chart(chart), tick(tick), type(type)
		{
		}

	public:
		void Undo() override
		{
			chart.Targets.Remove(tick, type);
		}

		void Redo() override
		{
			chart.Targets.Add(tick, type);
		}

		Undo::MergeResult TryMerge(Command& commandToMerge) override
		{
			return Undo::MergeResult::Failed;
		}

		std::string_view GetName() const override { return "Add Target"; }

	private:
		Chart& chart;
		TimelineTick tick;
		ButtonType type;
	};

	class RemoveTarget : public Undo::Command
	{
	public:
		RemoveTarget(Chart& chart, TimelineTick tick, ButtonType type)
			: chart(chart), tick(tick), type(type)
		{
		}

	public:
		void Undo() override
		{
			chart.Targets.Add(tick, type);
		}

		void Redo() override
		{
			// TODO: Store previous properties (?)
			chart.Targets.Remove(tick, type);
		}

		Undo::MergeResult TryMerge(Command& commandToMerge) override
		{
			return Undo::MergeResult::Failed;
		}

		std::string_view GetName() const override { return "Remove Target"; }

	private:
		Chart& chart;
		TimelineTick tick;
		ButtonType type;
	};
}

namespace Comfy::Studio::Editor
{
	class AddTempoChange : public Undo::Command
	{
	public:
		AddTempoChange(Chart& chart, TempoChange newValue)
			: chart(chart), newValue(newValue)
		{
		}

	public:
		void Undo() override
		{
			chart.TempoMap.RemoveTempoChange(newValue.Tick);
			chart.UpdateMapTimes();
		}

		void Redo() override
		{
			chart.TempoMap.SetTempoChange(newValue.Tick, newValue.Tempo, newValue.Signature);
			chart.UpdateMapTimes();
		}

		Undo::MergeResult TryMerge(Command& commandToMerge) override
		{
			return Undo::MergeResult::Failed;
		}

		std::string_view GetName() const override { return "Add Tempo Change"; }

	private:
		Chart& chart;
		TempoChange newValue;
	};

	class RemoveTempoChange : public Undo::Command
	{
	public:
		RemoveTempoChange(Chart& chart, TimelineTick tick)
			: chart(chart), oldValue(chart.TempoMap.FindTempoChangeAtTick(tick))
		{
			assert(oldValue.Tick == tick);
		}

	public:
		void Undo() override
		{
			chart.TempoMap.SetTempoChange(oldValue.Tick, oldValue.Tempo, oldValue.Signature);
			chart.UpdateMapTimes();
		}

		void Redo() override
		{
			chart.TempoMap.RemoveTempoChange(oldValue.Tick);
			chart.UpdateMapTimes();
		}

		Undo::MergeResult TryMerge(Command& commandToMerge) override
		{
			return Undo::MergeResult::Failed;
		}

		std::string_view GetName() const override { return "Remove Tempo Change"; }

	private:
		Chart& chart;
		TempoChange oldValue;
	};

	class UpdateTempoChange : public Undo::Command
	{
	public:
		UpdateTempoChange(Chart& chart, TempoChange newValue)
			: chart(chart), newValue(newValue), oldValue(chart.TempoMap.FindTempoChangeAtTick(newValue.Tick))
		{
			assert(newValue.Tick == oldValue.Tick);
		}

	public:
		void Undo() override
		{
			chart.TempoMap.SetTempoChange(oldValue.Tick, oldValue.Tempo, oldValue.Signature);
			chart.UpdateMapTimes();
		}

		void Redo() override
		{
			chart.TempoMap.SetTempoChange(newValue.Tick, newValue.Tempo, newValue.Signature);
			chart.UpdateMapTimes();
		}

		Undo::MergeResult TryMerge(Command& commandToMerge) override
		{
			auto* other = static_cast<decltype(this)>(&commandToMerge);
			if (&other->chart != &chart || other->newValue.Tick != newValue.Tick)
				return Undo::MergeResult::Failed;

			newValue = other->newValue;
			return Undo::MergeResult::ValueUpdated;
		}

		std::string_view GetName() const override { return "Update Tempo Change"; }

	private:
		Chart& chart;
		TempoChange newValue, oldValue;
	};
}
