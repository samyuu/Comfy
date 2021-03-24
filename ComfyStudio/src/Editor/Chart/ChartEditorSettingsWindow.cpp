#include "ChartEditorSettingsWindow.h"
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
		constexpr f32 GuiSettingsLabelSpacing = 4.0f;
		constexpr f32 GuiSettingsCheckboxLabelSpacing = 6.0f;

		void GuiBeginSettingsColumns()
		{
			Gui::BeginColumns(nullptr, 2, ImGuiColumnsFlags_NoBorder | ImGuiColumnsFlags_NoResize);
		}

		void GuiEndSettingsColumns()
		{
			Gui::EndColumns();
		}

		void GuiSettingsRightAlignedLabel(std::string_view label)
		{
			const vec2 textSize = Gui::CalcTextSize(Gui::StringViewStart(label), Gui::StringViewEnd(label), true);
			const vec2 cursorPosition = Gui::GetCursorScreenPos();
			Gui::SetCursorScreenPos(vec2(cursorPosition.x + (Gui::GetContentRegionAvailWidth() - textSize.x) - GuiSettingsLabelSpacing, cursorPosition.y));

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

		bool GuiSettingsSliderF32(std::string_view label, f32& inOutValue, f32 minValue, f32 maxValue, const char* format = "%.3f")
		{
			GuiSettingsRightAlignedLabel(label);
			Gui::NextColumn();

			Gui::PushID(Gui::StringViewStart(label), Gui::StringViewEnd(label));
			Gui::PushItemWidth(GuiSettingsItemWidth);
			const bool result = Gui::SliderFloat("##SettingsSlider", &inOutValue, minValue, maxValue, format);
			Gui::PopItemWidth();
			Gui::PopID();
			Gui::NextColumn();

			return result;
		}

		bool GuiSettingsVolumeSlider(std::string_view label, f32& inOutValue, bool extendedRange = false)
		{
			GuiSettingsRightAlignedLabel(label);
			Gui::NextColumn();

			Gui::PushID(Gui::StringViewStart(label), Gui::StringViewEnd(label));
			Gui::PushItemWidth(GuiSettingsItemWidth);

			bool result = false;
#if 0
			if (f32 v = inOutValue * 100.0f; result = Gui::SliderFloat("##SettingsVolumeSlider", &v, 0.0f, extendedRange ? 125.0f : 100.0f, "%.0f%%"))
				inOutValue = v / 100.0f;
#elif 0
			constexpr f32 minDB = -60.0f, maxDB = 0.0f;
			if (f32 dB = Audio::LinearVolumeToDecibel(inOutValue); result = Gui::SliderFloat("##SettingsVolumeSlider", &dB, minDB, maxDB, "%.2f dB"))
				inOutValue = (dB <= minDB) ? 0.0f : Audio::DecibelToLinearVolume(dB);
#else
			if (f32 s = Audio::LinearVolumeToSquare(inOutValue) * 100.0f; result = Gui::SliderFloat("##SettingsVolumeSlider", &s, 0.0f, extendedRange ? 115.0f : 100.0f, "%.0f%%"))
				inOutValue = Audio::SquareToLinearVolume(s / 100.0f);
#endif

			Gui::PopItemWidth();
			Gui::PopID();
			Gui::NextColumn();

			return result;
		}

		bool GuiSettingsInputI32(std::string_view label, i32& inOutValue, i32 step = 1, i32 stepFast = 100, ImGuiInputTextFlags flags = 0, const char* format = nullptr)
		{
			GuiSettingsRightAlignedLabel(label);
			Gui::NextColumn();

			Gui::PushID(Gui::StringViewStart(label), Gui::StringViewEnd(label));
			Gui::PushItemWidth(GuiSettingsItemWidth);
			const bool result = Gui::InputScalar("##SettingsInput", ImGuiDataType_S32, &inOutValue,
				(step > 0 ? &step : nullptr),
				(stepFast > 0 ? &stepFast : nullptr),
				(format != nullptr) ? format : (flags & ImGuiInputTextFlags_CharsHexadecimal) ? "%08X" : "%d",
				flags);
			Gui::PopItemWidth();
			Gui::PopID();
			Gui::NextColumn();

			return result;
		}

		bool GuiSettingsInputF32(std::string_view label, f32& inOutValue, f32 step = 0.0f, f32 stepFast = 0.0f, ImGuiInputTextFlags flags = 0, const char* format = "%.2f")
		{
			GuiSettingsRightAlignedLabel(label);
			Gui::NextColumn();

			Gui::PushID(Gui::StringViewStart(label), Gui::StringViewEnd(label));
			Gui::PushItemWidth(GuiSettingsItemWidth);
			const bool result = Gui::InputScalar("##SettingsInput", ImGuiDataType_Float, &inOutValue,
				(step > 0 ? &step : nullptr),
				(stepFast > 0 ? &stepFast : nullptr),
				format,
				flags);
			Gui::PopItemWidth();
			Gui::PopID();
			Gui::NextColumn();

			return result;
		}

		bool GuiSettingsInputVec3(std::string_view label, vec3& inOutValue, ImGuiInputTextFlags flags = 0, const char* format = "%.2f")
		{
			GuiSettingsRightAlignedLabel(label);
			Gui::NextColumn();

			Gui::PushID(Gui::StringViewStart(label), Gui::StringViewEnd(label));
			Gui::PushItemWidth(GuiSettingsItemWidth);
			const bool result = Gui::InputScalarN("##SettingsInput", ImGuiDataType_Float, glm::value_ptr(inOutValue), 3, nullptr, nullptr, format, flags);
			Gui::PopItemWidth();
			Gui::PopID();
			Gui::NextColumn();

			return result;
		}

		bool GuiSettingsInputText(std::string_view label, std::string& inOutValue, const char* hintText = nullptr, ImGuiInputTextFlags flags = ImGuiInputTextFlags_None)
		{
			GuiSettingsRightAlignedLabel(label);
			Gui::NextColumn();

			Gui::PushID(Gui::StringViewStart(label), Gui::StringViewEnd(label));
			Gui::PushItemWidth(GuiSettingsItemWidth);
			const bool result = (hintText != nullptr) ? Gui::InputTextWithHint("##SettingsText", hintText, &inOutValue, flags) : Gui::InputText("##SettingsText", &inOutValue, flags);
			Gui::PopItemWidth();
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
	}

	namespace
	{
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
	}

	void ChartEditorSettingsWindow::Gui()
	{
		lastFrameAnyItemActive = thisFrameAnyItemActive;
		thisFrameAnyItemActive = Gui::IsAnyItemActive();

		constexpr vec2 explicitMargin = vec2(6.0f);
		constexpr f32 windowHeight = 540.0f;
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
				if (Gui::Selectable(namedTabs[i].Name, (i == selectedTabIndex)))
					selectedTabIndex = i;
			}
			Gui::PopStyleVar(1);
		}
		Gui::EndChild();

		Gui::SameLine(0.0f, 4.0f);

		Gui::BeginChild("TabContentChild", vec2(tabContentWidth, windowHeight), false, ImGuiWindowFlags_AlwaysVerticalScrollbar);
		{
			Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(style.ItemSpacing.x, 3.0f));
			if (selectedTabIndex < std::size(namedTabs))
				(this->*namedTabs[selectedTabIndex].GuiFunction)(GlobalUserData.Mutable());
			Gui::PopStyleVar(1);
		}
		Gui::EndChild();
		Gui::SameLine(0.0f, 4.0f);

		Gui::BeginChild("TabControlChild", vec2(Gui::GetContentRegionAvailWidth() - 4.0f, windowHeight), false, ImGuiWindowFlags_None);
		{
			if (Gui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !thisFrameAnyItemActive && !lastFrameAnyItemActive)
			{
				if (Input::IsAnyPressed(GlobalUserData.Input.App_Dialog_YesOrOk, false))
					RequestWindowCloseAndKeepChanges();
				else if (Input::IsAnyPressed(GlobalUserData.Input.App_Dialog_Cancel, false))
					RequestWindowCloseAndRevertChanges();

				if (Input::IsAnyPressed(GlobalUserData.Input.App_Dialog_SelectNextTab, true))
					SelectNextTab(+1);
				if (Input::IsAnyPressed(GlobalUserData.Input.App_Dialog_SelectPreviousTab, true))
					SelectNextTab(-1);
			}

			Gui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);
			Gui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
			Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(style.ItemSpacing.x, 3.0f));
			Gui::PushStyleVar(ImGuiStyleVar_FramePadding, vec2(style.FramePadding.x, 4.0f));
			Gui::PushStyleColor(ImGuiCol_Border, Gui::GetStyleColorVec4(ImGuiCol_SliderGrab));
			Gui::PushStyleColor(ImGuiCol_BorderShadow, vec4(0.0f));
			{
				const f32 buttonWidth = Gui::GetContentRegionAvailWidth();
				if (Gui::Button("OK", vec2(buttonWidth, 0.0f)))
					RequestWindowCloseAndKeepChanges();
				if (Gui::Button("Cancel", vec2(buttonWidth, 0.0f)))
					RequestWindowCloseAndRevertChanges();
				if (Gui::Button("Apply", vec2(buttonWidth, 0.0f)))
					KeepChangesSaveToFile();
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
	}

	void ChartEditorSettingsWindow::RestoreUserDataCopy()
	{
		GlobalUserData.Mutable() = userDataPreEditCopy;
	}

	void ChartEditorSettingsWindow::KeepChangesSaveToFile()
	{
		userDataPreEditCopy = GlobalUserData;
		GlobalUserData.SaveToFile();

		auto& audioEngine = Audio::AudioEngine().GetInstance();
		const bool wasStreamRunnig = audioEngine.GetIsStreamOpenRunning();

		audioEngine.SetAudioBackend(GlobalUserData.System.Audio.RequestExclusiveDeviceAccess ? Audio::AudioBackend::WASAPIExclusive : Audio::AudioBackend::WASAPIShared);

		if (wasStreamRunnig)
			audioEngine.EnsureStreamRunning();
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
			GuiSettingsInputText("Chart Creator Default Name", userData.ChartProperties.ChartCreatorDefaultName, "n/a");
			GuiEndSettingsColumns();
		}

		if (Gui::CollapsingHeader("Target Preview", ImGuiTreeNodeFlags_DefaultOpen))
		{
			GuiBeginSettingsColumns();

			GuiSettingsCheckbox("Show Target Buttons", userData.TargetPreview.ShowButtons);
			GuiSettingsCheckbox("Preview Hold Info", userData.TargetPreview.ShowHoldInfo);
			if (auto v = userData.TargetPreview.PostHitLingerDuration.Ticks(); GuiSettingsInputI32("Post Hit Linger Duration", v, BeatTick::TicksPerBeat / 4, BeatTick::TicksPerBeat, ImGuiInputTextFlags_None, "%d Ticks"))
				userData.TargetPreview.PostHitLingerDuration = BeatTick::FromTicks(v);

			GuiSettingsCheckbox("Display Practice Background", userData.TargetPreview.DisplayPracticeBackground);

			Gui::PushItemDisabledAndTextColorIf(userData.TargetPreview.DisplayPracticeBackground);
			GuiSettingsCheckbox("Show Target Grid", userData.TargetPreview.ShowGrid);
			GuiSettingsCheckbox("Show Background Checkerboard", userData.TargetPreview.ShowBackgroundCheckerboard);
			if (auto v = userData.TargetPreview.BackgroundDim * 100.0f; GuiSettingsSliderF32("Background Dim", v, 0.0f, 100.0f, "%.f%%"))
				userData.TargetPreview.BackgroundDim = (v / 100.0f);
			Gui::PopItemDisabledAndTextColorIf(userData.TargetPreview.DisplayPracticeBackground);

			GuiEndSettingsColumns();
		}

		if (Gui::CollapsingHeader("Position Tool", ImGuiTreeNodeFlags_DefaultOpen))
		{
			GuiBeginSettingsColumns();
			GuiSettingsInputF32("Mouse Row Movement Distance Threshold", userData.PositionTool.MouseRowMovementDistanceThreshold);
			GuiEndSettingsColumns();
		}

		if (Gui::CollapsingHeader("Path Tool", ImGuiTreeNodeFlags_DefaultOpen))
		{
			GuiBeginSettingsColumns();

			if (auto v = vec3(userData.PathTool.AngleMouseSnap, userData.PathTool.AngleMouseSnapRough, userData.PathTool.AngleMouseSnapPrecise);
				GuiSettingsInputVec3("Angle Snap (Normal, Rough, Precise)", v, ImGuiInputTextFlags_None, "%.2f" DEGREE_SIGN))
			{
				userData.PathTool.AngleMouseSnap = v[0];
				userData.PathTool.AngleMouseSnapRough = v[1];
				userData.PathTool.AngleMouseSnapPrecise = v[2];
			}

			if (auto v = vec3(userData.PathTool.AngleMouseScrollStep, userData.PathTool.AngleMouseScrollRough, userData.PathTool.AngleMouseScrollPrecise);
				GuiSettingsInputVec3("Angle Scroll Step (Normal, Rough, Precise)", v, ImGuiInputTextFlags_None, "%.2f" DEGREE_SIGN))
			{
				userData.PathTool.AngleMouseScrollStep = v[0];
				userData.PathTool.AngleMouseScrollRough = v[1];
				userData.PathTool.AngleMouseScrollPrecise = v[2];
			}

			if (auto v = (userData.PathTool.AngleMouseScrollDirection > 0.0f); GuiSettingsCheckbox("Flip Mouse Scroll Direction", v))
			{
				userData.PathTool.AngleMouseScrollDirection = (v ? +1.0f : -1.0f);
			}

			GuiEndSettingsColumns();
		}

		if (Gui::CollapsingHeader("Target Inspector", ImGuiTreeNodeFlags_DefaultOpen))
		{
			auto guiDropdownVector = [this](auto& inOutVector)
			{
				i32& selectedItemIndex = inspectorDropdownItemIndices[selectedInspectorDropdownPropertyType];
				bool& scrollToBottomOnNextFrame = inspectorDropdownScrollToBottomOnNextFrames[selectedInspectorDropdownPropertyType];

				Gui::PushID(&inOutVector);
				constexpr bool isInt = std::is_same_v<std::remove_reference_t<decltype(inOutVector)>::value_type, i32>;

				if (!InBounds(selectedItemIndex, inOutVector))
					selectedItemIndex = 0;

				Gui::SetCursorPosX(Gui::GetCursorPosX() + GuiSettingsInnerMargin);
				const vec2 dropdownListChildSize = vec2(Gui::GetContentRegionAvailWidth(), Gui::GetFrameHeight() * 5.0f);
				Gui::BeginChild("DropdownListChild", dropdownListChildSize, false, ImGuiWindowFlags_NoScrollWithMouse);
				Gui::PushItemWidth(Gui::GetContentRegionAvailWidth());
				const bool listBoxOpen = Gui::ListBoxHeader("##DropdownValueList", vec2(Gui::GetContentRegionAvail()) - vec2(1.0f, 0.0f));
				if (listBoxOpen)
				{
					for (i32 i = 0; i < static_cast<i32>(inOutVector.size()); i++)
					{
						auto& item = inOutVector[i];

						char labelBuffer[128];
						sprintf_s(labelBuffer, isInt ? "%d) %d" : "%d) %.2f", i, item);

						Gui::PushID(&item);
						if (Gui::Selectable(labelBuffer, (i == selectedItemIndex)))
							selectedItemIndex = i;
						Gui::PopID();
					}

					if (scrollToBottomOnNextFrame)
					{
						Gui::SetScrollHereY(0.0f);
						scrollToBottomOnNextFrame = false;
					}

					Gui::ListBoxFooter();
				}
				Gui::PopItemWidth();
				Gui::EndChild();

				Gui::NextColumn();
				Gui::BeginChild("DropdownEditChild", vec2(Gui::GetContentRegionAvailWidth() - GuiSettingsInnerMargin, dropdownListChildSize.y), false, ImGuiWindowFlags_NoScrollWithMouse);

				Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(2.0f));
				bool moveItemUp = false, moveItemDown = false, addItem = false, removeItem = false;
				const f32 halfButtonWidth = (Gui::GetContentRegionAvailWidth() - Gui::GetStyle().ItemSpacing.x) / 2.0f;
				moveItemUp = Gui::Button("Move Up", vec2(halfButtonWidth, 0.0f));
				Gui::SameLine();
				moveItemDown = Gui::Button("Move Down", vec2(halfButtonWidth, 0.0f));

				Gui::PushItemWidth(Gui::GetContentRegionAvailWidth());
				auto* selectedValue = IndexOrNull(selectedItemIndex, inOutVector);
				Gui::PushItemDisabledAndTextColorIf(selectedValue == nullptr);
				if constexpr (isInt)
				{
					i32 dummyValue = 0;
					Gui::InputInt("##ValueI32", (selectedValue != nullptr) ? selectedValue : &dummyValue, 1, 5);
				}
				else
				{
					f32 dummyValue = 0.0f;
					Gui::InputFloat("##ValueF32", (selectedValue != nullptr) ? selectedValue : &dummyValue, 25.0f, 250.0f, "%.2f");
				}
				Gui::PopItemDisabledAndTextColorIf(selectedValue == nullptr);
				Gui::PopItemWidth();

				addItem = Gui::Button("Add", vec2(halfButtonWidth, 0.0f));
				Gui::SameLine();
				removeItem = Gui::Button("Remove ", vec2(halfButtonWidth, 0.0f));
				Gui::PopStyleVar(1);

				const bool indexInBounds = InBounds(selectedItemIndex, inOutVector);
				if ((moveItemUp || moveItemDown) && indexInBounds)
				{
					const auto valueToMove = inOutVector[selectedItemIndex];
					if (moveItemUp && selectedItemIndex > 0)
					{
						inOutVector.erase(inOutVector.begin() + selectedItemIndex);
						inOutVector.emplace(inOutVector.begin() + selectedItemIndex - 1, valueToMove);
						selectedItemIndex--;
					}
					else if (moveItemDown && (selectedItemIndex + 1) < static_cast<i32>(inOutVector.size()))
					{
						inOutVector.erase(inOutVector.begin() + selectedItemIndex);
						inOutVector.emplace(inOutVector.begin() + selectedItemIndex + 1, valueToMove);
						selectedItemIndex++;
					}
				}
				else if (addItem)
				{
					selectedItemIndex = static_cast<i32>(inOutVector.size());
					inOutVector.emplace_back();
					scrollToBottomOnNextFrame = true;
				}
				else if (removeItem && indexInBounds)
				{
					inOutVector.erase(inOutVector.begin() + selectedItemIndex);
					if (selectedItemIndex > 0 && selectedItemIndex == static_cast<i32>(inOutVector.size()))
						selectedItemIndex--;
				}

				Gui::EndChild();
				Gui::PopID();
				Gui::NextColumn();
			};

			GuiBeginSettingsColumns();
			GuiSettingsCombo("Combo Box Dropdown Property", selectedInspectorDropdownPropertyType, TargetPropertyTypeNames);
			if (selectedInspectorDropdownPropertyType >= TargetPropertyType_Count)
				selectedInspectorDropdownPropertyType = FindFirstNonEmptyInspectorDropdownPropertyType(userData);
			switch (selectedInspectorDropdownPropertyType)
			{
			case TargetPropertyType_PositionX: guiDropdownVector(userData.TargetPreset.InspectorDropdown.PositionsX); break;
			case TargetPropertyType_PositionY: guiDropdownVector(userData.TargetPreset.InspectorDropdown.PositionsY); break;
			case TargetPropertyType_Angle: guiDropdownVector(userData.TargetPreset.InspectorDropdown.Angles); break;
			case TargetPropertyType_Frequency: guiDropdownVector(userData.TargetPreset.InspectorDropdown.Frequencies); break;
			case TargetPropertyType_Amplitude: guiDropdownVector(userData.TargetPreset.InspectorDropdown.Amplitudes); break;
			case TargetPropertyType_Distance: guiDropdownVector(userData.TargetPreset.InspectorDropdown.Distances); break;
			}
			GuiEndSettingsColumns();
		}

		if (Gui::CollapsingHeader("BPM Calculator", ImGuiTreeNodeFlags_DefaultOpen))
		{
			GuiBeginSettingsColumns();
			GuiSettingsCombo("Tap Sound Type", userData.BPMCalculator.TapSoundType, BPMTapSoundTypeNames);
			GuiSettingsCheckbox("Auto Reset Enabled", userData.BPMCalculator.AutoResetEnabled);
			GuiSettingsCheckbox("Apply To Tempo Map", userData.BPMCalculator.ApplyToTempoMap);
			GuiEndSettingsColumns();
		}

		if (Gui::CollapsingHeader("Playtest", ImGuiTreeNodeFlags_DefaultOpen))
		{
			GuiBeginSettingsColumns();
			GuiSettingsCheckbox("Enter Fullscreen on Maximized Start", userData.Playtest.EnterFullscreenOnMaximizedStart);
			GuiSettingsCheckbox("Auto Hide Mouse Cursor", userData.Playtest.AutoHideCursor);
			GuiEndSettingsColumns();
		}
	}

	void ChartEditorSettingsWindow::GuiTabAudio(ComfyStudioUserSettings& userData)
	{
		if (Gui::CollapsingHeader("Volume Levels", ImGuiTreeNodeFlags_DefaultOpen))
		{
			GuiBeginSettingsColumns();
			GuiSettingsVolumeSlider("Song Volume", userData.System.Audio.SongVolume);
			GuiSettingsVolumeSlider("Button Sound Volume", userData.System.Audio.ButtonSoundVolume);
			GuiSettingsVolumeSlider("Sound Effect Volume", userData.System.Audio.SoundEffectVolume);
			GuiSettingsVolumeSlider("Metronome Volume", userData.System.Audio.MetronomeVolume, true);
			GuiEndSettingsColumns();
		}

		if (Gui::CollapsingHeader("Behavior", ImGuiTreeNodeFlags_DefaultOpen))
		{
			GuiBeginSettingsColumns();
			GuiSettingsCheckbox("Open Device on Startup", userData.System.Audio.OpenDeviceOnStartup);
			GuiSettingsCheckbox("Close Device on Idle Focus Loss", userData.System.Audio.CloseDeviceOnIdleFocusLoss);
			GuiSettingsCheckbox("Request Exclusive Device Access", userData.System.Audio.RequestExclusiveDeviceAccess);
			GuiEndSettingsColumns();
		}
	}

	void ChartEditorSettingsWindow::GuiTabInput(ComfyStudioUserSettings& userData)
	{
		if (Gui::CollapsingHeader("Dummy", ImGuiTreeNodeFlags_DefaultOpen))
		{
			Gui::TextUnformatted("Dummy Text Here");
		}
	}

	void ChartEditorSettingsWindow::GuiTabThemeDebug(ComfyStudioUserSettings& userData)
	{
		if (Gui::CollapsingHeader("Gui Style Editor", ImGuiTreeNodeFlags_DefaultOpen))
		{
			Gui::ShowStyleEditor();
		}
	}
}
