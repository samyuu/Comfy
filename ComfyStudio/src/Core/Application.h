#pragma once
#include "Types.h"
#include "ApplicationHost.h"
#include "BaseWindow.h"
#include "Editor/Core/Editor.h"
#include "Audio/Core/AudioEngine.h"
#include "Graphics/Graphics.h"
#include "App/Engine.h"
#include "License/LicenseWindow.h"
#include "ImGui/GuiRenderer.h"

class Application
{
public:
	Application();
	Application(const Application&) = delete;
	Application& operator= (const Application&) = delete;
	~Application();

	// NOTE: Initialize and enter the main loop.
	void Run();

	// NOTE: Break out of the main loop
	void Exit();

public:
	ApplicationHost& GetHost();

private:
	// NOTE: Initialize the application
	bool BaseInitialize();

	// NOTE: Call update methods.
	void BaseUpdate();

	// NOTE: Call draw methods.
	void BaseDraw();

	// NOTE: Dispose the application
	void BaseDispose();

	// NOTE: Initialization
	bool InitializeCheckRom();
	bool InitializeGuiRenderer();
	bool InitializeEditorComponents();

	// NOTE: Update methods
	void ProcessInput();
	void ProcessTasks();

	// NOTE: Draw methods
	void DrawGui();
	void DrawAppEngineWindow();
	void DrawAppEngineMenus(const char* header);

	void DrawGuiBaseWindowMenus(const char* header, const std::vector<RefPtr<BaseWindow>>& components);
	void DrawGuiBaseWindowWindows(const std::vector<RefPtr<BaseWindow>>& components);

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
	bool showSwapInterval = false;
	bool versionWindowOpen = false;

	UniquePtr<App::Engine> appEngine = nullptr;
	UniquePtr<Editor::EditorManager> editorManager = nullptr;
	std::vector<RefPtr<BaseWindow>> dataTestComponents;

	static constexpr const char* mainDockSpaceID = "MainDockSpace##Application";
};
