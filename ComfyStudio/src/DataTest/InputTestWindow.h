#pragma once
#include "CoreTypes.h"
#include "Core/BaseWindow.h"
#include "Input/DirectInput/DualShock4.h"

namespace Comfy::Studio::DataTest
{
	class InputTestWindow : public BaseWindow
	{
	public:
		InputTestWindow(ComfyStudioApplication&);
		~InputTestWindow() = default;

	public:
		const char* GetName() const override;
		ImGuiWindowFlags GetFlags() const override;
		void Gui() override;

	private:
		static constexpr std::array<const char*, EnumCount<Input::DS4Button>()> ds4ButtonNames =
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
