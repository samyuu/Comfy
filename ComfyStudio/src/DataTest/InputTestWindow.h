#pragma once
#include "CoreTypes.h"
#include "Core/BaseWindow.h"
#include "Input/DirectInput/DualShock4.h"

namespace Comfy::DataTest
{
	class InputTestWindow : public BaseWindow
	{
	public:
		InputTestWindow(Application*);
		~InputTestWindow();

		void DrawGui() override;
		const char* GetGuiName() const override;
		ImGuiWindowFlags GetWindowFlags() const override;

	private:
		static constexpr std::array<const char*, static_cast<size_t>(Ds4Button::Count)> ds4ButtonNames =
		{
			"Square",
			"Cross",
			"Circle",
			"Triangle",
			"L1",
			"R1",
			"L_Trigger",
			"R_Trigger",
			"Share",
			"Options",
			"L3",
			"R3",
			"PS",
			"Touch",
			"DPad_Up",
			"DPad_Right",
			"DPad_Down",
			"DPad_Left",
			"L_Stick_Up",
			"L_Stick_Right",
			"L_Stick_Down",
			"L_Stick_Left",
			"R_Stick_Up",
			"R_Stick_Right",
			"R_Stick_Down",
			"R_Stick_Left",
		};

		void RefreshDevices();
	};
}
