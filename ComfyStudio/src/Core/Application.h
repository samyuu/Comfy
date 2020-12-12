#pragma once
#include "Types.h"
#include "Window/ApplicationHost.h"
#include "BaseWindow.h"
#include "Editor/Core/EditorManager.h"
#include "License/LicenseWindow.h"

namespace Comfy::Studio
{
	class Application : NonCopyable
	{
	public:
		Application(std::string_view fileToOpen = "");
		~Application() = default;

		// NOTE: Initialize and enter the main loop
		void Run();

		// NOTE: Break out of the main loop
		void Exit();

	public:
		ApplicationHost& GetHost();
		std::string_view GetFileToOpenOnStartup() const;

		void SetFormattedWindowTitle(std::string_view subTitle);

		bool GetExclusiveFullscreenGui() const;
		void SetExclusiveFullscreenGui(bool value);

		// NOTE: Specifically to be used as a parent for file dialogs, dialog boxes etc.
		static void* GetGlobalWindowFocusHandle();

	private:
		ApplicationHost::ConstructionParam CreateHostParam();

	public:
		bool BaseInitialize();
		bool InitializeEditorComponents();

	private:
		void Gui();
		void UpdateWindowFocusAudioEngineResponse();

		void GuiMainMenuBar();
		void GuiApplicationWindowMenu();

		void GuiTestWindowMenus();
		void GuiTestWindowWindows();

		void GuiHelpMenus();
		void GuiLicensePopup();
		void GuiHelpVersionPopup();
		void GuiMenuBarAudioAndPerformanceDisplay();

	private:
		void BaseDispose();
		void DisposeSaveConfig();

	private:
		std::unique_ptr<ApplicationHost> host = nullptr;

		std::string fileToOpenOnStartup;

		// NOTE: Should probably be disabled for final release builds but needlessly adds a lot of closing latency
		const bool skipApplicationCleanup = true;

		// NOTE: Specifically to avoid exclusive mode taking over all system sounds while idle
		bool audioEngineRunningIdleOnFocusLost = false;

		bool exclusiveFullscreenGui = false;
		bool showMainMenuBar = true;

		bool showStyleEditor = false;
		bool showDemoWindow = false;

		LicenseWindow licenseWindow = {};
		bool versionWindowIsOpen = false;

		std::unique_ptr<Editor::EditorManager> editorManager = nullptr;
		std::vector<std::unique_ptr<BaseWindow>> dataTestComponents;
	};
}
