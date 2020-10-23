#include "TargetInspector.h"
#include "TargetPropertyRules.h"
#include "ChartCommands.h"
#include "ImGui/Gui.h"
#include "ImGui/Extensions/PropertyEditor.h"

// TODO: Move to "PropertyEditor.h" once refined
namespace ImGui::PropertyEditor::Widgets
{
	// TODO: template <typename ValueType>
	bool InputF32Ex(std::string_view label, f32& inOutValue, f32& outIncrement, const char* customDisplayText = nullptr, f32 dragSpeed = 1.0f, std::optional<vec2> dragRange = {}, const char* format = nullptr)
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

			if (customDisplayText != nullptr)
			{
				PushStyleColor(ImGuiCol_Text, vec4(0.0f));
				PushStyleColor(ImGuiCol_TextSelectedBg, vec4(0.0f));
			}

			const auto inputScaleScreenPos = Gui::GetCursorScreenPos();

			using Lookup = Detail::TypeLookup::DataType<f32>;
			const bool valueChanged = Gui::InputScalar(Detail::DummyLabel, ImGuiDataType_Float, &inOutValue, nullptr, nullptr, (format != nullptr) ? format : Lookup::Format);

			if (customDisplayText != nullptr)
			{
				PopStyleColor(2);
				GetWindowDrawList()->AddText(inputScaleScreenPos + GetStyle().FramePadding, GetColorU32(ImGuiCol_TextDisabled), customDisplayText);
			}

			if (valueChanged && dragRange.has_value())
				inOutValue = std::clamp(inOutValue, dragRange->x, dragRange->y);

			return valueChanged;
		});
	}
}

namespace Comfy::Studio::Editor
{
	TargetInspector::TargetInspector(Undo::UndoManager& undoManager) : undoManager(undoManager)
	{
	}

	void TargetInspector::Gui(Chart& chart)
	{
		for (auto& target : chart.Targets)
		{
			if (target.IsSelected)
				selectedTargets.push_back({ &target, Rules::TryGetProperties(target) });
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
			frontSelectedTarget = !selectedTargets.empty() ? selectedTargets.front().Target : nullptr;
			frontSelectedProperties = !selectedTargets.empty() ? selectedTargets.front().PropertiesOrPreset : TargetProperties {};

			if (frontSelectedTarget == nullptr)
				Gui::PushItemFlag(ImGuiItemFlags_Disabled, true);

			auto isHoldValueGetter = [](auto& t) { return static_cast<GuiProperty::Boolean>(t.Flags.IsHold); };
			auto isHoldConditionGetter = [](auto& t) { return !IsSlideButtonType(t.Type); };
			BooleanGui("Is Hold", isHoldValueGetter, isHoldConditionGetter, [&](const bool newValue)
			{
				std::vector<ChangeTargetListIsHold::Data> targetData;
				targetData.reserve(selectedTargets.size());

				for (const auto& targetView : selectedTargets)
				{
					if (!isHoldConditionGetter(*targetView))
						continue;

					auto& data = targetData.emplace_back();
					data.TargetIndex = GetSelectedTargetIndex(chart, targetView.Target);
					data.NewValue = newValue;
				}

				if (!targetData.empty())
					undoManager.Execute<ChangeTargetListIsHold>(chart, std::move(targetData));
			});

			auto usePresetValueGetter = [](auto& t) { return static_cast<GuiProperty::Boolean>(!t.Flags.HasProperties); };
			BooleanGui("Use Preset", usePresetValueGetter, [](auto& t) { return true; }, [&](const bool newValue)
			{
				std::vector<ChangeTargetListHasProperties::Data> targetData;
				targetData.reserve(selectedTargets.size());

				for (const auto& targetView : selectedTargets)
					targetData.emplace_back().TargetIndex = GetSelectedTargetIndex(chart, targetView.Target);

				if (!targetData.empty())
					undoManager.Execute<ChangeTargetListHasProperties>(chart, std::move(targetData), !newValue);
			});

			PropertyGui(chart, "Position X", TargetPropertyType_PositionX);
			PropertyGui(chart, "Position Y", TargetPropertyType_PositionY);
			PropertyGui(chart, "Angle", TargetPropertyType_Angle);
			PropertyGui(chart, "Frequency", TargetPropertyType_Frequency);
			PropertyGui(chart, "Amplitude", TargetPropertyType_Amplitude);
			PropertyGui(chart, "Distance", TargetPropertyType_Distance);

			if (frontSelectedTarget == nullptr)
				Gui::PopItemFlag();
		});
	}

	template<typename ValueGetter, typename ConditionGetter, typename OnChange>
	void TargetInspector::BooleanGui(std::string_view label, ValueGetter valueGetter, ConditionGetter conditionGetter, OnChange onChange)
	{
		const bool allAreValid = std::all_of(selectedTargets.begin(), selectedTargets.end(), [&](auto& t) { return conditionGetter(*t); });

		const auto frontValue = (allAreValid && frontSelectedTarget != nullptr) ? valueGetter(*frontSelectedTarget) : GuiProperty::Boolean::Count;
		const auto allValuesSame = (allAreValid && std::all_of(selectedTargets.begin(), selectedTargets.end(), [&](auto& t) { return valueGetter(*t) == frontValue; }));

		auto commonBoolean = (allAreValid && allValuesSame && !selectedTargets.empty()) ? frontValue : GuiProperty::Boolean::Count;
		const auto commonBooleanOld = commonBoolean;

		if (GuiProperty::ComboBoolean(label, commonBoolean) && (commonBoolean != commonBooleanOld))
		{
			const bool newValue = static_cast<bool>(commonBoolean);
			onChange(newValue);
		}
	}

	void TargetInspector::PropertyGui(Chart& chart, std::string_view label, TargetPropertyType property, f32 dragSpeed)
	{
		// TODO: Cast input / output to i32 in case of TargetPropertyType_Frequency (?)
		auto commonValue = frontSelectedProperties[property];

		auto valueGetter = [property](const auto& t) { return t.PropertiesOrPreset[property]; };
		auto comparison = [&](const auto& a, const auto& b) { return valueGetter(a) < valueGetter(b); };

		const auto minValue = selectedTargets.empty() ? 0.0f : valueGetter(*std::min_element(selectedTargets.begin(), selectedTargets.end(), comparison));
		const auto maxValue = selectedTargets.empty() ? 0.0f : valueGetter(*std::max_element(selectedTargets.begin(), selectedTargets.end(), comparison));

		char displayTextBuffer[64];
		const char* customDisplayText = selectedTargets.empty() ? "" : nullptr;

		if (minValue != maxValue)
		{
			sprintf_s(displayTextBuffer, "(%.2f ... %.2f)", minValue, maxValue);
			customDisplayText = displayTextBuffer;
		}

		if (propertyInputWidgetActiveStates[property])
			customDisplayText = nullptr;

		f32 outIncrement = 0.0f;
		if (GuiProperty::InputF32Ex(label, commonValue, outIncrement, customDisplayText, 1.0f, {}, "%.2f"))
		{
			std::vector<ChangeTargetListProperties::Data> targetData;
			targetData.reserve(selectedTargets.size());

			if (outIncrement == 0.0f)
			{
				for (const auto& targetView : selectedTargets)
				{
					auto& data = targetData.emplace_back();
					data.TargetIndex = GetSelectedTargetIndex(chart, targetView.Target);
					data.NewValue[property] = commonValue;
				}
			}
			else
			{
				for (const auto& targetView : selectedTargets)
				{
					auto& data = targetData.emplace_back();
					data.TargetIndex = GetSelectedTargetIndex(chart, targetView.Target);
					data.NewValue[property] = (valueGetter(targetView) + outIncrement);
				}
			}

			undoManager.Execute<ChangeTargetListProperties>(chart, std::move(targetData), static_cast<TargetPropertyFlags>(1 << property));
		}

		propertyInputWidgetActiveStates[property] = Gui::IsItemActive();
	}

	i32 TargetInspector::GetSelectedTargetIndex(const Chart& chart, const TimelineTarget* selectedTarget) const
	{
		assert(chart.Targets.size() > 0);
		return static_cast<i32>(std::distance(&chart.Targets[0], selectedTarget));
	}
}
