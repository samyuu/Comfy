#include "ChartEditorSettingsWindow.h"
#include "Editor/Chart/ChartEditor.h"
#include "Input/Input.h"
#include "Audio/Audio.h"
#include <FontIcons.h>

namespace Comfy::Studio::Editor
{
	namespace
	{
		constexpr std::array<const char*, TargetPropertyType_Count> TargetPropertyTypeNames =
		{
			"Position X",
			"Position Y",
			"Angle",
			"Frequency",
			"Amplitude",
			"Distance",
		};

		constexpr f32 GuiSettingsInnerMargin = 40.0f;
		constexpr f32 GuiSettingsItemWidth = -GuiSettingsInnerMargin;

		constexpr f32 GuiSettingsInnerSubChildMargin = 8.0f;
		constexpr f32 GuiSettingsSubChildItemWidth = -GuiSettingsInnerSubChildMargin;

		constexpr f32 GuiSettingsLabelSpacing = 4.0f;
		constexpr f32 GuiSettingsCheckboxLabelSpacing = 6.0f;

		void GuiBeginSettingsColumns()
		{
			Gui::BeginColumns(nullptr, 2, ImGuiOldColumnFlags_NoBorder | ImGuiOldColumnFlags_NoResize);
		}

		void GuiEndSettingsColumns()
		{
			Gui::EndColumns();
		}

		void GuiSettingsRightAlignedLabel(std::string_view label)
		{
			const vec2 textSize = Gui::CalcTextSize(Gui::StringViewStart(label), Gui::StringViewEnd(label), true);
			const vec2 cursorPosition = Gui::GetCursorScreenPos();
			Gui::SetCursorScreenPos(vec2(cursorPosition.x + (Gui::GetContentRegionAvail().x - textSize.x) - GuiSettingsLabelSpacing, cursorPosition.y));

			Gui::AlignTextToFramePadding();
			Gui::TextUnformatted(Gui::StringViewStart(label), Gui::StringViewEnd(label));
		}

		bool GuiSettingsCheckbox(std::string_view label, bool& inOutValue)
		{
			Gui::NextColumn();

			Gui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, vec2(GuiSettingsCheckboxLabelSpacing, GImGui->Style.ItemInnerSpacing.y));
			const bool result = Gui::Checkbox(label.data(), &inOutValue);
			Gui::PopStyleVar(1);
			Gui::NextColumn();

			return result;
		}

		bool GuiSettingsCheckboxInverted(std::string_view label, bool& inOutValue)
		{
			bool inOutInverted = !inOutValue;
			if (GuiSettingsCheckbox(label, inOutInverted))
			{
				inOutValue = !inOutInverted;
				return true;
			}

			return false;
		}

		bool GuiSettingsSliderI32(std::string_view label, i32& inOutValue, i32 minValue, i32 maxValue, const char* format = "%d", ImGuiSliderFlags flags = ImGuiSliderFlags_None)
		{
			GuiSettingsRightAlignedLabel(label);
			Gui::NextColumn();

			Gui::PushID(Gui::StringViewStart(label), Gui::StringViewEnd(label));
			Gui::SetNextItemWidth(GuiSettingsItemWidth);
			const bool result = Gui::SliderInt("##SettingsSlider", &inOutValue, minValue, maxValue, format, flags);
			Gui::PopID();
			Gui::NextColumn();

			return result;
		}

		bool GuiSettingsSliderF32(std::string_view label, f32& inOutValue, f32 minValue, f32 maxValue, const char* format = "%.3f", ImGuiSliderFlags flags = ImGuiSliderFlags_None)
		{
			GuiSettingsRightAlignedLabel(label);
			Gui::NextColumn();

			Gui::PushID(Gui::StringViewStart(label), Gui::StringViewEnd(label));
			Gui::SetNextItemWidth(GuiSettingsItemWidth);
			const bool result = Gui::SliderFloat("##SettingsSlider", &inOutValue, minValue, maxValue, format, flags);
			Gui::PopID();
			Gui::NextColumn();

			return result;
		}

		bool GuiSettingsVolumeSlider(std::string_view label, f32& inOutValue, ImGuiSliderFlags flags = ImGuiSliderFlags_None, bool extendedRange = false)
		{
			GuiSettingsRightAlignedLabel(label);
			Gui::NextColumn();

			Gui::PushID(Gui::StringViewStart(label), Gui::StringViewEnd(label));
			Gui::SetNextItemWidth(GuiSettingsItemWidth);

			bool result = false;
#if 0
			if (f32 v = ToPercent(inOutValue); result = Gui::SliderFloat("##SettingsVolumeSlider", &v, 0.0f, extendedRange ? 125.0f : 100.0f, "%.0f%%", flags))
				inOutValue = FromPercent(v);
#elif 0
			constexpr f32 minDB = -60.0f, maxDB = 0.0f;
			if (f32 dB = Audio::LinearVolumeToDecibel(inOutValue); result = Gui::SliderFloat("##SettingsVolumeSlider", &dB, minDB, maxDB, "%.2f dB", flags))
				inOutValue = (dB <= minDB) ? 0.0f : Audio::DecibelToLinearVolume(dB);
#else
			if (f32 s = ToPercent(Audio::LinearVolumeToSquare(inOutValue)); result = Gui::SliderFloat("##SettingsVolumeSlider", &s, 0.0f, extendedRange ? 115.0f : 100.0f, "%.0f%%", flags))
				inOutValue = Audio::SquareToLinearVolume(FromPercent(s));
#endif

			Gui::PopID();
			Gui::NextColumn();

			return result;
		}

		bool GuiSettingsInputI32(std::string_view label, i32& inOutValue, i32 step = 1, i32 stepFast = 100, ImGuiInputTextFlags flags = 0, const char* format = nullptr)
		{
			GuiSettingsRightAlignedLabel(label);
			Gui::NextColumn();

			Gui::PushID(Gui::StringViewStart(label), Gui::StringViewEnd(label));
			Gui::SetNextItemWidth(GuiSettingsItemWidth);
			const bool result = Gui::InputScalar("##SettingsInput", ImGuiDataType_S32, &inOutValue,
				(step > 0 ? &step : nullptr),
				(stepFast > 0 ? &stepFast : nullptr),
				(format != nullptr) ? format : (flags & ImGuiInputTextFlags_CharsHexadecimal) ? "%08X" : "%d",
				flags);
			Gui::PopID();
			Gui::NextColumn();

			return result;
		}

		bool GuiSettingsInputF32(std::string_view label, f32& inOutValue, f32 step = 0.0f, f32 stepFast = 0.0f, ImGuiInputTextFlags flags = 0, const char* format = "%.2f")
		{
			GuiSettingsRightAlignedLabel(label);
			Gui::NextColumn();

			Gui::PushID(Gui::StringViewStart(label), Gui::StringViewEnd(label));
			Gui::SetNextItemWidth(GuiSettingsItemWidth);
			const bool result = Gui::InputScalar("##SettingsInput", ImGuiDataType_Float, &inOutValue,
				(step > 0 ? &step : nullptr),
				(stepFast > 0 ? &stepFast : nullptr),
				format,
				flags);
			Gui::PopID();
			Gui::NextColumn();

			return result;
		}

		bool GuiSettingsInputVec3(std::string_view label, vec3& inOutValue, ImGuiInputTextFlags flags = 0, const char* format = "%.2f")
		{
			GuiSettingsRightAlignedLabel(label);
			Gui::NextColumn();

			Gui::PushID(Gui::StringViewStart(label), Gui::StringViewEnd(label));
			Gui::SetNextItemWidth(GuiSettingsItemWidth);
			const bool result = Gui::InputScalarN("##SettingsInput", ImGuiDataType_Float, glm::value_ptr(inOutValue), 3, nullptr, nullptr, format, flags);
			Gui::PopID();
			Gui::NextColumn();

			return result;
		}

		bool GuiSettingsInputText(std::string_view label, std::string& inOutValue, const char* hintText = nullptr, ImGuiInputTextFlags flags = ImGuiInputTextFlags_None)
		{
			GuiSettingsRightAlignedLabel(label);
			Gui::NextColumn();

			Gui::PushID(Gui::StringViewStart(label), Gui::StringViewEnd(label));
			Gui::SetNextItemWidth(GuiSettingsItemWidth);
			Gui::PushItemDisabledAndTextColorIf(flags & ImGuiInputTextFlags_ReadOnly);
			const bool result = (hintText != nullptr) ? Gui::InputTextWithHint("##SettingsText", hintText, &inOutValue, flags) : Gui::InputText("##SettingsText", &inOutValue, flags);
			Gui::PopItemDisabledAndTextColorIf(flags & ImGuiInputTextFlags_ReadOnly);
			Gui::PopID();
			Gui::NextColumn();

			return result;
		}

		template <typename IndexToStringFunc>
		bool GuiSettingsCombo(std::string_view label, i32& inOutIndex, i32 startRange, i32 endRange, ImGuiComboFlags flags, IndexToStringFunc indexToString)
		{
			GuiSettingsRightAlignedLabel(label);
			Gui::NextColumn();

			Gui::PushID(Gui::StringViewStart(label), Gui::StringViewEnd(label));
			Gui::PushItemWidth(GuiSettingsItemWidth);
			const char* previewValue = indexToString(inOutIndex);
			bool result = false;
			if (Gui::BeginCombo("##SettingsCombo", (previewValue == nullptr) ? "" : previewValue))
			{
				for (i32 i = startRange; i < endRange; i++)
				{
					if (const char* itemLabel = indexToString(i); itemLabel != nullptr)
					{
						const bool isSelected = (i == inOutIndex);
						if (Gui::Selectable(itemLabel, isSelected))
						{
							inOutIndex = i;
							result = true;
						}

						if (isSelected)
							Gui::SetItemDefaultFocus();
					}
				}

				Gui::EndCombo();
			}

			Gui::PopItemWidth();
			Gui::PopID();
			Gui::NextColumn();
			return result;
		}

		template <typename EnumType, size_t LookupSize>
		bool GuiSettingsCombo(std::string_view label, EnumType& inOutValue, const std::array<const char*, LookupSize>& nameLookup, ImGuiComboFlags flags = ImGuiComboFlags_None)
		{
			static_assert(sizeof(EnumType) <= sizeof(i32));
			auto indexToString = [&nameLookup](i32 i) { return IndexOr(i, nameLookup, nullptr); };

			bool result = false;
			if (i32 i = static_cast<i32>(inOutValue); GuiSettingsCombo(label, i, 0, static_cast<i32>(nameLookup.size()), flags, indexToString))
			{
				inOutValue = static_cast<EnumType>(i);
				result = true;
			}

			return result;
		}

		template <typename EnumType, size_t LookupSize>
		bool GuiSettingsCombo(std::string_view label, EnumType& inOutValue, const std::array<const char*, LookupSize>& nameLookup, EnumType firstValue, EnumType lastValue, ImGuiComboFlags flags = ImGuiComboFlags_None)
		{
			static_assert(sizeof(EnumType) <= sizeof(i32));
			auto indexToString = [&nameLookup](i32 i) { return IndexOr(i, nameLookup, nullptr); };

			bool result = false;
			if (i32 i = static_cast<i32>(inOutValue); GuiSettingsCombo(label, i, static_cast<i32>(firstValue), static_cast<i32>(lastValue) + 1, flags, indexToString))
			{
				inOutValue = static_cast<EnumType>(i);
				result = true;
			}

			return result;
		}

		bool GuiSettingsColorEdit3(std::string_view label, vec3& inOutColor, ImGuiColorEditFlags flags = ImGuiColorEditFlags_None)
		{
			GuiSettingsRightAlignedLabel(label);
			Gui::NextColumn();

			Gui::PushID(Gui::StringViewStart(label), Gui::StringViewEnd(label));
			Gui::SetNextItemWidth(GuiSettingsItemWidth);
			const bool result = Gui::ColorEdit3("##SettingsColorEdit3", glm::value_ptr(inOutColor), flags);
			Gui::PopID();
			Gui::NextColumn();

			return result;
		}

		bool GuiSettingsColorEdit4(std::string_view label, vec4& inOutColor, ImGuiColorEditFlags flags = ImGuiColorEditFlags_None)
		{
			GuiSettingsRightAlignedLabel(label);
			Gui::NextColumn();

			Gui::PushID(Gui::StringViewStart(label), Gui::StringViewEnd(label));
			Gui::SetNextItemWidth(GuiSettingsItemWidth);
			const bool result = Gui::ColorEdit4("##SettingsColorEdit4", glm::value_ptr(inOutColor), flags);
			Gui::PopID();
			Gui::NextColumn();

			return result;
		}

		bool GuiSettingsInteractiveAwaitKeyPressInputBindingButton(Input::Binding& inOutBinding, vec2 buttonSize, Input::Binding*& inOutAwaitInputBinding, Stopwatch& inOutAwaitInputStopwatch)
		{
			using namespace Input;
			constexpr auto timeoutThreshold = TimeSpan::FromSeconds(4.0);
			constexpr auto mouseClickThreshold = TimeSpan::FromMilliseconds(200.0);
			constexpr f32 blinkFrequency = static_cast<f32>(timeoutThreshold.TotalSeconds() * 1.2);
			constexpr f32 blinkLow = 0.25f, blinkHigh = 1.45f;
			constexpr f32 blinkMin = 0.25f, blinkMax = 1.00f;
			auto getSinBlinkOpacity = [&](TimeSpan elapsed) { return Clamp(ConvertRange(-1.0f, +1.0f, blinkLow, blinkHigh, glm::sin(static_cast<f32>(elapsed.TotalSeconds() * blinkFrequency) + 1.0f)), blinkMin, blinkMax); };

			auto requestAsignmentForNewBinding = [&](Binding* bindingToRequestAsignmentFor)
			{
				inOutAwaitInputBinding = bindingToRequestAsignmentFor;
				inOutAwaitInputStopwatch.Restart();
			};

			auto finishBindingAsignment = [&](std::optional<Binding> newBindingToAsign)
			{
				inOutAwaitInputBinding = nullptr;
				inOutAwaitInputStopwatch.Stop();
				if (newBindingToAsign.has_value())
					inOutBinding = newBindingToAsign.value();
			};

			const Binding inBindingCopy = inOutBinding;
			FormatBuffer buffer = ToString(inOutBinding);

			if (!inOutBinding.IsEmpty())
			{
				if (inOutBinding.Type == BindingType::Keyboard)
				{
					if (inOutBinding.Keyboard.Key >= KeyCode_MouseFirst && inOutBinding.Keyboard.Key <= KeyCode_MouseLast)
						strcat_s(buffer.data(), buffer.size(), "  (Mouse)");
					else if (inOutBinding.Keyboard.Key >= KeyCode_KeyboardFirst && inOutBinding.Keyboard.Key <= KeyCode_KeyboardLast)
						strcat_s(buffer.data(), buffer.size(), "  (Keyboard)");
				}
				else if (inOutBinding.Type == BindingType::Controller)
				{
					if (inOutBinding.Controller.Button >= Button::First && inOutBinding.Controller.Button <= Button::Last)
						strcat_s(buffer.data(), buffer.size(), "  (Controller)");
				}
			}
			else
			{
				strcpy_s(buffer.data(), buffer.size(), "(None)");
			}

			if (inOutAwaitInputBinding == &inOutBinding)
				strcpy_s(buffer.data(), buffer.size(), "[Press any Key]");

			strcat_s(buffer.data(), buffer.size(), "###BindingButton");

			if (inOutAwaitInputBinding == &inOutBinding)
			{
				Gui::PushStyleColor(ImGuiCol_Text, vec4(0.814f, 0.814f, 0.242f, getSinBlinkOpacity(inOutAwaitInputStopwatch.GetElapsed())));
				Gui::PushStyleColor(ImGuiCol_Button, Gui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
			}
			else
			{
				Gui::PushStyleColor(ImGuiCol_Text, Gui::GetStyleColorVec4(ImGuiCol_Text));
				Gui::PushStyleColor(ImGuiCol_Button, Gui::GetStyleColorVec4(ImGuiCol_Button));
			}

			if (Gui::Button(buffer.data(), buttonSize))
			{
				if (inOutAwaitInputBinding != &inOutBinding)
					requestAsignmentForNewBinding(&inOutBinding);
			}
			const bool buttonHovered = Gui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
			Gui::PopStyleColor(2);

			if (inOutAwaitInputBinding == &inOutBinding)
			{
				const bool timedOut = (inOutAwaitInputStopwatch.GetElapsed() > timeoutThreshold);
				const bool mouseClickCancelRequest = (inOutAwaitInputStopwatch.GetElapsed() > mouseClickThreshold && !buttonHovered && (Gui::IsMouseClicked(ImGuiMouseButton_Left, false) || Gui::IsMouseClicked(ImGuiMouseButton_Right, false)));

				if (timedOut || mouseClickCancelRequest)
					finishBindingAsignment(std::nullopt);

				Gui::SetActiveID(Gui::GetID(&inOutBinding), Gui::GetCurrentWindow());

				for (KeyCode keyCode = KeyCode_KeyboardFirst; keyCode <= KeyCode_KeyboardLast; keyCode++)
				{
					if (IsKeyReleased(keyCode))
					{
						KeyModifiers heldModifiers = KeyModifiers_None;
						ForEachKeyCodeInKeyModifiers(KeyModifiers_All, [&](KeyCode modifierKey)
						{
							if (IsKeyDown(modifierKey))
								heldModifiers |= KeyCodeToKeyModifiers(modifierKey);
						});

						finishBindingAsignment(Binding(keyCode, heldModifiers));
					}
				}

				// NOTE: Assume for now that binding the left / right mouse button is never desired
				//		 otherwise there will have to be a different system for asigning those such as through a context menu
				for (KeyCode mouseKeyCode : { KeyCode_MouseMiddle, KeyCode_MouseX1, KeyCode_MouseX2 })
				{
					if (IsKeyReleased(mouseKeyCode))
						finishBindingAsignment(Binding(mouseKeyCode));
				}

				for (size_t i = 0; i <= EnumCount<Button>(); i++)
				{
					const auto button = static_cast<Button>(i);
					if (IsButtonReleased(button))
						finishBindingAsignment(Binding(button));
				}
			}

			return (inOutBinding != inBindingCopy);
		}
	}

	namespace
	{
		constexpr i32 DecimalAndNewLineFilterCallback(ImGuiInputTextCallbackData* data)
		{
			return (data->EventFlag == ImGuiInputTextFlags_CallbackCharFilter) && (std::string_view("0123456789.+-\n").find_first_of(static_cast<char>(data->EventChar)) == std::string_view::npos);
		}

		TargetPropertyType FindFirstNonEmptyInspectorDropdownPropertyType(const ComfyStudioUserSettings& userData)
		{
			if (!userData.TargetPreset.InspectorDropdown.PositionsX.empty())
				return TargetPropertyType_PositionX;
			else if (!userData.TargetPreset.InspectorDropdown.PositionsY.empty())
				return TargetPropertyType_PositionY;
			else if (!userData.TargetPreset.InspectorDropdown.Angles.empty())
				return TargetPropertyType_Angle;
			else if (!userData.TargetPreset.InspectorDropdown.Frequencies.empty())
				return TargetPropertyType_Frequency;
			else if (!userData.TargetPreset.InspectorDropdown.Amplitudes.empty())
				return TargetPropertyType_Amplitude;
			else if (!userData.TargetPreset.InspectorDropdown.Distances.empty())
				return TargetPropertyType_Distance;
			else
				return TargetPropertyType_PositionX;
		}

		constexpr PlayTestSlidePositionType ExpectedPhyisicalBindingLocationToPlayTestSlidePosition(const Input::Binding& binding)
		{
			using namespace Input;
			if (binding.Type == BindingType::Keyboard)
			{
				// NOTE: Somewhat assuming US keyboard layout although most of the special OEM keys tend to not differ too much in their approximate sector position
				//		 and even if, getting these slide positions wrong isn't a major problem
				const auto phsyicalLeftSideKeys =
				{
					KeyCode_None, KeyCode_MouseLeft, KeyCode_MouseMiddle, KeyCode_MouseX1, KeyCode_MouseX2,
					KeyCode_Tab, KeyCode_Shift, KeyCode_Ctrl, KeyCode_Alt, KeyCode_CapsLock, KeyCode_Escape, KeyCode_Space,
					KeyCode_1, KeyCode_2, KeyCode_3, KeyCode_4, KeyCode_5, KeyCode_6,
					KeyCode_A, KeyCode_B, KeyCode_C, KeyCode_D, KeyCode_E, KeyCode_F,
					KeyCode_G, KeyCode_H, KeyCode_Q, KeyCode_R, KeyCode_S, KeyCode_T,
					KeyCode_V, KeyCode_W, KeyCode_X, KeyCode_Y, KeyCode_Z,
					KeyCode_F1, KeyCode_F2, KeyCode_F3, KeyCode_F4, KeyCode_F5,
					KeyCode_LeftWin, KeyCode_LeftShift, KeyCode_LeftCtrl, KeyCode_LeftAlt, KeyCode_OEMTilde,
				};

				for (const KeyCode leftSideKey : phsyicalLeftSideKeys)
				{
					if (binding.Keyboard.Key == leftSideKey)
						return PlayTestSlidePositionType::Left;
				}
				return PlayTestSlidePositionType::Right;
			}
			else if (binding.Type == BindingType::Controller)
			{
				switch (binding.Controller.Button)
				{
				case Button::DPadUp:
				case Button::DPadLeft:
				case Button::DPadDown:
				case Button::DPadRight:
				case Button::LeftStickUp:
				case Button::LeftStickLeft:
				case Button::LeftStickDown:
				case Button::LeftStickRight:
				case Button::LeftStickClick:
				case Button::LeftBumper:
				case Button::LeftTrigger:
				case Button::Select:
				case Button::Home:
				case Button::TouchPad:
					return PlayTestSlidePositionType::Left;

				case Button::FaceUp:
				case Button::FaceLeft:
				case Button::FaceDown:
				case Button::FaceRight:
				case Button::RightStickUp:
				case Button::RightStickLeft:
				case Button::RightStickDown:
				case Button::RightStickRight:
				case Button::RightStickClick:
				case Button::RightBumper:
				case Button::RightTrigger:
				case Button::Start:
					return PlayTestSlidePositionType::Right;
				}
			}

			return PlayTestSlidePositionType::Left;
		}

		constexpr PlayTestSlidePositionType AutomaticallyDeterminePlayTestSlidePosition(ButtonTypeFlags buttonTypes, Input::Binding& binding)
		{
			// NOTE: For simplicity sake determine slide position using expected physical key/button layout
			if (buttonTypes & ButtonTypeFlags_SlideAll)
				return ExpectedPhyisicalBindingLocationToPlayTestSlidePosition(binding);
			else
				return PlayTestSlidePositionType::None;
		}
	}

	void ChartEditorSettingsWindow::Gui()
	{
		lastFrameAnyItemActive = thisFrameAnyItemActive;
		thisFrameAnyItemActive = Gui::IsAnyItemActive();

		// NOTE: Adjust window height in the future to try and fit everything inside the window without scroll bars. for now at least...
		constexpr vec2 explicitMargin = vec2(6.0f);
		constexpr f32 windowHeight = 554.0f;
		constexpr f32 tabListWidth = 120.0f;
		constexpr f32 tabContentWidth = 600.0f;
		constexpr f32 tabControlWidth = 82.0f;
		const auto& style = Gui::GetStyle();

		Gui::BeginChild("OutterChild", vec2(tabListWidth + tabContentWidth + tabControlWidth + (explicitMargin.x * 4.0f), windowHeight + (explicitMargin.y * 2.0f)));

		Gui::SetCursorScreenPos(Gui::GetCursorScreenPos() + explicitMargin);

		Gui::BeginChild("TabListChild", vec2(tabListWidth, windowHeight), true, ImGuiWindowFlags_None);
		{
			Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(style.ItemSpacing.x, 3.0f));
			for (i32 i = 0; i < static_cast<i32>(std::size(namedTabs)); i++)
			{
				Gui::PushID(i);
				if (Gui::Selectable("##SelectableTab", (i == selectedTabIndex)))
					selectedTabIndex = i;
				Gui::SameLine(0.0f, 1.0f);
				Gui::TextUnformatted(namedTabs[i].Name);
				Gui::PopID();
			}
			Gui::PopStyleVar(1);
		}
		Gui::EndChild();

		Gui::SameLine(0.0f, 4.0f);

		// TODO: Implement box (at the bottom?) to display a discription about the last hovered settings item
		//		 or easier, just tooltips
		Gui::BeginChild("TabContentChild", vec2(tabContentWidth, windowHeight), false, alwaysShowVerticalScrollBar ? ImGuiWindowFlags_AlwaysVerticalScrollbar : ImGuiWindowFlags_None);
		{
			Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(style.ItemSpacing.x, 3.0f));
			if (selectedTabIndex < std::size(namedTabs))
				(this->*namedTabs[selectedTabIndex].GuiFunction)(GlobalUserData.Mutable());
			Gui::PopStyleVar(1);
		}
		Gui::EndChild();
		Gui::SameLine(0.0f, 4.0f);

		Gui::BeginChild("TabControlChild", vec2(Gui::GetContentRegionAvail().x - 4.0f, windowHeight), false, ImGuiWindowFlags_None);
		{
			if (Gui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !thisFrameAnyItemActive && !lastFrameAnyItemActive)
			{
				if (IsAnyPressed(GlobalUserData.Input.App_Dialog_YesOrOk, false))
					RequestWindowCloseAndKeepChanges();
				else if (IsAnyPressed(GlobalUserData.Input.App_Dialog_Cancel, false))
					RequestWindowCloseAndRevertChanges();

				if (IsAnyPressed(GlobalUserData.Input.App_Dialog_SelectNextTab, true))
					SelectNextTab(+1);
				if (IsAnyPressed(GlobalUserData.Input.App_Dialog_SelectPreviousTab, true))
					SelectNextTab(-1);
			}

			Gui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);
			Gui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
			Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(style.ItemSpacing.x, 3.0f));
			Gui::PushStyleVar(ImGuiStyleVar_FramePadding, vec2(style.FramePadding.x, 4.0f));
			Gui::PushStyleColor(ImGuiCol_Border, Gui::GetStyleColorVec4(ImGuiCol_SliderGrab));
			Gui::PushStyleColor(ImGuiCol_BorderShadow, vec4(0.0f));
			{
				const f32 buttonWidth = Gui::GetContentRegionAvail().x;
				if (Gui::Button("OK", vec2(buttonWidth, 0.0f)))
					RequestWindowCloseAndKeepChanges();
				if (Gui::Button("Cancel", vec2(buttonWidth, 0.0f)))
					RequestWindowCloseAndRevertChanges();

				const bool disableApplyButton = !pendingChanges;
				Gui::PushItemDisabledAndTextColorIf(disableApplyButton);
				if (Gui::Button("Apply", vec2(buttonWidth, 0.0f)))
					KeepChangesSaveToFile();
				Gui::PopItemDisabledAndTextColorIf(disableApplyButton);

				if (Gui::Button("Prev", vec2(buttonWidth, 0.0f)))
					SelectNextTab(-1);
				if (Gui::Button("Next", vec2(buttonWidth, 0.0f)))
					SelectNextTab(+1);
			}
			Gui::PopStyleColor(2);
			Gui::PopStyleVar(4);
		}
		Gui::EndChild();

		Gui::EndChild();
	}

	void ChartEditorSettingsWindow::OnWindowOpen()
	{
		SaveUserDataCopy();
	}

	void ChartEditorSettingsWindow::OnCloseButtonClicked()
	{
		RestoreUserDataCopy();
	}

	bool ChartEditorSettingsWindow::GetAndClearCloseRequestThisFrame()
	{
		const bool result = closeWindowThisFrame;
		closeWindowThisFrame = false;
		return result;
	}

	void ChartEditorSettingsWindow::SelectNextTab(i32 direction)
	{
		if (direction < 0)
			selectedTabIndex = (--selectedTabIndex < 0) ? (static_cast<i32>(std::size(namedTabs)) - 1) : selectedTabIndex;
		else if (direction > 0)
			selectedTabIndex = (++selectedTabIndex >= static_cast<i32>(std::size(namedTabs))) ? 0 : selectedTabIndex;
		else
			assert(false);
	}

	void ChartEditorSettingsWindow::SaveUserDataCopy()
	{
		userDataPreEditCopy = GlobalUserData;
		pendingChanges = false;
	}

	void ChartEditorSettingsWindow::RestoreUserDataCopy()
	{
		GlobalUserData.Mutable() = userDataPreEditCopy;
		pendingChanges = false;
	}

	void ChartEditorSettingsWindow::KeepChangesSaveToFile()
	{
		userDataPreEditCopy = GlobalUserData;
		if (pendingChanges)
		{
			pendingChanges = false;
			GlobalUserData.SaveToFile();

			auto& audioEngine = Audio::AudioEngine().GetInstance();
			const bool wasStreamRunnig = audioEngine.GetIsStreamOpenRunning();

			audioEngine.SetAudioBackend(GlobalUserData.System.Audio.RequestExclusiveDeviceAccess ? Audio::AudioBackend::WASAPIExclusive : Audio::AudioBackend::WASAPIShared);

			if (wasStreamRunnig)
				audioEngine.EnsureStreamRunning();
		}
	}

	void ChartEditorSettingsWindow::RequestWindowCloseAndKeepChanges()
	{
		closeWindowThisFrame = true;
		KeepChangesSaveToFile();
	}

	void ChartEditorSettingsWindow::RequestWindowCloseAndRevertChanges()
	{
		closeWindowThisFrame = true;
		RestoreUserDataCopy();
	}

	void ChartEditorSettingsWindow::GuiTabGeneral(ComfyStudioUserSettings& userData)
	{
		if (Gui::CollapsingHeader("Chart Properties", ImGuiTreeNodeFlags_DefaultOpen))
		{
			GuiBeginSettingsColumns();
			pendingChanges |= GuiSettingsInputText("Chart Creator Default Name", userData.ChartProperties.ChartCreatorDefaultName, "n/a");

			// TODO: Default button sounds (?)
			// TODO: Default difficulty <--

			GuiEndSettingsColumns();
		}

		if (Gui::CollapsingHeader("Target Preview", ImGuiTreeNodeFlags_DefaultOpen))
		{
			GuiBeginSettingsColumns();

			pendingChanges |= GuiSettingsCheckboxInverted("Hide Button Sprites", userData.TargetPreview.ShowButtons);
			pendingChanges |= GuiSettingsCheckbox("Show Hold Info Preview", userData.TargetPreview.ShowHoldInfo);

			if (auto v = userData.TargetPreview.PostHitLingerDuration.BeatsFraction();
				GuiSettingsInputF32("Post Hit Linger Duration", v, BeatTick::FromTicks(BeatTick::TicksPerBeat / 4).BeatsFraction(), BeatTick::FromBeats(1).BeatsFraction(), ImGuiInputTextFlags_None, "%.4gx Beat"))
			{
				userData.TargetPreview.PostHitLingerDuration = Clamp(BeatTick::FromBeatsFraction(v), BeatTick::Zero(), BeatTick::FromBars(1));
				pendingChanges = true;
			}

			GuiEndSettingsColumns();
		}

		if (Gui::CollapsingHeader("Target Inspector", ImGuiTreeNodeFlags_DefaultOpen))
		{
			static auto forEachNullTerminatedLine = [](std::string& mutableMultilineString, auto perLineFunc)
			{
				for (size_t i = 0; i < mutableMultilineString.size(); i++)
				{
					const std::string_view remaining = std::string_view(mutableMultilineString).substr(i);
					const std::string_view line = remaining.substr(0, remaining.find_first_of('\n'));

					if (line.empty())
						continue;
					else if (line.size() > 1)
						const_cast<char*>(line.data())[line.size()] = '\0';

					perLineFunc(line);
					i += line.size();
				}
			};

			static auto vectorToMultilineString = [](std::string& outFormattedMultilineString, const std::vector<f32>* inFloats, const std::vector<i32>* inInts)
			{
				char b[64];

				outFormattedMultilineString.clear();
				if (inFloats)
					for (const f32 value : *inFloats) { outFormattedMultilineString += std::string_view(b, sprintf_s(b, "%.2f\n", value)); }
				else
					for (const i32 value : *inInts) { outFormattedMultilineString += std::string_view(b, sprintf_s(b, "%d\n", value)); }

				if (!outFormattedMultilineString.empty())
					outFormattedMultilineString.pop_back();
			};

			static auto parseMultilineTextStringAndUpdateVector = [](std::string& formattedMultilineString, std::vector<f32>* outFloats, std::vector<i32>* outInts)
			{
				if (outFloats)
				{
					outFloats->clear();
					forEachNullTerminatedLine(formattedMultilineString, [outFloats](std::string_view line) { f32 out = {}; sscanf_s(line.data(), "%f", &out); outFloats->push_back(out); });
				}
				else
				{
					outInts->clear();
					forEachNullTerminatedLine(formattedMultilineString, [outInts](std::string_view line) { i32 out = {}; sscanf_s(line.data(), "%d", &out); outInts->push_back(out); });
				}
			};

			auto guiDropdownVectorFloatsOrInts = [this](std::vector<f32>* inOutFloats, std::vector<i32>* inOutInts)
			{
				Gui::PushID(inOutFloats ? static_cast<void*>(inOutFloats) : static_cast<void*>(inOutInts));
				Gui::NextColumn();
				{
					vectorToMultilineString(inspectorDropdownFormattedMultilineString, inOutFloats, inOutInts);

					if (inspectorDropdownInputTextActiveLastFrame) Gui::PushStyleColor(ImGuiCol_Text, vec4(0.83f, 0.75f, 0.42f, 1.0f));

					Gui::SetNextItemWidth(GuiSettingsItemWidth);
					if (Gui::InputTextMultiline("##InputText", &inspectorDropdownFormattedMultilineString, {}, ImGuiInputTextFlags_CallbackCharFilter | ImGuiInputTextFlags_NoHorizontalScroll, DecimalAndNewLineFilterCallback))
					{
						parseMultilineTextStringAndUpdateVector(inspectorDropdownFormattedMultilineString, inOutFloats, inOutInts);
						pendingChanges = true;
					}

					if (inspectorDropdownInputTextActiveLastFrame) Gui::PopStyleColor();
					inspectorDropdownInputTextActiveLastFrame = Gui::IsItemActive();
				}
				Gui::NextColumn();
				Gui::PopID();
			};

			GuiBeginSettingsColumns();
			GuiSettingsCombo("Combo Box Dropdown", selectedInspectorDropdownPropertyType, TargetPropertyTypeNames);
			if (selectedInspectorDropdownPropertyType >= TargetPropertyType_Count)
				selectedInspectorDropdownPropertyType = FindFirstNonEmptyInspectorDropdownPropertyType(userData);
			switch (selectedInspectorDropdownPropertyType)
			{
			case TargetPropertyType_PositionX: guiDropdownVectorFloatsOrInts(&userData.TargetPreset.InspectorDropdown.PositionsX, nullptr); break;
			case TargetPropertyType_PositionY: guiDropdownVectorFloatsOrInts(&userData.TargetPreset.InspectorDropdown.PositionsY, nullptr); break;
			case TargetPropertyType_Angle: guiDropdownVectorFloatsOrInts(&userData.TargetPreset.InspectorDropdown.Angles, nullptr); break;
			case TargetPropertyType_Frequency: guiDropdownVectorFloatsOrInts(nullptr, &userData.TargetPreset.InspectorDropdown.Frequencies); break;
			case TargetPropertyType_Amplitude: guiDropdownVectorFloatsOrInts(&userData.TargetPreset.InspectorDropdown.Amplitudes, nullptr); break;
			case TargetPropertyType_Distance: guiDropdownVectorFloatsOrInts(&userData.TargetPreset.InspectorDropdown.Distances, nullptr); break;
			}
			GuiEndSettingsColumns();
		}

		if (Gui::CollapsingHeader("BPM Calculator", ImGuiTreeNodeFlags_DefaultOpen))
		{
			GuiBeginSettingsColumns();
			pendingChanges |= GuiSettingsCombo("Tap Sound Type", userData.BPMCalculator.TapSoundType, BPMTapSoundTypeNames);
			pendingChanges |= GuiSettingsCheckbox("Auto Reset Enabled", userData.BPMCalculator.AutoResetEnabled);
			pendingChanges |= GuiSettingsCheckbox("Apply To Tempo Map", userData.BPMCalculator.ApplyToTempoMap);
			GuiEndSettingsColumns();
		}

#if COMFY_DEBUG && 1 // DEBUG: Mostly for internal testing, probably won't need to expose these to the user
		auto guiSettingsSliderVec3 = [](std::string_view label, vec3& inOutValue, f32 minValue, f32 maxValue, const char* format = "%.3f", ImGuiSliderFlags flags = ImGuiSliderFlags_None)
		{
			GuiSettingsRightAlignedLabel(label);
			Gui::NextColumn();

			Gui::PushID(Gui::StringViewStart(label), Gui::StringViewEnd(label));
			Gui::SetNextItemWidth(GuiSettingsItemWidth);
			const bool result = Gui::SliderFloat3("##SettingsSlider", glm::value_ptr(inOutValue), minValue, maxValue, format, flags);
			Gui::PopID();
			Gui::NextColumn();

			return result;
		};

		if (Gui::CollapsingHeader("UserData.System.Video - ColorCorrection", ImGuiTreeNodeFlags_None /*ImGuiTreeNodeFlags_DefaultOpen*/))
		{
			GuiBeginSettingsColumns();

			GuiSettingsCheckbox("Enabled (Editor)", userData.System.Video.EnableColorCorrectionEditor);
			GuiSettingsCheckbox("Enabled (Playtest)", userData.System.Video.EnableColorCorrectionPlaytest);

			GuiSettingsSliderF32("Gamma", userData.System.Video.ColorCorrectionParam.Gamma, 2.0f, 3.0f);
			GuiSettingsSliderF32("Contrast", userData.System.Video.ColorCorrectionParam.Contrast, 0.35f, 0.80f);

			guiSettingsSliderVec3("Coefficients R", userData.System.Video.ColorCorrectionParam.ColorCoefficientsRGB[0], 0.0f, 1.25f);
			guiSettingsSliderVec3("Coefficients G", userData.System.Video.ColorCorrectionParam.ColorCoefficientsRGB[1], 0.0f, 1.25f);
			guiSettingsSliderVec3("Coefficients B", userData.System.Video.ColorCorrectionParam.ColorCoefficientsRGB[2], 0.0f, 1.25f);

			GuiEndSettingsColumns();
		}
#endif
	}

	void ChartEditorSettingsWindow::GuiTabTimeline(ComfyStudioUserSettings& userData)
	{
		if (Gui::CollapsingHeader("Target Timeline", ImGuiTreeNodeFlags_DefaultOpen))
		{
			GuiBeginSettingsColumns();

			pendingChanges |= GuiSettingsInputF32("Scroll Speed", userData.TargetTimeline.MouseWheelScrollSpeed, 0.1f, 1.0f, ImGuiInputTextFlags_None, "%gx");
			pendingChanges |= GuiSettingsInputF32("Scroll Speed (Shift)", userData.TargetTimeline.MouseWheelScrollSpeedShift, 0.1f, 1.0f, ImGuiInputTextFlags_None, "%gx");

			// NOTE: Having the "x Grid Division" and "x Beat" format strings embeded here feels a bit iffy, in case their behavior ever changes...
			pendingChanges |= GuiSettingsInputF32("Playback Scroll Factor", userData.TargetTimeline.PlaybackMouseWheelScrollFactor, 0.1f, 1.0f, ImGuiInputTextFlags_None, "%gx Grid Division");
			pendingChanges |= GuiSettingsInputF32("Playback Scroll Factor (Shift)", userData.TargetTimeline.PlaybackMouseWheelScrollFactorShift, 0.1f, 1.0f, ImGuiInputTextFlags_None, "%gx Beat");

			if (auto v = (userData.TargetTimeline.MouseWheelScrollDirection < 0.0f);
				GuiSettingsCheckbox("Flip Mouse Scroll Direction", v))
			{
				userData.TargetTimeline.MouseWheelScrollDirection = (v ? -1.0f : +1.0f);
				pendingChanges = true;
			}

			Gui::Separator();

			if (auto v = ToPercent(userData.TargetTimeline.PlaybackAutoScrollCursorPositionFactor);
				GuiSettingsSliderF32("Playback Auto Scroll Cursor Position", v, 10.0f, 95.0f, "%.2f%% of Timeline", ImGuiSliderFlags_AlwaysClamp))
			{
				userData.TargetTimeline.PlaybackAutoScrollCursorPositionFactor = FromPercent(v);
				pendingChanges = true;
			}

			auto guiSettingsPlacementCursorOffset = [](std::string_view label, TimeSpan& inOutValue)
			{
				constexpr f32 minMaxOffsetMS = 100.0f;
				if (auto v = static_cast<f32>(inOutValue.TotalMilliseconds());
					GuiSettingsSliderF32(label, v, -minMaxOffsetMS, +minMaxOffsetMS, "%.2f ms from Cursor", ImGuiSliderFlags_AlwaysClamp))
				{
					inOutValue = TimeSpan::FromMilliseconds(v);
					return true;
				}

				return false;
			};

			pendingChanges |= guiSettingsPlacementCursorOffset("Shared Mode - Playback Placement Offset", userData.TargetTimeline.PlaybackCursorPlacementOffsetWasapiShared);
			pendingChanges |= guiSettingsPlacementCursorOffset("Exclusive Mode - Playback Placement Offset", userData.TargetTimeline.PlaybackCursorPlacementOffsetWasapiExclusive);

			Gui::Separator();

			auto isSmoothScrollDisabled = [](f32 speedSec) { return (speedSec <= 0.0f); };

			if (auto v = isSmoothScrollDisabled(userData.TargetTimeline.SmoothScrollSpeedSec);
				GuiSettingsCheckbox("Disable Smooth Scrolling", v))
			{
				targetTimelineSmoothScrollSpeedPreDisable = (v ? userData.TargetTimeline.SmoothScrollSpeedSec : targetTimelineSmoothScrollSpeedPreDisable);
				userData.TargetTimeline.SmoothScrollSpeedSec = (v ? 0.0f : targetTimelineSmoothScrollSpeedPreDisable);

				pendingChanges = true;
			}

			auto f32SecToMS = [](f32 sec) { return static_cast<f32>(TimeSpan::FromSeconds(sec).TotalMilliseconds()); };

			Gui::PushItemDisabledAndTextColorIf(isSmoothScrollDisabled(userData.TargetTimeline.SmoothScrollSpeedSec));
			if (auto v = f32SecToMS(userData.TargetTimeline.SmoothScrollSpeedSec);
				GuiSettingsSliderF32("Smooth Scroll Speed", v, f32SecToMS(TimelineMinSmoothScrollSpeedSec.x), f32SecToMS(TimelineMaxSmoothScrollSpeedSec.x), "%.2f ms", ImGuiSliderFlags_AlwaysClamp))
			{
				userData.TargetTimeline.SmoothScrollSpeedSec = static_cast<f32>(TimeSpan::FromMilliseconds(v).TotalSeconds());
				pendingChanges = true;
			}
			Gui::PopItemDisabledAndTextColorIf(isSmoothScrollDisabled(userData.TargetTimeline.SmoothScrollSpeedSec));

			Gui::Separator();

			if (auto v = userData.TargetTimeline.ShowStartEndMarkersSong && userData.TargetTimeline.ShowStartEndMarkersMovie;
				GuiSettingsCheckbox("Show Start / End Markers", v))
			{
				userData.TargetTimeline.ShowStartEndMarkersSong = v;
				userData.TargetTimeline.ShowStartEndMarkersMovie = v;
				pendingChanges = true;
			}

			Gui::Separator();

			// NOTE: To avoid half-pixel hitboxes
			auto floorToMultipleOfTwo = [](f32 rowHeight) { return glm::floor(rowHeight / 2.0f) * 2.0f; };

			// TODO: "Preset" buttons for selecting common "fixed" values (?)
			pendingChanges |= GuiSettingsCombo("Timeline Scaling Behavior", userData.TargetTimeline.ScalingBehavior, TargetTimelineScalingBehaviorNames);
			if (userData.TargetTimeline.ScalingBehavior == TargetTimelineScalingBehavior::AutoFit)
			{
				f32& userDataMinRowHeight = userData.TargetTimeline.ScalingBehaviorAutoFit.MinRowHeight;
				f32& userDataMaxRowHeight = userData.TargetTimeline.ScalingBehaviorAutoFit.MaxRowHeight;

				if (auto v = userDataMinRowHeight;
					GuiSettingsInputF32("Min Row Height", v, 2.0f, 8.0f, ImGuiInputTextFlags_None, "%.2f px"))
				{
					userDataMinRowHeight = Clamp(floorToMultipleOfTwo(v), TargetTimelineMinRowHeight, TargetTimelineMaxRowHeight);
					userDataMaxRowHeight = Clamp(userDataMaxRowHeight, userDataMinRowHeight, TargetTimelineMaxRowHeight);
					pendingChanges = true;
				}

				if (auto v = userDataMaxRowHeight;
					GuiSettingsInputF32("Max Row Height", v, 2.0f, 8.0f, ImGuiInputTextFlags_None, "%.2f px"))
				{
					userDataMaxRowHeight = Clamp(floorToMultipleOfTwo(v), TargetTimelineMinRowHeight, TargetTimelineMaxRowHeight);
					userDataMinRowHeight = Clamp(userDataMinRowHeight, TargetTimelineMinRowHeight, userDataMaxRowHeight);
					pendingChanges = true;
				}
			}
			else if (userData.TargetTimeline.ScalingBehavior == TargetTimelineScalingBehavior::FixedSize)
			{
				if (auto v = ToPercent(userData.TargetTimeline.ScalingBehaviorFixedSize.IconScale);
					GuiSettingsSliderF32("Icon Scale", v, ToPercent(TargetTimelineMinIconScale), ToPercent(TargetTimelineMaxIconScale), "%.2f%%", ImGuiSliderFlags_AlwaysClamp))
				{
					userData.TargetTimeline.ScalingBehaviorFixedSize.IconScale = FromPercent(v);
					pendingChanges = true;
				}

				if (auto v = userData.TargetTimeline.ScalingBehaviorFixedSize.RowHeight;
					GuiSettingsInputF32("Row Height", v, 2.0f, 8.0f, ImGuiInputTextFlags_None, "%.2f px"))
				{
					userData.TargetTimeline.ScalingBehaviorFixedSize.RowHeight = Clamp(floorToMultipleOfTwo(v), TargetTimelineMinRowHeight, TargetTimelineMaxRowHeight);
					pendingChanges = true;
				}
			}

			Gui::Separator();

			pendingChanges |= GuiSettingsCombo("Cursor Scrubbing - Edge Auto Scroll", userData.TargetTimeline.CursorScrubbingEdgeAutoScrollThreshold, TargetTimelineCursorScrubbingEdgeAutoScrollThresholdNames);
			if (userData.TargetTimeline.CursorScrubbingEdgeAutoScrollThreshold == TargetTimelineCursorScrubbingEdgeAutoScrollThreshold::FixedSize)
			{
				pendingChanges |= GuiSettingsInputF32("Auto Scroll Threshold", userData.TargetTimeline.CursorScrubbingEdgeAutoScrollThresholdFixedSize.Pixels, 1.0f, 10.0f, ImGuiInputTextFlags_None, "%.2f px");
			}
			else if (userData.TargetTimeline.CursorScrubbingEdgeAutoScrollThreshold == TargetTimelineCursorScrubbingEdgeAutoScrollThreshold::Proportional)
			{
				if (auto v = ToPercent(userData.TargetTimeline.CursorScrubbingEdgeAutoScrollThresholdProportional.Factor);
					GuiSettingsSliderF32("Auto Scroll Threshold", v, ToPercent(TargetTimelineMinCursorScrubEdgeAutoScrollWidthFactor), ToPercent(TargetTimelineMaxCursorScrubEdgeAutoScrollWidthFactor),
						"%.2f%% of Timeline", ImGuiSliderFlags_AlwaysClamp))
				{
					userData.TargetTimeline.CursorScrubbingEdgeAutoScrollThresholdProportional.Factor = FromPercent(v);
					pendingChanges = true;
				}
			}

			GuiEndSettingsColumns();
		}
	}

	void ChartEditorSettingsWindow::GuiTabTools(ComfyStudioUserSettings& userData)
	{
		if (Gui::CollapsingHeader("Position Tool", ImGuiTreeNodeFlags_DefaultOpen))
		{
			GuiBeginSettingsColumns();

			if (auto v = vec3(userData.PositionTool.PositionMouseSnap, userData.PositionTool.PositionMouseSnapRough, userData.PositionTool.PositionMouseSnapPrecise);
				GuiSettingsInputVec3("Position Mouse Snap (Normal, Rough, Precise)", v, ImGuiInputTextFlags_None, "%.2f px"))
			{
				userData.PositionTool.PositionMouseSnap = v[0];
				userData.PositionTool.PositionMouseSnapRough = v[1];
				userData.PositionTool.PositionMouseSnapPrecise = v[2];
				pendingChanges = true;
			}
			pendingChanges |= GuiSettingsInputF32("Position Interpolation Command Snap", userData.PositionTool.PositionInterpolationCommandSnap, 0.0f, 0.0f, ImGuiInputTextFlags_None, "%.2f px");

			if (auto v = vec3(userData.PositionTool.PositionKeyMoveStep, userData.PositionTool.PositionKeyMoveStepRough, userData.PositionTool.PositionKeyMoveStepPrecise);
				GuiSettingsInputVec3("Position Key Move Step (Normal, Rough, Precise)", v, ImGuiInputTextFlags_None, "%.2f px"))
			{
				userData.PositionTool.PositionKeyMoveStep = v[0];
				userData.PositionTool.PositionKeyMoveStepRough = v[1];
				userData.PositionTool.PositionKeyMoveStepPrecise = v[2];
				pendingChanges = true;
			}

			if (showRarelyUsedSettings)
				pendingChanges |= GuiSettingsInputF32("Mouse Row Center Distance Threshold", userData.PositionTool.MouseRowCenterDistanceThreshold, 0.0f, 0.0f, ImGuiTextFlags_None, "%.2f px");

			Gui::Separator();

			pendingChanges |= GuiSettingsCheckbox("Show Distance Guide Circles", userData.PositionTool.ShowDistanceGuides);
			pendingChanges |= GuiSettingsCheckbox("Show Target Grab Tooltip", userData.PositionTool.ShowTargetGrabTooltip);

			Gui::Separator();

			pendingChanges |= GuiSettingsCheckbox("Use Axis Snap Guides", userData.PositionTool.UseAxisSnapGuides);
			Gui::PushItemDisabledAndTextColorIf(!userData.PositionTool.UseAxisSnapGuides);
			pendingChanges |= GuiSettingsInputF32("Axis Snap Guide Threshold", userData.PositionTool.AxisSnapGuideDistanceThreshold, 0.0f, 0.0f, ImGuiInputTextFlags_None, "%.2f px");
			Gui::PopItemDisabledAndTextColorIf(!userData.PositionTool.UseAxisSnapGuides);

			GuiEndSettingsColumns();
		}

		if (Gui::CollapsingHeader("Path Tool", ImGuiTreeNodeFlags_DefaultOpen))
		{
			GuiBeginSettingsColumns();

			if (auto v = vec3(userData.PathTool.AngleMouseScrollStep, userData.PathTool.AngleMouseScrollRough, userData.PathTool.AngleMouseScrollPrecise);
				GuiSettingsInputVec3("Angle Scroll Step (Normal, Rough, Precise)", v, ImGuiInputTextFlags_None, "%.2f" DEGREE_SIGN))
			{
				userData.PathTool.AngleMouseScrollStep = v[0];
				userData.PathTool.AngleMouseScrollRough = v[1];
				userData.PathTool.AngleMouseScrollPrecise = v[2];
				pendingChanges = true;
			}

			if (auto v = (userData.PathTool.AngleMouseScrollDirection > 0.0f); GuiSettingsCheckbox("Flip Mouse Scroll Direction", v))
			{
				userData.PathTool.AngleMouseScrollDirection = (v ? +1.0f : -1.0f);
				pendingChanges = true;
			}

			Gui::Separator();

			if (auto v = vec3(userData.PathTool.AngleMouseSnap, userData.PathTool.AngleMouseSnapRough, userData.PathTool.AngleMouseSnapPrecise);
				GuiSettingsInputVec3("Angle Mouse Snap (Normal, Rough, Precise)", v, ImGuiInputTextFlags_None, "%.2f" DEGREE_SIGN))
			{
				userData.PathTool.AngleMouseSnap = v[0];
				userData.PathTool.AngleMouseSnapRough = v[1];
				userData.PathTool.AngleMouseSnapPrecise = v[2];
				pendingChanges = true;
			}

			pendingChanges |= GuiSettingsInputF32("Angle Mouse Movement Threshold", userData.PathTool.AngleMouseMovementDistanceThreshold, 0.0f, 0.0f, ImGuiTextFlags_None, "%.2f px");

			if (showRarelyUsedSettings)
				pendingChanges |= GuiSettingsInputF32("Angle Mouse Target Center Threshold", userData.PathTool.AngleMouseTargetCenterDistanceThreshold, 0.0f, 0.0f, ImGuiTextFlags_None, "%.2f px");

			GuiEndSettingsColumns();
		}
	}

	void ChartEditorSettingsWindow::GuiTabInterface(ComfyStudioUserSettings& userData)
	{
		if (Gui::CollapsingHeader("Chart Background", ImGuiTreeNodeFlags_DefaultOpen))
		{
			GuiBeginSettingsColumns();

			// NOTE: Technically doesn't belong in this category but this seems better for now than having it inside its own CollpasingHeader
			pendingChanges |= GuiSettingsCombo("Game Theme", userData.Interface.Theme, GameThemeNames);

			Gui::Separator();

			pendingChanges |= GuiSettingsCombo("Editor", userData.Interface.BackgroundDisplayType.Editor, ChartBackgroundDisplayTypeNames, ChartBackgroundDisplayType::FirstNoMovie, ChartBackgroundDisplayType::LastNoMovie);
			pendingChanges |= GuiSettingsCombo("Editor (Movie Loaded)", userData.Interface.BackgroundDisplayType.EditorWithMovie, ChartBackgroundDisplayTypeNames);
			pendingChanges |= GuiSettingsCombo("Playtest", userData.Interface.BackgroundDisplayType.Playtest, ChartBackgroundDisplayTypeNames, ChartBackgroundDisplayType::FirstNoMovie, ChartBackgroundDisplayType::LastNoMovie);
			pendingChanges |= GuiSettingsCombo("Playtest (Movie Loaded)", userData.Interface.BackgroundDisplayType.PlaytestWithMovie, ChartBackgroundDisplayTypeNames);

			GuiEndSettingsColumns();
		}

		if (Gui::CollapsingHeader("Placement Grid", ImGuiTreeNodeFlags_DefaultOpen))
		{
			GuiBeginSettingsColumns();

			pendingChanges |= GuiSettingsCheckbox("Horizontal Sync Markers", userData.Interface.PlacementGrid.ShowHorizontalSyncMarkers);

			GuiEndSettingsColumns();
		}

		if (Gui::CollapsingHeader("Checkerboard Background", ImGuiTreeNodeFlags_DefaultOpen))
		{
			GuiBeginSettingsColumns();

			if (auto v = vec3(userData.Interface.CheckerboardBackground.PrimaryColor);
				GuiSettingsColorEdit3("Primary Color", v))
			{
				userData.Interface.CheckerboardBackground.PrimaryColor = vec4(v, 1.0f);
				pendingChanges = true;
			}

			if (auto v = vec3(userData.Interface.CheckerboardBackground.SecondaryColor);
				GuiSettingsColorEdit3("Secondary Color", v))
			{
				userData.Interface.CheckerboardBackground.SecondaryColor = vec4(v, 1.0f);
				pendingChanges = true;
			}

			pendingChanges |= GuiSettingsSliderI32("Cell Size", userData.Interface.CheckerboardBackground.ScreenPixelSize, 0, 16, "%d px", ImGuiSliderFlags_AlwaysClamp);

			GuiEndSettingsColumns();
		}

		if (Gui::CollapsingHeader("Practice Background", ImGuiTreeNodeFlags_DefaultOpen))
		{
			GuiBeginSettingsColumns();

			pendingChanges |= GuiSettingsCheckbox("Disable Background Grid", userData.Interface.PracticeBackground.DisableBackgroundGrid);
			pendingChanges |= GuiSettingsCheckbox("Disable Background Dim", userData.Interface.PracticeBackground.DisableBackgroundDim);
			pendingChanges |= GuiSettingsCheckbox("Hide 'Now Printing' Placeholder Images", userData.Interface.PracticeBackground.HideNowPrintingPlaceholderImages);
			pendingChanges |= GuiSettingsCheckbox("Always Hide Cover Image", userData.Interface.PracticeBackground.HideCoverImage);
			pendingChanges |= GuiSettingsCheckbox("Always Hide Logo Image", userData.Interface.PracticeBackground.HideLogoImage);

			GuiEndSettingsColumns();
		}

		if (Gui::CollapsingHeader("Image and Movie Background", ImGuiTreeNodeFlags_DefaultOpen))
		{
			GuiBeginSettingsColumns();

			if (auto v = ToPercent(userData.Interface.ImageMovieBackground.OverlayColor.a);
				GuiSettingsSliderF32("Overlay Dim", v, 0.0f, 95.0f, "%.f%%", ImGuiSliderFlags_AlwaysClamp))
			{
				userData.Interface.ImageMovieBackground.OverlayColor.a = FromPercent(v);
				pendingChanges = true;
			}

			if (auto v = ToPercent(userData.Interface.ImageMovieBackground.OverlayColorGrid.a);
				GuiSettingsSliderF32("Overlay Dim (Grid)", v, 0.0f, 95.0f, "%.f%%", ImGuiSliderFlags_AlwaysClamp))
			{
				userData.Interface.ImageMovieBackground.OverlayColorGrid.a = FromPercent(v);
				pendingChanges = true;
			}

			pendingChanges |= GuiSettingsColorEdit4("Movie Pre-Start / Post-End Color", userData.Interface.ImageMovieBackground.PreStartPostEndMovieColor);

			GuiEndSettingsColumns();
		}
	}

	void ChartEditorSettingsWindow::GuiTabAutoSave(ComfyStudioUserSettings& userData)
	{
		if (Gui::CollapsingHeader("Auto Save", ImGuiTreeNodeFlags_DefaultOpen))
		{
			GuiBeginSettingsColumns();

			pendingChanges |= GuiSettingsCheckbox("Enable Auto Save", userData.SaveAndLoad.AutoSaveEnabled);

			Gui::PushItemDisabledAndTextColorIf(!userData.SaveAndLoad.AutoSaveEnabled);

			if (auto v = static_cast<f32>(userData.SaveAndLoad.AutoSaveInterval.TotalMinutes());
				GuiSettingsInputF32("Auto Save Interval", v, 1.0f, 10.0f, ImGuiInputTextFlags_None, "%.0f min"))
			{
				userData.SaveAndLoad.AutoSaveInterval = TimeSpan::FromMinutes(Clamp(v, 1.0f, 60.0f));
				pendingChanges = true;
			}

			if (auto v = userData.SaveAndLoad.MaxAutoSaveFiles;
				GuiSettingsInputI32("Max Auto Saves", v, 1, 10, ImGuiInputTextFlags_None, (v == 1) ? "%d file" : "%d files"))
			{
				userData.SaveAndLoad.MaxAutoSaveFiles = Clamp(v, 1, 999);
				pendingChanges = true;
			}

			Gui::Separator();

			{
				autoSaveDirectoryBuffer.clear();
				if (!userData.SaveAndLoad.RelativeAutoSaveDirectory.empty())
				{
					autoSaveDirectoryBuffer += "${ComfyStudio}/";
					autoSaveDirectoryBuffer += userData.SaveAndLoad.RelativeAutoSaveDirectory;
					autoSaveDirectoryBuffer += "/";
				}
				else
				{
					autoSaveDirectoryBuffer += "(None)";
				}

				GuiSettingsInputText("Auto Save Directory", autoSaveDirectoryBuffer, nullptr, ImGuiInputTextFlags_ReadOnly);
			}

			Gui::PopItemDisabledAndTextColorIf(!userData.SaveAndLoad.AutoSaveEnabled);

			GuiEndSettingsColumns();
		}
	}

	void ChartEditorSettingsWindow::GuiTabPlaytest(ComfyStudioUserSettings& userData)
	{
		if (Gui::CollapsingHeader("Playtest", ImGuiTreeNodeFlags_DefaultOpen))
		{
			GuiBeginSettingsColumns();

			auto guiSettingsSongOffset = [](std::string_view label, TimeSpan& inOutValue)
			{
				constexpr f32 minMaxOffsetMS = 100.0f;
				if (auto v = static_cast<f32>(inOutValue.TotalMilliseconds());
					GuiSettingsSliderF32(label, v, -minMaxOffsetMS, +minMaxOffsetMS, "%.2f ms", ImGuiSliderFlags_AlwaysClamp))
				{
					inOutValue = TimeSpan::FromMilliseconds(v);
					return true;
				}

				return false;
			};

			pendingChanges |= guiSettingsSongOffset("Shared Mode - Song Offset", userData.Playtest.SongOffsetWasapiShared);
			pendingChanges |= guiSettingsSongOffset("Exclusive Mode - Song Offset", userData.Playtest.SongOffsetWasapiExclusive);

			Gui::Separator();

			pendingChanges |= GuiSettingsCheckbox("Enter Fullscreen on Maximized Start", userData.Playtest.EnterFullscreenOnMaximizedStart);
			pendingChanges |= GuiSettingsCheckbox("Auto Hide Mouse Cursor", userData.Playtest.AutoHideCursor);

			// TODO: Maybe eventually options for how the transition between ChartEditor and PlayTestCore should occure (?)
			//		 - don't pause on playtest exit, don't play on playtest enter, don't rewind one bar, etc.

			GuiEndSettingsColumns();

#if 1 // TODO: Is this a good idea to have here..?
			{
				Gui::Separator();
				Gui::SetCursorPosX(GuiSettingsInnerMargin);

				Gui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				Gui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, { 0.0f, 0.5f });
				Gui::PushStyleColor(ImGuiCol_Text, Gui::GetColorU32(ImGuiCol_Text, 0.55f));
				Gui::PushStyleColor(ImGuiCol_Button, {});
				Gui::ButtonEx(
					"\n"
					"NOTE: The direction of the offset slider matches\n"
					"that in which the song waveform would be visually moved towards.\n"
					"Typically the intention is to delay the song by a few milliseconds\n"
					"which in this case would mean moving the slider slightly to the right.",
					{ Gui::GetContentRegionAvail().x - GuiSettingsInnerMargin, 0.0f });
				Gui::PopStyleColor(2);
				Gui::PopStyleVar(1);
				Gui::PopItemFlag();
			}
#endif
		}
	}

#if COMFY_COMILE_WITH_DLL_DISCORD_RICH_PRESENCE_INTEGRATION
	void ChartEditorSettingsWindow::GuiTabDiscord(ComfyStudioUserSettings& userData)
	{
		// NOTE: A single header with so few options looks a bit empty 
		//		 but it's fundementally different from the other "General" options so separating it seems to make sense
		if (Gui::CollapsingHeader("Discord Integration", ImGuiTreeNodeFlags_DefaultOpen))
		{
			GuiBeginSettingsColumns();
			pendingChanges |= GuiSettingsCheckbox("Enaled Discord Rich Presence", userData.System.Discord.EnableRichPresence);

			Gui::PushItemDisabledAndTextColorIf(!userData.System.Discord.EnableRichPresence);
			pendingChanges |= GuiSettingsCheckbox("Share Elapsed Time", userData.System.Discord.ShareElapsedTime);
			Gui::PopItemDisabledAndTextColorIf(!userData.System.Discord.EnableRichPresence);

			GuiEndSettingsColumns();
		}
	}
#endif

	void ChartEditorSettingsWindow::GuiTabAudio(ComfyStudioUserSettings& userData)
	{
		if (Gui::CollapsingHeader("Volume Levels", ImGuiTreeNodeFlags_DefaultOpen))
		{
			GuiBeginSettingsColumns();
			pendingChanges |= GuiSettingsVolumeSlider("Song Volume", userData.System.Audio.SongVolume, ImGuiSliderFlags_AlwaysClamp);
			pendingChanges |= GuiSettingsVolumeSlider("Button Sound Volume", userData.System.Audio.ButtonSoundVolume, ImGuiSliderFlags_AlwaysClamp);
			pendingChanges |= GuiSettingsVolumeSlider("Sound Effect Volume", userData.System.Audio.SoundEffectVolume, ImGuiSliderFlags_AlwaysClamp);
			pendingChanges |= GuiSettingsVolumeSlider("Metronome Volume", userData.System.Audio.MetronomeVolume, ImGuiSliderFlags_AlwaysClamp, true);
			GuiEndSettingsColumns();
		}

		if (Gui::CollapsingHeader("Device Access Behavior", ImGuiTreeNodeFlags_DefaultOpen))
		{
			GuiBeginSettingsColumns();
			pendingChanges |= GuiSettingsCheckbox("Open Device on Startup", userData.System.Audio.OpenDeviceOnStartup);
			pendingChanges |= GuiSettingsCheckbox("Close Device on Idle Focus Loss", userData.System.Audio.CloseDeviceOnIdleFocusLoss);
			pendingChanges |= GuiSettingsCheckbox("Request Exclusive Device Access", userData.System.Audio.RequestExclusiveDeviceAccess);
			GuiEndSettingsColumns();
		}
	}

	void ChartEditorSettingsWindow::GuiTabControllerLayout(ComfyStudioUserSettings& userData)
	{
		if (Gui::CollapsingHeader("Controller Layout", ImGuiTreeNodeFlags_DefaultOpen))
		{
			Gui::PushStyleColor(ImGuiCol_ChildBg, Gui::GetStyleColorVec4(ImGuiCol_FrameBg));
			Gui::SetCursorPosX(Gui::GetCursorPosX() + GuiSettingsInnerSubChildMargin);
			Gui::BeginChild("OutterLayoutsChild", vec2(GuiSettingsSubChildItemWidth, 0.0f), true, ImGuiWindowFlags_None);
			Gui::PopStyleColor(1);

			if (Input::GlobalSystemGetConnectedControllerCount() < 1)
			{
				Gui::PushStyleColor(ImGuiCol_Text, Gui::GetStyleColorVec4(ImGuiCol_TextDisabled));
				Gui::PushStyleColor(ImGuiCol_Button, Gui::GetColorU32(ImGuiCol_Button, 0.25f));
				Gui::ButtonEx("(No Controller Connected)", Gui::GetContentRegionAvail(), ImGuiButtonFlags_Disabled);
				Gui::PopStyleColor(2);
			}
			else
			{
				GuiControllerLayoutTabBarAndContent(userData);
			}

			Gui::EndChild();
		}
	}

	void ChartEditorSettingsWindow::GuiTabEditorBindings(ComfyStudioUserSettings& userData)
	{
		struct NamedBinding
		{
			Input::MultiBinding* MultiBinding;
			std::string_view Name;
		};

		// TODO: Somehow refactor this to combine with json IDs..?
		const NamedBinding namedMultiBindings[] =
		{
			{ &userData.Input.App_ToggleFullscreen, "App - Toggle Fullscreen" },
			{ &userData.Input.App_Dialog_YesOrOk, "App - Dialog - Yes or OK" },
			{ &userData.Input.App_Dialog_No, "App - Dialog - No" },
			{ &userData.Input.App_Dialog_Cancel, "App - Dialog - Cancel" },
			{ &userData.Input.App_Dialog_SelectNextTab, "App - Dialog - Select Next Tab" },
			{ &userData.Input.App_Dialog_SelectPreviousTab, "App - Dialog - Select Previous Tab" },
			{ &userData.Input.ChartEditor_ChartNew, "Chart Editor - Chart New" },
			{ &userData.Input.ChartEditor_ChartOpen, "Chart Editor - Chart Open" },
			{ &userData.Input.ChartEditor_ChartSave, "Chart Editor - Chart Save" },
			{ &userData.Input.ChartEditor_ChartSaveAs, "Chart Editor - Chart Save As" },
			{ &userData.Input.ChartEditor_ChartOpenDirectory, "Chart Editor - Chart Open Directory" },
			{ &userData.Input.ChartEditor_ImportUPDCChart, "Chart Editor - Import UPDC Chart" },
			{ &userData.Input.ChartEditor_ImportPVScriptChart, "Chart Editor - Import PV Script Chart" },
			{ &userData.Input.ChartEditor_ExportUPDCChart, "Chart Editor - Export UPDC Chart" },
			{ &userData.Input.ChartEditor_ExportPVScriptMData, "Chart Editor - Export PV Script MData" },
			{ &userData.Input.ChartEditor_ExportPVScriptChart, "Chart Editor - Export PV Script Chart" },
			{ &userData.Input.ChartEditor_Undo, "Chart Editor - Undo" },
			{ &userData.Input.ChartEditor_Redo, "Chart Editor - Redo" },
			{ &userData.Input.ChartEditor_OpenSettings, "Chart Editor - Open Settings" },
			{ &userData.Input.ChartEditor_StartPlaytestFromStart, "Chart Editor - Start Playtest from Start" },
			{ &userData.Input.ChartEditor_StartPlaytestFromCursor, "Chart Editor - Start Playtest from Cursor" },
			{ &userData.Input.ChartEditor_CreateManualAutoSave, "Chart Editor - Create Manual Auto Save" },
			{ &userData.Input.ChartEditor_OpenAutoSaveDirectory, "Chart Editor - Open Auto Save Directory" },
			{ &userData.Input.Timeline_CenterCursor, "Timeline - Center Cursor" },
			{ &userData.Input.Timeline_TogglePlayback, "Timeline - Toggle Playback" },
			{ &userData.Input.Timeline_StopPlayback, "Timeline - Stop Playback" },
			{ &userData.Input.TargetTimeline_Cut, "Target Timeline - Cut" },
			{ &userData.Input.TargetTimeline_Copy, "Target Timeline - Copy" },
			{ &userData.Input.TargetTimeline_Paste, "Target Timeline - Paste" },
			{ &userData.Input.TargetTimeline_MoveCursorLeft, "Target Timeline - Move Cursor Left" },
			{ &userData.Input.TargetTimeline_MoveCursorRight, "Target Timeline - Move Cursor Right" },
			{ &userData.Input.TargetTimeline_GoToStartOfTimeline, "Target Timeline - Go to Start of Timeline" },
			{ &userData.Input.TargetTimeline_GoToEndOfTimeline, "Target Timeline - Go to End of Timeline" },
			{ &userData.Input.TargetTimeline_IncreaseGridPrecision, "Target Timeline - Increase Grid Precision" },
			{ &userData.Input.TargetTimeline_DecreaseGridPrecision, "Target Timeline - Decrease Grid Precision" },
			{ &userData.Input.TargetTimeline_SetGridDivision_4, "Target Timeline - Set Grid Division 1 / 4" },
			{ &userData.Input.TargetTimeline_SetGridDivision_8, "Target Timeline - Set Grid Division 1 / 8" },
			{ &userData.Input.TargetTimeline_SetGridDivision_12, "Target Timeline - Set Grid Division 1 / 12" },
			{ &userData.Input.TargetTimeline_SetGridDivision_16, "Target Timeline - Set Grid Division 1 / 16" },
			{ &userData.Input.TargetTimeline_SetGridDivision_24, "Target Timeline - Set Grid Division 1 / 24" },
			{ &userData.Input.TargetTimeline_SetGridDivision_32, "Target Timeline - Set Grid Division 1 / 32" },
			{ &userData.Input.TargetTimeline_SetGridDivision_48, "Target Timeline - Set Grid Division 1 / 48" },
			{ &userData.Input.TargetTimeline_SetGridDivision_64, "Target Timeline - Set Grid Division 1 / 64" },
			{ &userData.Input.TargetTimeline_SetGridDivision_96, "Target Timeline - Set Grid Division 1 / 96" },
			{ &userData.Input.TargetTimeline_SetGridDivision_192, "Target Timeline - Set Grid Division 1 / 192" },
			{ &userData.Input.TargetTimeline_SetChainSlideGridDivision_12, "Target Timeline - Set Chain Slide Grid Division 1 / 12" },
			{ &userData.Input.TargetTimeline_SetChainSlideGridDivision_16, "Target Timeline - Set Chain Slide Grid Division 1 / 16" },
			{ &userData.Input.TargetTimeline_SetChainSlideGridDivision_24, "Target Timeline - Set Chain Slide Grid Division 1 / 24" },
			{ &userData.Input.TargetTimeline_SetChainSlideGridDivision_32, "Target Timeline - Set Chain Slide Grid Division 1 / 32" },
			{ &userData.Input.TargetTimeline_SetChainSlideGridDivision_48, "Target Timeline - Set Chain Slide Grid Division 1 / 48" },
			{ &userData.Input.TargetTimeline_SetChainSlideGridDivision_64, "Target Timeline - Set Chain Slide Grid Division 1 / 64" },
			{ &userData.Input.TargetTimeline_StartEndRangeSelection, "Target Timeline - Start/End Range Selection" },
			{ &userData.Input.TargetTimeline_DeleteSelection, "Target Timeline - Delete Selection" },
			{ &userData.Input.TargetTimeline_SelectAll, "Target Timeline - Select All" },
			{ &userData.Input.TargetTimeline_DeselectAll, "Target Timeline - Deselect All" },
			{ &userData.Input.TargetTimeline_IncreasePlaybackSpeed, "Target Timeline - Increase Playback Speed" },
			{ &userData.Input.TargetTimeline_DecreasePlaybackSpeed, "Target Timeline - Decrease Playback Speed" },
			{ &userData.Input.TargetTimeline_SetPlaybackSpeed_100Percent, "Target Timeline - Set Playback Speed 100%" },
			{ &userData.Input.TargetTimeline_SetPlaybackSpeed_75Percent, "Target Timeline - Set Playback Speed 75%" },
			{ &userData.Input.TargetTimeline_SetPlaybackSpeed_50Percent, "Target Timeline - Set Playback Speed 50%" },
			{ &userData.Input.TargetTimeline_SetPlaybackSpeed_25Percent, "Target Timeline - Set Playback Speed 25%" },
			{ &userData.Input.TargetTimeline_ToggleMetronome, "Target Timeline - Toggle Metronome" },
			{ &userData.Input.TargetTimeline_ToggleTargetHolds, "Target Timeline - Toggle Target Holds" },
			{ &userData.Input.TargetTimeline_PlaceTriangle, "Target Timeline - Place Triangle" },
			{ &userData.Input.TargetTimeline_PlaceSquare, "Target Timeline - Place Square" },
			{ &userData.Input.TargetTimeline_PlaceCross, "Target Timeline - Place Cross" },
			{ &userData.Input.TargetTimeline_PlaceCircle, "Target Timeline - Place Circle" },
			{ &userData.Input.TargetTimeline_PlaceSlideL, "Target Timeline - Place Slide L" },
			{ &userData.Input.TargetTimeline_PlaceSlideR, "Target Timeline - Place Slide R" },
			{ &userData.Input.TargetTimeline_ModifyTargetsMirrorTypes, "Target Timeline - Modify Targets Mirror Types" },
			{ &userData.Input.TargetTimeline_ModifyTargetsExpandTime2To1, "Target Timeline - Modify Targets Expand Time 2:1" },
			{ &userData.Input.TargetTimeline_ModifyTargetsExpandTime3To2, "Target Timeline - Modify Targets Expand Time 3:2" },
			{ &userData.Input.TargetTimeline_ModifyTargetsExpandTime4To3, "Target Timeline - Modify Targets Expand Time 4:3" },
			{ &userData.Input.TargetTimeline_ModifyTargetsCompressTime1To2, "Target Timeline - Modify Targets Compress Time 1:2" },
			{ &userData.Input.TargetTimeline_ModifyTargetsCompressTime2To3, "Target Timeline - Modify Targets Compress Time 2:3" },
			{ &userData.Input.TargetTimeline_ModifyTargetsCompressTime3To4, "Target Timeline - Modify Targets Compress Time 3:4" },
			{ &userData.Input.TargetTimeline_RefineSelectionSelectEvery2ndTarget, "Target Timeline - Refine Selection Select every 2nd Target" },
			{ &userData.Input.TargetTimeline_RefineSelectionSelectEvery3rdTarget, "Target Timeline - Refine Selection Select every 3rd Target" },
			{ &userData.Input.TargetTimeline_RefineSelectionSelectEvery4thTarget, "Target Timeline - Refine Selection Select every 4th Target" },
			{ &userData.Input.TargetTimeline_RefineSelectionShiftSelectionLeft, "Target Timeline - Refine Selection Shift Selection Left" },
			{ &userData.Input.TargetTimeline_RefineSelectionShiftSelectionRight, "Target Timeline - Refine Selection Shift Selection Right" },
			{ &userData.Input.TargetTimeline_RefineSelectionSelectAllSingleTargets, "Target Timeline - Refine Selection Select all Single Targets" },
			{ &userData.Input.TargetTimeline_RefineSelectionSelectAllSyncTargets, "Target Timeline - Refine Selection Select all Sync Targets" },
			{ &userData.Input.TargetTimeline_RefineSelectionSelectAllPartiallySelectedSyncPairs, "Target Timeline - Refine Selection Select all partially selected Sync Pairs" },
			{ &userData.Input.TargetPreview_JumpToPreviousTarget, "Target Preview - Jump to Previous Target" },
			{ &userData.Input.TargetPreview_JumpToNextTarget, "Target Preview - Jump to Next Target" },
			{ &userData.Input.TargetPreview_TogglePlayback, "Target Preview - Toggle Playback" },
			{ &userData.Input.TargetPreview_SelectPositionTool, "Target Preview - Select Position Tool" },
			{ &userData.Input.TargetPreview_SelectPathTool, "Target Preview - Select Path Tool" },
			{ &userData.Input.TargetPreview_PositionTool_MoveUp, "Target Preview - Position Tool - Move Up" },
			{ &userData.Input.TargetPreview_PositionTool_MoveLeft, "Target Preview - Position Tool - Move Left" },
			{ &userData.Input.TargetPreview_PositionTool_MoveDown, "Target Preview - Position Tool - Move Down" },
			{ &userData.Input.TargetPreview_PositionTool_MoveRight, "Target Preview - Position Tool - Move Right" },
			{ &userData.Input.TargetPreview_PositionTool_FlipHorizontal, "Target Preview - Position Tool - Flip Horizontal" },
			{ &userData.Input.TargetPreview_PositionTool_FlipHorizontalLocal, "Target Preview - Position Tool - Flip Horizontal Local" },
			{ &userData.Input.TargetPreview_PositionTool_FlipVertical, "Target Preview - Position Tool - Flip Vertical" },
			{ &userData.Input.TargetPreview_PositionTool_FlipVerticalLocal, "Target Preview - Position Tool - Flip Vertical Local" },
			{ &userData.Input.TargetPreview_PositionTool_PositionInRow, "Target Preview - Position Tool - Position in Row" },
			{ &userData.Input.TargetPreview_PositionTool_PositionInRowBack, "Target Preview - Position Tool - Position in Row Back" },
			{ &userData.Input.TargetPreview_PositionTool_InterpolateLinear, "Target Preview - Position Tool - Interpolate Linear" },
			{ &userData.Input.TargetPreview_PositionTool_InterpolateCircular, "Target Preview - Position Tool - Interpolate Circular" },
			{ &userData.Input.TargetPreview_PositionTool_InterpolateCircularFlip, "Target Preview - Position Tool - Interpolate Circular Flip" },
			{ &userData.Input.TargetPreview_PositionTool_StackPositions, "Target Preview - Position Tool - Stack Positions" },
			{ &userData.Input.TargetPreview_PathTool_InvertFrequencies, "Target Preview - Path Tool - Invert Frequencies" },
			{ &userData.Input.TargetPreview_PathTool_InterpolateAnglesClockwise, "Target Preview - Path Tool - Interpolate Angles Clockwise" },
			{ &userData.Input.TargetPreview_PathTool_InterpolateAnglesCounterclockwise, "Target Preview - Path Tool - Interpolate Angles" },
			{ &userData.Input.TargetPreview_PathTool_InterpolateDistances, "Target Preview - Path Tool - Interpolate Distances" },
			{ &userData.Input.TargetPreview_PathTool_ApplyAngleIncrementsPositive, "Target Preview - Path Tool - Apply Angle Increments Positive" },
			{ &userData.Input.TargetPreview_PathTool_ApplyAngleIncrementsPositiveBack, "Target Preview - Path Tool - Apply Angle Increments Positive Back" },
			{ &userData.Input.TargetPreview_PathTool_ApplyAngleIncrementsNegative, "Target Preview - Path Tool - Apply Angle Increments Negative" },
			{ &userData.Input.TargetPreview_PathTool_ApplyAngleIncrementsNegativeBack, "Target Preview - Path Tool - Apply Angle Increments Negative Back" },
			{ &userData.Input.BPMCalculator_Tap, "BPM Calculator - Tap" },
			{ &userData.Input.BPMCalculator_Reset, "BPM Calculator - Reset" },
			{ &userData.Input.Playtest_ReturnToEditorCurrent, "Playtest - Return to Editor Current" },
			{ &userData.Input.Playtest_ReturnToEditorPrePlaytest, "Playtest - Return to Editor pre Playtest" },
			{ &userData.Input.Playtest_ToggleAutoplay, "Playtest - Toggle Autoplay" },
			{ &userData.Input.Playtest_TogglePause, "Playtest - Toggle Pause" },
			{ &userData.Input.Playtest_RestartFromResetPoint, "Playtest - Restart from Reset Point" },
			{ &userData.Input.Playtest_MoveResetPointBackward, "Playtest - Move Reset Point Backward" },
			{ &userData.Input.Playtest_MoveResetPointForward, "Playtest - Move Reset Point Forward" },
		};

		constexpr size_t definedBindingsCountInsideThisSourceFile = sizeof(namedMultiBindings) / sizeof(NamedBinding);
		constexpr size_t totalBindingsCountInsideUserDataHeader = (sizeof(userData.Input) - sizeof(userData.Input.ControllerLayoutMappings) - sizeof(userData.Input.PlaytestBindings)) / sizeof(Input::MultiBinding);
		static_assert(definedBindingsCountInsideThisSourceFile == totalBindingsCountInsideUserDataHeader);

		const auto& style = Gui::GetStyle();
		constexpr f32 primaryColumWidthFactor = 0.65f;

		if (Gui::CollapsingHeader("Editor Bindings", ImGuiTreeNodeFlags_DefaultOpen))
		{
			Gui::SetCursorPosX(Gui::GetCursorPosX() + GuiSettingsInnerSubChildMargin);
			bindingFilter.Draw("##Filter", ICON_FA_SEARCH "  Search...", GuiSettingsSubChildItemWidth);

			Gui::PushStyleColor(ImGuiCol_ChildBg, Gui::GetStyleColorVec4(ImGuiCol_FrameBg));
			Gui::SetCursorPosX(Gui::GetCursorPosX() + GuiSettingsInnerSubChildMargin);
			Gui::BeginChild("BindingsListChild", vec2(GuiSettingsSubChildItemWidth, Gui::GetContentRegionAvail().y * 0.65f), /*false*/1, ImGuiWindowFlags_AlwaysVerticalScrollbar);
			Gui::PopStyleColor(1);
			{
				if (selectedMultiBinding == nullptr)
					selectedMultiBinding = namedMultiBindings[0].MultiBinding;

				Gui::PushStyleColor(ImGuiCol_Separator, Gui::GetColorU32(ImGuiCol_TextDisabled, 0.25f));
				Gui::BeginColumns(nullptr, 2, ImGuiOldColumnFlags_NoBorder | ImGuiOldColumnFlags_NoResize);
				Gui::SetColumnWidth(0, Gui::GetWindowWidth() * primaryColumWidthFactor);

				for (const auto& namedBinding : namedMultiBindings)
				{
					if (!bindingFilter.PassFilter(Gui::StringViewStart(namedBinding.Name), Gui::StringViewEnd(namedBinding.Name)))
						continue;

					Gui::PushID(namedBinding.MultiBinding);
					if (Gui::Selectable("##MultiBindingSelectable", selectedMultiBinding == namedBinding.MultiBinding, ImGuiSelectableFlags_SpanAllColumns))
						selectedMultiBinding = namedBinding.MultiBinding;

					Gui::SameLine();
					Gui::TextUnformatted(Gui::StringViewStart(namedBinding.Name), Gui::StringViewEnd(namedBinding.Name));
					Gui::NextColumn();
					Gui::PushItemDisabledAndTextColor();
					if (namedBinding.MultiBinding->BindingCount > 0)
					{
						combinedBindingShortcutBuffer += '(';
						for (size_t i = 0; i < namedBinding.MultiBinding->BindingCount; i++)
						{
							if (namedBinding.MultiBinding->Bindings[i].IsEmpty())
								combinedBindingShortcutBuffer += "None";
							else
								combinedBindingShortcutBuffer += ToString(namedBinding.MultiBinding->Bindings[i]).data();

							if (i + 1 < namedBinding.MultiBinding->BindingCount)
								combinedBindingShortcutBuffer += ", ";
						}

						constexpr i32 targetCharCount = 32;
						if (combinedBindingShortcutBuffer.size() > targetCharCount)
						{
							combinedBindingShortcutBuffer.erase(combinedBindingShortcutBuffer.begin() + (targetCharCount - 1), combinedBindingShortcutBuffer.end());
							combinedBindingShortcutBuffer += "...";
						}

						combinedBindingShortcutBuffer += ')';

						Gui::TextUnformatted(Gui::StringViewStart(combinedBindingShortcutBuffer), Gui::StringViewEnd(combinedBindingShortcutBuffer));
						combinedBindingShortcutBuffer.clear();
					}
					else
					{
						Gui::TextUnformatted("(None)");
					}
					Gui::PopItemDisabledAndTextColor();
					Gui::NextColumn();

					Gui::PopID();

					Gui::Separator();
				}
				Gui::EndColumns();
				Gui::PopStyleColor(1);
			}
			Gui::EndChild();

			Gui::SetCursorPosX(Gui::GetCursorPosX() + GuiSettingsInnerSubChildMargin);
			Gui::BeginChild("BindingEditHeaderChild", vec2(GuiSettingsSubChildItemWidth, Gui::GetFrameHeight() + 4.0f), true, ImGuiWindowFlags_None);
			{
				std::string_view selectedMultiBindingName = "Unnamed Binding";
				for (size_t i = 0; i < std::size(namedMultiBindings); i++)
				{
					if (namedMultiBindings[i].MultiBinding == selectedMultiBinding)
						selectedMultiBindingName = namedMultiBindings[i].Name;
				}

				Gui::SameLine();
				Gui::AlignTextToFramePadding();
				Gui::Text("%.*s:", static_cast<i32>(selectedMultiBindingName.size()), selectedMultiBindingName.data());

				if (selectedMultiBinding != nullptr)
				{
					const bool canAddNewBinding =
						(selectedMultiBinding->BindingCount < Input::MaxMultiBindingCount &&
						(selectedMultiBinding->BindingCount == 0 || !selectedMultiBinding->Bindings[selectedMultiBinding->BindingCount - 1].IsEmpty()));

					// HACK: Hardcoded offset to align with buttons below
					Gui::SameLine(Gui::GetContentRegionAvail().x * primaryColumWidthFactor - 5.0f);

					Gui::PushItemDisabledAndTextColorIf(!canAddNewBinding);
					if (Gui::Button("Add", Gui::GetContentRegionAvail()))
					{
						awaitInputBinding = &selectedMultiBinding->Bindings[selectedMultiBinding->BindingCount];
						awaitInputStopwatch.Restart();
						selectedMultiBinding->Bindings[selectedMultiBinding->BindingCount++] = Input::Binding();
						pendingChanges = true;
					}
					Gui::PopItemDisabledAndTextColorIf(!canAddNewBinding);
				}
			}
			Gui::EndChild();

			Gui::SetCursorPosX(Gui::GetCursorPosX() + GuiSettingsInnerSubChildMargin);
			Gui::BeginChild("BindingEditChild", vec2(GuiSettingsSubChildItemWidth, 0.0f), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
			if (selectedMultiBinding != nullptr)
			{
				Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(style.ItemSpacing.x, 2.0f));
				const f32 buttonHeight = Gui::GetFrameHeight();

				if (selectedMultiBinding->BindingCount == 0)
				{
					Gui::SameLine();
					Gui::AlignTextToFramePadding();
					Gui::TextDisabled("No Binding assigned to this action");
					Gui::Separator();
				}

				size_t bindingIndexToRemove = Input::MaxMultiBindingCount;
				size_t bindingIndexToMoveUp = Input::MaxMultiBindingCount;
				size_t bindingIndexToMoveDown = Input::MaxMultiBindingCount;

				for (size_t i = 0; i < selectedMultiBinding->BindingCount; i++)
				{
					auto& binding = selectedMultiBinding->Bindings[i];
					Gui::PushID(&binding);

					pendingChanges |= GuiSettingsInteractiveAwaitKeyPressInputBindingButton(binding, vec2(Gui::GetContentRegionAvail().x * primaryColumWidthFactor, buttonHeight), awaitInputBinding, awaitInputStopwatch);
					Gui::SameLine(0.0f, style.ItemInnerSpacing.x);

					const bool hasSingleBinding = (selectedMultiBinding->BindingCount <= 1);
					Gui::PushItemDisabledAndTextColorIf(hasSingleBinding);
					{
						const f32 upDownButtonWidth = (Gui::GetContentRegionAvail().x - style.ItemInnerSpacing.x * 3.0f) / 4.0f;
						if (Gui::Button("Up", vec2(upDownButtonWidth, buttonHeight))) bindingIndexToMoveUp = i;
						Gui::SameLine(0.0f, style.ItemInnerSpacing.x);

						if (Gui::Button("Down", vec2(upDownButtonWidth, buttonHeight))) bindingIndexToMoveDown = i;
						Gui::SameLine(0.0f, style.ItemInnerSpacing.x);
					}
					Gui::PopItemDisabledAndTextColorIf(hasSingleBinding);

					if (Gui::Button("Remove", vec2(Gui::GetContentRegionAvail().x, buttonHeight)))
						bindingIndexToRemove = i;
					Gui::NextColumn();

					Gui::PopID();
					Gui::Separator();
				}

				if (bindingIndexToRemove < selectedMultiBinding->BindingCount)
				{
					if (selectedMultiBinding->BindingCount > 1)
					{
						for (size_t i = bindingIndexToRemove; i < selectedMultiBinding->BindingCount - 1; i++)
							selectedMultiBinding->Bindings[i] = selectedMultiBinding->Bindings[i + 1];
					}

					selectedMultiBinding->BindingCount--;
					pendingChanges = true;
				}
				else if (bindingIndexToMoveUp < selectedMultiBinding->BindingCount && bindingIndexToMoveUp > 0)
				{
					std::swap(selectedMultiBinding->Bindings[bindingIndexToMoveUp], selectedMultiBinding->Bindings[bindingIndexToMoveUp - 1]);
					pendingChanges = true;
				}
				else if (bindingIndexToMoveDown + 1 < selectedMultiBinding->BindingCount)
				{
					std::swap(selectedMultiBinding->Bindings[bindingIndexToMoveDown], selectedMultiBinding->Bindings[bindingIndexToMoveDown + 1]);
					pendingChanges = true;
				}

				Gui::PopStyleVar(1);
			}
			Gui::EndChild();
		}
	}

	void ChartEditorSettingsWindow::GuiTabPlaytestBindings(ComfyStudioUserSettings& userData)
	{
		using namespace Input;
		if (Gui::CollapsingHeader("Playtest Bindings", ImGuiTreeNodeFlags_DefaultOpen))
		{
			size_t bindingIndexToRemove = std::numeric_limits<size_t>::max();

			Gui::SetCursorPosX(Gui::GetCursorPosX() + GuiSettingsInnerSubChildMargin);
			Gui::BeginChild("BindingsListChild", vec2(GuiSettingsSubChildItemWidth, Gui::GetContentRegionAvail().y), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
			for (size_t bindingIndex = 0; bindingIndex < userData.Input.PlaytestBindings.size(); bindingIndex++)
			{
				auto& binding = userData.Input.PlaytestBindings[bindingIndex];
				Gui::PushID(&binding);

				const f32 availWidthTotal = Gui::GetContentRegionAvail().x;
				const f32 itemInnerSpacingX = Gui::GetStyle().ItemInnerSpacing.x;

				constexpr u32 conflictTextColor = 0xFF2424E3;
				const bool bindingConflicts = (FindIfOrNull(userData.Input.PlaytestBindings, [&](auto& b) { return (&b != &binding) && (b.InputSource == binding.InputSource); }) != nullptr);
				const bool bindingIsDownAnyNotBeingEdited = (awaitInputBinding == nullptr) && IsDown(binding.InputSource);

				if (bindingConflicts) Gui::PushStyleColor(ImGuiCol_Text, conflictTextColor);
				if (bindingIsDownAnyNotBeingEdited) Gui::PushStyleColor(ImGuiCol_Button, Gui::GetStyleColorVec4(ImGuiCol_ButtonActive));
				if (GuiSettingsInteractiveAwaitKeyPressInputBindingButton(binding.InputSource, vec2((availWidthTotal - itemInnerSpacingX - itemInnerSpacingX) / 3.0f, 0.0f), awaitInputBinding, awaitInputStopwatch))
				{
					if (binding.InputSource.Type == BindingType::Keyboard)
						binding.InputSource.Keyboard.Modifiers = KeyModifiers_None;
					binding.SlidePosition = AutomaticallyDeterminePlayTestSlidePosition(binding.ButtonTypes, binding.InputSource);
					pendingChanges = true;
				}
				if (bindingIsDownAnyNotBeingEdited) Gui::PopStyleColor();
				if (bindingConflicts) Gui::PopStyleColor();

				Gui::SameLine(0.0f, itemInnerSpacingX);
				Gui::PushItemWidth((availWidthTotal - itemInnerSpacingX - itemInnerSpacingX) / 2.0f);

				static constexpr std::array<const char*, EnumCount<ButtonType>()> buttonNames = { "Triangle", "Square", "Cross", "Circle", "Slide Left", "Slide Right", };

				char comboPreview[64]; comboPreview[0] = '\0';
				if (binding.ButtonTypes != ButtonTypeFlags_None)
				{
					for (size_t i = 0; i < EnumCount<ButtonType>(); i++)
					{
						if (binding.ButtonTypes & ButtonTypeToButtonTypeFlags(static_cast<ButtonType>(i)))
						{
							strcat_s(comboPreview, buttonNames[i]);
							strcat_s(comboPreview, "+");
						}
					}

					const size_t length = strlen(comboPreview);
					if (comboPreview[length - 1] == '+')
						comboPreview[length - 1] = '\0';

					if (binding.ButtonTypes & ButtonTypeFlags_SlideAll)
						strcat_s(comboPreview, (binding.SlidePosition == PlayTestSlidePositionType::None) ? " (X)" : (binding.SlidePosition == PlayTestSlidePositionType::Left) ? " (L)" : " (R)");
				}
				else
				{
					strcpy_s(comboPreview, "(None)");
				}

				if (Gui::BeginCombo("##BindingButtonTypesCombo", comboPreview, ImGuiComboFlags_None))
				{
					for (size_t i = 0; i < EnumCount<ButtonType>(); i++)
					{
						const auto buttonType = static_cast<ButtonType>(i);
						const auto buttonTypeFlags = ButtonTypeToButtonTypeFlags(buttonType);
						const bool flagIsSet = (binding.ButtonTypes & buttonTypeFlags);

						if (buttonType == ButtonType::SlideL)
							Gui::Separator();

						if (Gui::MenuItemDontClosePopup(buttonNames[i], nullptr, flagIsSet, true))
						{
							if (!flagIsSet)
								binding.ButtonTypes |= buttonTypeFlags;
							else
								binding.ButtonTypes &= ~buttonTypeFlags;

							// NOTE: Secret shortcut for quickly selecting single buttons only
							if (Gui::GetIO().KeyShift)
								binding.ButtonTypes = buttonTypeFlags;

							// NOTE: Only allow selecting either a single slide or any number of non slide buttons
							if (IsSlideButtonType(buttonType))
								binding.ButtonTypes = flagIsSet ? binding.ButtonTypes : buttonTypeFlags;
							else
								binding.ButtonTypes &= ButtonTypeFlags_NormalAll;

							binding.SlidePosition = AutomaticallyDeterminePlayTestSlidePosition(binding.ButtonTypes, binding.InputSource);
							pendingChanges = true;
						}
					}

					Gui::EndCombo();
				}
				Gui::PopItemWidth();

				Gui::SameLine(0.0f, itemInnerSpacingX);
				if (Gui::Button("Remove", vec2(Gui::GetContentRegionAvail().x, 0.0f)))
					bindingIndexToRemove = bindingIndex;

				Gui::PopID();
				Gui::Separator();
			}

			if (Gui::Button("Add", vec2(Gui::GetContentRegionAvail().x, 0.0f)))
			{
				userData.Input.PlaytestBindings.emplace_back();
				pendingChanges = true;
			}
			Gui::EndChild();

			if (InBounds(bindingIndexToRemove, userData.Input.PlaytestBindings))
			{
				userData.Input.PlaytestBindings.erase(userData.Input.PlaytestBindings.begin() + bindingIndexToRemove);
				pendingChanges = true;
			}
		}
	}

	void ChartEditorSettingsWindow::GuiTabThemeDebug(ComfyStudioUserSettings& userData)
	{
		if (Gui::CollapsingHeader("Gui Style Editor", ImGuiTreeNodeFlags_DefaultOpen))
		{
			Gui::ShowStyleEditor();
		}
	}

	void ChartEditorSettingsWindow::GuiControllerLayoutTabBarAndContent(ComfyStudioUserSettings& userData)
	{
		using Input::NativeButton;
		using Input::Button;

		if (Gui::BeginTabBar("ControllerLayoutTabBar", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton | ImGuiTabBarFlags_FittingPolicyScroll))
		{
			const size_t connectedControllerCount = Input::GlobalSystemGetConnectedControllerCount();
			for (size_t controllerIndex = 0; controllerIndex < connectedControllerCount; controllerIndex++)
			{
				const auto controllerInfo = Input::GlobalSystemGetConnectedControllerInfoAt(controllerIndex);
				auto* correspondingLayoutMapping = FindIfOrNull(userData.Input.ControllerLayoutMappings, [&](auto& m) { return m.ProductID.GUID == controllerInfo.ProductID.GUID; });

				std::array<bool, EnumCount<NativeButton>()> nativeButtonsDown;
				bool anyNonAxisNativeButtonDown = false;
				for (i32 i = 0; i < static_cast<i32>(nativeButtonsDown.size()); i++)
				{
					nativeButtonsDown[i] = IsNativeButtonDown(controllerInfo.InstanceID, static_cast<NativeButton>(i));

					if (nativeButtonsDown[i] && !(i >= static_cast<i32>(NativeButton::FirstAxis) && i <= static_cast<i32>(NativeButton::LastAxis)))
						anyNonAxisNativeButtonDown = true;
				}

				Gui::PushID(static_cast<i32>(controllerIndex));
				Gui::PushStyleColor(ImGuiCol_Text, Gui::GetStyleColorVec4(anyNonAxisNativeButtonDown ? ImGuiCol_Text : ImGuiCol_TextDisabled));
				const bool beginTabItem = Gui::BeginTabItem(controllerInfo.InstanceName.data());
				Gui::PopStyleColor();
				if (beginTabItem)
				{
					if (correspondingLayoutMapping != nullptr)
					{
						Gui::BeginChild("InnerLayoutChild", vec2(0.0f, 0.0f), false, ImGuiWindowFlags_NoBackground);
						GuiControllerLayoutTabItemInnerContent(userData, controllerInfo, *correspondingLayoutMapping, nativeButtonsDown.data());
						Gui::EndChild();
					}
					else
					{
						Gui::BeginChild("InnerLayoutChild", vec2(0.0f, 0.0f), true, ImGuiWindowFlags_None);
						Gui::PushStyleColor(ImGuiCol_Text, Gui::GetColorU32(ImGuiCol_Text, 0.85f));
						Gui::PushStyleColor(ImGuiCol_Button, Gui::GetColorU32(ImGuiCol_Button, 0.25f));
						Gui::PushStyleColor(ImGuiCol_ButtonHovered, Gui::GetColorU32(ImGuiCol_ButtonHovered, 0.25f));
						Gui::PushStyleColor(ImGuiCol_ButtonActive, Gui::GetColorU32(ImGuiCol_ButtonActive, 0.25f));
						if (Gui::ButtonEx("Click to Define a Layout for this Device Type...", Gui::GetContentRegionAvail(), ImGuiButtonFlags_None))
						{
							auto& newMapping = userData.Input.ControllerLayoutMappings.emplace_back();
							newMapping.ProductID = controllerInfo.ProductID;
							newMapping.Name = controllerInfo.ProductName;
							pendingChanges = true;
						}
						Gui::PopStyleColor(4);
						Gui::EndChild();
					}
					Gui::EndTabItem();
				}
				Gui::PopID();
			}
		}
		Gui::EndTabBar();
	}

	void ChartEditorSettingsWindow::GuiControllerLayoutTabItemInnerContent(ComfyStudioUserSettings& userData, const Input::ControllerInfoView& controllerInfo, Input::StandardControllerLayoutMapping& correspondingLayoutMapping, const bool nativeButtonsDown[])
	{
		// TODO: Configure analog axes... ~~auto assign NativeAxis based on axis button bindings..?~~ thought that doesn't really work for triggers
		// TODO: Eventually implement setup wizzard step by step prompting to press a specified button

		using namespace Input;
		const auto& style = Gui::GetStyle();
		const f32 buttonHeight = 24.0f;

		char labelBuffer[128];
		NativeButton userClickedNativeButton = NativeButton::Count;

		auto assignClickableButton = [&](const char* label, NativeButton nativeButton, Button assignedStandardButton, f32 width)
		{
			assert(nativeButton < NativeButton::Count);
			const bool isNativeButtonDown = nativeButtonsDown[static_cast<i32>(nativeButton)];

			Gui::PushID(static_cast<i32>(nativeButton));

			Gui::PushStyleColor(ImGuiCol_Text, Gui::GetStyleColorVec4(isNativeButtonDown ? ImGuiCol_Text : ImGuiCol_TextDisabled));
			Gui::PushStyleColor(ImGuiCol_Button, Gui::GetStyleColorVec4(
				assignedStandardButton != Button::None ? ImGuiCol_ButtonActive :
				isNativeButtonDown ? ImGuiCol_ButtonHovered : ImGuiCol_Button));

			if (Gui::Button(label, vec2(Gui::GetContentRegionAvail().x, buttonHeight)))
				userClickedNativeButton = nativeButton;

			Gui::PopStyleColor(2);

			Gui::PopID();
		};

		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(0.0f, style.ItemSpacing.y));
		const i32 clampedButtonCount = Clamp(controllerInfo.ButtonCount, 0, static_cast<i32>(NativeButton::ButtonCount));
		const i32 clampedDPadCount = Clamp(controllerInfo.DPadCount, 0, static_cast<i32>(NativeButton::DPadCount));
		const i32 clampedAxisCount = Clamp(controllerInfo.AxisCount, 0, static_cast<i32>(NativeButton::AxisCount));
		{
			if (clampedDPadCount > 0)
			{
				for (i32 dpadIndex = 0; dpadIndex < clampedDPadCount; dpadIndex++)
				{
					const NativeButton nativeButtonUp = static_cast<NativeButton>(static_cast<i32>(NativeButton::FirstDPad) + (dpadIndex * static_cast<i32>(NativeButton::PerDPadSubElements)) + 0);
					const NativeButton nativeButtonLeft = static_cast<NativeButton>(static_cast<i32>(NativeButton::FirstDPad) + (dpadIndex * static_cast<i32>(NativeButton::PerDPadSubElements)) + 1);
					const NativeButton nativeButtonDown = static_cast<NativeButton>(static_cast<i32>(NativeButton::FirstDPad) + (dpadIndex * static_cast<i32>(NativeButton::PerDPadSubElements)) + 2);
					const NativeButton nativeButtonRight = static_cast<NativeButton>(static_cast<i32>(NativeButton::FirstDPad) + (dpadIndex * static_cast<i32>(NativeButton::PerDPadSubElements)) + 3);

					Gui::PushID(dpadIndex);
					Gui::BeginChild("NativeButtonDPadChild", vec2(0.0f, 3.0f * (buttonHeight + style.ItemSpacing.y) + style.ChildBorderSize), true, ImGuiWindowFlags_NoScrollWithMouse);
					Gui::BeginColumns(nullptr, 3, ImGuiOldColumnFlags_NoBorder);
					{
						auto dpadDirectionButton = [&](const char* /*directionLabel*/, NativeButton nativeButton)
						{
							const Button assignedStandardButton = FindStandardButtonForNativeButton(correspondingLayoutMapping, nativeButton);
							sprintf_s(labelBuffer, "[%s]", GetButtonName(assignedStandardButton));
							assignClickableButton(labelBuffer, nativeButton, assignedStandardButton, Gui::GetContentRegionAvail().x);
						};

						// ... Up-Left
						Gui::NextColumn();
						dpadDirectionButton("Up", nativeButtonUp);
						Gui::NextColumn();
						// ... Up-Right
						Gui::NextColumn();
						dpadDirectionButton("Left", nativeButtonLeft);
						Gui::NextColumn();
						{
							Gui::PushStyleColor(ImGuiCol_Button, 0x00000000);
							sprintf_s(labelBuffer, "DPad %d", dpadIndex + 1);
							Gui::ButtonEx(labelBuffer, vec2(Gui::GetContentRegionAvail().x, 0.0f), ImGuiButtonFlags_Disabled);
							Gui::PopStyleColor(1);
						}
						Gui::NextColumn();
						dpadDirectionButton("Right", nativeButtonRight);
						Gui::NextColumn();
						// ... Down-Left
						Gui::NextColumn();
						dpadDirectionButton("Down", nativeButtonDown);
						Gui::NextColumn();
						// ... Down-Right
						Gui::NextColumn();
					}
					Gui::EndColumns();
					Gui::EndChild();
					Gui::PopID();
				}
			}

			if (clampedButtonCount > 0)
			{
				Gui::BeginChild("NativeButtonButtonsChild", vec2(0.0f, (clampedButtonCount / 2) * (buttonHeight + style.ItemSpacing.y) + style.ChildBorderSize), true, ImGuiWindowFlags_NoScrollWithMouse);
				Gui::BeginColumns(nullptr, 2, ImGuiOldColumnFlags_NoBorder);
				for (i32 buttonIndex = 0; buttonIndex < clampedButtonCount; buttonIndex++)
				{
					const NativeButton nativeButton = static_cast<NativeButton>(static_cast<i32>(NativeButton::FirstButton) + buttonIndex);
					const Button assignedStandardButton = FindStandardButtonForNativeButton(correspondingLayoutMapping, nativeButton);

					sprintf_s(labelBuffer, "Button %d  [%s]", buttonIndex + 1, GetButtonName(assignedStandardButton));
					assignClickableButton(labelBuffer, nativeButton, assignedStandardButton, Gui::GetContentRegionAvail().x);

					Gui::NextColumn();
				}
				Gui::EndColumns();
				Gui::EndChild();
			}

			if (clampedAxisCount > 0)
			{
				Gui::BeginChild("NativeButtonAxesChild", vec2(0.0f, clampedAxisCount * (buttonHeight + style.ItemSpacing.y) + style.ChildBorderSize), true, ImGuiWindowFlags_NoScrollWithMouse);
				Gui::BeginColumns(nullptr, static_cast<i32>(NativeButton::PerAxisSubElements), ImGuiOldColumnFlags_NoBorder);
				for (i32 relativeAxes = 0; relativeAxes < clampedAxisCount; relativeAxes++)
				{
					const NativeButton nativeButtonNegative = static_cast<NativeButton>(static_cast<i32>(NativeButton::FirstAxis) + (relativeAxes * static_cast<i32>(NativeButton::PerAxisSubElements)) + 0);
					const NativeButton nativeButtonPositive = static_cast<NativeButton>(static_cast<i32>(NativeButton::FirstAxis) + (relativeAxes * static_cast<i32>(NativeButton::PerAxisSubElements)) + 1);

					auto triggerButton = [&](const char* label, NativeButton nativeButton)
					{
						const Button assignedStandardButton = FindStandardButtonForNativeButton(correspondingLayoutMapping, nativeButton);
						sprintf_s(labelBuffer, "Axis %d %s  [%s]", relativeAxes + 1, label, GetButtonName(assignedStandardButton));
						assignClickableButton(labelBuffer, nativeButton, assignedStandardButton, Gui::GetContentRegionAvail().x);
						Gui::NextColumn();
					};

					triggerButton("-", nativeButtonNegative);
					triggerButton("+", nativeButtonPositive);
				}
				Gui::EndColumns();
				Gui::EndChild();
			}
		}
		Gui::PopStyleVar();

		char buttonPickerPopupID[128];
		sprintf_s(buttonPickerPopupID, "Assign Native Button %d###ButtonPickerPopup", static_cast<i32>(currentButtonPickerPopupNativeButton));

		if (userClickedNativeButton < NativeButton::Count)
		{
			currentButtonPickerPopupNativeButton = userClickedNativeButton;
			Gui::OpenPopup(buttonPickerPopupID);
		}

		const auto* viewport = Gui::GetMainViewport();
		Gui::SetNextWindowPos(viewport->Pos + (viewport->Size / 2.0f), ImGuiCond_Appearing, vec2(0.5f));

		bool isPopupOpen = true;
		if (Gui::WideBeginPopupModal(buttonPickerPopupID, &isPopupOpen, ImGuiWindowFlags_AlwaysAutoResize))
		{
			// HACK: Prevent this binding from closing both this and the outter popup
			thisFrameAnyItemActive = true;
			if (IsAnyPressed(GlobalUserData.Input.App_Dialog_Cancel, false))
				Gui::CloseCurrentPopup();

			GuiButtonPickerPopupContent(userData, correspondingLayoutMapping);
			Gui::EndPopup();
		}
	}

	void ChartEditorSettingsWindow::GuiButtonPickerPopupContent(ComfyStudioUserSettings& userData, Input::StandardControllerLayoutMapping& layoutMapping)
	{
		using namespace Input;
		static constexpr std::array<Button, 5> dPadUpLeftDownRight = { Button::DPadUp, Button::DPadLeft, Button::DPadDown, Button::DPadRight, Button::None };
		static constexpr std::array<Button, 5> faceUpLeftDownRight = { Button::FaceUp, Button::FaceLeft, Button::FaceDown, Button::FaceRight, Button::None };
		static constexpr std::array<Button, 5> leftStickUpLeftDownRight = { Button::LeftStickUp, Button::LeftStickLeft, Button::LeftStickDown, Button::LeftStickRight, Button::LeftStickClick };
		static constexpr std::array<Button, 5> rightStickUpLeftDownRight = { Button::RightStickUp, Button::RightStickLeft, Button::RightStickDown, Button::RightStickRight, Button::RightStickClick };

		constexpr f32 margin = 8.0f;
		constexpr f32 spacing = 4.0f;
		constexpr vec2 buttonSize = vec2(100.0f, 26.0f);
		constexpr vec2 buttonSizeTouchPad = vec2(260.0f, 72.0f);

		const Button assignedPopupStandardButton = FindStandardButtonForNativeButton(layoutMapping, currentButtonPickerPopupNativeButton);
		auto standardControllerButton = [&](Button standardButton, vec2 position, vec2 size)
		{
			Gui::SetCursorPos(position);
			Gui::PushID(static_cast<i32>(standardButton));

			const bool isAlreadyBound = (layoutMapping.StandardToNativeButtons[static_cast<i32>(standardButton)] != NativeButton::None);
			if (isAlreadyBound) Gui::PushStyleColor(ImGuiCol_Button, Gui::GetStyleColorVec4(ImGuiCol_ButtonActive));

			if (Gui::Button(GetButtonName(standardButton), size))
			{
				layoutMapping.StandardToNativeButtons[static_cast<i32>(assignedPopupStandardButton)] = NativeButton::None;
				layoutMapping.StandardToNativeButtons[static_cast<i32>(standardButton)] = currentButtonPickerPopupNativeButton;
				pendingChanges = true;
				Gui::CloseCurrentPopup();
			}

			if (isAlreadyBound) Gui::PopStyleColor(1);
			Gui::PopID();
		};

		auto standardControllerDirectionalButtons = [&](const std::array<Button, 5>& upLeftDownRightClick, vec2 position, vec2 perButtonSize)
		{
			const f32 centerOffset = (upLeftDownRightClick[4] == Button::None) ? (perButtonSize.x * 0.35f) : 0.0f;

			standardControllerButton(upLeftDownRightClick[0], position + vec2(perButtonSize.x + spacing, 0.0f), perButtonSize);

			standardControllerButton(upLeftDownRightClick[1], position + vec2(0.0f + centerOffset, perButtonSize.y + spacing), perButtonSize);
			standardControllerButton(upLeftDownRightClick[3], position + vec2((perButtonSize.x + spacing) * 2.0f - centerOffset, perButtonSize.y + spacing), perButtonSize);

			standardControllerButton(upLeftDownRightClick[2], position + vec2(perButtonSize.x + spacing, (perButtonSize.y + spacing) * 2.0f), perButtonSize);

			if (upLeftDownRightClick[4] != Button::None)
				standardControllerButton(upLeftDownRightClick[4], position + vec2(perButtonSize.x + spacing, perButtonSize.y + spacing), perButtonSize);
		};

		Gui::BeginChild("ButtonPickerBaseChild", vec2(680.0f, 408.0f), true, ImGuiWindowFlags_None);
		{
			const vec2 windowSize = Gui::GetWindowSize();
			const vec2 windowCenter = (windowSize * 0.5f);

			// TODO: Clean up, resign, analog axis asignment and add hints for which buttons are already bound
			standardControllerButton(Button::LeftTrigger, vec2(margin, margin + spacing), buttonSize);
			standardControllerButton(Button::LeftBumper, vec2(margin + (buttonSize.x * 0.35f) + spacing, margin + buttonSize.y + spacing + spacing), buttonSize);
			standardControllerButton(Button::RightTrigger, vec2(windowSize.x - margin - buttonSize.x, margin + spacing), buttonSize);
			standardControllerButton(Button::RightBumper, vec2(windowSize.x - margin - buttonSize.x - ((buttonSize.x * 0.35f) + spacing), margin + buttonSize.y + spacing + spacing), buttonSize);
			standardControllerButton(Button::TouchPad, vec2(windowCenter.x - (buttonSizeTouchPad.x / 2.0f), margin), buttonSizeTouchPad);
			standardControllerButton(Button::Home, vec2(windowCenter.x - (buttonSize.x / 2.0f), margin + buttonSizeTouchPad.y + spacing + (buttonSize.y * 0.5f) + spacing + spacing), buttonSize);
			standardControllerButton(Button::Select, vec2(windowCenter.x - (buttonSize.x / 2.0f) - (buttonSize.x + spacing), margin + buttonSizeTouchPad.y + spacing + spacing), buttonSize);
			standardControllerButton(Button::Start, vec2(windowCenter.x - (buttonSize.x / 2.0f) + (buttonSize.x + spacing), margin + buttonSizeTouchPad.y + spacing + spacing), buttonSize);
			standardControllerDirectionalButtons(dPadUpLeftDownRight, vec2(margin, 140.0f), buttonSize);
			standardControllerDirectionalButtons(faceUpLeftDownRight, vec2(windowSize.x - (spacing + (buttonSize.x + spacing) * 3.0f), 146.0f), buttonSize);
			standardControllerDirectionalButtons(leftStickUpLeftDownRight, vec2(margin, 260.0f), buttonSize);
			standardControllerDirectionalButtons(rightStickUpLeftDownRight, vec2(windowSize.x - (spacing + (buttonSize.x + spacing) * 3.0f), 260.0f), buttonSize);

			Gui::SetCursorPos(vec2(Gui::GetWindowWidth() * 0.25f, (Gui::GetWindowHeight() - buttonSize.y) - (Gui::GetStyle().WindowPadding.y + margin)));
			if (Gui::Button("(None)", vec2(Gui::GetWindowWidth() * 0.5f, buttonSize.y)))
			{
				layoutMapping.StandardToNativeButtons[static_cast<i32>(assignedPopupStandardButton)] = NativeButton::None;
				pendingChanges = true;
				Gui::CloseCurrentPopup();
			}
		}
		Gui::EndChild();
	}
}
