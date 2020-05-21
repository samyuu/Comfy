#include "InputTestWindow.h"
#include "Core/Application.h"
#include "Input/Input.h"

namespace Comfy::Studio::DataTest
{
	InputTestWindow::InputTestWindow(Application* parent) : BaseWindow(parent)
	{
		CloseWindow();
	}

	InputTestWindow::~InputTestWindow()
	{
	}

	void InputTestWindow::DrawGui()
	{
		static const ImVec4 onColor = ImVec4(0.14f, 0.78f, 0.21f, 1.00f);
		static const ImVec4 offColor = ImVec4(0.95f, 0.12f, 0.12f, 1.00f);

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
			Input::Keyboard* keyboard = Input::Keyboard::GetInstance();

			bool initialized = Input::Keyboard::GetInstanceInitialized();
			boolColoredText("KEYBOARD : ", "OK", "NG", initialized);

			if (initialized)
			{
				constexpr auto firstKeyboardKey = Input::KeyCode_Backspace;
				for (Input::KeyCode key = firstKeyboardKey; key < Input::KeyCode_Count; key++)
				{
					if (keyboard->IsDown(key))
					{
						const char* keyName = Input::GetKeyCodeName(key);
						if (keyName != nullptr)
							Gui::BulletText(keyName);
					}
				}
			}
		}
		Gui::Separator();

		if (Gui::CollapsingHeader("DualShock4", ImGuiTreeNodeFlags_DefaultOpen))
		{
			Input::DualShock4* ds4 = Input::DualShock4::GetInstance();

			bool initialized = Input::DualShock4::GetInstanceInitialized();
			boolColoredText("DUALSHOCK4 : ", "OK", "NG", initialized);

			if (initialized)
			{
				for (size_t button = 0; button < static_cast<size_t>(Input::Ds4Button::Count); button++)
				{
					if (ds4->IsDown(static_cast<Input::Ds4Button>(button)))
						Gui::BulletText(ds4ButtonNames[button]);
				}
			}
		}
		Gui::Separator();
	}

	const char* InputTestWindow::GetGuiName() const
	{
		return u8"Input Test";
	}

	ImGuiWindowFlags InputTestWindow::GetWindowFlags() const
	{
		return ImGuiWindowFlags_None;
	}

	void InputTestWindow::RefreshDevices()
	{
		if (!Input::Keyboard::GetInstanceInitialized())
			Input::Keyboard::TryInitializeInstance(/*GetParent()->GetHost().GetWindow()*/);

		if (!Input::DualShock4::GetInstanceInitialized())
			Input::DualShock4::TryInitializeInstance();
	}
}
