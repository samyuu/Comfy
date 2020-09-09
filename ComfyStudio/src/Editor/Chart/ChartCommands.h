#pragma once
#include "Types.h"
#include "Undo/Undo.h"
#include "Chart.h"
#include "TargetPropertyRules.h"

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
		AddTarget(Chart& chart, TimelineTarget target)
			: chart(chart), target(target)
		{
		}

	public:
		void Undo() override
		{
			chart.Targets.Remove(target);
		}

		void Redo() override
		{
			chart.Targets.Add(target);
		}

		Undo::MergeResult TryMerge(Command& commandToMerge) override
		{
			return Undo::MergeResult::Failed;
		}

		std::string_view GetName() const override { return "Add Target"; }

	private:
		Chart& chart;
		TimelineTarget target;
	};

	class RemoveTarget : public Undo::Command
	{
	public:
		RemoveTarget(Chart& chart, TimelineTarget target)
			: chart(chart), target(target)
		{
		}

	public:
		void Undo() override
		{
			chart.Targets.Add(target);
		}

		void Redo() override
		{
			// TODO: Store previous properties (?)
			chart.Targets.Remove(target);
		}

		Undo::MergeResult TryMerge(Command& commandToMerge) override
		{
			return Undo::MergeResult::Failed;
		}

		std::string_view GetName() const override { return "Remove Target"; }

	private:
		Chart& chart;
		TimelineTarget target;
	};

	class RemoveTargetList : public Undo::Command
	{
	public:
		RemoveTargetList(Chart& chart, std::vector<TimelineTarget> targets)
			: chart(chart), targets(std::move(targets))
		{
		}

	public:
		void Undo() override
		{
			for (auto& target : targets)
				chart.Targets.Add(target);
		}

		void Redo() override
		{
			for (auto& target : targets)
				chart.Targets.Remove(target);
		}

		Undo::MergeResult TryMerge(Command& commandToMerge) override
		{
			return Undo::MergeResult::Failed;
		}

		std::string_view GetName() const override { return "Remove Targets"; }

	private:
		Chart& chart;
		std::vector<TimelineTarget> targets;
	};

	class MoveTargetList : public Undo::Command
	{
	public:
		MoveTargetList(Chart& chart, std::vector<i32> targetIndices, TimelineTick tickIncrement, TimelineTick tickOnStart)
			: chart(chart), targetIndices(std::move(targetIndices)), tickIncrement(tickIncrement), tickOnStart(tickOnStart)
		{
		}

	public:
		void Undo() override
		{
			for (const auto index : targetIndices)
				chart.Targets[index].Tick -= tickIncrement;
		}

		void Redo() override
		{
			for (const auto index : targetIndices)
				chart.Targets[index].Tick += tickIncrement;
		}

		Undo::MergeResult TryMerge(Command& commandToMerge) override
		{
			auto* other = static_cast<decltype(this)>(&commandToMerge);
			if (&other->chart != &chart || other->tickOnStart != tickOnStart)
				return Undo::MergeResult::Failed;

			if (!std::equal(targetIndices.begin(), targetIndices.end(), other->targetIndices.begin(), other->targetIndices.end()))
				return Undo::MergeResult::Failed;

			Undo();
			tickIncrement += other->tickIncrement;

			return Undo::MergeResult::ValueUpdated;
		}

		std::string_view GetName() const override { return "Move Targets"; }

	private:
		Chart& chart;
		std::vector<i32> targetIndices;
		TimelineTick tickIncrement, tickOnStart;
	};

	class ChangeTargetListProperties : public Undo::Command
	{
	public:
		struct Data
		{
			i32 TargetIndex;
			bool HadProperties;
			TargetProperties NewValue, OldValue;
		};

	public:
		ChangeTargetListProperties(Chart& chart, std::vector<Data> data, TargetPropertyFlags propertyFlags)
			: chart(chart), targetData(std::move(data)), propertyFlags(propertyFlags)
		{
			for (auto& data : targetData)
			{
				auto& target = chart.Targets[data.TargetIndex];
				data.HadProperties = target.Flags.HasProperties;
				data.OldValue = target.Properties;
			}
		}

	public:
		void Undo() override
		{
			for (const auto& data : targetData)
			{
				auto& target = chart.Targets[data.TargetIndex];
				for (TargetPropertyType p = 0; p < TargetPropertyType_Count; p++)
				{
					if (propertyFlags & (1 << p))
					{
						target.Flags.HasProperties = data.HadProperties;
						target.Properties[p] = data.OldValue[p];
					}
				}
			}
		}

		void Redo() override
		{
			for (const auto& data : targetData)
			{
				auto& target = chart.Targets[data.TargetIndex];

				if (!data.HadProperties)
				{
					target.Properties = Rules::PresetTargetProperties(target.Type, target.Tick, target.Flags);
					target.Flags.HasProperties = true;
				}

				for (TargetPropertyType p = 0; p < TargetPropertyType_Count; p++)
				{
					if (propertyFlags & (1 << p))
						target.Properties[p] = data.NewValue[p];
				}
			}
		}

		Undo::MergeResult TryMerge(Command& commandToMerge) override
		{
			auto* other = static_cast<decltype(this)>(&commandToMerge);
			if (&other->chart != &chart || other->propertyFlags != propertyFlags)
				return Undo::MergeResult::Failed;

			if (other->targetData.size() != targetData.size())
				return Undo::MergeResult::Failed;

			for (size_t i = 0; i < targetData.size(); i++)
			{
				if (targetData[i].TargetIndex != other->targetData[i].TargetIndex)
					return Undo::MergeResult::Failed;
			}

			for (size_t i = 0; i < targetData.size(); i++)
				targetData[i].NewValue = other->targetData[i].NewValue;

			return Undo::MergeResult::ValueUpdated;
		}

		std::string_view GetName() const override { return "Change Target Properties"; }

	private:
		Chart& chart;
		std::vector<Data> targetData;
		TargetPropertyFlags propertyFlags;
	};

	class ChangeTargetListHasProperties : public Undo::Command
	{
	public:
		struct Data
		{
			i32 TargetIndex;
			bool HadProperties;
		};

	public:
		ChangeTargetListHasProperties(Chart& chart, std::vector<Data> data, bool newHasProperties)
			: chart(chart), targetData(std::move(data)), newHasProperties(newHasProperties)
		{
			for (auto& data : targetData)
				data.HadProperties = chart.Targets[data.TargetIndex].Flags.HasProperties;
		}

	public:
		void Undo() override
		{
			for (const auto& data : targetData)
				chart.Targets[data.TargetIndex].Flags.HasProperties = data.HadProperties;
		}

		void Redo() override
		{
			for (const auto& data : targetData)
			{
				auto& target = chart.Targets[data.TargetIndex];

				if (target.Flags.HasProperties != newHasProperties)
				{
					target.Properties = Rules::PresetTargetProperties(target.Type, target.Tick, target.Flags);
					target.Flags.HasProperties = newHasProperties;
				}
			}
		}

		Undo::MergeResult TryMerge(Command& commandToMerge) override
		{
			return Undo::MergeResult::Failed;
		}

		std::string_view GetName() const override { return "Change Targets Use Presets"; }

	private:
		Chart& chart;
		std::vector<Data> targetData;
		bool newHasProperties;
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
