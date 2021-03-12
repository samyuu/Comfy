#include "Application.h"
#include "ComfyStudioSettings.h"
#include "DataTest/AudioTestWindow.h"
#include "DataTest/IconTestWindow.h"
#include "DataTest/InputTestWindow.h"
#include "System/Profiling/Profiler.h"
#include "System/Version/BuildConfiguration.h"
#include "System/Version/BuildVersion.h"
#include "Input/Input.h"
#include "System/ComfyData.h"
#include "Core/Logger.h"
#include "IO/File.h"
#include "IO/Shell.h"

#include "../res/resource.h"

namespace Comfy::Studio
{
	constexpr std::string_view ComfyStudioWindowTitle = "Comfy Studio";
	constexpr std::string_view ComfyCopyrightNotice = "Copyright (C) 2021 Samyuu";
	constexpr std::string_view UserManualDocumentFilePath = "manual/comfy_manual.html";
	constexpr const char* AboutWindowName = "About##Application";
	constexpr Input::KeyCode ToggleFullscreenKey = Input::KeyCode_F11;

	namespace
	{
		Application* GlobalLastCreatedApplication = nullptr;
		bool GlobalAppDataExistedOnLoad = false;
		bool GlobalUserDataExistedOnLoad = false;
	}

	Application::Application(std::string_view fileToOpen) : fileToOpenOnStartup(fileToOpen)
	{
	}

	void Application::Run()
	{
		GlobalLastCreatedApplication = this;

		System::MountComfyData();

		GlobalAppDataExistedOnLoad = GlobalAppData.LoadFromFile();
		if (!GlobalAppDataExistedOnLoad)
			GlobalAppData.RestoreDefault();

		GlobalUserDataExistedOnLoad = GlobalUserData.Mutable().LoadFromFile();
		if (!GlobalUserDataExistedOnLoad)
			GlobalUserData.Mutable().RestoreDefault();

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

	bool Application::GetExclusiveFullscreenGui() const
	{
		return exclusiveFullscreenGui;
	}

	void Application::SetExclusiveFullscreenGui(bool value)
	{
		exclusiveFullscreenGui = value;
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
			DisposeSaveSettings();
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
		hostParam.StartupWindowState.RestoreRegion = GlobalAppData.LastSessionWindowState.RestoreRegion;
		hostParam.StartupWindowState.Position = GlobalAppData.LastSessionWindowState.Position;
		hostParam.StartupWindowState.Size = GlobalAppData.LastSessionWindowState.Size;
		hostParam.StartupWindowState.IsFullscreen = GlobalAppData.LastSessionWindowState.IsFullscreen;
		hostParam.StartupWindowState.IsMaximized = GlobalAppData.LastSessionWindowState.IsMaximized;
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

#if COMFY_DEBUG && 0 // TEMP:
		if (io.KeyCtrl && io.KeyShift && Gui::IsKeyPressed(Input::KeyCode_B, false))
			showMainMenuBar ^= true;

		if (Gui::IsKeyPressed(Input::KeyCode_F10))
			exclusiveFullscreenGui ^= true;
#endif

		const bool menuBarVisible = (showMainMenuBar && !exclusiveFullscreenGui);

		if (menuBarVisible)
			GuiMainMenuBar();

		host->GuiMainDockspace(menuBarVisible);

		// HACK: Not quite sure how to best handle this...
		if (host->IsWindowFocused() && Gui::GetActiveID() == 0)
		{
			if (Gui::IsKeyPressed(ToggleFullscreenKey, false))
				host->ToggleFullscreen();
		}

		if (exclusiveFullscreenGui)
		{
			const auto* viewport = Gui::GetMainViewport();
			Gui::SetNextWindowPos(viewport->Pos);
			Gui::SetNextWindowSize(viewport->Size);
			Gui::SetNextWindowViewport(viewport->ID);

			constexpr ImGuiWindowFlags fullscreenWindowFlags = ImGuiWindowFlags_None
				| ImGuiWindowFlags_NoDocking
				| ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
				| ImGuiWindowFlags_NoNavFocus
				| ImGuiWindowFlags_NoBackground
				| ImGuiWindowFlags_NoSavedSettings;

			Gui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			Gui::PushStyleVar(ImGuiStyleVar_WindowPadding, vec2(0.0f, 0.0f));
			const bool fullscreenWindowOpen = Gui::Begin("##ExclusiveWindow", nullptr, fullscreenWindowFlags);
			Gui::PopStyleVar(2);

			if (fullscreenWindowOpen)
				editorManager->GuiExclusiveFullscreen();
			Gui::End();
		}
		else
		{
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
		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(8.0f, Gui::GetStyle().ItemSpacing.y));
		if (Gui::BeginMainMenuBar())
		{
			editorManager->GuiComponentMenu();
			GuiApplicationWindowMenu();
			editorManager->GuiWorkSpaceMenu();
			GuiTestWindowMenus();
			GuiHelpMenus();
			GuiMenuBarAudioAndPerformanceDisplay();
			Gui::EndMainMenuBar();
		}
		Gui::PopStyleVar(1);
		Gui::PopStyleColor(1);
	}

	void Application::GuiApplicationWindowMenu()
	{
		if (Gui::BeginMenu("Window"))
		{
			if (Gui::MenuItem("Toggle Fullscreen", Input::GetKeyCodeName(ToggleFullscreenKey)))
				host->ToggleFullscreen();

			if (Gui::BeginMenu("Swap Interval"))
			{
				const auto swapInterval = host->GetSwapInterval();
				if (Gui::MenuItem("Swap Interval 0 - Unlimited", nullptr, (swapInterval == 0)))
					host->SetSwapInterval(0);

				if (Gui::MenuItem("Swap Interval 1 - VSync", nullptr, (swapInterval == 1)))
					host->SetSwapInterval(1);

				Gui::EndMenu();
			}

			Gui::Separator();

			// TODO: Reset to default layout

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

	void Application::GuiTestWindowMenus()
	{
		if (Gui::BeginMenu("Test Windows"))
		{
#if COMFY_DEBUG
			Gui::MenuItem("Style Editor", nullptr, &showStyleEditor);
			Gui::MenuItem("Demo Window", nullptr, &showDemoWindow);

			Gui::Separator();
#endif

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
		bool openAboutPopup = false;

		if (Gui::BeginMenu("Help"))
		{
			// TODO: Should probably be removed from here and as a more clean tab be added to the about window
			Gui::PushStyleColor(ImGuiCol_Text, Gui::GetColorU32(ImGuiCol_Text, 0.75f));
			Gui::TextUnformatted(Gui::StringViewStart(ComfyCopyrightNotice), Gui::StringViewEnd(ComfyCopyrightNotice));
			Gui::PopStyleColor();
			Gui::Separator();

			if (Gui::MenuItem("Open User Manual"))
				IO::Shell::OpenWithDefaultProgram(UserManualDocumentFilePath);
			Gui::Separator();

			if (Gui::MenuItem("About Comfy Studio"))
				openAboutPopup = true;

			Gui::EndMenu();
		}

		if (openAboutPopup)
		{
			aboutWindowIsOpen = true;
			Gui::OpenPopup(AboutWindowName);
		}
		GuiAboutPopup();
	}

	void Application::GuiAboutPopup()
	{
		const auto viewport = Gui::GetWindowViewport();
		Gui::SetNextWindowPos(viewport->Pos + viewport->Size / 4.0f, ImGuiCond_Appearing);
		Gui::SetNextWindowSize(viewport->Size * 0.5f, ImGuiCond_Appearing);

		if (Gui::WideBeginPopupModal(AboutWindowName, &aboutWindowIsOpen, ImGuiWindowFlags_None))
		{
			Gui::BeginTabBar("AboutTabBar", ImGuiTabBarFlags_NoTooltip);
			if (Gui::BeginTabItem("Version", nullptr, ImGuiTabItemFlags_NoCloseButton))
			{
				Gui::BeginChild("VersionWindowChild", vec2(0.0f, 0.0f), true);
				Gui::Columns(2);
				{
					auto guiPropertyValue = [&](const char* property, const char* value)
					{
						Gui::AlignTextToFramePadding();
						Gui::Text("%s %s", "BuildVersion", property);
						Gui::NextColumn();
						Gui::AlignTextToFramePadding();
						Gui::TextUnformatted(value);
						Gui::NextColumn();
						Gui::Separator();
					};

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

				Gui::EndTabItem();
			}
			if (Gui::BeginTabItem("License", nullptr, ImGuiTabItemFlags_NoCloseButton))
			{
				licenseWindow.DrawGui();
				Gui::EndTabItem();
			}
			Gui::EndTabBar();

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

	void Application::DisposeSaveSettings()
	{
		GlobalAppData.LastSessionWindowState.RestoreRegion = host->GetWindowRestoreRegion();
		GlobalAppData.LastSessionWindowState.Position = host->GetWindowPosition();
		GlobalAppData.LastSessionWindowState.Size = host->GetWindowSize();
		GlobalAppData.LastSessionWindowState.IsFullscreen = host->GetIsFullscreen();
		GlobalAppData.LastSessionWindowState.IsMaximized = host->GetIsMaximized();
		GlobalAppData.SaveToFile();

#if COMFY_DEBUG && 1 // DEBUG: Just for testing
		GlobalUserData.SaveToFile();
#else
		if (!GlobalUserDataExistedOnLoad)
			GlobalUserData.SaveToFile();
#endif
	}
}
