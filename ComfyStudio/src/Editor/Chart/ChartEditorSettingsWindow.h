#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Core/ComfyStudioSettings.h"
#include "ImGui/Gui.h"

namespace Comfy::Studio::Editor
{
	class ChartEditorSettingsWindow : NonCopyable
	{
	public:
		ChartEditorSettingsWindow() = default;
		~ChartEditorSettingsWindow() = default;

	public:
		void Gui();
		void OnWindowOpen();
		void OnCloseButtonClicked();
		bool GetAndClearCloseRequestThisFrame();

	private:
		void SaveUserDataCopy();
		void RestoreUserDataCopy();
		void RequestWindowCloseAndKeepChanges();
		void RequestWindowCloseAndRevertChanges();

	private:
		bool closeWindowThisFrame = false;
		bool thisFrameAnyItemActive = false, lastFrameAnyItemActive = false;

		ComfyStudioUserSettings userDataPreEditCopy = {};
	};
}
