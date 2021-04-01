#include "PVScriptExportWindow.h"
#include "Core/ComfyStudioSettings.h"
#include "ImGui/Extensions/ImGuiExtensions.h"

namespace Comfy::Studio::Editor
{
	void PVScriptExportWindow::Gui()
	{
		lastFrameAnyItemActive = thisFrameAnyItemActive;
		thisFrameAnyItemActive = Gui::IsAnyItemActive();

		// TODO:
		Gui::BeginChild("DummyChild", vec2(320.0f, 240.0f), true);
		Gui::TextUnformatted("Dummy Text Here...");
		Gui::EndChild();

		if (Gui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !thisFrameAnyItemActive && !lastFrameAnyItemActive)
		{
			if (Input::IsAnyPressed(GlobalUserData.Input.App_Dialog_Cancel, false))
				closeWindowThisFrame = true;
		}
	}

	bool PVScriptExportWindow::GetAndClearCloseRequestThisFrame()
	{
		const bool result = closeWindowThisFrame;
		closeWindowThisFrame = false;
		return result;
	}
}
