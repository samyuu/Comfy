#pragma once
#include "Types.h"
#include "Undo/Undo.h"
#include "Chart.h"
#include "TargetPropertyRules.h"

namespace Comfy::Studio::Editor
{
	class ChangeStartOffset : public Undo::Command
	{
	public:
		ChangeStartOffset(Chart& chart, TimeSpan value) : chart(chart), newValue(value), oldValue(chart.StartOffset) {}

	public:
		void Undo() override { chart.StartOffset = oldValue; }
		void Redo() override { chart.StartOffset = newValue; }

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
		ChangeSongDuration(Chart& chart, TimeSpan value) : chart(chart), newValue(value), oldValue(chart.Duration) {}

	public:
		void Undo() override { chart.Duration = oldValue; }
		void Redo() override { chart.Duration = newValue; }

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
}

namespace Comfy::Studio::Editor
{
	class AddTarget : public Undo::Command
	{
	public:
		AddTarget(Chart& chart, TimelineTarget target) : chart(chart), target(target) {}

	public:
		void Undo() override { chart.Targets.Remove(target); }
		void Redo() override { chart.Targets.Add(target); }

		Undo::MergeResult TryMerge(Command& commandToMerge) override { return Undo::MergeResult::Failed; }
		std::string_view GetName() const override { return "Add Target"; }

	private:
		Chart& chart;
		TimelineTarget target;
	};

	class AddTargetList : public Undo::Command
	{
	public:
		AddTargetList(Chart& chart, std::vector<TimelineTarget> targets) : chart(chart), targets(std::move(targets)) {}

	public:
		void Undo() override
		{
			for (const auto& target : targets)
				chart.Targets.Remove(target);
		}

		void Redo() override
		{
			// TODO: Optimizations for working on known size lists
			for (const auto& target : targets)
				chart.Targets.Add(target);
		}

		Undo::MergeResult TryMerge(Command& commandToMerge) override { return Undo::MergeResult::Failed; }
		std::string_view GetName() const override { return "Add Targets"; }

	private:
		Chart& chart;
		std::vector<TimelineTarget> targets;
	};

	class PasteTargetList : public AddTargetList
	{
	public:
		using AddTargetList::AddTargetList;

	public:
		std::string_view GetName() const override { return "Paste Targets"; }
	};

	class RemoveTarget : public Undo::Command
	{
	public:
		// TODO: i32 targetIndex then store old properties
		RemoveTarget(Chart& chart, TimelineTarget target) : chart(chart), target(target) {}

	public:
		void Undo() override { chart.Targets.Add(target); }
		void Redo() override { chart.Targets.Remove(target); }

		Undo::MergeResult TryMerge(Command& commandToMerge) override { return Undo::MergeResult::Failed; }
		std::string_view GetName() const override { return "Remove Target"; }

	private:
		Chart& chart;
		TimelineTarget target;
	};

	class RemoveTargetList : public Undo::Command
	{
	public:
		// TODO: i32 targetIndex then store old properties
		RemoveTargetList(Chart& chart, std::vector<TimelineTarget> targets) : chart(chart), targets(std::move(targets)) {}

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

		Undo::MergeResult TryMerge(Command& commandToMerge) override { return Undo::MergeResult::Failed; }
		std::string_view GetName() const override { return "Remove Targets"; }

	private:
		Chart& chart;
		std::vector<TimelineTarget> targets;
	};

	class CutTargetList : public RemoveTargetList
	{
	public:
		using RemoveTargetList::RemoveTargetList;

	public:
		std::string_view GetName() const override { return "Cut Targets"; }
	};

	class ChangeTargetListTypes : public Undo::Command
	{
	public:
		struct Data
		{
			i32 TargetIndex;
			ButtonType NewValue, OldValue;
		};

	public:
		ChangeTargetListTypes(Chart& chart, std::vector<Data> data)
			: chart(chart), targetData(std::move(data))
		{
			auto compareIndex = [](auto& a, auto& b) { return (a.TargetIndex < b.TargetIndex); };
			minIndex = targetData.empty() ? 0 : std::min_element(targetData.begin(), targetData.end(), compareIndex)->TargetIndex - 1;
			maxIndex = targetData.empty() ? 0 : std::max_element(targetData.begin(), targetData.end(), compareIndex)->TargetIndex + 1;

			for (auto& data : targetData)
				data.OldValue = chart.Targets[data.TargetIndex].Type;
		}

	public:
		void Undo() override
		{
			for (const auto& data : targetData)
				chart.Targets[data.TargetIndex].Type = data.OldValue;
			chart.Targets.ExplicitlyUpdateFlagsAndSort(minIndex, maxIndex);
		}

		void Redo() override
		{
			for (const auto& data : targetData)
				chart.Targets[data.TargetIndex].Type = data.NewValue;
			chart.Targets.ExplicitlyUpdateFlagsAndSort(minIndex, maxIndex);
		}

		Undo::MergeResult TryMerge(Command& commandToMerge) override
		{
			auto* other = static_cast<decltype(this)>(&commandToMerge);
			if (&other->chart != &chart)
				return Undo::MergeResult::Failed;

			if (targetData.size() != other->targetData.size())
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

		std::string_view GetName() const override { return "Change Target Types"; }

	private:
		Chart& chart;
		std::vector<Data> targetData;
		i32 minIndex, maxIndex;
	};

	class MoveTargetListTicks : public Undo::Command
	{
	public:
		MoveTargetListTicks(Chart& chart, std::vector<i32> indices, TimelineTick tickIncrement)
			: chart(chart), targetIndices(std::move(indices)), tickIncrement(tickIncrement)
		{
			minIndex = targetIndices.empty() ? 0 : *std::min_element(targetIndices.begin(), targetIndices.end()) - 1;
			maxIndex = targetIndices.empty() ? 0 : *std::max_element(targetIndices.begin(), targetIndices.end()) + 1;
		}

	public:
		void Undo() override
		{
			for (const auto index : targetIndices)
				chart.Targets[index].Tick -= tickIncrement;
			chart.Targets.ExplicitlyUpdateFlagsAndSort(minIndex, maxIndex);
		}

		void Redo() override
		{
			for (const auto index : targetIndices)
				chart.Targets[index].Tick += tickIncrement;
			chart.Targets.ExplicitlyUpdateFlagsAndSort(minIndex, maxIndex);
		}

		Undo::MergeResult TryMerge(Command& commandToMerge) override
		{
			auto* other = static_cast<decltype(this)>(&commandToMerge);
			if (&other->chart != &chart)
				return Undo::MergeResult::Failed;

			if (!std::equal(targetIndices.begin(), targetIndices.end(), other->targetIndices.begin(), other->targetIndices.end()))
				return Undo::MergeResult::Failed;

			for (const auto index : targetIndices)
				chart.Targets[index].Tick -= tickIncrement;
			tickIncrement += other->tickIncrement;

			return Undo::MergeResult::ValueUpdated;
		}

		std::string_view GetName() const override { return "Move Targets"; }

	private:
		Chart& chart;
		std::vector<i32> targetIndices;
		TimelineTick tickIncrement;
		i32 minIndex, maxIndex;
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
					target.Properties = Rules::Detail::PresetTargetProperties(target);
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

	class ChangeTargetListPositions : public ChangeTargetListProperties
	{
	public:
		ChangeTargetListPositions(Chart& chart, std::vector<Data> data) : ChangeTargetListProperties(chart, std::move(data), TargetPropertyFlags_PositionXY) {}

	public:
		std::string_view GetName() const override { return "Change Target Positions"; }
	};

	class ChangeTargetListPositionsRow : public ChangeTargetListPositions
	{
	public:
		using ChangeTargetListPositions::ChangeTargetListPositions;

	public:
		std::string_view GetName() const override { return "Position Target Row"; }
	};

	class FlipTargetListPropertiesHorizontal : public ChangeTargetListProperties
	{
	public:
		FlipTargetListPropertiesHorizontal(Chart& chart, std::vector<Data> data) : ChangeTargetListProperties(chart, std::move(data), (TargetPropertyFlags_PositionXY | TargetPropertyFlags_Angle | TargetPropertyFlags_Frequency)) {}

	public:
		std::string_view GetName() const override { return "Flip Targets Horizontally"; }
	};

	class FlipTargetListPropertiesVertical : public ChangeTargetListProperties
	{
	public:
		FlipTargetListPropertiesVertical(Chart& chart, std::vector<Data> data) : ChangeTargetListProperties(chart, std::move(data), (TargetPropertyFlags_PositionXY | TargetPropertyFlags_Angle | TargetPropertyFlags_Frequency)) {}

	public:
		std::string_view GetName() const override { return "Flip Targets Vertically"; }
	};

	class InterpolateTargetListPositions : public ChangeTargetListPositions
	{
	public:
		using ChangeTargetListPositions::ChangeTargetListPositions;

	public:
		std::string_view GetName() const override { return "Interpolate Target Positions"; }
	};

	class ChangeTargetListAngles : public ChangeTargetListProperties
	{
	public:
		ChangeTargetListAngles(Chart& chart, std::vector<Data> data) : ChangeTargetListProperties(chart, std::move(data), TargetPropertyFlags_Angle) {}

	public:
		std::string_view GetName() const override { return "Change Target Angles"; }
	};

	class InterpolateTargetListAngles : public ChangeTargetListAngles
	{
	public:
		using ChangeTargetListAngles::ChangeTargetListAngles;

	public:
		std::string_view GetName() const override { return "Interpolate Target Angles"; }
	};

	class ApplyTargetListAngleVarations : public ChangeTargetListAngles
	{
	public:
		using ChangeTargetListAngles::ChangeTargetListAngles;

	public:
		std::string_view GetName() const override { return "Apply Target Angle Variations"; }
	};

	class InvertTargetListFrequencies : public ChangeTargetListProperties
	{
	public:
		InvertTargetListFrequencies(Chart& chart, std::vector<Data> data) : ChangeTargetListProperties(chart, std::move(data), TargetPropertyFlags_Frequency) {}

	public:
		std::string_view GetName() const override { return "Invert Target Frequencies"; }
	};

	class ApplySyncPreset : public ChangeTargetListProperties
	{
	public:
		using ChangeTargetListProperties::ChangeTargetListProperties;

	public:
		std::string_view GetName() const override { return "Apply Sync Preset"; }
	};

	class ChangeTargetListHasProperties : public Undo::Command
	{
	public:
		struct Data
		{
			i32 TargetIndex;
			bool HadProperties;
			TargetProperties OldProperties;
		};

	public:
		ChangeTargetListHasProperties(Chart& chart, std::vector<Data> data, bool newHasProperties)
			: chart(chart), targetData(std::move(data)), newHasProperties(newHasProperties)
		{
			for (auto& data : targetData)
			{
				const auto& target = chart.Targets[data.TargetIndex];
				data.HadProperties = target.Flags.HasProperties;
				data.OldProperties = target.Properties;
			}
		}

	public:
		void Undo() override
		{
			for (const auto& data : targetData)
			{
				auto& target = chart.Targets[data.TargetIndex];
				target.Flags.HasProperties = data.HadProperties;
				target.Properties = data.OldProperties;
			}
		}

		void Redo() override
		{
			for (const auto& data : targetData)
			{
				auto& target = chart.Targets[data.TargetIndex];

				if (target.Flags.HasProperties != newHasProperties)
				{
					target.Properties = Rules::Detail::PresetTargetProperties(target);
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

	class ChangeTargetListIsHold : public Undo::Command
	{
	public:
		struct Data
		{
			i32 TargetIndex;
			bool NewValue, OldValue;
		};

	public:
		ChangeTargetListIsHold(Chart& chart, std::vector<Data> data)
			: chart(chart), targetData(std::move(data))
		{
			for (auto& data : targetData)
			{
				const auto& target = chart.Targets[data.TargetIndex];
				data.OldValue = target.Flags.IsHold;
			}
		}

	public:
		void Undo() override
		{
			for (const auto& data : targetData)
			{
				auto& target = chart.Targets[data.TargetIndex];
				target.Flags.IsHold = data.OldValue;
			}
		}

		void Redo() override
		{
			for (const auto& data : targetData)
			{
				auto& target = chart.Targets[data.TargetIndex];
				target.Flags.IsHold = data.NewValue;
			}
		}

		Undo::MergeResult TryMerge(Command& commandToMerge) override
		{
			return Undo::MergeResult::Failed;
		}

		std::string_view GetName() const override { return "Change Target Holds"; }

	private:
		Chart& chart;
		std::vector<Data> targetData;
	};
}

namespace Comfy::Studio::Editor
{
	class AddTempoChange : public Undo::Command
	{
	public:
		AddTempoChange(Chart& chart, TempoChange newValue) : chart(chart), newValue(newValue) {}

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

		Undo::MergeResult TryMerge(Command& commandToMerge) override { return Undo::MergeResult::Failed; }
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
