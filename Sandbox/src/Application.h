#pragma once
#include "BaseWindow.h"
#include "Rendering/VertexArray.h";
#include "Rendering/VertexBuffer.h";
#include "Rendering/RenderTarget.h";
#include "Rendering/ComfyVertex.h"
#include "Rendering/Shader/ComfyShader.h"
#include "Rendering/Shader/LineShader.h"
#include "Rendering/Shader/ScreenShader.h"
#include "Rendering/Texture.h"
#include "Rendering/Camera.h"
#include "Audio/AudioEngine.h"

constexpr float DEFAULT_WINDOW_WIDTH = 1280.0f;
constexpr float DEFAULT_WINDOW_HEIGHT = 720.0f;
constexpr const char* DEFAULT_WINDOW_TITLE = "Comfy window";

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

	inline float GetElapsed() { return elapsedTime; };
	inline float GetWidth() { return windowWidth; };
	inline float GetHeight() { return windowHeight; };

	// Window Utilities
	// ----------------
	bool IsFullscreen();
	void SetFullscreen(bool value);
	void ToggleFullscreen();

	GLFWmonitor* GetActiveMonitor();

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
	void InitializeRom();
	void InitializeApp();

	// Update Methods
	// --------

	void UpdatePollInput();
	void UpdateInput();
	void UpdateTasks();

	// Draw Methods
	// -------

	void DrawScene();
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
	ImVec4 baseClearColor = { .12f, .12f, .12f, 1.0f };

	// Vertex Storage
	// --------------
	struct
	{
		VertexArray cubeVao;
		VertexArray lineVao;
		VertexArray screenVao;

		VertexBuffer cubeVertexBuffer;
		VertexBuffer lineVertexBuffer;
		VertexBuffer screenVertexBuffer;
	};

	// Textures
	// --------
	struct
	{
		Texture feelsBadManTexture;
		Texture goodNiceTexture;

		Texture groundTexture;
		Texture skyTexture;
		Texture tileTexture;
	};

	// Shaders
	// -------
	struct
	{
		ComfyShader comfyShader;
		LineShader lineShader;
		ScreenShader screenShader;
	};

	// Render Targets
	// --------------
	struct
	{
		RenderTarget sceneRenderTarget;
		RenderTarget postProcessingRenderTarget;
	};

	// Input / UI variables
	// --------------------
	int mouseBeingUsed = true, keyboardBeingUsed = true;
	int mouseX, mouseY;
	int mouseDeltaX, mouseDeltaY;
	int lastMouseX, lastMouseY;
	float mouseWheel, lastMouseWheel;
	bool mouseScrolledUp, mouseScrolledDown;

	// Window Management
	// -----------------
	bool windowFocused = true, lastWindowFocused, focusLostFrame = false, focusGainedFrame = false;
	bool renderWindowHidden, renderWindowHover, renderWindowTitleHover, renderWindowResized;
	ImVec2 renderWindowPos, renderWindowSize;
	ImVec2 lastRenderWindowPos, lastRenderWindowSize;

	int windowXPosition, windowYPosition;
	float windowWidth = DEFAULT_WINDOW_WIDTH;
	float windowHeight = DEFAULT_WINDOW_HEIGHT;
	vec2 preFullScreenWindowPosition = { 0, 0 };
	vec2 preFullScreenWindowSize = { DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT };

	// Engine Timing
	// -------------
	float elapsedTime = 0.0f;
	double currentTime, lastTime;
	unsigned __int64 elapsedFrames = 0;

	// Scene Camera
	// ------------
	float cameraSmoothness = 65.0f;
	float cameraPitch, cameraYaw = -90.0f, cameraRoll;
	float targetCameraPitch, targetCameraYaw = -90.0f;
	float cameraSensitivity = 0.25f;
	Camera camera;

	// Editor Components
	// -----------------
	std::vector<std::shared_ptr<BaseWindow>> editorComponents;
	// -----------------

	// Data Test Components
	// -----------------
	std::vector<std::shared_ptr<BaseWindow>> dataTestComponents;
	// -----------------

	// Dynamic Scene Data
	// ------------------
	vec3 cubePositions[10] =
	{
		vec3(+0.0f, +0.0f, -10.0f),
		vec3(+2.0f, +5.0f, -15.0f),
		vec3(-1.5f, -2.2f, -02.5f),
		vec3(-3.8f, -2.0f, -12.3f),
		vec3(+2.4f, -0.4f, -03.5f),
		vec3(-1.7f, +3.0f, -07.5f),
		vec3(+1.3f, -2.0f, -02.5f),
		vec3(+1.5f, +2.0f, -02.5f),
		vec3(+1.5f, +0.2f, -01.5f),
		vec3(-1.3f, +1.0f, -01.5f),
	};
	// ------------------

	// Static Vertex Data
	// ------------------
	const vec4 cubeColor = vec4(0.9f, 0.8f, 0.01f, 1.00f);
	ComfyVertex cubeVertices[36]
	{
		{ vec3(-0.5f, -0.5f, -0.5f), vec2(0.0f, 0.0f), cubeColor },
		{ vec3(+0.5f, -0.5f, -0.5f), vec2(1.0f, 0.0f), cubeColor },
		{ vec3(+0.5f, +0.5f, -0.5f), vec2(1.0f, 1.0f), cubeColor },
		{ vec3(+0.5f, +0.5f, -0.5f), vec2(1.0f, 1.0f), cubeColor },
		{ vec3(-0.5f, +0.5f, -0.5f), vec2(0.0f, 1.0f), cubeColor },
		{ vec3(-0.5f, -0.5f, -0.5f), vec2(0.0f, 0.0f), cubeColor },

		{ vec3(-0.5f, -0.5f, +0.5f), vec2(0.0f, 0.0f), cubeColor },
		{ vec3(+0.5f, -0.5f, +0.5f), vec2(1.0f, 0.0f), cubeColor },
		{ vec3(+0.5f, +0.5f, +0.5f), vec2(1.0f, 1.0f), cubeColor },
		{ vec3(+0.5f, +0.5f, +0.5f), vec2(1.0f, 1.0f), cubeColor },
		{ vec3(-0.5f, +0.5f, +0.5f), vec2(0.0f, 1.0f), cubeColor },
		{ vec3(-0.5f, -0.5f, +0.5f), vec2(0.0f, 0.0f), cubeColor },

		{ vec3(-0.5f, +0.5f, +0.5f), vec2(1.0f, 0.0f), cubeColor },
		{ vec3(-0.5f, +0.5f, -0.5f), vec2(1.0f, 1.0f), cubeColor },
		{ vec3(-0.5f, -0.5f, -0.5f), vec2(0.0f, 1.0f), cubeColor },
		{ vec3(-0.5f, -0.5f, -0.5f), vec2(0.0f, 1.0f), cubeColor },
		{ vec3(-0.5f, -0.5f, +0.5f), vec2(0.0f, 0.0f), cubeColor },
		{ vec3(-0.5f, +0.5f, +0.5f), vec2(1.0f, 0.0f), cubeColor },
													   
		{ vec3(+0.5f, +0.5f, +0.5f), vec2(1.0f, 0.0f), cubeColor },
		{ vec3(+0.5f, +0.5f, -0.5f), vec2(1.0f, 1.0f), cubeColor },
		{ vec3(+0.5f, -0.5f, -0.5f), vec2(0.0f, 1.0f), cubeColor },
		{ vec3(+0.5f, -0.5f, -0.5f), vec2(0.0f, 1.0f), cubeColor },
		{ vec3(+0.5f, -0.5f, +0.5f), vec2(0.0f, 0.0f), cubeColor },
		{ vec3(+0.5f, +0.5f, +0.5f), vec2(1.0f, 0.0f), cubeColor },
													   
		{ vec3(-0.5f, -0.5f, -0.5f), vec2(0.0f, 1.0f), cubeColor },
		{ vec3(+0.5f, -0.5f, -0.5f), vec2(1.0f, 1.0f), cubeColor },
		{ vec3(+0.5f, -0.5f, +0.5f), vec2(1.0f, 0.0f), cubeColor },
		{ vec3(+0.5f, -0.5f, +0.5f), vec2(1.0f, 0.0f), cubeColor },
		{ vec3(-0.5f, -0.5f, +0.5f), vec2(0.0f, 0.0f), cubeColor },
		{ vec3(-0.5f, -0.5f, -0.5f), vec2(0.0f, 1.0f), cubeColor },
													   
		{ vec3(-0.5f, +0.5f, -0.5f), vec2(0.0f, 1.0f), cubeColor },
		{ vec3(+0.5f, +0.5f, -0.5f), vec2(1.0f, 1.0f), cubeColor },
		{ vec3(+0.5f, +0.5f, +0.5f), vec2(1.0f, 0.0f), cubeColor },
		{ vec3(+0.5f, +0.5f, +0.5f), vec2(1.0f, 0.0f), cubeColor },
		{ vec3(-0.5f, +0.5f, +0.5f), vec2(0.0f, 0.0f), cubeColor },
		{ vec3(-0.5f, +0.5f, -0.5f), vec2(0.0f, 1.0f), cubeColor },
	};

	LineVertex axisVertices[6] =
	{
		// X-Axis
		{ vec3(0.0f, 0.0f, 0.0f), vec4(1.0f, 0.1f, 0.3f, 1.0f) },
		{ vec3(1.0f, 0.0f, 0.0f), vec4(1.0f, 0.1f, 0.3f, 1.0f) },

		// Y-Axis										 
		{ vec3(0.0f, 0.0f, 0.0f), vec4(0.2f, 1.0f, 0.1f, 1.0f) },
		{ vec3(0.0f, 1.0f, 0.0f), vec4(0.2f, 1.0f, 0.1f, 1.0f) },

		// Z-Axis										 
		{ vec3(0.0f, 0.0f, 0.0f), vec4(0.1f, 0.7f, 1.0f, 1.0f) },
		{ vec3(0.0f, 0.0f, 1.0f), vec4(0.1f, 0.7f, 1.0f, 1.0f) },
	};

	ScreenVertex screenVertices[6]
	{
		{ vec2(-1.0f, +1.0f),  vec2(0.0f, 1.0f) }, // top left
		{ vec2(+1.0f, +1.0f),  vec2(1.0f, 1.0f) }, // top right
		{ vec2(+1.0f, -1.0f),  vec2(1.0f, 0.0f) }, // bottom rights

		{ vec2(-1.0f, +1.0f),  vec2(0.0f, 1.0f) }, // top left
		{ vec2(+1.0f, -1.0f),  vec2(1.0f, 0.0f) }, // bottom right
		{ vec2(-1.0f, -1.0f),  vec2(0.0f, 0.0f) }, // bottom left
	};
	// ------------------

	static Application* globalCallbackApplication;
};
