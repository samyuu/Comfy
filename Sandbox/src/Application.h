#pragma once
#include "Types.h"
#include "BaseWindow.h"
#include "Editor/Editor.h"
#include "Audio/AudioEngine.h"
#include "License/LicenseWindow.h"
#include "TimeSpan.h"
#include "Logger.h"
#include <glad/glad.h>
#include <glfw/glfw3.h>

constexpr float DEFAULT_WINDOW_WIDTH = 1280.0f;
constexpr float DEFAULT_WINDOW_HEIGHT = 720.0f;
constexpr const char* DEFAULT_WINDOW_TITLE = "Comfy Window";

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
	bool IsFullscreen();
	void SetFullscreen(bool value);
	void ToggleFullscreen();

	inline bool HasFocusBeenGained() { return focusGainedFrame; };
	inline bool HasFocusBeenLost() { return focusLostFrame; };
	// ----------------

	GLFWmonitor* GetActiveMonitor();
	void CheckConnectedDevices();

	bool GetDispatchFileDrop();
	void SetFileDropDispatched(bool value = true);
	const std::vector<std::string>* GetDroppedFiles();

private:
	// Base Methods
	// --------------

	// Initialize the application.
	int BaseInitialize();

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
	void InitializeGui();
	void InitializeApp();

	// Update Methods
	// --------------
	void UpdatePollInput();
	void UpdateInput();
	void UpdateTasks();

	// Draw Methods
	// ------------

	void DrawGui();

	void DrawGuiBaseWindowMenus(const char* header, std::vector<std::shared_ptr<BaseWindow>>& components);
	void DrawGuiBaseWindowWindows(std::vector<std::shared_ptr<BaseWindow>>& components);

	// Callbacks
	// ---------
	void MouseMoveCallback(int x, int y);
	void MouseScrollCallback(float offset);
	void WindowMoveCallback(int xPosition, int yPosition);
	void WindowResizeCallback(int width, int height);
	void WindowDropCallback(size_t count, const char* paths[]);
	void WindowFocusCallback(bool focused);

	// Member variables
	// -----------------
	bool hasBeenInitialized = false;
	bool hasBeenDisposed = false;
	bool mainLoopLowPowerSleep = false;

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
	float windowWidth = DEFAULT_WINDOW_WIDTH;
	float windowHeight = DEFAULT_WINDOW_HEIGHT;
	vec2 preFullScreenWindowPosition = { 0, 0 };
	vec2 preFullScreenWindowSize = { DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT };

	// Engine Timing
	// -------------
	TimeSpan elapsedTime = 0.0f;
	TimeSpan currentTime, lastTime;
	uint64_t elapsedFrames = 0;

	// ImGui Variables
	// ---------------
	LicenseWindow licenseWindow;
	bool showDemoWindow = true;
	bool showSwapInterval = true;
	// ---------------

	// Main Editor
	// -----------
	std::unique_ptr<Editor::PvEditor> pvEditor;
	// -----------------

	// Data Test Components
	// -----------------
	std::vector<std::shared_ptr<BaseWindow>> dataTestComponents;
	// -----------------

	static Application* globalCallbackApplication;
};
