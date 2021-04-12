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

		// TODO: Restructure all of this
		Gui::Text("Input Test:");
		Gui::Separator();

		if (Gui::Button("Refresh Devices", vec2(Gui::GetWindowWidth(), 0.0f)))
			Input::GlobalSystemRefreshDevices();
		Gui::Separator();

		if (Gui::CollapsingHeader("Keyboard"/*, ImGuiTreeNodeFlags_DefaultOpen*/))
		{
			for (Input::KeyCode key = Input::KeyCode_KeyboardFirst; key < Input::KeyCode_KeyboardLast; key++)
			{
				if (Input::IsDown(key))
				{
					if (const char* keyName = Input::GetKeyCodeName(key); keyName != nullptr)
						Gui::BulletText(keyName);
					else
						Gui::BulletText("Unknown Key 0x%02X", key);
				}
			}
		}
		Gui::Separator();

		if (Gui::CollapsingHeader("Combined Controller"/*, ImGuiTreeNodeFlags_DefaultOpen*/))
		{
			Gui::BeginChild("ButtonsChild", vec2(0.0f, 60.0f));
			for (size_t buttonIndex = 0; buttonIndex < EnumCount<Input::Button>(); buttonIndex++)
			{
				const auto button = static_cast<Input::Button>(buttonIndex);
				if (Input::IsDown(button))
				{
					if (const char* buttonName = Input::GetButtonName(button); buttonName != nullptr)
						Gui::BulletText(buttonName);
					else
						Gui::BulletText("Unknown Button 0x%02X", buttonIndex);
				}
			}
			Gui::EndChild();

			Gui::BeginChild("AxesChild", vec2(0.0f, 88.0f));
			auto stickGui = [](const char* label, Input::Stick stick)
			{
				Gui::Button(label, vec2(84.0f));
				const auto rect = ImRect(Gui::GetItemRectMin(), Gui::GetItemRectMax());
				Gui::GetWindowDrawList()->AddCircle(rect.GetCenter() + (Input::GetStick(stick) * (rect.GetSize() / 2.0f - vec2(4.0f))), 4.0f, 0xDD0000FF);
			};
			auto triggerGui = [](const char* label, Input::Axis axis)
			{
				Gui::Button(label, vec2(84.0f));
				const auto rect = ImRect(Gui::GetItemRectMin(), Gui::GetItemRectMax());
				Gui::GetWindowDrawList()->AddRectFilled(rect.GetTL(), rect.GetTR() + vec2(0.0f, rect.GetHeight() * Input::GetAxis(axis)), 0x440000FF);
			};
			stickGui("Left Stick", Input::Stick::LeftStick);
			Gui::SameLine();
			stickGui("Right Stick", Input::Stick::RightStick);
			Gui::SameLine();
			triggerGui("Left Trigger", Input::Axis::LeftTrigger);
			Gui::SameLine();
			triggerGui("Right Trigger", Input::Axis::RightTrigger);
			Gui::EndChild();
		}
		Gui::Separator();

		if (Gui::CollapsingHeader("Controllers", ImGuiTreeNodeFlags_DefaultOpen))
		{
			using namespace Input;

			GlobalSystemForEachConnectedController([&](size_t index, const ControllerInfoView& controllerView)
			{
				Gui::Text("%s - Product: %s Instance: %s", controllerView.ProductName.data(), ControllerIDToString(controllerView.ProductID).data(), ControllerIDToString(controllerView.InstanceID).data());
				Gui::BeginChild("ControllersButtonsChild", vec2(0.0f, 60.0f));
				for (i32 i = static_cast<i32>(NativeButton::FirstButton); i <= static_cast<i32>(NativeButton::LastButton); i++)
				{
					if (IsNativeButtonDown(controllerView.InstanceID, static_cast<NativeButton>(i)))
						Gui::BulletText("Button %d (Abs: %d)", i, i);
				}
				Gui::EndChild();

				Gui::BeginChild("ControllersButtonsDPadChild", vec2(0.0f, 60.0f));
				for (i32 i = static_cast<i32>(NativeButton::FirstDPad); i <= static_cast<i32>(NativeButton::LastDPad); i++)
				{
					if (IsNativeButtonDown(controllerView.InstanceID, static_cast<NativeButton>(i)))
						Gui::BulletText("DPad %d (Abs: %d)", i - static_cast<i32>(NativeButton::FirstDPad), i);
				}
				Gui::EndChild();

				Gui::BeginChild("ControllersButtonsAxesChild", vec2(0.0f, 60.0f));
				for (i32 i = static_cast<i32>(NativeButton::FirstAxis); i <= static_cast<i32>(NativeButton::LastAxis);)
				{
					const auto negative = static_cast<NativeButton>(i++);
					const auto positive = static_cast<NativeButton>(i++);

					if (IsNativeButtonDown(controllerView.InstanceID, negative))
						Gui::BulletText("Neg Axis %d (Abs: %d)", static_cast<i32>(negative) - static_cast<i32>(NativeButton::FirstAxis), static_cast<i32>(negative));

					if (IsNativeButtonDown(controllerView.InstanceID, positive))
						Gui::BulletText("Pos Axis %d (Abs: %d)", static_cast<i32>(positive) - static_cast<i32>(NativeButton::FirstAxis), static_cast<i32>(positive));
				}
				Gui::EndChild();

				Gui::BeginChild("ControllersAxesChild", vec2(0.0f, 0.0f));
				for (i32 i = static_cast<i32>(NativeAxis::First); i <= static_cast<i32>(NativeAxis::Last); i++)
				{
					const f32 axisValue = GetNativeAxis(controllerView.InstanceID, static_cast<NativeAxis>(i));

					char overlayBuffer[64];
					sprintf_s(overlayBuffer, "Axis %d", i);
					Gui::ProgressBar(axisValue, vec2(-1.0f, 0.0f), overlayBuffer);
				}
				Gui::EndChild();
			});
		}
		Gui::Separator();
	}
}
