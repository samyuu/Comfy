#pragma once
#include "Types.h"
#include "Undo/Undo.h"
#include "Chart.h"

namespace Comfy::Studio::Editor
{
	class ChangeStartOffset : public Undo::Command
	{
	public:
		ChangeStartOffset(Chart& chart, TimeSpan value)
			: chart(chart), newValue(value), oldValue(chart.GetStartOffset())
		{
		}

	public:
		void Undo() override
		{
			chart.SetStartOffset(oldValue);
		}

		void Redo() override
		{
			chart.SetStartOffset(newValue);
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
			: chart(chart), newValue(value), oldValue(chart.GetDuration())
		{
		}

	public:
		void Undo() override
		{
			chart.SetDuration(oldValue);
		}

		void Redo() override
		{
			chart.SetDuration(newValue);
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
			chart.GetTargets().Remove(tick, type);
		}

		void Redo() override
		{
			chart.GetTargets().Add(tick, type);
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
			chart.GetTargets().Add(tick, type);
		}

		void Redo() override
		{
			// TODO: Store previous properties (?)
			chart.GetTargets().Remove(tick, type);
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
			chart.GetTempoMap().RemoveTempoChange(newValue.Tick);
			chart.GetTimelineMap().CalculateMapTimes(chart.GetTempoMap());
		}

		void Redo() override
		{
			chart.GetTempoMap().SetTempoChange(newValue.Tick, newValue.Tempo, newValue.Signature);
			chart.GetTimelineMap().CalculateMapTimes(chart.GetTempoMap());
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
			: chart(chart), oldValue(chart.GetTempoMap().FindTempoChangeAtTick(tick))
		{
			assert(oldValue.Tick == tick);
		}

	public:
		void Undo() override
		{
			chart.GetTempoMap().SetTempoChange(oldValue.Tick, oldValue.Tempo, oldValue.Signature);
			chart.GetTimelineMap().CalculateMapTimes(chart.GetTempoMap());
		}

		void Redo() override
		{
			chart.GetTempoMap().RemoveTempoChange(oldValue.Tick);
			chart.GetTimelineMap().CalculateMapTimes(chart.GetTempoMap());
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
			: chart(chart), newValue(newValue), oldValue(chart.GetTempoMap().FindTempoChangeAtTick(newValue.Tick))
		{
			assert(newValue.Tick == oldValue.Tick);
		}

	public:
		void Undo() override
		{
			chart.GetTempoMap().SetTempoChange(oldValue.Tick, oldValue.Tempo, oldValue.Signature);
			chart.GetTimelineMap().CalculateMapTimes(chart.GetTempoMap());
		}

		void Redo() override
		{
			chart.GetTempoMap().SetTempoChange(newValue.Tick, newValue.Tempo, newValue.Signature);
			chart.GetTimelineMap().CalculateMapTimes(chart.GetTempoMap());
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
