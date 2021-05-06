#include "TargetInspector.h"
#include "TargetPropertyRules.h"
#include "ChartCommands.h"
#include "Core/ComfyStudioSettings.h"
#include "ImGui/Gui.h"
#include "ImGui/Extensions/PropertyEditor.h"
#include <FontIcons.h>

// TODO: Move to "PropertyEditor.h" once refined
namespace ImGui::PropertyEditor::Widgets
{
	template <typename ValueType>
	struct InputScalarParam
	{
		ValueType InOutValue = {};
		ValueType OutIncrement = {};
		f32 DragSpeed = 1.0f;
		std::optional<glm::vec<2, ValueType>> DragRange = {};
		const char* Format = nullptr;
		const char* CustomDisplayText = nullptr;
		size_t ComboboxValueCount = 0;
		const ValueType* ComboboxValues = nullptr;
		bool OutIsActive = false;
	};

	template <typename ValueType>
	bool InputScalarEx(std::string_view label, InputScalarParam<ValueType>& param)
	{
		RAII::ID id(label);
		return PropertyFuncValueFunc([&]
		{
			const auto preDragValue = param.InOutValue;
			const bool dragChanged = Detail::DragTextT<ValueType>(label, param.InOutValue, param.DragSpeed,
				param.DragRange.has_value() ? &param.DragRange->x : nullptr,
				param.DragRange.has_value() ? &param.DragRange->y : nullptr,
				0.0f);

			param.OutIncrement = (dragChanged) ? (param.InOutValue - preDragValue) : static_cast<ValueType>(0);
			return dragChanged;
		}, [&]
		{
			const f32 comboButtonWidth = GetFrameHeight();
			RAII::ItemWidth width((param.ComboboxValueCount > 0) ? (-1.0f - comboButtonWidth) : -1.0f);

			if (param.CustomDisplayText != nullptr)
			{
				PushStyleColor(ImGuiCol_Text, vec4(0.0f));
				PushStyleColor(ImGuiCol_TextSelectedBg, vec4(0.0f));
			}

			const auto inputScaleScreenPos = GetCursorScreenPos();

			using Lookup = Detail::TypeLookup::DataType<ValueType>;
			const char* formatString = (param.Format != nullptr) ? param.Format : Lookup::Format;
			bool valueChanged = InputScalar(Detail::DummyLabel, Lookup::TypeEnum, &param.InOutValue, nullptr, nullptr, formatString);

			const bool inputScalarItemActive = IsItemActive();
			param.OutIsActive = inputScalarItemActive;

			if (param.CustomDisplayText != nullptr)
			{
				PopStyleColor(2);
				GetWindowDrawList()->AddText(inputScaleScreenPos + GetStyle().FramePadding, GetColorU32(ImGuiCol_TextDisabled), param.CustomDisplayText);
			}

			if (param.ComboboxValueCount > 0)
			{
				assert(param.ComboboxValues != nullptr);
				const auto& style = GetStyle();
				const auto& context = *GetCurrentContext();

				auto* currentWindow = GetCurrentWindow();

				// NOTE: Right click to avoid active item focus issues
				const bool inputScalarClicked = IsItemClicked(1);
				const auto inputScalarRect = currentWindow->DC.LastItemRect;

				SameLine(0.0f, 0.0f);
				const bool arrowPressed = Button("##ComboArrow", vec2(comboButtonWidth, 0.0f));
				const auto arrowRect = currentWindow->DC.LastItemRect;
				RenderArrow(vec2(arrowRect.Max.x - comboButtonWidth + style.FramePadding.y, arrowRect.Min.y + style.FramePadding.y), ImGuiDir_Down);

				const auto id = currentWindow->GetID(StringViewStart(label), StringViewEnd(label));
				bool popupOpen = IsPopupOpen(id, ImGuiPopupFlags_None);

				if (((arrowPressed || inputScalarClicked) || context.NavActivateId == id) && !popupOpen)
				{
					if (currentWindow->DC.NavLayerCurrent == 0)
						currentWindow->NavLastIds[0] = id;
					OpenPopupEx(id);
					popupOpen = true;
				}

				if (popupOpen)
				{
					char name[16];
					sprintf_s(name, "##Combo_%02d", context.BeginPopupStack.Size);

					if (auto* popupWindow = FindWindowByName(name); popupWindow != nullptr && popupWindow->WasActive)
					{
						popupWindow->AutoPosLastDirection = ImGuiDir_Down; // "Below, Toward Right (default)"
						SetNextWindowPos(FindBestWindowPosForPopupEx(inputScalarRect.GetBL(), CalcWindowNextAutoFitSize(popupWindow), &popupWindow->AutoPosLastDirection, GetWindowAllowedExtentRect(popupWindow), inputScalarRect, ImGuiPopupPositionPolicy_ComboBox));
						SetNextWindowSize(vec2(inputScalarRect.GetWidth() + arrowRect.GetWidth(), 0.0f));
					}

					PushStyleVar(ImGuiStyleVar_WindowPadding, vec2(style.FramePadding.x, style.WindowPadding.y));
					const bool popupWindowOpen = Begin(name, nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_Popup | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);
					PopStyleVar();

					if (popupWindowOpen)
					{
						for (size_t i = 0; i < param.ComboboxValueCount; i++)
						{
							Gui::PushID(&param.ComboboxValues[i]);
							char b[128]; sprintf_s(b, formatString, param.ComboboxValues[i]);

							const bool itemSelected = (param.InOutValue == param.ComboboxValues[i]);
							if (Selectable(b, itemSelected, ImGuiSelectableFlags_None))
							{
								param.InOutValue = static_cast<ValueType>(param.ComboboxValues[i]);
								valueChanged = true;
							}
							Gui::PopID();

							if (itemSelected)
								SetItemDefaultFocus();
						}

						EndPopup();
					}
				}
			}

			if (valueChanged && param.DragRange.has_value())
				param.InOutValue = std::clamp(param.InOutValue, param.DragRange->x, param.DragRange->y);

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
					data.ID = targetView.Target->ID;
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
					targetData.emplace_back().ID = targetView.Target->ID;

				if (!targetData.empty())
					undoManager.Execute<ChangeTargetListHasProperties>(chart, std::move(targetData), !newValue);
			});

			PropertyGui(chart, "Position X", TargetPropertyType_PositionX);
			PropertyGui(chart, "Position Y", TargetPropertyType_PositionY);
			PropertyGui(chart, "Angle", TargetPropertyType_Angle, 1.0f, false, true);
			PropertyGui(chart, "Frequency", TargetPropertyType_Frequency, 0.05f, true);
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

	void TargetInspector::PropertyGui(Chart& chart, std::string_view label, TargetPropertyType property, f32 dragSpeed, bool isPropertyI32, bool degreeUnits)
	{
		auto commonValue = frontSelectedProperties[property];

		auto valueGetter = [property](const auto& t) { return t.PropertiesOrPreset[property]; };
		auto comparison = [&](const auto& a, const auto& b) { return valueGetter(a) < valueGetter(b); };

		const auto minValue = selectedTargets.empty() ? 0.0f : valueGetter(*std::min_element(selectedTargets.begin(), selectedTargets.end(), comparison));
		const auto maxValue = selectedTargets.empty() ? 0.0f : valueGetter(*std::max_element(selectedTargets.begin(), selectedTargets.end(), comparison));

		char displayTextBuffer[64];
		const char* customDisplayText = selectedTargets.empty() ? "" : nullptr;

		if (minValue != maxValue)
		{
			sprintf_s(displayTextBuffer, degreeUnits ? ("(%.2f" DEGREE_SIGN " ... %.2f" DEGREE_SIGN ")") : isPropertyI32 ? "(%.0f ... %.0f)" : "(%.2f ... %.2f)", minValue, maxValue);
			customDisplayText = displayTextBuffer;
		}

		if (propertyInputWidgetActiveStates[property])
			customDisplayText = nullptr;

		f32 outIncrement = 0.0f;
		bool changesWereMade = false;
		bool itemIsActive = false;

		if (isPropertyI32)
		{
			GuiProperty::InputScalarParam<i32> param;
			param.InOutValue = static_cast<i32>(commonValue);
			param.OutIncrement = {};
			param.DragSpeed = dragSpeed;
			param.DragRange = {};
			param.Format = "%d";
			param.CustomDisplayText = customDisplayText;

			if (property == TargetPropertyType_Frequency)
			{
				param.ComboboxValueCount = GlobalUserData.TargetPreset.InspectorDropdown.Frequencies.size();
				param.ComboboxValues = GlobalUserData.TargetPreset.InspectorDropdown.Frequencies.data();
			}

			if (GuiProperty::InputScalarEx<i32>(label, param))
			{
				commonValue = static_cast<f32>(param.InOutValue);
				outIncrement = static_cast<f32>(param.OutIncrement);
				changesWereMade = true;
			}
			itemIsActive = param.OutIsActive;
		}
		else
		{
			GuiProperty::InputScalarParam<f32> param;
			param.InOutValue = commonValue;
			param.OutIncrement = {};
			param.DragSpeed = dragSpeed;
			param.DragRange = {};
			param.Format = degreeUnits ? ("%.2f" DEGREE_SIGN) : "%.2f";
			param.CustomDisplayText = customDisplayText;

			auto* comboboxSourceVector = [property]() -> const std::vector<f32>*
			{
				switch (property)
				{
				case TargetPropertyType_PositionX: return &GlobalUserData.TargetPreset.InspectorDropdown.PositionsX;
				case TargetPropertyType_PositionY: return &GlobalUserData.TargetPreset.InspectorDropdown.PositionsY;
				case TargetPropertyType_Angle: return &GlobalUserData.TargetPreset.InspectorDropdown.Angles;
				case TargetPropertyType_Amplitude: return &GlobalUserData.TargetPreset.InspectorDropdown.Amplitudes;
				case TargetPropertyType_Distance: return &GlobalUserData.TargetPreset.InspectorDropdown.Distances;
				}
				return nullptr;
			}();

			if (comboboxSourceVector != nullptr)
			{
				param.ComboboxValueCount = comboboxSourceVector->size();
				param.ComboboxValues = comboboxSourceVector->data();
			}

			if (GuiProperty::InputScalarEx<f32>(label, param))
			{
				commonValue = param.InOutValue;
				outIncrement = param.OutIncrement;
				changesWereMade = true;
			}
			itemIsActive = param.OutIsActive;
		}

		if (changesWereMade)
		{
			std::vector<ChangeTargetListProperties::Data> targetData;
			targetData.reserve(selectedTargets.size());

			if (outIncrement == 0.0f)
			{
				for (const auto& targetView : selectedTargets)
				{
					auto& data = targetData.emplace_back();
					data.ID = targetView.Target->ID;
					data.NewValue[property] = commonValue;
				}
			}
			else
			{
				for (const auto& targetView : selectedTargets)
				{
					auto& data = targetData.emplace_back();
					data.ID = targetView.Target->ID;
					data.NewValue[property] = (valueGetter(targetView) + outIncrement);
				}
			}

			if (property == TargetPropertyType_Angle)
			{
				for (auto& data : targetData)
					data.NewValue.Angle = Rules::NormalizeAngle(data.NewValue.Angle);
			}

			undoManager.Execute<ChangeTargetListProperties>(chart, std::move(targetData), static_cast<TargetPropertyFlags>(1 << property));
		}

		propertyInputWidgetActiveStates[property] = itemIsActive;
	}
}
