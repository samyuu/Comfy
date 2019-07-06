#include "pch.h"
#include "Application.h"
#include "Editor/Theme.h"
#include "DataTest/InputTestWindow.h"
#include "DataTest/AudioTestWindow.h"
#include "DataTest/IconTestWindow.h"
#include "Input/DirectInput/DualShock4.h"
#include "Input/Keyboard.h"

Application* Application::globalCallbackApplication;

Application::Application()
{
}

Application::~Application()
{
	BaseDispose();
}

bool Application::IsFullscreen()
{
	return glfwGetWindowMonitor(window) != nullptr;
}

void Application::SetFullscreen(bool value)
{
	if (IsFullscreen() == value)
		return;

	if (value)
	{
		preFullScreenWindowPosition = { (float)windowXPosition, (float)windowYPosition };
		preFullScreenWindowSize = { (float)windowWidth, (float)windowHeight };

		GLFWmonitor* monitor = GetActiveMonitor();
		const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);

		glfwSetWindowMonitor(window, monitor, 0, 0, videoMode->width, videoMode->height, videoMode->refreshRate);
	}
	else
	{
		// restore last window size and position
		glfwSetWindowMonitor(window, nullptr,
			(int)preFullScreenWindowPosition.x,
			(int)preFullScreenWindowPosition.y,
			(int)preFullScreenWindowSize.x,
			(int)preFullScreenWindowSize.y,
			NULL);
	}
}

void Application::ToggleFullscreen()
{
	SetFullscreen(!IsFullscreen());
}

GLFWmonitor* Application::GetActiveMonitor()
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
			__max(0, __min(windowXPosition + windowWidth, monitorX + videoMode->width) - __max(windowXPosition, monitorX)) *
			__max(0, __min(windowYPosition + windowHeight, monitorY + videoMode->height) - __max(windowYPosition, monitorY));

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
			//Logger::Log(__FUNCTION__"(): Keyboard connected and initialized\n");
		}
	}

	if (!DualShock4::GetInstanceInitialized())
	{
		if (DualShock4::TryInitializeInstance())
		{
			//Logger::Log(__FUNCTION__"(): DualShock4 connected and initialized\n");
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

const std::vector<std::string>* Application::GetDroppedFiles()
{
	return &droppedFiles;
}

void Application::Run()
{
	BaseInitialize();

	//glfwSwapInterval(0);
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

int Application::BaseInitialize()
{
	if (hasBeenInitialized)
		return -1;
	hasBeenInitialized = true;

	int glfwInitResult = glfwInit();
	assert(glfwInitResult == GLFW_TRUE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(windowWidth, windowHeight, DEFAULT_WINDOW_TITLE, nullptr, nullptr);
	glfwGetWindowPos(window, &windowXPosition, &windowYPosition);

	assert(window != nullptr);

	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	BaseRegister();

	HRESULT directInputResult = InitializeDirectInput(GetModuleHandle(nullptr));
	assert(!FAILED(directInputResult));
	CheckConnectedDevices();

	AudioEngine::CreateInstance();
	AudioEngine::InitializeInstance();

	InitializeGui();
	InitializeApp();

	return 0;
}

void Application::BaseRegister()
{
	globalCallbackApplication = this;

	glfwSetWindowSizeCallback(window, [](GLFWwindow*, int width, int height)
	{
		globalCallbackApplication->WindowResizeCallback(width, height);
	});

	glfwSetWindowPosCallback(window, [](GLFWwindow*, int xPosition, int yPosition)
	{
		globalCallbackApplication->WindowMoveCallback(xPosition, yPosition);
	});

	glfwSetCursorPosCallback(window, [](GLFWwindow*, double x, double y)
	{
		globalCallbackApplication->MouseMoveCallback((int)x, (int)y);
	});

	glfwSetScrollCallback(window, [](GLFWwindow*, double xOffset, double yOffset)
	{
		globalCallbackApplication->MouseScrollCallback((float)yOffset);
	});

	glfwSetDropCallback(window, [](GLFWwindow*, int count, const char* paths[])
	{
		globalCallbackApplication->WindowDropCallback(count, paths);
	});

	glfwSetWindowFocusCallback(window, [](GLFWwindow*, int focused)
	{
		globalCallbackApplication->WindowFocusCallback(focused);
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

	GLCall(glClearColor(baseClearColor.x, baseClearColor.y, baseClearColor.z, baseClearColor.w));
	GLCall(glClear(GL_COLOR_BUFFER_BIT));

	GLCall(glViewport(0, 0, windowWidth, windowHeight));
	DrawGui();
}

void Application::BaseDispose()
{
	if (hasBeenDisposed)
		return;
	hasBeenDisposed = true;

	AudioEngine::DisposeInstance();
	AudioEngine::DeleteInstance();

	Keyboard::DeleteInstance();
	DualShock4::DeleteInstance();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
}

void Application::InitializeGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = "ram/imgui.ini";
	io.LogFilename = "ram/imgui_log.txt";
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;
	io.KeyRepeatDelay = 0.500f;
	io.KeyRepeatRate = 0.075f;
	io.ConfigWindowsMoveFromTitleBarOnly = true;
	io.ConfigDockingTabBarOnSingleWindows = true;

	ImFontAtlas* fonts = io.Fonts;
	constexpr float fontSize = 16.0f;

	// Load Text Font
	// --------------
	{
		const bool loadJapaneseRange = false;
		fonts->AddFontFromFileTTF("rom/font/NotoSansCJKjp-Regular.otf", fontSize, nullptr, loadJapaneseRange ? fonts->GetGlyphRangesJapanese() : fonts->GetGlyphRangesDefault());
	}
	// Load Icon Font
	// --------------
	{
		static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };

		ImFontConfig config;
		config.MergeMode = true;
		config.GlyphMinAdvanceX = 13.0f;

		// Load Time: 0.05 MS
		//fonts->AddFontFromFileTTF("rom/font/" FONT_ICON_FILE_NAME_FAR, fontSize - 2.0f, &config, icon_ranges); 
		
		// Load Time: 0.10 MS
		fonts->AddFontFromFileTTF("rom/font/" FONT_ICON_FILE_NAME_FAS, fontSize - 2.0f, &config, icon_ranges); 
	}

	ImGui::StyleComfy();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 420");
}

void Application::InitializeApp()
{
	// Base PV Editor
	// --------------
	{
		pvEditor = std::make_unique<Editor::PvEditor>(this);
	}
	// Data Test Components
	// --------------------
	{
		dataTestComponents.reserve(2);
		dataTestComponents.push_back(std::make_shared<InputTestWindow>(this));
		dataTestComponents.push_back(std::make_shared<AudioTestWindow>(this));
		dataTestComponents.push_back(std::make_shared<IconTestWindow>(this));
	}
}

void Application::UpdatePollInput()
{
	double doubleX, doubleY;
	glfwGetCursorPos(window, &doubleX, &doubleY);

	lastMouseX = mouseX;
	lastMouseY = mouseY;
	mouseX = (int)doubleX;
	mouseY = (int)doubleY;

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
			Logger::LogLine("Application::UpdatePollInput(): DualShock4 connection lost");
		}
	}
}

void Application::UpdateInput()
{
	if (Keyboard::IsTapped(GLFW_KEY_F11))
		ToggleFullscreen();
}

void Application::UpdateTasks()
{
}

void Application::DrawGui()
{
	ImGuiIO& io = ImGui::GetIO();

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	{
		// Main Menu Bar
		// -------------
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Debug"))
			{
				if (ImGui::MenuItem("Toggle Fullscreen", nullptr))
					ToggleFullscreen();

				if (ImGui::BeginMenu("Swap Interval", &showSwapInterval))
				{
					if (ImGui::MenuItem("glfwSwapInterval(0)", nullptr))
						glfwSwapInterval(0);

					if (ImGui::MenuItem("glfwSwapInterval(1)", nullptr))
						glfwSwapInterval(1);

					ImGui::EndMenu();
				}

				if (ImGui::MenuItem("Test Print", nullptr))
					Logger::LogLine("DrawGui(): Test");

				ImGui::MenuItem("Show Demo Window", nullptr, &showDemoWindow);
				ImGui::Separator();

				if (ImGui::MenuItem("Exit...", nullptr))
					Exit();

				ImGui::EndMenu();
			}

			// Editor Menus Items
			// ------------------
			pvEditor->DrawGuiMenuItems();

			// Data Test Menus
			// ---------------
			DrawGuiBaseWindowMenus("Data Test", dataTestComponents);

			if (ImGui::BeginMenu(u8"UTF8 Test"))
			{
				if (ImGui::MenuItem(u8"test - ƒeƒXƒg", nullptr)) { ; }
				if (ImGui::MenuItem(u8"shinitai - Ž€‚É‚½‚¢", nullptr)) { ; }
				ImGui::EndMenu();
			}

			bool openLicensePopup = false;
			if (ImGui::BeginMenu("Help"))
			{
				if (ImGui::MenuItem("License"))
					openLicensePopup = true;
				ImGui::EndMenu();
			}

			if (openLicensePopup)
			{
				*licenseWindow.GetIsWindowOpen() = true;
				ImGui::OpenPopup(licenseWindow.GetWindowName());
			}

			if (ImGui::BeginPopupModal(licenseWindow.GetWindowName(), licenseWindow.GetIsWindowOpen(), ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
			{
				auto viewPortPos = ImVec2(windowXPosition, windowYPosition);
				auto viewPortSize = ImVec2(windowWidth, windowHeight);
				
				ImGuiWindow* window = ImGui::FindWindowByName(licenseWindow.GetWindowName());
				ImGui::SetWindowPos(window, viewPortPos + viewPortSize / 8, ImGuiCond_Always);
				ImGui::SetWindowSize(window, viewPortSize * .75f, ImGuiCond_Always);

				licenseWindow.DrawGui();

				if (ImGui::IsKeyPressed(GLFW_KEY_ESCAPE))
					ImGui::CloseCurrentPopup();

				ImGui::EndPopup();
			}

			if (false && ImGui::MenuItem("Open Popup", nullptr))
				ImGui::OpenPopup("Test Popup");

			if (ImGui::BeginPopupModal("Test Popup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text("Test!\n\n");
				if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
				ImGui::SameLine();
				if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
				ImGui::EndPopup();
			}

			if (focusLostFrame && false)
				ImGui::OpenPopup("PeepoSleep zzzZZZ");

			if (ImGui::BeginPopupModal("PeepoSleep zzzZZZ", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text("Window focus has been lost");
				if (focusGainedFrame)
					ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
			}

			char infoBuffer[32];
			sprintf_s(infoBuffer, sizeof(infoBuffer), "%.3f ms (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

			ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::CalcTextSize(infoBuffer).x - ImGui::GetStyle().WindowPadding.x);
			ImGui::Text(infoBuffer);

			ImGui::EndMainMenuBar();
		}

		// Window Dockspace
		// ----------------
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);

			ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
			window_flags |= ImGuiWindowFlags_NoBackground;

			ImGui::Begin("main_docking_space", nullptr, window_flags);
			ImGuiID dockspace_id = ImGui::GetID("main_docking_space");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
			ImGui::End();

			ImGui::PopStyleVar(3);
		}

		// Demo Window
		// -----------
		if (showDemoWindow)
			ImGui::ShowDemoWindow(&showDemoWindow);

		// Editor Windows
		// --------------
		pvEditor->DrawGuiWindows();

		// Data Test Windows
		// -----------------
		DrawGuiBaseWindowWindows(dataTestComponents);
	}
	ImGui::Render();

	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		GLFWwindow* context = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(context);
	}

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Application::DrawGuiBaseWindowMenus(const char* header, std::vector<std::shared_ptr<BaseWindow>>& components)
{
	if (ImGui::BeginMenu(header))
	{
		for (const auto& component : components)
			ImGui::MenuItem(component->GetGuiName(), nullptr, component->GetIsGuiOpenPtr());
		ImGui::EndMenu();
	}
}

void Application::DrawGuiBaseWindowWindows(std::vector<std::shared_ptr<BaseWindow>>& components)
{
	for (const auto& component : components)
	{
		if (*component->GetIsGuiOpenPtr())
		{
			if (ImGui::Begin(component->GetGuiName(), component->GetIsGuiOpenPtr(), component->GetWindowFlags()))
				component->DrawGui();
			ImGui::End();
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
	windowWidth = (float)width;
	windowHeight = (float)height;

	GLCall(glViewport(0, 0, width, height));
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