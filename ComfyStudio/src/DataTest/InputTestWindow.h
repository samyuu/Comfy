#pragma once
#include "Types.h"
#include "Core/BaseWindow.h"
#include "Input/Input.h"

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
		void KeyboardTabTabItemGui();
		void CombinedControllerTabItemGui();
		void ConnectedControllersTabItemGui();

	private:
		bool showHeldDownKeysOnly = false;
		bool showMouseButtonKeys = false;
		bool showHeldDownButtonsOnly = false;
		bool showHeldDownNativeButtonsOnly = false;
	};
}
