#include "InputTestWindow.h"
#include "Core/ComfyStudioApplication.h"
#include "Input/Input.h"

namespace Comfy::Studio::DataTest
{
	using namespace Input;

	InputTestWindow::InputTestWindow(ComfyStudioApplication& parent) : BaseWindow(parent)
	{
		Close();
	}

	const char* InputTestWindow::GetName() const
	{
		return "Input Test";
	}

	ImGuiWindowFlags InputTestWindow::GetFlags() const
	{
		return ImGuiWindowFlags_None;
	}

	void InputTestWindow::Gui()
	{
		Gui::BeginChild("InputTestWindowOutterChild", vec2(0.0f, -30.0f), true);
		if (Gui::BeginTabBar("InputTestWindowTabBar", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton | ImGuiTabBarFlags_NoTooltip))
		{
			KeyboardTabTabItemGui();
			CombinedControllerTabItemGui();
			ConnectedControllersTabItemGui();
			// TODO: Add Key/Button/Binding string parse test gui with roundtrip display (char by char comparison with red/green/yellow text coloring depending on match?)
			// TODO: Add Binding/MultiBinding trigger test gui (?)
			// TODO: Add LayoutMapping test gui (?)
			Gui::EndTabBar();
		}
		Gui::EndChild();

		Gui::BeginChild("RefreshDevicesChild", vec2(0.0f, 0.0f), true);
		if (Gui::Button("Refresh Devices", Gui::GetContentRegionAvail()))
			Input::GlobalSystemRefreshDevices();
		Gui::EndChild();
	}

	void InputTestWindow::KeyboardTabTabItemGui()
	{
		bool isAnyKeyboardKeyDown = false;
		if (Gui::BeginTabItem("Keyboard State"))
		{
			Gui::BeginChild("KeyboardChild", vec2(0.0f, 0.0f), true);

			Gui::Checkbox("Held Down Keys Only", &showHeldDownKeysOnly);
			Gui::Checkbox("Include Mouse Buttons", &showMouseButtonKeys);
			Gui::Separator();

			// TODO: It would be nice to have a grid of buttons arranged in a standard keyboard layout instead
			Gui::BeginChild("KeyListChild", vec2(0.0f, 0.0f), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
			Gui::BeginColumns(nullptr, 4, ImGuiOldColumnFlags_NoResize);
			Gui::SetColumnWidth(0, Gui::GetWindowWidth() * 0.35f);
			{
				Gui::AlignTextToFramePadding(); Gui::TextUnformatted("KeyCode Name"); Gui::NextColumn();
				Gui::AlignTextToFramePadding(); Gui::TextUnformatted("Down"); Gui::NextColumn();
				Gui::AlignTextToFramePadding(); Gui::TextUnformatted("Pressed"); Gui::NextColumn();
				Gui::AlignTextToFramePadding(); Gui::TextUnformatted("Released"); Gui::NextColumn();
				Gui::Separator();

				for (KeyCode key = (showMouseButtonKeys ? KeyCode_MouseFirst : KeyCode_KeyboardFirst); key <= KeyCode_Last; key++)
				{
					const char* keyCodeName = Input::GetKeyCodeName(key);
					const char* keyCodeEnumName = Input::GetKeyCodeEnumName(key);
					if (keyCodeEnumName == nullptr || keyCodeEnumName[0] == '\0')
						continue;

					const bool isDown = Input::IsKeyDown(key);
					const bool isPressed = Input::IsKeyPressed(key, false);
					const bool isPressedRepeat = Input::IsKeyPressed(key, true);
					const bool isReleased = Input::IsKeyReleased(key);

					if (isDown && (key >= KeyCode_KeyboardFirst && key <= KeyCode_KeyboardLast))
						isAnyKeyboardKeyDown = true;

					if (showHeldDownKeysOnly && !(isDown || isReleased))
						continue;

					Gui::PushItemDisabledAndTextColorIf(!isDown);
					Gui::Text("%s (%s)", keyCodeEnumName, keyCodeName); Gui::NextColumn();
					Gui::TextUnformatted(isDown ? "Down" : "Up"); Gui::NextColumn();
					Gui::Text("%s%s", isPressed ? "Pressed" : "-", isPressedRepeat ? " (Repeat)" : ""); Gui::NextColumn();
					Gui::TextUnformatted(isReleased ? "Released" : "-"); Gui::NextColumn();
					Gui::PopItemDisabledAndTextColorIf(!isDown);

					Gui::Separator();
				}
			}
			Gui::EndColumns();
			Gui::EndChild();

			Gui::EndChild();
			Gui::EndTabItem();
		}

		// NOTE: Mostly to prevent global keybindings from triggering
		if (isAnyKeyboardKeyDown && Gui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
			Gui::SetActiveID(Gui::GetID("KeyboardTest"), Gui::GetCurrentWindow());
	}

	void InputTestWindow::CombinedControllerTabItemGui()
	{
		if (Gui::BeginTabItem("Combined Controller State"))
		{
			Gui::BeginChild("CombinedControllerChild", vec2(0.0f, 0.0f), true);

			// Gui::BeginChild("AxesChild", vec2(0.0f, 88.0f), true);
			{
				auto stickGui = [](const char* label, Stick stick)
				{
					Gui::Button(label, vec2(84.0f));
					const auto rect = ImRect(Gui::GetItemRectMin(), Gui::GetItemRectMax());
					Gui::GetWindowDrawList()->AddCircle(rect.GetCenter() + (Input::GetStick(stick) * (rect.GetSize() / 2.0f - vec2(4.0f))), 4.0f, 0xDD0000FF);
				};
				auto triggerGui = [](const char* label, Axis axis)
				{
					Gui::Button(label, vec2(84.0f));
					const auto rect = ImRect(Gui::GetItemRectMin(), Gui::GetItemRectMax());
					Gui::GetWindowDrawList()->AddRectFilled(rect.GetTL(), rect.GetTR() + vec2(0.0f, rect.GetHeight() * Input::GetAxis(axis)), 0x440000FF);
				};
				stickGui("L-Stick", Stick::LeftStick);
				Gui::SameLine();
				stickGui("R-Stick", Stick::RightStick);
				Gui::SameLine();
				triggerGui("L-Trigger", Axis::LeftTrigger);
				Gui::SameLine();
				triggerGui("R-Trigger", Axis::RightTrigger);
			}
			// Gui::EndChild();
			Gui::Separator();

			Gui::Checkbox("Held Down Buttons Only", &showHeldDownButtonsOnly);
			Gui::Separator();

			Gui::BeginChild("ButtonListChild", vec2(0.0f, 0.0f), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
			Gui::BeginColumns(nullptr, 4, ImGuiOldColumnFlags_NoResize);
			Gui::SetColumnWidth(0, Gui::GetWindowWidth() * 0.35f);
			{
				Gui::AlignTextToFramePadding(); Gui::TextUnformatted("Button Name"); Gui::NextColumn();
				Gui::AlignTextToFramePadding(); Gui::TextUnformatted("Down"); Gui::NextColumn();
				Gui::AlignTextToFramePadding(); Gui::TextUnformatted("Pressed"); Gui::NextColumn();
				Gui::AlignTextToFramePadding(); Gui::TextUnformatted("Released"); Gui::NextColumn();
				Gui::Separator();

				for (i32 buttonIndex = static_cast<i32>(Button::First); buttonIndex <= static_cast<i32>(Button::Last); buttonIndex++)
				{
					const Button button = static_cast<Button>(buttonIndex);
					const char* buttonName = Input::GetButtonName(button);
					const char* buttonEnumName = Input::GetButtonEnumName(button);
					if (buttonEnumName == nullptr || buttonEnumName[0] == '\0')
						continue;

					const bool isDown = Input::IsButtonDown(button);
					const bool isPressed = Input::IsButtonPressed(button, false);
					const bool isPressedRepeat = Input::IsButtonPressed(button, true);
					const bool isReleased = Input::IsButtonReleased(button);

					if (showHeldDownButtonsOnly && !(isDown || isReleased))
						continue;

					Gui::PushItemDisabledAndTextColorIf(!isDown);
					Gui::Text("%s (%s)", buttonEnumName, buttonName); Gui::NextColumn();
					Gui::TextUnformatted(isDown ? "Down" : "Up"); Gui::NextColumn();
					Gui::Text("%s%s", isPressed ? "Pressed" : "-", isPressedRepeat ? " (Repeat)" : ""); Gui::NextColumn();
					Gui::TextUnformatted(isReleased ? "Released" : "-"); Gui::NextColumn();
					Gui::PopItemDisabledAndTextColorIf(!isDown);

					Gui::Separator();
				}
			}
			Gui::EndColumns();
			Gui::EndChild();

			Gui::EndChild();
			Gui::EndTabItem();
		}
	}

	void InputTestWindow::ConnectedControllersTabItemGui()
	{
		if (Gui::BeginTabItem("Connected Controllers"))
		{
			Gui::BeginChild("ControllersChild", vec2(0.0f, 0.0f), true);

			const size_t controllerCount = GlobalSystemGetConnectedControllerCount();
			if (controllerCount < 1)
			{
				Gui::PushItemDisabledAndTextColor();
				Gui::Button("No Controller Connected", Gui::GetContentRegionAvail());
				Gui::PopItemDisabledAndTextColor();
			}
			else if (Gui::BeginTabBar("ConnectedControllersTabBar", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton | ImGuiTabBarFlags_FittingPolicyScroll))
			{
				for (size_t controllerIndex = 0; controllerIndex < controllerCount; controllerIndex++)
				{
					const auto controllerView = GlobalSystemGetConnectedControllerInfoAt(controllerIndex);
					Gui::PushID(static_cast<i32>(controllerIndex));
					if (Gui::BeginTabItem(controllerView.InstanceName.data()))
					{
						Gui::BeginChild("InnerControllerChild", vec2(0.0f, 0.0f), true);

						Gui::BeginColumns(nullptr, 2, ImGuiOldColumnFlags_NoResize);
						{
							Gui::TextUnformatted("Instance Name"); Gui::NextColumn();
							Gui::TextUnformatted(Gui::StringViewStart(controllerView.InstanceName), Gui::StringViewEnd(controllerView.InstanceName)); Gui::NextColumn();
							Gui::Separator();
							Gui::TextUnformatted("Instance ID"); Gui::NextColumn();
							Gui::TextUnformatted(ControllerIDToString(controllerView.InstanceID).data()); Gui::NextColumn();
							Gui::Separator();

							Gui::TextUnformatted("Product Name"); Gui::NextColumn();
							Gui::TextUnformatted(Gui::StringViewStart(controllerView.ProductName), Gui::StringViewEnd(controllerView.ProductName)); Gui::NextColumn();
							Gui::Separator();
							Gui::TextUnformatted("Product ID"); Gui::NextColumn();
							Gui::TextUnformatted(ControllerIDToString(controllerView.ProductID).data()); Gui::NextColumn();
							Gui::Separator();

							Gui::TextUnformatted("Button Count"); Gui::NextColumn();
							Gui::Text("%d", controllerView.ButtonCount); Gui::NextColumn();
							Gui::Separator();

							Gui::TextUnformatted("DPad Count"); Gui::NextColumn();
							Gui::Text("%d", controllerView.DPadCount); Gui::NextColumn();
							Gui::Separator();

							Gui::TextUnformatted("Axis Count"); Gui::NextColumn();
							Gui::Text("%d", controllerView.AxisCount); Gui::NextColumn();
							Gui::Separator();
						}
						Gui::EndColumns();

						// Gui::BeginChild("ControllersAxesChild", vec2(0.0f, 0.0f));
						for (i32 i = static_cast<i32>(NativeAxis::First); i <= std::min(static_cast<i32>(NativeAxis::Last), controllerView.AxisCount); i++)
						{
							const f32 axisValue = Input::GetNativeAxis(controllerView.InstanceID, static_cast<NativeAxis>(i));

							char overlayBuffer[64];
							sprintf_s(overlayBuffer, "Axis %d", i);
							Gui::ProgressBar(axisValue, vec2(-1.0f, 0.0f), overlayBuffer);
						}
						// Gui::EndChild();
						Gui::Separator();

						Gui::Checkbox("Held Down Buttons Only", &showHeldDownNativeButtonsOnly);
						Gui::Separator();

						Gui::BeginChild("ButtonListChild", vec2(0.0f, 0.0f), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
						Gui::BeginColumns(nullptr, 2, ImGuiOldColumnFlags_NoResize);
						Gui::SetColumnWidth(0, Gui::GetWindowWidth() * 0.5f);
						{
							Gui::AlignTextToFramePadding(); Gui::TextUnformatted("Native Button Name"); Gui::NextColumn();
							Gui::AlignTextToFramePadding(); Gui::TextUnformatted("Down"); Gui::NextColumn();
							Gui::Separator();

							for (i32 nativeButtonIndex = static_cast<i32>(NativeButton::FirstAll); nativeButtonIndex <= static_cast<i32>(NativeButton::LastAll); nativeButtonIndex++)
							{
								const NativeButton nativeButton = static_cast<NativeButton>(nativeButtonIndex);
								const bool isDown = Input::IsNativeButtonDown(controllerView.InstanceID, nativeButton);

								if (showHeldDownNativeButtonsOnly && !isDown)
									continue;

								i32 subTypeIndex = nativeButtonIndex;
								const char* buttonTypeName = "";
								const char* buttonSubTypeName = "";

								if (nativeButton >= NativeButton::FirstButton && nativeButton <= NativeButton::LastButton)
								{
									const i32 relativeIndex = static_cast<i32>(nativeButton) - static_cast<i32>(NativeButton::FirstButton);

									subTypeIndex = relativeIndex;
									buttonTypeName = "Button";

									if (subTypeIndex >= controllerView.ButtonCount)
										continue;
								}
								else if (nativeButton >= NativeButton::FirstDPad && nativeButton <= NativeButton::LastDPad)
								{
									const i32 relativeIndex = static_cast<i32>(nativeButton) - static_cast<i32>(NativeButton::FirstDPad);
									const i32 dpadIndex = (relativeIndex / static_cast<i32>(NativeButton::PerDPadSubElements));
									const i32 directionIndex = (relativeIndex % static_cast<i32>(NativeButton::PerDPadSubElements));

									subTypeIndex = dpadIndex;
									buttonTypeName = "DPad";
									buttonSubTypeName = std::array { " Up", " Left", " Down", " Right" }[directionIndex];

									if (subTypeIndex >= controllerView.DPadCount)
										continue;
								}
								else if (nativeButton >= NativeButton::FirstAxis && nativeButton <= NativeButton::LastAxis)
								{
									const i32 relativeIndex = static_cast<i32>(nativeButton) - static_cast<i32>(NativeButton::FirstAxis);
									const i32 axisIndex = (relativeIndex / static_cast<i32>(NativeButton::PerAxisSubElements));
									const i32 directionIndex = (relativeIndex % static_cast<i32>(NativeButton::PerAxisSubElements));

									subTypeIndex = axisIndex;
									buttonTypeName = "Axis";
									buttonSubTypeName = std::array { " -", " +" }[directionIndex];

									if (subTypeIndex >= controllerView.AxisCount)
										continue;
								}
								else
								{
									continue;
								}

								Gui::PushItemDisabledAndTextColorIf(!isDown);
								Gui::Text("%s %d%s (ID: %d)", buttonTypeName, subTypeIndex + 1, buttonSubTypeName, nativeButtonIndex); Gui::NextColumn();
								Gui::TextUnformatted(isDown ? "Down" : "Up"); Gui::NextColumn();
								Gui::PopItemDisabledAndTextColorIf(!isDown);

								Gui::Separator();
							}
						}
						Gui::EndColumns();
						Gui::EndChild();

						Gui::EndChild();
						Gui::EndTabItem();
					}
					Gui::PopID();
				}
				Gui::EndTabBar();
			}

			Gui::EndChild();
			Gui::EndTabItem();
		}
	}
}
