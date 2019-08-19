#include "Application.h"
#include "Editor/Core/Theme.h"
#include "DataTest/AudioTestWindow.h"
#include "DataTest/IconTestWindow.h"
#include "DataTest/InputTestWindow.h"
#include "DataTest/ShaderTestWindow.h"
#include "Input/DirectInput/DualShock4.h"
#include "Input/Keyboard.h"
#include "FileSystem/FileHelper.h"
#include "ImGui/Gui.h"
#include "ImGui/Implementation/Imgui_Impl.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include "../res/resource.h"
#include <glfw/glfw3native.h>

Application* Application::globalCallbackApplication;

static HMODULE GlobalModuleHandle = NULL;
static HICON GlobalIconHandle = NULL;

static void GlfwErrorCallback(int error, const char* description)
{
	Logger::LogErrorLine(__FUNCTION__"(): [GLFW Error: 0x%X] %s", error, description);
}

Application::Application()
{
}

Application::~Application()
{
	BaseDispose();
}

bool Application::IsFullscreen() const
{
	return glfwGetWindowMonitor(window) != nullptr;
}

void Application::SetFullscreen(bool value)
{
	if (IsFullscreen() == value)
		return;

	if (value)
	{
		preFullScreenWindowPosition = vec2(windowXPosition, windowYPosition);
		preFullScreenWindowSize = vec2(windowWidth, windowHeight);

		GLFWmonitor* monitor = GetActiveMonitor();
		const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);

		glfwSetWindowMonitor(window, monitor, 0, 0, videoMode->width, videoMode->height, videoMode->refreshRate);
	}
	else
	{
		// restore last window size and position
		glfwSetWindowMonitor(window, nullptr,
			static_cast<int>(preFullScreenWindowPosition.x),
			static_cast<int>(preFullScreenWindowPosition.y),
			static_cast<int>(preFullScreenWindowSize.x),
			static_cast<int>(preFullScreenWindowSize.y),
			GLFW_DONT_CARE);
	}
}

void Application::ToggleFullscreen()
{
	SetFullscreen(!IsFullscreen());
}

GLFWmonitor* Application::GetActiveMonitor() const
{
	GLFWmonitor* monitor = glfwGetWindowMonitor(window);

	if (monitor != nullptr)
		return monitor;

	int monitorCount;
	GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);

	int bestoverlap = 0;

	for (int i = 0; i < monitorCount; i++)
	{
		const GLFWvidmode* videoMode = glfwGetVideoMode(monitors[i]);

		int monitorX, monitorY;
		glfwGetMonitorPos(monitors[i], &monitorX, &monitorY);

		int overlap =
			glm::max(0, glm::min(static_cast<int>(windowXPosition + windowWidth), monitorX + videoMode->width) - glm::max(windowXPosition, monitorX)) *
			glm::max(0, glm::min(static_cast<int>(windowYPosition + windowHeight), monitorY + videoMode->height) - glm::max(windowYPosition, monitorY));

		if (bestoverlap < overlap)
		{
			bestoverlap = overlap;
			monitor = monitors[i];
		}
	}

	return monitor;
}

void Application::CheckConnectedDevices()
{
	if (!Keyboard::GetInstanceInitialized())
	{
		if (Keyboard::TryInitializeInstance(GetWindow()))
		{
			// Logger::LogLine(__FUNCTION__"(): Keyboard connected and initialized");
		}
	}

	if (!DualShock4::GetInstanceInitialized())
	{
		if (DualShock4::TryInitializeInstance())
		{
			// Logger::LogLine(__FUNCTION__"(): DualShock4 connected and initialized");
		}
	}
}

bool Application::GetDispatchFileDrop()
{
	return filesDroppedThisFrame && !fileDropDispatched;
}

void Application::SetFileDropDispatched(bool value)
{
	fileDropDispatched = value;
}

const std::vector<std::string>& Application::GetDroppedFiles() const
{
	return droppedFiles;
}

void Application::LoadComfyWindowIcon()
{
	assert(GlobalIconHandle == NULL);
	GlobalIconHandle = ::LoadIconA(GlobalModuleHandle, MAKEINTRESOURCE(COMFY_ICON));
}

void Application::SetComfyWindowIcon(GLFWwindow* window)
{
	assert(GlobalIconHandle != NULL);
	
	HWND windowHandle = glfwGetWin32Window(window);
	::SendMessageA(windowHandle, WM_SETICON, ICON_SMALL, (LPARAM)GlobalIconHandle);
	::SendMessageA(windowHandle, WM_SETICON, ICON_BIG, (LPARAM)GlobalIconHandle);
}

void Application::Run()
{
	if (!BaseInitialize())
		return;

	// glfwSwapInterval(0);
	while (!glfwWindowShouldClose(window))
	{
		BaseUpdate();
		BaseDraw();

		glfwSwapBuffers(window);
		glfwPollEvents();

		lastTime = currentTime;
		currentTime = TimeSpan::GetTimeNow();
		elapsedTime = (currentTime - lastTime);

		if (mainLoopLowPowerSleep)
			Sleep(1);
	}

	BaseDispose();
}

void Application::Exit()
{
	glfwSetWindowShouldClose(window, GLFW_TRUE);
}

bool Application::BaseInitialize()
{
	if (hasBeenInitialized)
		return false;

	hasBeenInitialized = true;

	glfwSetErrorCallback(&GlfwErrorCallback);

	int glfwInitResult = glfwInit();
	if (glfwInitResult == GLFW_FALSE)
		return false;

	GlobalModuleHandle = ::GetModuleHandleA(nullptr);

	if (!InitializeWindow())
		return false;

	glfwGetWindowPos(window, &windowXPosition, &windowYPosition);
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	BaseRegister();

	InitializeDirectInput(GlobalModuleHandle);
	CheckConnectedDevices();

	AudioEngine::CreateInstance();
	AudioEngine::InitializeInstance();

	InitializeGui();
	InitializeApp();

	return true;
}

void Application::BaseRegister()
{
	globalCallbackApplication = this;
	glfwSetWindowUserPointer(window, this);

	glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height)
	{
		GetApplicationPointer(window)->WindowResizeCallback(width, height);
	});

	glfwSetWindowPosCallback(window, [](GLFWwindow* window, int xPosition, int yPosition)
	{
		GetApplicationPointer(window)->WindowMoveCallback(xPosition, yPosition);
	});

	glfwSetCursorPosCallback(window, [](GLFWwindow* window, double x, double y)
	{
		GetApplicationPointer(window)->MouseMoveCallback((int)x, (int)y);
	});

	glfwSetScrollCallback(window, [](GLFWwindow* window, double xOffset, double yOffset)
	{
		GetApplicationPointer(window)->MouseScrollCallback((float)yOffset);
	});

	glfwSetDropCallback(window, [](GLFWwindow* window, int count, const char* paths[])
	{
		GetApplicationPointer(window)->WindowDropCallback(count, paths);
	});

	glfwSetWindowFocusCallback(window, [](GLFWwindow* window, int focused)
	{
		GetApplicationPointer(window)->WindowFocusCallback(focused);
	});

	glfwSetWindowCloseCallback(window, [](GLFWwindow* window)
	{
		GetApplicationPointer(window)->WindowClosingCallback();
	});

	glfwSetJoystickCallback([](int id, int event)
	{
		if (event == GLFW_CONNECTED || event == GLFW_DISCONNECTED)
			globalCallbackApplication->CheckConnectedDevices();
	});
}

void Application::BaseUpdate()
{
	filesDroppedThisFrame = filesDropped && !filesLastDropped;
	filesLastDropped = filesDropped;
	filesDropped = false;

	if (elapsedFrames > 2)
	{
		focusLostFrame = lastWindowFocused && !windowFocused;
		focusGainedFrame = !lastWindowFocused && windowFocused;
	}
	lastWindowFocused = windowFocused;

	UpdatePollInput();
	UpdateInput();
	UpdateTasks();

	elapsedFrames++;
}

void Application::BaseDraw()
{
	const ImVec4 baseClearColor = ImColor(Editor::GetColor(Editor::EditorColor_BaseClear));

	Graphics::RenderCommand::SetClearColor(baseClearColor);
	Graphics::RenderCommand::Clear(Graphics::ClearTarget_ColorBuffer);

	Graphics::RenderCommand::SetViewport(vec2(windowWidth, windowHeight));
	DrawGui();
}

void Application::BaseDispose()
{
	if (hasBeenDisposed)
		return;

	hasBeenDisposed = true;

	if (skipApplicationCleanup)
		return;

	// Force delete before the OpenGL context is destroyed
	pvEditor.reset();

	AudioEngine::DisposeInstance();
	AudioEngine::DeleteInstance();

	Keyboard::DeleteInstance();
	DualShock4::DeleteInstance();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	Gui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
}

bool Application::InitializeWindow()
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(static_cast<int>(windowWidth), static_cast<int>(windowHeight), DefaultWindowTitle, nullptr, nullptr);

	if (window == nullptr)
		return false;

	glfwSetWindowSizeLimits(window, WindowWidthMin, WindowHeightMin, GLFW_DONT_CARE, GLFW_DONT_CARE);

	Application::LoadComfyWindowIcon();
	Application::SetComfyWindowIcon(window);

	return true;
}

bool Application::InitializeGui()
{
	IMGUI_CHECKVERSION();
	Gui::CreateContext();

	ImGuiIO& io = Gui::GetIO();
	io.IniFilename = "ram/imgui.ini";
	io.LogFilename = "ram/imgui_log.txt";
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	// io.ConfigFlags |= ImGuiConfigFlags_NavNoCaptureKeyboard;
	io.ConfigDockingWithShift = false;
	io.ConfigViewportsNoDecoration = true;
	io.KeyRepeatDelay = 0.250f;
	io.KeyRepeatRate = 0.050f;
	io.ConfigWindowsMoveFromTitleBarOnly = true;
	io.ConfigDockingTabBarOnSingleWindows = true;

	ImFontAtlas* fonts = io.Fonts;
	constexpr float fontSize = 16.0f;

	// Load Text Font
	// --------------
	{
		fonts->AddFontFromFileTTF("rom/font/NotoSansCJKjp-Regular.otf", fontSize, nullptr, fonts->GetGlyphRangesJapanese());
	}
	// Load Icon Font
	// --------------
	{
		static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };

		ImFontConfig config;
		config.MergeMode = true;
		config.GlyphMinAdvanceX = 13.0f;

		fonts->AddFontFromFileTTF("rom/font/" FONT_ICON_FILE_NAME_FAS, fontSize - 2.0f, &config, icon_ranges);
	}

	Gui::StyleComfy();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 420");

	return true;
}

bool Application::InitializeApp()
{
	if (!FileSystem::DirectoryExists("rom"))
	{
		Logger::LogErrorLine(__FUNCTION__"(): Unable to locate rom directory");
		return false;
	}

	// Base PV Editor
	// --------------
	{
		pvEditor = MakeUnique<Editor::EditorManager>(this);
	}
	// Data Test Components
	// --------------------
	{
		dataTestComponents.reserve(4);
		dataTestComponents.push_back(MakeRefPtr<DataTest::InputTestWindow>(this));
		dataTestComponents.push_back(MakeRefPtr<DataTest::AudioTestWindow>(this));
		dataTestComponents.push_back(MakeRefPtr<DataTest::IconTestWindow>(this));
		dataTestComponents.push_back(MakeRefPtr<DataTest::ShaderTestWindow>(this));
	}

	return true;
}

void Application::UpdatePollInput()
{
	double doubleX, doubleY;
	glfwGetCursorPos(window, &doubleX, &doubleY);

	lastMouseX = mouseX;
	lastMouseY = mouseY;
	mouseX = static_cast<int>(doubleX);
	mouseY = static_cast<int>(doubleY);

	mouseDeltaX = mouseX - lastMouseX;
	mouseDeltaY = lastMouseY - mouseY;

	mouseScrolledUp = lastMouseWheel < mouseWheel;
	mouseScrolledDown = lastMouseWheel > mouseWheel;
	lastMouseWheel = mouseWheel;

	if (Keyboard::GetInstanceInitialized())
		Keyboard::GetInstance()->PollInput();

	if (DualShock4::GetInstanceInitialized())
	{
		if (!DualShock4::GetInstance()->PollInput())
		{
			DualShock4::DeleteInstance();
			Logger::LogLine(__FUNCTION__"(): DualShock4 connection lost");
		}
	}
}

void Application::UpdateInput()
{
	ImGuiIO& io = Gui::GetIO();

	if (Gui::IsKeyPressed(KeyCode_F11) || (io.KeyAlt && Gui::IsKeyPressed(KeyCode_Enter)))
		ToggleFullscreen();
}

void Application::UpdateTasks()
{
}

void Application::DrawGui()
{
	ImGuiIO& io = Gui::GetIO();

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	Gui::NewFrame();
	Gui::UpdateExtendedState();
	{
		//if (Gui::IsKeyPressed(KeyCode_F9))
		//	showMainMenuBar ^= true;

		if (Gui::IsKeyPressed(KeyCode_F10))
		{
			showMainAppEngineWindow = true;
			exclusiveAppEngineWindow ^= true;
		}

		// Main Menu Bar
		// -------------
		if (showMainMenuBar && !exclusiveAppEngineWindow && Gui::BeginMainMenuBar())
		{
			if (Gui::BeginMenu("Debug"))
			{
				Gui::PushStyleColor(ImGuiCol_Text, Gui::GetStyleColorVec4(ImGuiCol_PlotHistogramHovered));
				if (Gui::MenuItem("Recompile Shaders", nullptr))
					Graphics::ShaderProgram::RecompileAllShaders();
				Gui::PopStyleColor();

				if (Gui::MenuItem("Toggle Fullscreen", nullptr))
					ToggleFullscreen();

				if (Gui::BeginMenu("Swap Interval", &showSwapInterval))
				{
					if (Gui::MenuItem("glfwSwapInterval(0)", nullptr))
						glfwSwapInterval(0);

					if (Gui::MenuItem("glfwSwapInterval(1)", nullptr))
						glfwSwapInterval(1);

					Gui::EndMenu();
				}

				if (Gui::MenuItem("Test Print", nullptr))
					Logger::LogLine(__FUNCTION__"(): Test");

				Gui::Separator();

				if (Gui::MenuItem("Exit...", nullptr))
					Exit();

				Gui::EndMenu();
			}

			// Editor Menus Items
			// ------------------
			pvEditor->DrawGuiMenuItems();

			// App Engine Menu Items
			// ---------------------
			DrawAppEngineMenus("Engine");

			// Data Test Menus
			// ---------------
			DrawGuiBaseWindowMenus("Data Test", dataTestComponents);

			if (Gui::BeginMenu(u8"UTF8 Test"))
			{
				if (Gui::MenuItem(u8"test - テスト", nullptr)) { ; }
				if (Gui::MenuItem(u8"shinitai - 死にたい", nullptr)) { ; }
				Gui::EndMenu();
			}

			bool openLicensePopup = false;
			if (Gui::BeginMenu("Help"))
			{
				if (Gui::MenuItem("License"))
					openLicensePopup = true;
				Gui::EndMenu();
			}

			if (openLicensePopup)
			{
				*licenseWindow.GetIsWindowOpen() = true;
				Gui::OpenPopup(licenseWindow.GetWindowName());
			}

			if (Gui::BeginPopupModal(licenseWindow.GetWindowName(), licenseWindow.GetIsWindowOpen(), ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
			{
				ImGuiViewport* viewPort = Gui::GetMainViewport();
				ImGuiWindow* window = Gui::FindWindowByName(licenseWindow.GetWindowName());
				Gui::SetWindowPos(window, viewPort->Pos + viewPort->Size / 8, ImGuiCond_Always);
				Gui::SetWindowSize(window, viewPort->Size * .75f, ImGuiCond_Always);

				licenseWindow.DrawGui();

				if (Gui::IsKeyPressed(KeyCode_Escape))
					Gui::CloseCurrentPopup();

				Gui::EndPopup();
			}

			if (false && Gui::MenuItem("Open Popup", nullptr))
				Gui::OpenPopup("Test Popup");

			if (Gui::BeginPopupModal("Test Popup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
				Gui::Text("Test!\n\n");
				if (Gui::Button("OK", ImVec2(120, 0))) { Gui::CloseCurrentPopup(); }
				Gui::SameLine();
				if (Gui::Button("Cancel", ImVec2(120, 0))) { Gui::CloseCurrentPopup(); }
				Gui::EndPopup();
			}

			if (focusLostFrame && false)
				Gui::OpenPopup("PeepoSleep zzzZZZ");

			if (Gui::BeginPopupModal("PeepoSleep zzzZZZ", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
				Gui::Text("Window focus has been lost");
				if (focusGainedFrame)
					Gui::CloseCurrentPopup();
				Gui::EndPopup();
			}

			char infoBuffer[32];
			sprintf_s(infoBuffer, sizeof(infoBuffer), "%.3f ms (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

			Gui::SetCursorPosX(Gui::GetWindowWidth() - Gui::CalcTextSize(infoBuffer).x - Gui::GetStyle().WindowPadding.x);
			Gui::Text(infoBuffer);

			Gui::EndMainMenuBar();
		}

		// Window Dockspace
		// ----------------
		{
			Gui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			Gui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			Gui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

			ImGuiViewport* viewport = Gui::GetMainViewport();
			Gui::SetNextWindowPos(viewport->Pos);
			Gui::SetNextWindowSize(viewport->Size);
			Gui::SetNextWindowViewport(viewport->ID);

			ImGuiWindowFlags dockspaceWindowFlags = ImGuiWindowFlags_NoDocking;
			dockspaceWindowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			dockspaceWindowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
			dockspaceWindowFlags |= ImGuiWindowFlags_NoBackground;
			if (showMainMenuBar)
				dockspaceWindowFlags |= ImGuiWindowFlags_MenuBar;

			Gui::Begin(dockSpaceID, nullptr, dockspaceWindowFlags);
			ImGuiID dockspaceID = Gui::GetID(dockSpaceID);
			Gui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
			Gui::End();

			Gui::PopStyleVar(3);
		}

		// App Engine Window
		// -----------------
		if (exclusiveAppEngineWindow)
		{
			ImGuiViewport* viewport = Gui::GetMainViewport();
			Gui::SetNextWindowPos(viewport->Pos);
			Gui::SetNextWindowSize(viewport->Size);
			Gui::SetNextWindowViewport(viewport->ID);

			ImGuiWindowFlags engineWindowFlags = ImGuiWindowFlags_None;
			engineWindowFlags |= ImGuiWindowFlags_NoDocking;
			engineWindowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			engineWindowFlags |= ImGuiWindowFlags_NoNavFocus;
			engineWindowFlags |= ImGuiWindowFlags_NoSavedSettings;

			if (showMainAppEngineWindow)
			{
				Editor::RenderWindowBase::PushWindowPadding();
				if (Gui::Begin("App::Engine::Window##Application", &showMainAppEngineWindow, engineWindowFlags))
					DrawAppEngineWindow();
				Gui::End();
				Editor::RenderWindowBase::PopWindowPadding();
			}
		}
		else
		{
			// App Engine Window
			// -----------------
			if (showMainAppEngineWindow)
			{
				Editor::RenderWindowBase::PushWindowPadding();
				if (Gui::Begin("Engine Window##Application", &showMainAppEngineWindow, ImGuiWindowFlags_None))
					DrawAppEngineWindow();
				Gui::End();
				Editor::RenderWindowBase::PopWindowPadding();
			}

			// Style Editor
			// ------------
			if (showStyleEditor)
			{
				Gui::Begin("Style Editor##Application", &showStyleEditor);
				Gui::ShowStyleEditor();
				Gui::End();
			}

			// Demo Window
			// -----------
			if (showDemoWindow)
			{
				Gui::ShowDemoWindow(&showDemoWindow);
			}

			// Editor Windows
			// --------------
			pvEditor->DrawGuiWindows();

			// Data Test Windows
			// -----------------
			DrawGuiBaseWindowWindows(dataTestComponents);
		}
	}
	Gui::Render();

	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		GLFWwindow* context = glfwGetCurrentContext();
		Gui::UpdatePlatformWindows();
		Gui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(context);
	}

	ImGui_ImplOpenGL3_RenderDrawData(Gui::GetDrawData());
}

void Application::DrawAppEngineWindow()
{
	if (appEngine == nullptr)
		appEngine = MakeUnique<App::Engine>();

	appEngine->Tick();
}

void Application::DrawAppEngineMenus(const char* header)
{
	if (Gui::BeginMenu(header))
	{
		Gui::MenuItem("Engine Window", nullptr, &showMainAppEngineWindow);
		Gui::EndMenu();
	}
}

void Application::DrawGuiBaseWindowMenus(const char* header, std::vector<RefPtr<BaseWindow>>& components)
{
	if (Gui::BeginMenu(header))
	{
		DEBUG_ONLY(Gui::MenuItem("Style Editor", nullptr, &showStyleEditor));
		DEBUG_ONLY(Gui::MenuItem("Demo Window", nullptr, &showDemoWindow));

		for (const auto& component : components)
			Gui::MenuItem(component->GetGuiName(), nullptr, component->GetIsGuiOpenPtr());

		Gui::EndMenu();
	}
}

void Application::DrawGuiBaseWindowWindows(std::vector<RefPtr<BaseWindow>>& components)
{
	for (const auto& component : components)
	{
		if (*component->GetIsGuiOpenPtr())
		{
			if (Gui::Begin(component->GetGuiName(), component->GetIsGuiOpenPtr(), component->GetWindowFlags()))
				component->DrawGui();
			Gui::End();
		}
	}
}

void Application::MouseMoveCallback(int x, int y)
{

}

void Application::MouseScrollCallback(float offset)
{
	mouseWheel += offset;
}

void Application::WindowMoveCallback(int xPosition, int yPosition)
{
	windowXPosition = xPosition;
	windowYPosition = yPosition;
}

void Application::WindowResizeCallback(int width, int height)
{
	windowWidth = static_cast<float>(width);
	windowHeight = static_cast<float>(height);

	Graphics::RenderCommand::SetViewport(width, height);
}

void Application::WindowDropCallback(size_t count, const char* paths[])
{
	fileDropDispatched = false;
	filesDropped = true;

	droppedFiles.clear();
	droppedFiles.reserve(count);

	for (size_t i = 0; i < count; i++)
		droppedFiles.emplace_back(paths[i]);
}

void Application::WindowFocusCallback(bool focused)
{
	windowFocused = focused;
}

void Application::WindowClosingCallback()
{
	AudioEngine* audioEngine = AudioEngine::GetInstance();

	if (audioEngine != nullptr)
		audioEngine->StopStream();
}

Application* Application::GetApplicationPointer(GLFWwindow* window)
{
	return static_cast<Application*>(glfwGetWindowUserPointer(window));
}
