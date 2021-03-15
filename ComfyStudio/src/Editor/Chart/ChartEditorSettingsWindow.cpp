#include "ChartEditorSettingsWindow.h"

namespace Comfy::Studio::Editor
{
	void ChartEditorSettingsWindow::Gui()
	{
		lastFrameAnyItemActive = thisFrameAnyItemActive;
		thisFrameAnyItemActive = Gui::IsAnyItemActive();

		// TODO: ...
		Gui::BeginChild("DummyChild", vec2(320.0f, 240.0f));
		Gui::Text("Dummy");
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

	void ChartEditorSettingsWindow::SaveUserDataCopy()
	{
		userDataPreEditCopy = GlobalUserData;
	}

	void ChartEditorSettingsWindow::RestoreUserDataCopy()
	{
		GlobalUserData.Mutable() = userDataPreEditCopy;
	}

	void ChartEditorSettingsWindow::RequestWindowCloseAndKeepChanges()
	{
		closeWindowThisFrame = true;
		GlobalUserData.SaveToFile();
	}

	void ChartEditorSettingsWindow::RequestWindowCloseAndRevertChanges()
	{
		RestoreUserDataCopy();
		closeWindowThisFrame = true;
	}
}
