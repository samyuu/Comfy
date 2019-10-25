#include "Application.h"
#include "App/Engine.h"
#include "FileSystem/FileHelper.h"
#include "DataTest/AudioTestWindow.h"
#include "DataTest/IconTestWindow.h"
#include "DataTest/InputTestWindow.h"
#include "DataTest/ShaderTestWindow.h"
#include "System/Profiling/Profiler.h"
#include "System/Version/BuildConfiguration.h"
#include "System/Version/BuildVersion.h"
#include "Input/KeyCode.h"

Application::Application()
{
}

Application::~Application()
{
	BaseDispose();
}

void Application::Run()
{
	if (!BaseInitialize())
		return;

	System::Profiler& profiler = System::Profiler::Get();

	host.EnterProgramLoop([&]()
	{
		profiler.StartFrame();
		BaseUpdate();
		BaseDraw();
		profiler.EndFrame();
	});

	BaseDispose();
}

void Application::Exit()
{
	host.Exit();
}

ApplicationHost& Application::GetHost()
{
	return host;
}

bool Application::BaseInitialize()
{
	if (hasBeenInitialized)
		return false;

	hasBeenInitialized = true;

	if (!host.Initialize())
		return false;

	host.RegisterWindowResizeCallback([](ivec2 size) 
	{
		Graphics::RenderCommand::SetViewport(size);
	});

	host.RegisterWindowClosingCallback([]() 
	{
		Audio::AudioEngine* audioEngine = Audio::AudioEngine::GetInstance();

		if (audioEngine != nullptr)
			audioEngine->StopStream();
	});

	Audio::AudioEngine::CreateInstance();
	Audio::AudioEngine::InitializeInstance();

	if (!InitializeCheckRom())
		return false;

	if (!InitializeGuiRenderer())
		return false;

	if (!InitializeEditorComponents())
		return false;

	return true;
}

void Application::BaseUpdate()
{
	ProcessInput();
	ProcessTasks();
}

void Application::BaseDraw()
{
	const vec4 baseClearColor = Editor::GetColorVec4(Editor::EditorColor_BaseClear);
	Graphics::RenderCommand::SetClearColor(baseClearColor);
	Graphics::RenderCommand::Clear(Graphics::ClearTarget_ColorBuffer);
	Graphics::RenderCommand::SetViewport(host.GetWindowSize());

	DrawGui();
}

void Application::BaseDispose()
{
	if (hasBeenDisposed)
		return;

	hasBeenDisposed = true;

	if (skipApplicationCleanup)
		return;

	Audio::AudioEngine::DisposeInstance();
	Audio::AudioEngine::DeleteInstance();

	// NOTE: Force deletion before the OpenGL context is destroyed
	editorManager.reset();

	guiRenderer.Dispose();
	host.Dispose();
}

bool Application::InitializeCheckRom()
{
	if (!FileSystem::DirectoryExists("rom"))
	{
		Logger::LogErrorLine(__FUNCTION__"(): Unable to locate rom directory");
		return false;
	}

	return true;
}

bool Application::InitializeGuiRenderer()
{
	return guiRenderer.Initialize();
}

bool Application::InitializeEditorComponents()
{
	editorManager = MakeUnique<Editor::EditorManager>(this);

	dataTestComponents.reserve(4);
	dataTestComponents.push_back(MakeRef<DataTest::InputTestWindow>(this));
	dataTestComponents.push_back(MakeRef<DataTest::AudioTestWindow>(this));
	dataTestComponents.push_back(MakeRef<DataTest::IconTestWindow>(this));
	dataTestComponents.push_back(MakeRef<DataTest::ShaderTestWindow>(this));

	return true;
}

void Application::ProcessInput()
{
	const auto& io = Gui::GetIO();

	if (Gui::IsKeyPressed(KeyCode_F11) || (io.KeyAlt && Gui::IsKeyPressed(KeyCode_Enter)))
		host.ToggleFullscreen();
}

void Application::ProcessTasks()
{
}

void Application::DrawGui()
{
	const auto& io = Gui::GetIO();

	guiRenderer.BeginFrame();
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
		if (showMainMenuBar && !exclusiveAppEngineWindow)
		{
			Gui::PushStyleColor(ImGuiCol_MenuBarBg, Gui::GetStyleColorVec4(ImGuiCol_TitleBg));
			if (Gui::BeginMainMenuBar())
			{
				if (Gui::BeginMenu("Debug"))
				{
					Gui::PushStyleColor(ImGuiCol_Text, Gui::GetStyleColorVec4(ImGuiCol_PlotHistogramHovered));
					if (Gui::MenuItem("Recompile Shaders", nullptr))
						Graphics::ShaderProgram::RecompileAllShaders();
					Gui::PopStyleColor();

					if (Gui::MenuItem("Toggle Fullscreen", nullptr))
						host.ToggleFullscreen();

					if (Gui::BeginMenu("Swap Interval", &showSwapInterval))
					{
						if (Gui::MenuItem("SwapInterval(0)", nullptr))
							host.SetSwapInterval(0);

						if (Gui::MenuItem("SwapInterval(1)", nullptr))
							host.SetSwapInterval(1);

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
				editorManager->DrawGuiMenuItems();

				// App Engine Menu Items
				// ---------------------
				DrawAppEngineMenus("Engine");

				// Data Test Menus
				// ---------------
				DrawGuiBaseWindowMenus("Data Test", dataTestComponents);

				bool openLicensePopup = false;
				if (Gui::BeginMenu("Help"))
				{
					Gui::TextUnformatted("Copyright (C) 2019 Samyuu");
					if (Gui::MenuItem("License"))
						openLicensePopup = true;
					if (Gui::MenuItem("Version"))
						versionWindowOpen = true;
					Gui::EndMenu();
				}

				if (openLicensePopup)
				{
					*licenseWindow.GetIsWindowOpen() = true;
					Gui::OpenPopup(licenseWindow.GetWindowName());
				}

				if (Gui::BeginPopupModal(licenseWindow.GetWindowName(), licenseWindow.GetIsWindowOpen(), ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
				{
					ImGuiViewport* viewport = Gui::GetMainViewport();
					ImGuiWindow* window = Gui::FindWindowByName(licenseWindow.GetWindowName());
					Gui::SetWindowPos(window, viewport->Pos + viewport->Size / 8, ImGuiCond_Always);
					Gui::SetWindowSize(window, viewport->Size * .75f, ImGuiCond_Always);

					licenseWindow.DrawGui();

					if (Gui::IsKeyPressed(KeyCode_Escape))
						Gui::CloseCurrentPopup();

					Gui::EndPopup();
				}

				// TODO: Make window class and use undockable window instead of popup window since it doesn't need to block input
				if (versionWindowOpen)
				{
					ImGuiViewport* viewport = ImGui::GetMainViewport();
					Gui::SetNextWindowPos(viewport->Pos + viewport->Size * 0.5f, ImGuiCond_Appearing, vec2(0.5f, 0.5f));

					if (Gui::Begin("About - Version##Application", &versionWindowOpen, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking))
					{
						const vec2 windowSize = vec2(640, 320);

						Gui::BeginChild("AboutWindowChild", windowSize, true);
						Gui::Columns(2);
						{
							Gui::TextUnformatted("Property"); Gui::NextColumn();
							Gui::TextUnformatted("Value"); Gui::NextColumn();
							Gui::Separator();

							const char* buildVersionClassName = "BuildVersion";
							const char* buildVersionFormatStrin = "%s::%s";

							Gui::TextUnformatted("BuildConfiguration");
							Gui::NextColumn(); Gui::TextUnformatted(BuildConfiguration::Debug ? "Debug" : BuildConfiguration::Release ? "Release" : "Unknown");
							Gui::NextColumn();
							Gui::Text(buildVersionFormatStrin, buildVersionClassName, "Author");
							Gui::NextColumn(); Gui::TextUnformatted(BuildVersion::Author);
							Gui::NextColumn();
							Gui::Text(buildVersionFormatStrin, buildVersionClassName, "CommitHash");
							Gui::NextColumn(); Gui::TextUnformatted(BuildVersion::CommitHash);
							Gui::NextColumn();
							Gui::Text(buildVersionFormatStrin, buildVersionClassName, "CommitTime");
							Gui::NextColumn(); Gui::TextUnformatted(BuildVersion::CommitTime);
							Gui::NextColumn();
							Gui::Text(buildVersionFormatStrin, buildVersionClassName, "CommitNumber");
							Gui::NextColumn(); Gui::TextUnformatted(BuildVersion::CommitNumber);
							Gui::NextColumn();
							Gui::Text(buildVersionFormatStrin, buildVersionClassName, "Branch");
							Gui::NextColumn(); Gui::TextUnformatted(BuildVersion::Branch);
							Gui::NextColumn();
							Gui::Text(buildVersionFormatStrin, buildVersionClassName, "CompileTime");
							Gui::NextColumn(); Gui::TextUnformatted(BuildVersion::CompileTime);
							Gui::NextColumn();
						}
						Gui::Columns(1);
						Gui::EndChild();

					}
					Gui::End();
				}

				char infoBuffer[32];
				sprintf_s(infoBuffer, sizeof(infoBuffer), "%.3f ms (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

				Gui::SetCursorPosX(Gui::GetWindowWidth() - Gui::CalcTextSize(infoBuffer).x - Gui::GetStyle().WindowPadding.x);
				Gui::TextUnformatted(infoBuffer);

				Gui::EndMainMenuBar();
			}
			Gui::PopStyleColor(1);
		}

		// Window Dockspace
		// ----------------
		{
			Gui::PushStyleVar(ImGuiStyleVar_WindowPadding, vec2(0.0f, 0.0f));
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

			Gui::Begin(mainDockSpaceID, nullptr, dockspaceWindowFlags);
			ImGuiID dockspaceID = Gui::GetID(mainDockSpaceID);
			Gui::DockSpace(dockspaceID, vec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
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
			editorManager->DrawGuiWindows();

			// Data Test Windows
			// -----------------
			DrawGuiBaseWindowWindows(dataTestComponents);
		}
	}
	guiRenderer.EndFrame();
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

void Application::DrawGuiBaseWindowMenus(const char* header, const std::vector<RefPtr<BaseWindow>>& components)
{
	if (Gui::BeginMenu(header))
	{
		Gui::MenuItem("Style Editor", nullptr, &showStyleEditor, BuildConfiguration::Debug);
		Gui::MenuItem("Demo Window", nullptr, &showDemoWindow, BuildConfiguration::Debug);

		for (const auto& component : components)
			Gui::MenuItem(component->GetGuiName(), nullptr, component->GetIsGuiOpenPtr());

		Gui::EndMenu();
	}
}

void Application::DrawGuiBaseWindowWindows(const std::vector<RefPtr<BaseWindow>>& components)
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
