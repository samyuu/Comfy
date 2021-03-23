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
		void SelectNextTab(i32 direction);

		void SaveUserDataCopy();
		void RestoreUserDataCopy();
		void KeepChangesSaveToFile();
		void RequestWindowCloseAndKeepChanges();
		void RequestWindowCloseAndRevertChanges();

	private:
		void GuiTabGeneral(ComfyStudioUserSettings& userData);
		void GuiTabAudio(ComfyStudioUserSettings& userData);
		void GuiTabInput(ComfyStudioUserSettings& userData);
		void GuiTabThemeDebug(ComfyStudioUserSettings& userData);

	private:
		bool closeWindowThisFrame = false;
		bool thisFrameAnyItemActive = false, lastFrameAnyItemActive = false;

		i32 selectedTabIndex = {};
		ComfyStudioUserSettings userDataPreEditCopy = {};

		std::array<i32, TargetPropertyType_Count> inspectorDropdownItemIndices = {};
		std::array<bool, TargetPropertyType_Count> inspectorDropdownScrollToBottomOnNextFrames = {};
		TargetPropertyType selectedInspectorDropdownPropertyType = TargetPropertyType_Count;

	private:
		struct NamedTab { const char* Name; void(ChartEditorSettingsWindow::*GuiFunction)(ComfyStudioUserSettings&); };
		static constexpr NamedTab namedTabs[] =
		{
			{ "General", &GuiTabGeneral },
			{ "Audio", &GuiTabAudio },
			{ "Input", &GuiTabInput },
#if COMFY_DEBUG
			{ "Theme (Debug)", &GuiTabThemeDebug },
#endif
		};
	};
}
