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

	static Application* GlobalLastCreatedApplication = nullptr;

	Application::Application(std::string_view fileToOpen) : fileToOpenOnStartup(fileToOpen)
	{
	}

	void Application::Run()
	{
		GlobalLastCreatedApplication = this;

		System::MountComfyData();
		System::LoadComfyConfig();

		const auto hostParam = CreateHostParam();
		host = std::make_unique<ApplicationHost>(hostParam);

		if (!BaseInitialize())
			return;

		host->EnterProgramLoop([&]()
		{
			Gui();
			UpdateWindowFocusAudioEngineResponse();
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

	std::string_view Application::GetFileToOpenOnStartup() const
	{
		return fileToOpenOnStartup;
	}

	void Application::SetFormattedWindowTitle(std::string_view subTitle)
	{
		std::string formattedTitle;
		formattedTitle.reserve(ComfyStudioWindowTitle.size() + subTitle.size() + 24);

		formattedTitle += ComfyStudioWindowTitle;

		if (!subTitle.empty())
		{
			formattedTitle += " - ";
			formattedTitle += subTitle;
		}

		formattedTitle += " (";
		formattedTitle += std::string_view(BuildVersion::CommitHash, 8);
		formattedTitle += " - ";
		formattedTitle += BuildConfiguration::Debug ? "Debug" : "Release";
		formattedTitle += ")";

		host->SetWindowTitle(formattedTitle);
	}

	void* Application::GetGlobalWindowFocusHandle()
	{
		return (GlobalLastCreatedApplication != nullptr) ? GlobalLastCreatedApplication->GetHost().GetWindowHandle() : nullptr;
	}

	bool Application::BaseInitialize()
	{
		host->RegisterWindowResizeCallback([this](ivec2 size)
		{
		});

		host->RegisterWindowClosingCallback([this]()
		{
			return (editorManager != nullptr) ? editorManager->OnApplicationClosing() : ApplicationHostCloseResponse::Exit;
		});

		host->RegisterWindowDestoyCallback([this]()
		{
			DisposeSaveConfig();
		});

		if (!InitializeEditorComponents())
			return false;

		return true;
	}

	ApplicationHost::ConstructionParam Application::CreateHostParam()
	{
		const auto comfyIcon = ::LoadIconW(::GetModuleHandleW(nullptr), MAKEINTRESOURCEW(COMFY_ICON));

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
			GuiTestWindowWindows();
		}

	}

	void Application::UpdateWindowFocusAudioEngineResponse()
	{
		if (!Audio::AudioEngine::InstanceValid())
			return;

		auto& audioEngine = Audio::AudioEngine::GetInstance();

		if (host->HasFocusBeenLost())
		{
			audioEngineRunningIdleOnFocusLost = (audioEngine.GetIsStreamOpenRunning() && audioEngine.GetAllVoicesAreIdle());

			if (audioEngineRunningIdleOnFocusLost)
				audioEngine.StopCloseStream();
		}

		if (host->HasFocusBeenGained())
		{
			if (audioEngineRunningIdleOnFocusLost)
				audioEngine.OpenStartStream();
		}
	}

	void Application::GuiMainMenuBar()
	{
		Gui::PushStyleColor(ImGuiCol_MenuBarBg, Gui::GetStyleColorVec4(ImGuiCol_TitleBg));
		if (Gui::BeginMainMenuBar())
		{
			editorManager->GuiComponentMenu();
			GuiApplicationWindowMenu();
			editorManager->GuiWorkSpaceMenu();
			GuiAppEngineMenus();
			GuiTestWindowMenus();
			GuiHelpMenus();
			GuiMenuBarAudioAndPerformanceDisplay();
			Gui::EndMainMenuBar();
		}
		Gui::PopStyleColor(1);
	}

	void Application::GuiApplicationWindowMenu()
	{
		if (Gui::BeginMenu("Window"))
		{
			if (Gui::MenuItem("Toggle Fullscreen", "Alt + Enter"))
				host->ToggleFullscreen();

			if (Gui::BeginMenu("Swap Interval"))
			{
				if (Gui::MenuItem("SwapInterval(0) - Unlimited", nullptr))
					host->SetSwapInterval(0);

				if (Gui::MenuItem("SwapInterval(1) - VSync", nullptr))
					host->SetSwapInterval(1);

				Gui::EndMenu();
			}

			Gui::Separator();

			if (Gui::BeginMenu("ImGui Config"))
			{
				if (Gui::MenuItem("Save To Memory", nullptr))
					Gui::SaveIniSettingsToMemory();

				if (Gui::MenuItem("Save To Disk", nullptr))
					Gui::SaveIniSettingsToDisk(Gui::GetIO().IniFilename);

				Gui::EndMenu();
			}

			Gui::EndMenu();
		}
	}

	void Application::GuiAppEngineWindow()
	{
#if 0
		if (appEngine == nullptr)
			appEngine = std::make_unique<App::Engine>();

		appEngine->BeginEndGui();
		appEngine->Tick();
#endif
	}

	void Application::GuiAppEngineMenus()
	{
#if 0
		if (Gui::BeginMenu("Engine", false))
		{
			Gui::MenuItem("Engine Window", nullptr, &showMainAppEngineWindow);
			Gui::EndMenu();
		}
#endif
	}

	void Application::GuiTestWindowMenus()
	{
		if (Gui::BeginMenu("Test Windows"))
		{
			const bool imguiDebugWindowsEnabled = BuildConfiguration::Debug;
			Gui::MenuItem("Style Editor", nullptr, &showStyleEditor, imguiDebugWindowsEnabled);
			Gui::MenuItem("Demo Window", nullptr, &showDemoWindow, imguiDebugWindowsEnabled);

			Gui::Separator();

			for (const auto& component : dataTestComponents)
				Gui::MenuItem(component->GetName(), nullptr, &component->GetIsOpen());

			Gui::EndMenu();
		}
	}

	void Application::GuiTestWindowWindows()
	{
		for (const auto& component : dataTestComponents)
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
		bool openVersionPopup = false;

		if (Gui::BeginMenu("Help"))
		{
			Gui::TextUnformatted(Gui::StringViewStart(CopyrightNotice), Gui::StringViewEnd(CopyrightNotice));
			Gui::Separator();
			if (Gui::MenuItem("License"))
				openLicensePopup = true;
			if (Gui::MenuItem("Version"))
				openVersionPopup = true;
			Gui::EndMenu();
		}

		if (openLicensePopup)
		{
			*licenseWindow.GetIsWindowOpen() = true;
			Gui::OpenPopup(licenseWindow.GetWindowName());
		}
		GuiLicensePopup();

		if (openVersionPopup)
		{
			aboutWindow.IsOpen = true;
			Gui::OpenPopup(aboutWindow.Name);
		}
		GuiHelpVersionPopup();
	}

	void Application::GuiLicensePopup()
	{
		if (Gui::BeginPopupModal(licenseWindow.GetWindowName(), licenseWindow.GetIsWindowOpen(), ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
		{
			const auto viewport = Gui::GetMainViewport();
			const auto window = Gui::FindWindowByName(licenseWindow.GetWindowName());
			Gui::SetWindowPos(window, viewport->Pos + viewport->Size / 8.0f, ImGuiCond_Always);
			Gui::SetWindowSize(window, viewport->Size * 0.75f, ImGuiCond_Always);

			licenseWindow.DrawGui();

			if (Gui::IsKeyPressed(Input::KeyCode_Escape, false))
				Gui::CloseCurrentPopup();

			Gui::EndPopup();
		}
	}

	void Application::GuiHelpVersionPopup()
	{
		if (Gui::BeginPopupModal(aboutWindow.Name, &aboutWindow.IsOpen, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
		{
			const auto viewport = Gui::GetMainViewport();
			const auto window = Gui::FindWindowByName(aboutWindow.Name);
			Gui::SetWindowPos(window, viewport->Pos + viewport->Size / 4.0f, ImGuiCond_Always);
			Gui::SetWindowSize(window, viewport->Size * 0.5f, ImGuiCond_Always);

			Gui::BeginChild("AboutWindowChild", vec2(0.0f, 0.0f), true);
			Gui::Columns(2);
			{
				auto guiPropertyValue = [&](const char* property, const char* value)
				{
					Gui::Text("%s::%s", "BuildVersion", property);
					Gui::NextColumn();
					Gui::TextUnformatted(value);
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

			if (Gui::IsKeyPressed(Input::KeyCode_Escape, false))
				Gui::CloseCurrentPopup();

			Gui::EndPopup();
		}
	}

	void Application::GuiMenuBarAudioAndPerformanceDisplay()
	{
		char performanceText[64];
		char audioText[128];

		const auto frameRate = Gui::GetIO().Framerate;
		sprintf_s(performanceText, "[ %.3f ms (%.1f FPS) ]", (1000.0f / frameRate), frameRate);

		auto audioBackendToString = [](Audio::AudioBackend backend) -> const char*
		{
			switch (backend)
			{
			case Audio::AudioBackend::WASAPIShared: return "WASAPI Shared";
			case Audio::AudioBackend::WASAPIExclusive: return "WASAPI Exclusive";
			default: return "Invalid";
			}
		};

		auto* audioEngine = Audio::AudioEngine::InstanceValid() ? &Audio::AudioEngine::GetInstance() : nullptr;
		if (audioEngine != nullptr && audioEngine->GetIsStreamOpenRunning())
		{
			const auto sampleRatekHz = static_cast<f64>(audioEngine->GetSampleRate()) / 1000.0;
			const auto sampleBitSize = sizeof(i16) * CHAR_BIT;
			const auto bufferDuration = TimeSpan::FromSeconds(static_cast<f64>(audioEngine->GetBufferFrameSize()) / static_cast<f64>(audioEngine->GetSampleRate()));
			const auto backendString = audioBackendToString(audioEngine->GetAudioBackend());

			sprintf_s(audioText, "[ %gkHz %zubit %dch ~%.0fms %s ]",
				sampleRatekHz,
				sampleBitSize,
				audioEngine->GetChannelCount(),
				bufferDuration.TotalMilliseconds(),
				backendString);
		}
		else
		{
			strcpy_s(audioText, "[ Audio Device Closed ]");
		}

		const auto windowWidth = Gui::GetWindowWidth();
		const auto perItemItemSpacing = (Gui::GetStyle().ItemSpacing.x * 2.0f);
		const auto performanceWidth = Gui::CalcTextSize(performanceText).x + perItemItemSpacing;
		const auto audioWidth = Gui::CalcTextSize(audioText).x + perItemItemSpacing;

		Gui::SetCursorPos(vec2(windowWidth - performanceWidth - audioWidth, Gui::GetCursorPos().y));
		if (Gui::BeginMenu(audioText))
		{
			const bool deviceIsOpen = audioEngine->GetIsStreamOpenRunning();

			if (Gui::MenuItem("Open Audio Device", nullptr, false, !deviceIsOpen))
				audioEngine->OpenStartStream();
			if (Gui::MenuItem("Close Audio Device", nullptr, false, deviceIsOpen))
				audioEngine->StopCloseStream();
			Gui::Separator();

			static constexpr std::array availableBackends = { Audio::AudioBackend::WASAPIShared, Audio::AudioBackend::WASAPIExclusive, };

			const auto currentBackend = audioEngine->GetAudioBackend();
			for (const auto backendType : availableBackends)
			{
				char buffer[32];
				sprintf_s(buffer, "Use %s", audioBackendToString(backendType));

				if (Gui::MenuItem(buffer, nullptr, (backendType == currentBackend), (backendType != currentBackend)))
					audioEngine->SetAudioBackend(backendType);
			}

			Gui::EndMenu();
		}

		// TODO: Come up with a better performance overlay and move data into a class member
		static struct PerformanceData
		{
			bool ShowOverlay;
			std::array<f32, 256> FrameTimes;
			size_t FrameTimeIndex;
			size_t FrameTimeSize;
		} performance = {};

		if (Gui::MenuItem(performanceText))
			performance.ShowOverlay ^= true;

		if (performance.ShowOverlay)
		{
			auto* mainViewport = Gui::GetMainViewport();
			Gui::SetNextWindowPos(vec2(mainViewport->Pos.x + mainViewport->Size.x, mainViewport->Pos.y + 24.0f), ImGuiCond_Always, vec2(1.0f, 0.0f));
			Gui::SetNextWindowViewport(mainViewport->ID);

			Gui::PushStyleColor(ImGuiCol_WindowBg, Gui::GetStyleColorVec4(ImGuiCol_PopupBg));

			constexpr auto overlayWindowFlags = (ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav /*| ImGuiWindowFlags_NoInputs*/);
			if (Gui::Begin("PerformanceOverlay", nullptr, overlayWindowFlags))
			{
				performance.FrameTimes[performance.FrameTimeIndex++] = Gui::GetIO().DeltaTime;
				if (performance.FrameTimeIndex >= performance.FrameTimes.size())
					performance.FrameTimeIndex = 0;
				performance.FrameTimeSize = std::max(performance.FrameTimeIndex, performance.FrameTimeSize);

				f32 averageFrameTime = 0.0f;
				for (size_t i = 0; i < performance.FrameTimeSize; i++)
					averageFrameTime += performance.FrameTimes[i];
				averageFrameTime /= static_cast<f32>(performance.FrameTimeSize);

				f32 minFrameTime = std::numeric_limits<f32>::max();
				for (size_t i = 0; i < performance.FrameTimeSize; i++)
					minFrameTime = std::min(minFrameTime, performance.FrameTimes[i]);

				f32 maxFrameTime = std::numeric_limits<f32>::min();
				for (size_t i = 0; i < performance.FrameTimeSize; i++)
					maxFrameTime = std::max(maxFrameTime, performance.FrameTimes[i]);

				char overlayText[64];
				sprintf_s(overlayText,
					"Average: %.3f ms\n"
					"Min: %.3f ms\n"
					"Max: %.3f ms",
					averageFrameTime * 1000.0f,
					minFrameTime * 1000.0f,
					maxFrameTime * 1000.0f);

				Gui::PlotLines("##PerformanceHistoryPlot",
					performance.FrameTimes.data(),
					static_cast<int>(performance.FrameTimes.size()),
					0,
					overlayText,
					minFrameTime - 0.001f,
					maxFrameTime + 0.001f,
					vec2(static_cast<f32>(performance.FrameTimes.size()), 120.0f));

				Gui::End();
			}

			Gui::PopStyleColor();
		}
	}

	void Application::BaseDispose()
	{
		if (skipApplicationCleanup)
			return;

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
