#pragma once
#include "Types.h"
#include "Core/ComfyStudioSettings.h"
#include "Core/ComfyStudioDiscord.h"
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
		void GuiTabTimeline(ComfyStudioUserSettings& userData);
		void GuiTabTools(ComfyStudioUserSettings& userData);
		void GuiTabAutoSave(ComfyStudioUserSettings& userData);
		void GuiTabPlaytest(ComfyStudioUserSettings& userData);
#if COMFY_COMILE_WITH_DLL_DISCORD_RICH_PRESENCE_INTEGRATION
		void GuiTabDiscord(ComfyStudioUserSettings& userData);
#endif
		void GuiTabAudio(ComfyStudioUserSettings& userData);
		void GuiTabControllerLayout(ComfyStudioUserSettings& userData);
		void GuiTabEditorBindings(ComfyStudioUserSettings& userData);
		void GuiTabPlaytestBindings(ComfyStudioUserSettings& userData);
		void GuiTabThemeDebug(ComfyStudioUserSettings& userData);

		void GuiControllerLayoutTabBarAndContent(ComfyStudioUserSettings& userData);
		void GuiControllerLayoutTabItemInnerContent(ComfyStudioUserSettings& userData, const Input::ControllerInfoView& controllerInfo, Input::StandardControllerLayoutMapping& correspondingLayoutMapping, const bool nativeButtonsDown[]);
		void GuiButtonPickerPopupContent(ComfyStudioUserSettings& userData, Input::StandardControllerLayoutMapping& layoutMapping);

	private:
		bool closeWindowThisFrame = false;
		bool thisFrameAnyItemActive = false, lastFrameAnyItemActive = false;

		bool alwaysShowVerticalScrollBar = false;
		bool showRarelyUsedSettings = false;
		bool pendingChanges = false;

		i32 selectedTabIndex = {};
		ComfyStudioUserSettings userDataPreEditCopy = {};

		f32 targetTimelineSmoothScrollSpeedPreDisable = TimelineDefaultSmoothScrollSpeedSec.x;

		std::array<i32, TargetPropertyType_Count> inspectorDropdownItemIndices = {};
		std::array<bool, TargetPropertyType_Count> inspectorDropdownScrollToBottomOnNextFrames = {};
		TargetPropertyType selectedInspectorDropdownPropertyType = TargetPropertyType_Count;

		Gui::ExtendedImGuiTextFilter bindingFilter = {};
		// NOTE: Pointing inside a global struct so don't need to worry about lifetime or becoming invalidated
		Input::MultiBinding* selectedMultiBinding = nullptr;

		std::string combinedBindingShortcutBuffer;

		Input::Binding* awaitInputBinding = nullptr;
		Stopwatch awaitInputStopwatch = {};

		Input::NativeButton currentButtonPickerPopupNativeButton;

	private:
		struct NamedTab { const char* Name; void(ChartEditorSettingsWindow::*GuiFunction)(ComfyStudioUserSettings&); };
		static constexpr NamedTab namedTabs[] =
		{
			{ "General", &GuiTabGeneral },
			{ "Timeline", &GuiTabTimeline },
			{ "Tools", &GuiTabTools },
			{ "Playtest", &GuiTabPlaytest },
			{ "Auto Save", &GuiTabAutoSave },
#if COMFY_COMILE_WITH_DLL_DISCORD_RICH_PRESENCE_INTEGRATION
			{ "Discord", &GuiTabDiscord },
#endif
			{ "Audio", &GuiTabAudio },
			{ "Controller Layout", &GuiTabControllerLayout },
			// TODO: Only problem is that this also includes things like "Playtest Pause" so not really exclusive to "Editor"...
			{ "Editor Bindings", &GuiTabEditorBindings },
			{ "Playtest Bindings", &GuiTabPlaytestBindings },
#if COMFY_DEBUG && 0
			{ "Theme (Debug)", &GuiTabThemeDebug },
#endif
		};
	};
}
