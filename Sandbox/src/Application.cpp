#include "Application.h"
#include "pch.h"
#include "DataTest/InputTestWindow.h"
#include "DataTest/AudioTestWindow.h"
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
			//printf("Application::CheckConnectedDevices(): Keyboard connected and initialized\n");
		}
	}

	if (!DualShock4::GetInstanceInitialized())
	{
		if (DualShock4::TryInitializeInstance())
		{
			//printf("Application::CheckConnectedDevices(): DualShock4 connected and initialized\n");
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
		return 0;
	hasBeenInitialized = true;

	assert(glfwInit());

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

	InitializeRom();
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

	camera.AspectRatio = (renderWindowSize.x == 0 || renderWindowSize.y == 0) ? (GetWidth() / GetHeight()) : (renderWindowSize.x / renderWindowSize.y);
	camera.Update();

	elapsedFrames++;
}

void Application::BaseDraw()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(baseClearColor.x, baseClearColor.y, baseClearColor.z, baseClearColor.w);
	DrawScene();

	glViewport(0, 0, windowWidth, windowHeight);
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

void Application::InitializeRom()
{
	feelsBadManTexture.LoadFromFile("rom/spr/FeelsBadMan.png");
	goodNiceTexture.LoadFromFile("rom/spr/GoodNiceOne.png");

	groundTexture.LoadFromFile("rom/spr/stgtst007_ground.png");
	skyTexture.LoadFromFile("rom/spr/stgtst007_sky.png");
	tileTexture.LoadFromFile("rom/spr/stgtst007_tile.png");
}

void Application::InitializeGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = "ram/imgui.ini";
	io.LogFilename = "ram/imgui_log.txt";
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.KeyRepeatDelay = 0.500f;
	io.KeyRepeatRate = 0.075f;
	io.ConfigWindowsMoveFromTitleBarOnly = true;
	io.ConfigDockingTabBarOnSingleWindows = true;

	const bool loadJapaneseRange = false;
	ImFontAtlas* fonts = io.Fonts;
	fonts->AddFontFromFileTTF("rom/font/NotoSansCJKjp-Regular.otf", 16.0f, nullptr, loadJapaneseRange ? fonts->GetGlyphRangesJapanese() : fonts->GetGlyphRangesDefault());

	ImGui::StyleComfy();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 420");
}

void Application::InitializeApp()
{
	// Cube Vertex Data
	// ----------------
	{
		comfyShader.Initialize();

		cubeVertexBuffer.Initialize();
		cubeVertexBuffer.Bind();
		cubeVertexBuffer.BufferData(cubeVertices, sizeof(cubeVertices), BufferUsage::StaticDraw);

		BufferLayout layout =
		{
			{ ShaderDataType::Vec3, "in_position" },
			{ ShaderDataType::Vec2, "in_texture_coords" },
			{ ShaderDataType::Vec4, "in_color" },
		};

		cubeVao.Initialize();
		cubeVao.Bind();
		cubeVao.SetLayout(layout);
	}

	// Line Vertex Data
	// ----------------
	{
		lineShader.Initialize();

		lineVertexBuffer.Initialize();
		lineVertexBuffer.Bind();
		lineVertexBuffer.BufferData(axisVertices, sizeof(axisVertices), BufferUsage::StaticDraw);

		BufferLayout layout =
		{
			{ ShaderDataType::Vec3, "in_position" },
			{ ShaderDataType::Vec4, "in_color" },
		};

		lineVao.Initialize();
		lineVao.Bind();
		lineVao.SetLayout(layout);
	}

	// Screen Vertex Data
	// ------------------
	{
		screenShader.Initialize();

		screenVertexBuffer.Initialize();
		screenVertexBuffer.Bind();
		screenVertexBuffer.BufferData(screenVertices, sizeof(screenVertices), BufferUsage::StaticDraw);

		BufferLayout layout =
		{
			{ ShaderDataType::Vec2, "in_position" },
			{ ShaderDataType::Vec2, "in_texture_coords" },
		};

		screenVao.Initialize();
		screenVao.Bind();
		screenVao.SetLayout(layout);
	}

	// Render Targets
	// --------------
	{
		sceneRenderTarget.Initialize(GetWidth(), GetHeight());
		postProcessingRenderTarget.Initialize(GetWidth(), GetHeight());
	}

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	//glFrontFace(GL_CCW);
	//glCullFace(GL_BACK);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);
	//glFrontFace(GL_CCW);

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

	glm::vec3 front;
	front.x = cos(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
	front.y = sin(glm::radians(cameraPitch));
	front.z = sin(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));

	const bool fastCamera = Keyboard::IsDown(GLFW_KEY_LEFT_SHIFT);
	const bool slowCamera = Keyboard::IsDown(GLFW_KEY_LEFT_ALT);

	const float cameraSpeed = (slowCamera ? 0.25f : (fastCamera ? 5.5f : 2.25f)) * elapsedTime.TotalSeconds();

	if (!mouseBeingUsed)
	{
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT))
		{
			targetCameraYaw += mouseDeltaX * cameraSensitivity;
			targetCameraPitch += mouseDeltaY * cameraSensitivity;
		}

		const float scrollStep = slowCamera ? 0.5f : (fastCamera ? 12.5f : 1.5f);

		if (mouseScrolledUp)
			camera.Position += front * scrollStep;
		if (mouseScrolledDown)
			camera.Position -= front * scrollStep;
	}

	if (targetCameraPitch > +89.0f) targetCameraPitch = +89.0f;
	if (targetCameraPitch < -89.0f) targetCameraPitch = -89.0f;

	if (!keyboardBeingUsed)
	{
		if (Keyboard::IsDown(GLFW_KEY_W) == GLFW_PRESS)
			camera.Position += front * cameraSpeed;
		if (Keyboard::IsDown(GLFW_KEY_S) == GLFW_PRESS)
			camera.Position -= front * cameraSpeed;
		if (Keyboard::IsDown(GLFW_KEY_A) == GLFW_PRESS)
			camera.Position -= glm::normalize(glm::cross(front, camera.UpDirection)) * cameraSpeed;
		if (Keyboard::IsDown(GLFW_KEY_D) == GLFW_PRESS)
			camera.Position += glm::normalize(glm::cross(front, camera.UpDirection)) * cameraSpeed;

		if (Keyboard::IsDown(GLFW_KEY_SPACE))
			camera.Position += camera.UpDirection * cameraSpeed;
		if (Keyboard::IsDown(GLFW_KEY_LEFT_CONTROL))
			camera.Position -= camera.UpDirection * cameraSpeed;
	}

	camera.Target = camera.Position + glm::normalize(front);

	auto lerpValue = [&](float& value, float targetValue)
	{
		if (value == targetValue)
			return;

		constexpr auto lerp = [](float a, float b, float f) { return a + f * (b - a); };
		value = lerp(value, targetValue, cameraSmoothness * elapsedTime.TotalSeconds());
	};

	lerpValue(cameraYaw, targetCameraYaw);
	lerpValue(cameraPitch, targetCameraPitch);
}

void Application::UpdateTasks()
{
}

void Application::DrawScene()
{
	if (renderWindowHidden)
		return;

	if (renderWindowResized && elapsedFrames > 2)
	{
		sceneRenderTarget.Bind();
		sceneRenderTarget.Resize(renderWindowSize.x, renderWindowSize.y);
		postProcessingRenderTarget.Bind();
		postProcessingRenderTarget.Resize(renderWindowSize.x, renderWindowSize.y);
	}

	postProcessingRenderTarget.Bind();
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(baseClearColor.x, baseClearColor.y, baseClearColor.z, baseClearColor.w);

		glViewport(0, 0, sceneRenderTarget.GetWidth(), sceneRenderTarget.GetHeight());
		{
			mat4 cubeModelMatrices[_countof(cubePositions)];
			for (size_t i = 0; i < _countof(cubePositions); i++)
				cubeModelMatrices[i] = glm::translate(mat4(1.0f), cubePositions[i]);

			feelsBadManTexture.Bind(0);
			goodNiceTexture.Bind(1);
			comfyShader.Use();
			comfyShader.SetUniform(comfyShader.Texture0Location, 0);
			comfyShader.SetUniform(comfyShader.Texture1Location, 1);
			comfyShader.SetUniform(comfyShader.ViewLocation, camera.GetViewMatrix());
			comfyShader.SetUniform(comfyShader.ProjectionLocation, camera.GetProjectionMatrix());

			cubeVao.Bind();
			for (size_t i = 0; i < _countof(cubePositions); i++)
			{
				comfyShader.SetUniform(comfyShader.ModelLocation, cubeModelMatrices[i]);
				glDrawArrays(GL_TRIANGLES, 0, _countof(cubeVertices));
			}

			feelsBadManTexture.Bind(0);
			tileTexture.Bind(1);
			tileTexture.Bind(0);
			mat4 tileModelMatrix = glm::scale(glm::translate(mat4(1.0f), vec3(0, -4.0f, 0)), vec3(39.0f, 1.0f, 39.0f));
			comfyShader.SetUniform(comfyShader.ModelLocation, tileModelMatrix);
			glDrawArrays(GL_TRIANGLES, 0, _countof(cubeVertices));

			feelsBadManTexture.Bind(0);
			skyTexture.Bind(1);
			skyTexture.Bind(0);
			mat4 skyModelMatrix = glm::scale(mat4(1.0f), vec3(1000.0f, 1000.0f, 1000.0f));
			comfyShader.SetUniform(comfyShader.ModelLocation, skyModelMatrix);
			glDrawArrays(GL_TRIANGLES, 0, _countof(cubeVertices));

			feelsBadManTexture.Bind(0);
			groundTexture.Bind(1);
			groundTexture.Bind(0);
			mat4 groundModelMatrix = glm::scale(glm::translate(mat4(1.0f), vec3(0, -5.0f, 0)), vec3(999.9f, 1.0f, 999.9));
			comfyShader.SetUniform(comfyShader.ModelLocation, groundModelMatrix);
			glDrawArrays(GL_TRIANGLES, 0, _countof(cubeVertices));

			{
				lineShader.Use();
				lineShader.SetUniform(lineShader.ViewLocation, camera.GetViewMatrix());
				lineShader.SetUniform(lineShader.ProjectionLocation, camera.GetProjectionMatrix());

				lineVao.Bind();
				for (size_t i = 0; i < _countof(cubePositions); i++)
				{
					lineShader.SetUniform(lineShader.ModelLocation, cubeModelMatrices[i]);
					glDrawArrays(GL_LINES, 0, _countof(axisVertices));
				}
			}
		}
	}
	postProcessingRenderTarget.UnBind();

	sceneRenderTarget.Bind();
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(baseClearColor.x, baseClearColor.y, baseClearColor.z, baseClearColor.w);

		screenVao.Bind();
		screenShader.Use();
		postProcessingRenderTarget.GetTexture().Bind();
		glDrawArrays(GL_TRIANGLES, 0, _countof(screenVertices));
	}
	sceneRenderTarget.UnBind();
}

void Application::DrawGui()
{
	static const ImVec2 uv0 = { 0, 1 }, uv1 = { 1, 0 };

	mouseBeingUsed = ImGui::GetIO().WantCaptureMouse;
	keyboardBeingUsed = ImGui::GetIO().WantCaptureKeyboard;

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	{
		static bool showComfyDebug = true;
		static bool showDemoWindow = true;
		static bool showSwapInterval = true;

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

				ImGui::MenuItem("Show Comfy Debug", nullptr, &showComfyDebug);
				ImGui::MenuItem("Show Demo Window", nullptr, &showDemoWindow);
				ImGui::Separator();

				if (ImGui::MenuItem("Targets", nullptr)) { ; }
				if (ImGui::MenuItem("Lyrics", nullptr)) { ; }
				if (ImGui::MenuItem("Cameras", nullptr)) { ; }
				if (ImGui::MenuItem("Stage Changes", nullptr)) { ; }
				if (ImGui::MenuItem("Effects", nullptr)) { ; }

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

			if (ImGui::BeginMenu(u8"shinitai - 死にたい"))
			{
				if (ImGui::MenuItem(u8"test - テスト", nullptr)) { ; }
				ImGui::EndMenu();
			}

			if (ImGui::MenuItem("Open Popup", nullptr))
			{
				ImGui::OpenPopup("Test Popup");
			}
			if (ImGui::BeginPopupModal("Test Popup", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text("Test!\n\n");
				if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
				ImGui::SameLine();
				if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
				ImGui::EndPopup();
			}

			//if (ImGui::BeginPopupModal("Focus Lost Popup", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			//{
			//	if (focusGainedFrame)
			//		ImGui::CloseCurrentPopup();

			//	ImGui::Text("Window focus has been lost.\nwould you like to retain the exclusive audio mode?\n\n");
			//	if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
			//	ImGui::SameLine();
			//	if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
			//	ImGui::EndPopup();
			//}

			if (focusLostFrame)
				ImGui::OpenPopup("PeepoSleep zzzZZZ");

			if (ImGui::BeginPopupModal("PeepoSleep zzzZZZ", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text("Window focus has been lost");
				if (focusGainedFrame)
					ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
			}

			char infoBuffer[32];
			sprintf_s(infoBuffer, sizeof(infoBuffer), "%.3f ms (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

			ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::CalcTextSize(infoBuffer).x - ImGui::GetStyle().WindowPadding.x);
			ImGui::Text(infoBuffer);

			ImGui::EndMainMenuBar();
		}

		// Window Dockspace
		// ----------------
		if (true)
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

		// Comfy Debug Windows
		// -------------------
		if (showComfyDebug)
		{
			if (ImGui::Begin("Comfy Debug", &showComfyDebug))
			{
				if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::Text("Camera");
					ImGui::DragFloat("Field Of View", &camera.FieldOfView, 1.0f, 1.0f, 180.0f);
					ImGui::DragFloat("Near Plane", &camera.NearPlane, 0.001f, 0.001f, 1.0f);
					ImGui::DragFloat("Far Plane", &camera.FarPlane);
					ImGui::DragFloat3("Position", glm::value_ptr(camera.Position), 0.01f);
					ImGui::DragFloat3("Target", glm::value_ptr(camera.Target), 0.01f);
					ImGui::DragFloat("Smoothness", &cameraSmoothness, 1.0f, 1.0f, 250.0f);

					ImGui::Text("Camera Rotation");
					ImGui::DragFloat("Pitch", &targetCameraPitch, 1.0f);
					ImGui::DragFloat("Yaw", &targetCameraYaw, 1.0f);
				}

				if (ImGui::CollapsingHeader("Cubes"))
				{
					char nameBuffer[32];
					for (size_t i = 0; i < _countof(cubePositions); i++)
					{
						sprintf_s(nameBuffer, sizeof(nameBuffer), "CUBE[%zd]", i);
						ImGui::DragFloat3(nameBuffer, glm::value_ptr(cubePositions[i]), 0.1f);
					}
				}

				if (ImGui::CollapsingHeader("Test Tree"))
				{
					static bool treeNodeCheckboxBool[100];
					static bool treeNodeParticleboxBool[100];
					char buffer[32];
					char hiddenBuffer[sizeof(buffer)];

					for (size_t i = 0; i < 100; i++)
					{
						sprintf_s(buffer, sizeof(buffer), "eff_pv%03d_F%04zd", i, i + 1800);
						sprintf_s(hiddenBuffer, sizeof(hiddenBuffer), "##%s", buffer);

						bool treeNode = ImGui::TreeNode(hiddenBuffer);
						ImGui::SameLine();
						ImGui::Checkbox(buffer, &treeNodeCheckboxBool[i]);

						if (treeNode)
						{
							if (ImGui::TreeNode("Emitter"))
							{
								ImGui::Checkbox("Particle", &treeNodeParticleboxBool[i]);
								ImGui::TreePop();
							}
							ImGui::TreePop();
						}
					}
				}

			}

			if (ImGui::CollapsingHeader("Image Test"))
			{
				struct { const char* Name; Texture* Texture; } namedTextures[] =
				{
					{ "feels bad", &feelsBadManTexture},
					{ "good nice", &goodNiceTexture},
					{ "ground", &groundTexture},
					{ "sky", &skyTexture},
					{ "tile", &tileTexture},
				};

				for (size_t i = 0; i < _countof(namedTextures); i++)
				{
					ImGui::Text(namedTextures[i].Name);
					ImGui::Image(namedTextures[i].Texture->GetVoidTexture(), { 200, 200 }, uv0, uv1);
				}
			}

			ImGui::End();

			renderWindowHidden = true;
			if (ImGui::Begin("Render Window"))
			{
				ImGuiWindow* currentWindow = ImGui::GetCurrentWindow();
				const int titleBarHeight = static_cast<int>(currentWindow->TitleBarHeight());
				renderWindowHidden = currentWindow->Hidden;

				renderWindowResized = (renderWindowSize.x != lastRenderWindowSize.x) || (renderWindowSize.y != lastRenderWindowSize.y);
				lastRenderWindowPos = renderWindowPos;
				lastRenderWindowSize = renderWindowSize;
				renderWindowPos = ImGui::GetWindowPos();
				renderWindowSize = ImGui::GetWindowSize();

				renderWindowTitleHover =
					(mouseX > renderWindowPos.x) && (mouseX < renderWindowPos.x + renderWindowSize.x) &&
					(mouseY > renderWindowPos.y) && (mouseY < renderWindowPos.y + titleBarHeight);

				renderWindowHover = !renderWindowTitleHover && ImGui::IsWindowHovered();

				renderWindowPos.y += static_cast<float>(titleBarHeight);
				renderWindowSize.y -= static_cast<float>(titleBarHeight);

				if (ImGui::IsWindowFocused() && !renderWindowTitleHover)
				{
					if (ImGui::IsWindowHovered() | ImGui::IsMouseDown(0))
						mouseBeingUsed = false;
					keyboardBeingUsed = false;
				}
				else
				{
					keyboardBeingUsed = true;
				}

				//ImU32 backgroundColor = ImGui::GetColorU32(ImGui::GetStyle().Colors[ImGuiCol_DockingEmptyBg]);
				//ImGui::GetWindowDrawList()->AddRectFilled(renderWindowPos, { renderWindowPos.x + renderWindowSize.x, renderWindowPos.y + renderWindowSize.y }, backgroundColor);

				if (false) // adjust aspect ratio, only for 2d render buffer
				{
					const float targetAspectRatio = 16.0f / 9.0f;
					const float outputAspect = renderWindowSize.x / renderWindowSize.y;

					// TODO: resize framebuffer but darken non 16/9 aet region
					if (outputAspect <= targetAspectRatio)
					{
						// output is taller than it is wider, bars on top/bottom
						int presentHeight = static_cast<int>((renderWindowSize.x / targetAspectRatio) + 0.5f);
						int barHeight = static_cast<int>((renderWindowSize.y - presentHeight) / 2);

						renderWindowPos.y += static_cast<float>(barHeight);
						renderWindowSize.y = static_cast<float>(presentHeight);
						//RenderRectangle = new Rectangle(0, barHeight, renderWindowSize.x, presentHeight);
					}
					else
					{
						// output is wider than it is tall, bars left/right
						int presentWidth = static_cast<int>((renderWindowSize.y * targetAspectRatio) + 0.5f);
						int barWidth = static_cast<int>((renderWindowSize.x - presentWidth) / 2);

						renderWindowPos.x += static_cast<int>(barWidth);
						renderWindowSize.x = static_cast<int>(presentWidth);
						//RenderRectangle = new Rectangle(barWidth, 0, presentWidth, renderWindowSize.y);
					}
				}

				ImGui::GetWindowDrawList()->AddImage(sceneRenderTarget.GetTexture().GetVoidTexture(), renderWindowPos, { renderWindowPos.x + renderWindowSize.x, renderWindowPos.y + renderWindowSize.y }, uv0, uv1);
			}
			ImGui::End();
		}


		// Temp Tests
		// ----------
		if (false)
		{
			if (ImGui::Begin("Temp Test"))
			{
				static float scale = .15f;
				ImGui::AddTexture(ImGui::GetWindowDrawList(), &feelsBadManTexture, ImGui::GetMousePos(), scale);
				ImGui::SliderFloat("scale", &scale, 0.01f, 1.0f);
			}
			ImGui::End();
		}
	}
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	if (!windowFocused || !ImGui::IsAnyWindowFocused())
		mouseBeingUsed = keyboardBeingUsed = true;
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

	glViewport(0, 0, width, height);
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