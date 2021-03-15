#include "InputTestWindow.h"
#include "Core/ComfyStudioApplication.h"
#include "Input/Input.h"

namespace Comfy::Studio::DataTest
{
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
		static constexpr auto onColor = vec4(0.14f, 0.78f, 0.21f, 1.00f);
		static constexpr auto offColor = vec4(0.95f, 0.12f, 0.12f, 1.00f);

		auto boolColoredText = [](const char* label, const char* trueText, const char* falseText, bool condition)
		{
			Gui::Text(label);
			Gui::SameLine();
			Gui::TextColored(condition ? onColor : offColor, condition ? trueText : falseText);
		};

		Gui::Text("Input Test:");
		Gui::Separator();

		if (Gui::Button("Refresh Devices", vec2(Gui::GetWindowWidth(), 0.0f)))
			RefreshDevices();
		Gui::Separator();

		if (Gui::CollapsingHeader("Keyboard", ImGuiTreeNodeFlags_DefaultOpen))
		{
			const bool initialized = Input::Keyboard::GetInstanceInitialized();
			boolColoredText("KEYBOARD : ", "OK", "NG", initialized);

			if (initialized)
			{
				for (Input::KeyCode key = Input::KeyCode_KeyboardFirst; key < Input::KeyCode_KeyboardLast; key++)
				{
					if (Input::Keyboard::IsDown(key))
					{
						if (const char* keyName = Input::GetKeyCodeName(key); keyName != nullptr)
							Gui::BulletText(keyName);
						else
							Gui::BulletText("Unknown Key 0x%02X", key);
					}
				}
			}
		}
		Gui::Separator();

		if (Gui::CollapsingHeader("DualShock4", ImGuiTreeNodeFlags_DefaultOpen))
		{
			const bool initialized = Input::DualShock4::GetInstanceInitialized();
			boolColoredText("DUALSHOCK4 : ", "OK", "NG", initialized);

			if (initialized)
			{
				for (size_t button = 0; button < EnumCount<Input::DS4Button>(); button++)
				{
					if (Input::DualShock4::IsDown(static_cast<Input::DS4Button>(button)))
						Gui::BulletText(ds4ButtonNames[button]);
				}
			}
		}
		Gui::Separator();
	}

	void InputTestWindow::RefreshDevices()
	{
		if (!Input::Keyboard::GetInstanceInitialized())
			Input::Keyboard::TryInitializeInstance(/*GetParent()->GetHost().GetWindow()*/);

		if (!Input::DualShock4::GetInstanceInitialized())
			Input::DualShock4::TryInitializeInstance();
	}
}
