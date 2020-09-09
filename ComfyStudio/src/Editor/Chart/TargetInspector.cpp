#include "TargetInspector.h"
#include "TargetPropertyRules.h"
#include "ChartCommands.h"
#include "ImGui/Gui.h"
#include "ImGui/Extensions/PropertyEditor.h"

// TODO: Move to "PropertyEditor.h" once refined
namespace ImGui::PropertyEditor::Widgets
{
	// TODO: template <typename ValueType>
	bool InputF32Ex(std::string_view label, f32& inOutValue, f32& outIncrement, bool hideValue, f32 dragSpeed = 1.0f, std::optional<vec2> dragRange = {})
	{
		RAII::ID id(label);
		return PropertyFuncValueFunc([&]
		{
			f32 preDragValue = inOutValue;
			const bool dragChanged = Detail::DragTextT<f32>(label, inOutValue, dragSpeed,
				dragRange.has_value() ? &dragRange->x : nullptr,
				dragRange.has_value() ? &dragRange->y : nullptr,
				0.0f);

			outIncrement = (dragChanged) ? (inOutValue - preDragValue) : 0.0f;
			return dragChanged;
		}, [&]
		{
			RAII::ItemWidth width(-1.0f);

			if (hideValue) { PushStyleColor(ImGuiCol_Text, vec4(0.0f)); PushStyleColor(ImGuiCol_TextSelectedBg, vec4(0.0f)); }

			using Lookup = Detail::TypeLookup::DataType<f32>;
			const bool valueChanged = Gui::InputScalar(Detail::DummyLabel, ImGuiDataType_Float, &inOutValue, nullptr, nullptr, Lookup::Format);

			if (hideValue) { PopStyleColor(2); }

			if (valueChanged && dragRange.has_value())
				inOutValue = std::clamp(inOutValue, dragRange->x, dragRange->y);

			return valueChanged;
		});
	}
}

namespace Comfy::Studio::Editor
{
	namespace
	{
		constexpr TargetProperties TryGetPropertes(const TimelineTarget& target)
		{
			return target.Flags.HasProperties ? target.Properties : Rules::PresetTargetProperties(target.Type, target.Tick, target.Flags);
		}
	}

	TargetInspector::TargetInspector(Undo::UndoManager& undoManager) : undoManager(undoManager)
	{
	}

	void TargetInspector::Gui(Chart& chart)
	{
		// TODO: Pre calculate preset positions (store in second vector?) as they will be used multipled times in later loops (?)
		for (auto& target : chart.Targets)
		{
			if (target.IsSelected)
				selectedTargets.push_back(&target);
		}

		GuiPropertyRAII::PropertyValueColumns columns;
		GuiSelectedTargets(chart);

		selectedTargets.clear();
		frontSelectedTarget = nullptr;
	}

	std::string_view TargetInspector::FormatSelectedTargetsValueBuffer(const Chart& chart)
	{
		auto& buffer = selectedTargetsValueBuffer;
		if (selectedTargets.empty())
		{
			return std::string_view(buffer.data(), sprintf_s(buffer.data(), buffer.size(),
				"(%zu selected)",
				selectedTargets.size()));
		}
		else if (selectedTargets.front()->Tick == selectedTargets.back()->Tick)
		{
			return std::string_view(buffer.data(), sprintf_s(buffer.data(), buffer.size(),
				"(%zu selected : %s)",
				selectedTargets.size(),
				chart.TimelineMap.GetTimeAt(selectedTargets.front()->Tick).FormatTime().data()));
		}
		else
		{
			return std::string_view(buffer.data(), sprintf_s(buffer.data(), buffer.size(),
				"(%zu selected : %s - %s)",
				selectedTargets.size(),
				chart.TimelineMap.GetTimeAt(selectedTargets.front()->Tick).FormatTime().data(),
				chart.TimelineMap.GetTimeAt(selectedTargets.back()->Tick).FormatTime().data()));
		}
	}

	void TargetInspector::GuiSelectedTargets(Chart& chart)
	{
		GuiProperty::TreeNode("Selected Targets", FormatSelectedTargetsValueBuffer(chart), ImGuiTreeNodeFlags_DefaultOpen, [&]
		{
			frontSelectedTarget = !selectedTargets.empty() ? selectedTargets.front() : nullptr;
			frontSelectedHasProperties = (frontSelectedTarget != nullptr) ? frontSelectedTarget->Flags.HasProperties : false;
			frontSelectedProperties = (frontSelectedTarget != nullptr) ? TryGetPropertes(*frontSelectedTarget) : TargetProperties {};

			// NOTE: Invert "has properties" to "use preset" to make it more user friendly. HasProperties used internally to allow for "0 is initialization"
			const bool sameHasProperties = std::all_of(selectedTargets.begin(), selectedTargets.end(), [&](auto* t) { return t->Flags.HasProperties == frontSelectedHasProperties; });
			auto commonUsePreset = (sameHasProperties && !selectedTargets.empty()) ? static_cast<GuiProperty::Boolean>(!frontSelectedHasProperties) : GuiProperty::Boolean::Count;
			const auto previousCommonUsePreset = commonUsePreset;

			if (GuiProperty::ComboBoolean("Use Preset", commonUsePreset) && (commonUsePreset != previousCommonUsePreset))
			{
				std::vector<ChangeTargetListHasProperties::Data> targetData;
				targetData.reserve(selectedTargets.size());

				for (const auto* target : selectedTargets)
					targetData.emplace_back().TargetIndex = GetSelectedTargetIndex(chart, target);

				undoManager.Execute<ChangeTargetListHasProperties>(chart, std::move(targetData), !static_cast<bool>(commonUsePreset));
			}

			PropertyGui(chart, "Position X", TargetPropertyType_PositionX);
			PropertyGui(chart, "Position Y", TargetPropertyType_PositionY);
			PropertyGui(chart, "Angle", TargetPropertyType_Angle);
			PropertyGui(chart, "Frequency", TargetPropertyType_Frequency);
			PropertyGui(chart, "Amplitude", TargetPropertyType_Amplitude);
			PropertyGui(chart, "Distance", TargetPropertyType_Distance);
		});
	}

	void TargetInspector::PropertyGui(Chart& chart, std::string_view label, TargetPropertyType property, f32 dragSpeed)
	{
		// TODO: Cast input / output to i32 in case of TargetPropertyType_Frequency (?)
		auto commonValue = frontSelectedProperties[property];

		const bool allAreSame = std::all_of(selectedTargets.begin(), selectedTargets.end(), [&](auto* t) { return TryGetPropertes(*t)[property] == commonValue; });
		const bool hideValue = !allAreSame || selectedTargets.empty();

		f32 outIncrement = 0.0f;
		if (GuiProperty::InputF32Ex(label, commonValue, outIncrement, hideValue))
		{
			std::vector<ChangeTargetListProperties::Data> targetData;
			targetData.reserve(selectedTargets.size());

			if (outIncrement == 0.0f)
			{
				for (const auto* target : selectedTargets)
				{
					auto& data = targetData.emplace_back();
					data.TargetIndex = GetSelectedTargetIndex(chart, target);
					data.NewValue[property] = commonValue;
				}
			}
			else
			{
				for (const auto* target : selectedTargets)
				{
					auto& data = targetData.emplace_back();
					data.TargetIndex = GetSelectedTargetIndex(chart, target);
					data.NewValue[property] = (TryGetPropertes(*target)[property] + outIncrement);
				}
			}

			undoManager.Execute<ChangeTargetListProperties>(chart, std::move(targetData), static_cast<TargetPropertyFlags>(1 << property));
		}
	}

	i32 TargetInspector::GetSelectedTargetIndex(const Chart& chart, const TimelineTarget* selectedTarget) const
	{
		assert(chart.Targets.size() > 0);
		return static_cast<i32>(std::distance(&chart.Targets[0], selectedTarget));
	}
}
