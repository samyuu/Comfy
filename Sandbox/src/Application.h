#pragma once
#include "Types.h"
#include "BaseWindow.h"
#include "Editor/Editor.h"
#include "Audio/AudioEngine.h"
#include "Graphics/Graphics.h"
#include "App/Engine.h"
#include "License/LicenseWindow.h"
#include "TimeSpan.h"
#include "Logger.h"
#include <glfw/glfw3.h>

constexpr float DefaultWindowWidth = 1280.0f;
constexpr float DefaultWindowHeight = 720.0f;

constexpr int WindowWidthMin = static_cast<int>(640);
constexpr int WindowHeightMin = static_cast<int>(WindowWidthMin * (9.0f / 16.0f));

constexpr const char* DefaultWindowTitle = "Comfy Window";

class Application
{
public:
	Application();
	~Application();

	// Initialize and enter the main loop.
	void Run();

	// Break out of the main loop
	void Exit();

	inline GLFWwindow* GetWindow() { return window; };

	inline TimeSpan GetElapsed() { return elapsedTime; };
	inline float GetWidth() { return windowWidth; };
	inline float GetHeight() { return windowHeight; };

	// Window Utilities
	// ----------------
	bool IsFullscreen() const;
	void SetFullscreen(bool value);
	void ToggleFullscreen();

	inline bool HasFocusBeenGained() const { return focusGainedFrame; };
	inline bool HasFocusBeenLost() const { return focusLostFrame; };
	// ----------------

	GLFWmonitor* GetActiveMonitor() const;
	void CheckConnectedDevices();

	bool GetDispatchFileDrop();
	void SetFileDropDispatched(bool value = true);
	const std::vector<std::string>& GetDroppedFiles() const;

private:
	// Base Methods
	// --------------

	// Initialize the application.
	bool BaseInitialize();

	// Register window callbacks.
	void BaseRegister();

	// Call update methods.
	void BaseUpdate();

	// Call draw methods.
	void BaseDraw();

	// Dispose the application.
	void BaseDispose();

	// Initialization
	// --------------
	bool InitializeWindow();
	bool InitializeGui();
	bool InitializeApp();

	// Update Methods
	// --------------
	void UpdatePollInput();
	void UpdateInput();
	void UpdateTasks();

	// Draw Methods
	// ------------

	void DrawGui();
	void DrawAppEngineWindow();
	void DrawAppEngineMenus(const char* header);

	void DrawGuiBaseWindowMenus(const char* header, std::vector<RefPtr<BaseWindow>>& components);
	void DrawGuiBaseWindowWindows(std::vector<RefPtr<BaseWindow>>& components);

	// Callbacks
	// ---------
	void MouseMoveCallback(int x, int y);
	void MouseScrollCallback(float offset);
	void WindowMoveCallback(int xPosition, int yPosition);
	void WindowResizeCallback(int width, int height);
	void WindowDropCallback(size_t count, const char* paths[]);
	void WindowFocusCallback(bool focused);
	void WindowClosingCallback();

	// Member variables
	// -----------------
	bool hasBeenInitialized = false;
	bool hasBeenDisposed = false;
	bool mainLoopLowPowerSleep = false;
	bool skipApplicationCleanup = true;

	GLFWwindow *window = nullptr;

	// Input / UI variables
	// --------------------
	int mouseX, mouseY;
	int mouseDeltaX, mouseDeltaY;
	int lastMouseX, lastMouseY;
	float mouseWheel, lastMouseWheel;
	bool mouseScrolledUp, mouseScrolledDown;

	// Window Management
	// -----------------
	std::vector<std::string> droppedFiles;
	bool filesDroppedThisFrame, filesDropped, filesLastDropped, fileDropDispatched;
	bool windowFocused = true, lastWindowFocused, focusLostFrame = false, focusGainedFrame = false;

	int windowXPosition, windowYPosition;
	float windowWidth = DefaultWindowWidth;
	float windowHeight = DefaultWindowHeight;
	vec2 preFullScreenWindowPosition = { 0, 0 };
	vec2 preFullScreenWindowSize = { DefaultWindowWidth, DefaultWindowHeight };

	// Engine Timing
	// -------------
	TimeSpan elapsedTime = 0.0f;
	TimeSpan currentTime, lastTime;
	uint64_t elapsedFrames = 0;

	// ImGui Variables
	// ---------------
	bool showMainAppEngineWindow = false;
	bool exclusiveAppEngineWindow = false;
	bool showMainMenuBar = true;
	const char* dockSpaceID = "MainDockSpace##Application";

	LicenseWindow licenseWindow;
	bool showStyleEditor = false;
	bool showDemoWindow = false;
	bool showSwapInterval = true;
	// ---------------

	// App Engine
	// -----------
	UniquePtr<App::Engine> appEngine = nullptr;
	// -----------------

	// Main Editor
	// -----------
	UniquePtr<Editor::EditorManager> pvEditor = nullptr;
	// -----------------

	// Data Test Components
	// -----------------
	std::vector<RefPtr<BaseWindow>> dataTestComponents;
	// -----------------

	static Application* globalCallbackApplication;
	static Application* GetApplicationPointer(GLFWwindow* window);
};
