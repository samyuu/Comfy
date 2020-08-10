#pragma once
#include "Types.h"
#include "Undo/Undo.h"
#include "Chart.h"

namespace Comfy::Studio::Editor
{
	class ChangeStartOffset : public Undo::ICommand
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

		Undo::MergeResult TryMerge(ICommand& commandToMerge) override
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

	class ChangeSongDuration : public Undo::ICommand
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

		Undo::MergeResult TryMerge(ICommand& commandToMerge) override
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

	class AddTarget : public Undo::ICommand
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

		Undo::MergeResult TryMerge(ICommand& commandToMerge) override
		{
			return Undo::MergeResult::Failed;
		}

		std::string_view GetName() const override { return "Add Target"; }

	private:
		Chart& chart;
		TimelineTick tick;
		ButtonType type;
	};

	class RemoveTarget : public Undo::ICommand
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

		Undo::MergeResult TryMerge(ICommand& commandToMerge) override
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
	class AddTempoChange : public Undo::ICommand
	{
	public:
		AddTempoChange(Chart& chart, TimelineTick tick, Tempo tempo)
			: chart(chart), tick(tick), tempo(tempo)
		{
		}

	public:
		void Undo() override
		{
			chart.GetTempoMap().RemoveTempoChange(tick);
			chart.GetTimelineMap().CalculateMapTimes(chart.GetTempoMap());
		}

		void Redo() override
		{
			chart.GetTempoMap().SetTempoChange(tick, tempo);
			chart.GetTimelineMap().CalculateMapTimes(chart.GetTempoMap());
		}

		Undo::MergeResult TryMerge(ICommand& commandToMerge) override
		{
			return Undo::MergeResult::Failed;
		}

		std::string_view GetName() const override { return "Add Tempo Change"; }

	private:
		Chart& chart;
		TimelineTick tick;
		Tempo tempo;
	};

	class RemoveTempoChange : public Undo::ICommand
	{
	public:
		RemoveTempoChange(Chart& chart, TimelineTick tick)
			: chart(chart), tick(tick), oldTempo(chart.GetTempoMap().FindTempoChangeAtTick(tick).Tempo)
		{
		}

	public:
		void Undo() override
		{
			chart.GetTempoMap().SetTempoChange(tick, oldTempo);
			chart.GetTimelineMap().CalculateMapTimes(chart.GetTempoMap());
		}

		void Redo() override
		{
			chart.GetTempoMap().RemoveTempoChange(tick);
			chart.GetTimelineMap().CalculateMapTimes(chart.GetTempoMap());
		}

		Undo::MergeResult TryMerge(ICommand& commandToMerge) override
		{
			return Undo::MergeResult::Failed;
		}

		std::string_view GetName() const override { return "Remove Tempo Change"; }

	private:
		Chart& chart;
		TimelineTick tick;
		Tempo oldTempo;
	};

	class ChangeTempo : public Undo::ICommand
	{
	public:
		ChangeTempo(Chart& chart, TimelineTick tick, Tempo newTempo)
			: chart(chart), tick(tick), newValue(newTempo), oldValue(chart.GetTempoMap().FindTempoChangeAtTick(tick).Tempo)
		{
		}

	public:
		void Undo() override
		{
			chart.GetTempoMap().SetTempoChange(tick, oldValue);
			chart.GetTimelineMap().CalculateMapTimes(chart.GetTempoMap());
		}

		void Redo() override
		{
			chart.GetTempoMap().SetTempoChange(tick, newValue);
			chart.GetTimelineMap().CalculateMapTimes(chart.GetTempoMap());
		}

		Undo::MergeResult TryMerge(ICommand& commandToMerge) override
		{
			auto* other = static_cast<decltype(this)>(&commandToMerge);
			if (&other->chart != &chart)
				return Undo::MergeResult::Failed;

			newValue = other->newValue;
			return Undo::MergeResult::ValueUpdated;
		}

		std::string_view GetName() const override { return "Change Tempo"; }

	private:
		Chart& chart;
		TimelineTick tick;
		Tempo newValue, oldValue;
	};
}
