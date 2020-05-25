#include "Application.h"
#include "IO/File.h"
#include "DataTest/AudioTestWindow.h"
#include "DataTest/IconTestWindow.h"
#include "DataTest/InputTestWindow.h"
#include "System/Profiling/Profiler.h"
#include "System/Version/BuildConfiguration.h"
#include "System/Version/BuildVersion.h"
#include "Input/Input.h"
#include "System/ComfyData.h"
#include "Core/Logger.h"

#include "../res/resource.h"

namespace Comfy::Studio
{
	namespace ApplicationConfigIDs
	{
		constexpr std::string_view RestoreRegion = "Comfy::Studio::Application::RestoreRegion";
		constexpr std::string_view Position = "Comfy::Studio::Application::Position";
		constexpr std::string_view Size = "Comfy::Studio::Application::Size";
		constexpr std::string_view IsFullscreen = "Comfy::Studio::Application::IsFullscreen";
		constexpr std::string_view IsMaximized = "Comfy::Studio::Application::IsMaximized";
	}

	void Application::Run()
	{
		System::MountComfyData();
		System::LoadComfyConfig();

		const auto hostParam = CreateHostParam();
		host = std::make_unique<ApplicationHost>(hostParam);

		if (!BaseInitialize())
			return;

		host->EnterProgramLoop([&]()
		{
			Gui();
		});

		BaseDispose();
	}

	void Application::Exit()
	{
		host->Exit();
	}

	ApplicationHost& Application::GetHost()
	{
		return *host;
	}

	bool Application::BaseInitialize()
	{
		host->RegisterWindowResizeCallback([&](ivec2 size)
		{
		});

		host->RegisterWindowClosingCallback([&]()
		{
			DisposeSaveConfig();
		});

		if (!InitializeEditorComponents())
			return false;

		return true;
	}

	ApplicationHost::ConstructionParam Application::CreateHostParam()
	{
		const auto comfyIcon = ::LoadIconA(::GetModuleHandleA(nullptr), MAKEINTRESOURCEA(COMFY_ICON));

		ApplicationHost::ConstructionParam hostParam;
		hostParam.StartupWindowState.Title = ComfyStudioWindowTitle;
		hostParam.IconHandle = comfyIcon;
		hostParam.StartupWindowState.RestoreRegion = System::Config.GetIVec4(ApplicationConfigIDs::RestoreRegion);
		hostParam.StartupWindowState.Position = System::Config.GetIVec2(ApplicationConfigIDs::Position);
		hostParam.StartupWindowState.Size = System::Config.GetIVec2(ApplicationConfigIDs::Size);
		hostParam.StartupWindowState.IsFullscreen = System::Config.GetBool(ApplicationConfigIDs::IsFullscreen);
		hostParam.StartupWindowState.IsMaximized = System::Config.GetBool(ApplicationConfigIDs::IsMaximized);
		return hostParam;
	}

	bool Application::InitializeEditorComponents()
	{
		editorManager = std::make_unique<Editor::EditorManager>(*this);

		dataTestComponents.reserve(3);
		dataTestComponents.push_back(std::move(std::make_unique<DataTest::InputTestWindow>(*this)));
		dataTestComponents.push_back(std::move(std::make_unique<DataTest::AudioTestWindow>(*this)));
		dataTestComponents.push_back(std::move(std::make_unique<DataTest::IconTestWindow>(*this)));

		return true;
	}

	void Application::Gui()
	{
		const auto& io = Gui::GetIO();

		if (io.KeyCtrl && io.KeyAlt && Gui::IsKeyPressed(Input::KeyCode_B, false))
			showMainMenuBar ^= true;

		if (Gui::IsKeyPressed(Input::KeyCode_F10))
		{
			exclusiveAppEngineWindow ^= true;
			showMainAppEngineWindow = exclusiveAppEngineWindow;
		}

		const bool menuBarVisible = showMainMenuBar && !exclusiveAppEngineWindow;

		if (menuBarVisible)
			GuiMainMenuBar();

		host->GuiMainDockspace(menuBarVisible);

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
				GuiAppEngineWindow();
		}
		else
		{
			if (showMainAppEngineWindow)
				GuiAppEngineWindow();

			if (showStyleEditor)
			{
				if (Gui::Begin("Style Editor##Application", &showStyleEditor))
					Gui::ShowStyleEditor();
				Gui::End();
			}

			if (showDemoWindow)
				Gui::ShowDemoWindow(&showDemoWindow);

			editorManager->GuiWindows();
			GuiBaseWindowWindows(dataTestComponents);
		}

	}

	void Application::GuiMainMenuBar()
	{
		Gui::PushStyleColor(ImGuiCol_MenuBarBg, Gui::GetStyleColorVec4(ImGuiCol_TitleBg));
		if (Gui::BeginMainMenuBar())
		{
			GuiDebugMenu();
			editorManager->GuiMenuItems();
			GuiAppEngineMenus();
			GuiBaseWindowMenus(dataTestComponents);
			GuiHelpMenus();
			GuiMenuBarPerformanceDisplay();
			Gui::EndMainMenuBar();
		}
		Gui::PopStyleColor(1);
	}

	void Application::GuiDebugMenu()
	{
		if (Gui::BeginMenu("Debug"))
		{
			if (Gui::MenuItem("Toggle Fullscreen", nullptr))
				host->ToggleFullscreen();

			if (Gui::BeginMenu("Swap Interval"))
			{
				if (Gui::MenuItem("SwapInterval(0)", nullptr))
					host->SetSwapInterval(0);

				if (Gui::MenuItem("SwapInterval(1)", nullptr))
					host->SetSwapInterval(1);

				Gui::EndMenu();
			}

			if (Gui::BeginMenu("ImGui Config"))
			{
				if (Gui::MenuItem("Refresh", nullptr))
					Gui::SaveIniSettingsToMemory();

				if (Gui::MenuItem("Save To Disk", nullptr))
					Gui::SaveIniSettingsToDisk(Gui::GetIO().IniFilename);

				Gui::EndMenu();
			}

			if (Gui::MenuItem("Test Print", nullptr))
				Logger::LogLine(__FUNCTION__"(): Test");

			Gui::Separator();

			if (Gui::MenuItem("Exit...", nullptr))
				Exit();

			Gui::EndMenu();
		}
	}

	void Application::GuiAppEngineWindow()
	{
		/*
		if (appEngine == nullptr)
			appEngine = std::make_unique<App::Engine>();

		appEngine->BeginEndGui();
		appEngine->Tick();
		*/
	}

	void Application::GuiAppEngineMenus()
	{
		if (Gui::BeginMenu("Engine"))
		{
			Gui::MenuItem("Engine Window", nullptr, &showMainAppEngineWindow);
			Gui::EndMenu();
		}
	}

	void Application::GuiBaseWindowMenus(const std::vector<std::unique_ptr<BaseWindow>>& components)
	{
		if (Gui::BeginMenu("Data Test"))
		{
			Gui::MenuItem("Style Editor", nullptr, &showStyleEditor, BuildConfiguration::Debug);
			Gui::MenuItem("Demo Window", nullptr, &showDemoWindow, BuildConfiguration::Debug);

			for (const auto& component : components)
				Gui::MenuItem(component->GetName(), nullptr, &component->GetIsOpen());

			Gui::EndMenu();
		}
	}

	void Application::GuiBaseWindowWindows(const std::vector<std::unique_ptr<BaseWindow>>& components)
	{
		for (const auto& component : components)
		{
			if (component->GetIsOpen())
			{
				if (Gui::Begin(component->GetName(), &component->GetIsOpen(), component->GetFlags()))
					component->Gui();
				Gui::End();
			}
		}
	}

	void Application::GuiHelpMenus()
	{
		bool openLicensePopup = false;

		if (Gui::BeginMenu("Help"))
		{
			Gui::TextUnformatted(Gui::StringViewStart(CopyrightNotice), Gui::StringViewEnd(CopyrightNotice));
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
			const auto viewport = Gui::GetMainViewport();
			const auto window = Gui::FindWindowByName(licenseWindow.GetWindowName());
			Gui::SetWindowPos(window, viewport->Pos + viewport->Size / 8, ImGuiCond_Always);
			Gui::SetWindowSize(window, viewport->Size * .75f, ImGuiCond_Always);

			licenseWindow.DrawGui();

			if (Gui::IsKeyPressed(Input::KeyCode_Escape, false))
				Gui::CloseCurrentPopup();

			Gui::EndPopup();
		}

		if (versionWindowOpen)
			GuiHelpVersionWindow();
	}

	void Application::GuiHelpVersionWindow()
	{
		// TODO: Make window class and use undockable window instead of popup window since it doesn't need to block input
		const auto viewport = ImGui::GetMainViewport();
		Gui::SetNextWindowPos(viewport->Pos + viewport->Size * 0.5f, ImGuiCond_Appearing, vec2(0.5f, 0.5f));

		if (Gui::Begin("About - Version##Application", &versionWindowOpen, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking))
		{
			const vec2 windowSize = vec2(680, 280);

			Gui::BeginChild("AboutWindowChild", windowSize, true);
			Gui::Columns(2);
			{
				auto guiPropertyValue = [&](const char* property, const char* value)
				{
					Gui::TextUnformatted(property);
					Gui::NextColumn();
					Gui::Text("%s::%s", "BuildVersion", value);
					Gui::NextColumn();
				};

				Gui::TextUnformatted("Property");
				Gui::NextColumn();
				Gui::TextUnformatted("Value");
				Gui::NextColumn();

				Gui::Separator();

				guiPropertyValue("BuildConfiguration", BuildConfiguration::Debug ? "Debug" : BuildConfiguration::Release ? "Release" : "Unknown");
				guiPropertyValue("Author", BuildVersion::Author);
				guiPropertyValue("CommitHash", BuildVersion::CommitHash);
				guiPropertyValue("CommitTime", BuildVersion::CommitTime);
				guiPropertyValue("CommitNumber", BuildVersion::CommitNumber);
				guiPropertyValue("Branch", BuildVersion::Branch);
				guiPropertyValue("CompileTime", BuildVersion::CompileTime);
			}
			Gui::Columns(1);
			Gui::EndChild();
		}
		Gui::End();
	}

	void Application::GuiMenuBarPerformanceDisplay()
	{
		const auto frameRate = Gui::GetIO().Framerate;

		// TODO: Popup window on hover (+ double click to create overlay) showing a frametime plot
		char infoBuffer[32];
		sprintf_s(infoBuffer, sizeof(infoBuffer), "%.3f ms (%.1f FPS)", (1000.0f / frameRate), frameRate);

		Gui::SetCursorPosX(Gui::GetWindowWidth() - Gui::CalcTextSize(infoBuffer).x - Gui::GetStyle().WindowPadding.x);
		Gui::TextUnformatted(infoBuffer);
	}

	void Application::BaseDispose()
	{
		if (skipApplicationCleanup)
			return;

		DisposeSaveConfig();

		// NOTE: Force deletion before the graphics context is destroyed
		editorManager = nullptr;
		host = nullptr;
	}

	void Application::DisposeSaveConfig()
	{
		System::Config.SetIVec4(ApplicationConfigIDs::RestoreRegion, host->GetWindowRestoreRegion());
		System::Config.SetIVec2(ApplicationConfigIDs::Position, host->GetWindowPosition());
		System::Config.SetIVec2(ApplicationConfigIDs::Size, host->GetWindowSize());
		System::Config.SetBool(ApplicationConfigIDs::IsFullscreen, host->GetIsFullscreen());
		System::Config.SetBool(ApplicationConfigIDs::IsMaximized, host->GetIsMaximized());
		System::SaveComfyConfig();
	}
}
