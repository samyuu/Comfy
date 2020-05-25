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
		static constexpr std::string_view ComfyStudioWindowTitle = "Comfy Studio";
		static constexpr std::string_view CopyrightNotice = "Copyright (C) 2020 Samyuu";

	public:
		Application() = default;
		~Application() = default;

		// NOTE: Initialize and enter the main loop
		void Run();

		// NOTE: Break out of the main loop
		void Exit();

	public:
		ApplicationHost& GetHost();

	private:
		ApplicationHost::ConstructionParam CreateHostParam();

	public:
		bool BaseInitialize();
		bool InitializeEditorComponents();

	private:
		void Gui();

		void GuiMainMenuBar();
		void GuiDebugMenu();
		void GuiAppEngineWindow();
		void GuiAppEngineMenus();

		void GuiBaseWindowMenus(const std::vector<std::unique_ptr<BaseWindow>>& components);
		void GuiBaseWindowWindows(const std::vector<std::unique_ptr<BaseWindow>>& components);

		void GuiHelpMenus();
		void GuiHelpVersionWindow();
		void GuiMenuBarPerformanceDisplay();

	private:
		void BaseDispose();
		void DisposeSaveConfig();

	private:
		std::unique_ptr<ApplicationHost> host = nullptr;

		// NOTE: Should probably be disabled for final release builds but needlessly adds a lot of closing latency
		const bool skipApplicationCleanup = true;

		bool showMainAppEngineWindow = false;
		bool exclusiveAppEngineWindow = false;
		bool showMainMenuBar = true;

		LicenseWindow licenseWindow;
		bool showStyleEditor = false;
		bool showDemoWindow = false;
		bool versionWindowOpen = false;

		std::unique_ptr<Editor::EditorManager> editorManager = nullptr;
		std::vector<std::unique_ptr<BaseWindow>> dataTestComponents;
	};
}
