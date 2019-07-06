#include "InputTestWindow.h"
#include "Application.h"
#include "Input/Keyboard.h"

InputTestWindow::InputTestWindow(Application* parent) : BaseWindow(parent)
{
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
		ImGui::Text(label);
		ImGui::SameLine();
		ImGui::TextColored(condition ? onColor : offColor, condition ? trueText : falseText);
	};

	ImGui::Text("INPUT TEST:");
	ImGui::Separator();

	if (ImGui::Button("Refresh Devices", ImVec2(ImGui::GetWindowWidth(), 0)))
		RefreshDevices();
	ImGui::Separator();

	if (ImGui::CollapsingHeader("Keyboard", ImGuiTreeNodeFlags_DefaultOpen))
	{
		Keyboard* keyboard = Keyboard::GetInstance();

		bool initialized = Keyboard::GetInstanceInitialized();
		boolColoredText("KEYBOARD : ", "OK", "NG", initialized);
	
		if (initialized)
		{
			for (KeyCode key = 0; key < KEY_COUNT; key++)
			{
				if (keyboard->IsDown(key))
				{
					const char* keyName = glfwGetKeyName(key, glfwGetKeyScancode(key));
					if (keyName != nullptr)
						ImGui::BulletText(keyName);
				}
			}
		}
	}
	ImGui::Separator();

	if (ImGui::CollapsingHeader("DualShock4", ImGuiTreeNodeFlags_DefaultOpen))
	{
		DualShock4* ds4 = DualShock4::GetInstance();

		bool initialized = DualShock4::GetInstanceInitialized();
		boolColoredText("DUALSHOCK4 : ", "OK", "NG", initialized);

		if (initialized)
		{
			for (size_t button = 0; button < DS4_BUTTON_MAX; button++)
			{
				if (ds4->IsDown((Ds4Button)button))
					ImGui::BulletText(ds4ButtonNames[button]);
			}
		}
	}
	ImGui::Separator();
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
	if (!Keyboard::GetInstanceInitialized())
		Keyboard::TryInitializeInstance(GetParent()->GetWindow());

	if (!DualShock4::GetInstanceInitialized())
		DualShock4::TryInitializeInstance();
}