#pragma once
#include "Types.h"
#include "ApplicationHost.h"
#include "BaseWindow.h"
#include "Editor/Core/EditorManager.h"
#include "License/LicenseWindow.h"
#include "ImGui/GuiRenderer.h"

// NOTE: Forward declare to avoid include polution
namespace App { class Engine; }

class Application
{
public:
	Application();
	Application(const Application&) = delete;
	Application& operator= (const Application&) = delete;
	~Application();

	// NOTE: Initialize and enter the main loop
	void Run();

	// NOTE: Break out of the main loop
	void Exit();

public:
	ApplicationHost& GetHost();

public:
	static constexpr const char* MainDockSpaceID = "MainDockSpace##Application";

private:
	// NOTE: Initialize the application
	bool BaseInitialize();

	// NOTE: Call update methods
	void BaseUpdate();

	// NOTE: Call draw methods
	void BaseDraw();

	// NOTE: Dispose the application
	void BaseDispose();

	// NOTE: Initialization
	bool InitializeLoadConfig();
	bool InitializeMountRomData();
	bool InitializeGuiRenderer();
	bool InitializeEditorComponents();

	// NOTE: Update methods
	void ProcessInput();
	void ProcessTasks();

	// NOTE: Draw methods
	void DrawGui();
	void DrawAppEngineWindow();
	void DrawAppEngineMenus(const char* header);

	void DrawGuiBaseWindowMenus(const char* header, const std::vector<UniquePtr<BaseWindow>>& components);
	void DrawGuiBaseWindowWindows(const std::vector<UniquePtr<BaseWindow>>& components);

	// NOTE: Dispose methods
	void DisposeUnmountRomData();
	void DisposeSaveConfig();
	void DisposeShutdownAudioEngine();

private:
	// NOTE: Core
	ApplicationHost host;
	Gui::GuiRenderer guiRenderer = { host };

	bool hasBeenInitialized = false;
	bool hasBeenDisposed = false;

	// NOTE: Should probably be disabled for final release builds but needlessly adds a lot of closing latency
	const bool skipApplicationCleanup = true;

	// NOTE: Gui
	bool showMainAppEngineWindow = false;
	bool exclusiveAppEngineWindow = false;
	bool showMainMenuBar = true;

	LicenseWindow licenseWindow;
	bool showStyleEditor = false;
	bool showDemoWindow = false;
	bool versionWindowOpen = false;

	UniquePtr<App::Engine> appEngine = nullptr;
	UniquePtr<Editor::EditorManager> editorManager = nullptr;
	std::vector<UniquePtr<BaseWindow>> dataTestComponents;
};
