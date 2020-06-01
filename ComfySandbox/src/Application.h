#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "CoreMacros.h"
#include "Window/ApplicationHost.h"
#include "PerformanceOverlay.h"
#include "Tests/TestTask.h"
#include "System/ComfyData.h"

namespace Comfy::Sandbox
{
	class Application : NonCopyable
	{
	public:
		static constexpr std::string_view ComfySandboxWindowTitle = "Comfy Sandbox";

	public:
		Application() = default;
		~Application() = default;

	public:
		void Run()
		{
			System::MountComfyData();
			System::LoadComfyConfig();

			ApplicationHost::ConstructionParam hostParam;
			hostParam.StartupWindowState.Title = ComfySandboxWindowTitle;

			host = std::make_unique<ApplicationHost>(hostParam);
			host->SetSwapInterval(1);

			SetStartupTestTask();

			host->EnterProgramLoop([&]()
			{
				Gui();
			});
		}

	private:
		void SetStartupTestTask()
		{
			static constexpr const char* startupTaskName = "Comfy::Sandbox::Tests::MenuTest";

			if (startupTaskName == nullptr)
				return;

			Tests::TestTaskInitializer::IterateRegistered([&](const Tests::TestTaskInitializer& initializer)
			{
				if (std::strcmp(initializer.DerivedName, startupTaskName) == 0)
					SetCurrentTestTask(initializer.Function(), initializer.DerivedName);
			});
		}

	private:
		void Gui()
		{
			if (showMenuBar)
				GuiMainMenuBar();

			host->GuiMainDockspace(showMenuBar);

			CheckGlobalKeyBindings();

			if (currentTestTask == nullptr)
				GuiTestTaskSelection();
			else
				currentTestTask->Update();

			if (showPerformanceOverlay)
				performanceOverlay.Gui(showPerformanceOverlay);

			// Gui::DEBUG_NOSAVE_WINDOW("ShowStyleEditor", [&] { Gui::ShowStyleEditor(); }, ImGuiWindowFlags_None);
			// Gui::DEBUG_NOSAVE_WINDOW("ShowDemoWindow", [&] { Gui::ShowDemoWindow(); }, ImGuiWindowFlags_None);
		}

		void GuiMainMenuBar()
		{
			if (Gui::BeginMainMenuBar())
			{
				if (Gui::BeginMenu("YEP"))
				{
					Gui::MenuItem("COCK");
					Gui::EndMenu();
				}

				if (Gui::BeginMenu("NOP"))
				{
					Gui::MenuItem("COCK");
					Gui::EndMenu();
				}

				Gui::EndMainMenuBar();
			}
		}

		void CheckGlobalKeyBindings()
		{
			if (Gui::IsKeyPressed(keyBindings.TestSelectionReturnKey, false))
				currentTestTask = nullptr;

			if (Gui::IsKeyPressed(keyBindings.SwapIntervalOneKey))
				host->SetSwapInterval(1);

			if (Gui::IsKeyPressed(keyBindings.SwapIntervalZeroKey))
				host->SetSwapInterval(0);

			if (Gui::IsKeyPressed(keyBindings.TogglePerformanceOverlayKey))
				showPerformanceOverlay ^= true;
		}

		void GuiTestTaskSelection()
		{
			const auto fullscreenWindowFlags = (ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoSavedSettings);
			if (Gui::Begin("TestTaskSelection", nullptr, fullscreenWindowFlags))
			{
				Gui::TextUnformatted("Available ITestTasks:");
				Gui::SameLine();
				Gui::TextDisabled("(Return to TestTaskSelection by pressing '%s')", Input::GetKeyCodeName(keyBindings.TestSelectionReturnKey));
				Gui::Separator();
				Tests::TestTaskInitializer::IterateRegistered([&](const Tests::TestTaskInitializer& initializer)
				{
					if (Gui::Selectable(initializer.DerivedName))
						SetCurrentTestTask(initializer.Function(), initializer.DerivedName);
				});
			}
			Gui::End();
		}

	private:
		void SetCurrentTestTask(std::unique_ptr<Tests::ITestTask> task, std::string_view taskName)
		{
			host->SetWindowTitle(std::string(ComfySandboxWindowTitle) + " - " + std::string(taskName));
			currentTestTask = std::move(task);
		}

	private:
		std::unique_ptr<ApplicationHost> host = nullptr;

		const bool showMenuBar = false;

		bool showPerformanceOverlay = true;
		PerformanceOverlay performanceOverlay = {};

		struct KeyBindingsData
		{
			const Input::KeyCode TestSelectionReturnKey = Input::KeyCode_F4;
			const Input::KeyCode SwapIntervalOneKey = Input::KeyCode_F1;
			const Input::KeyCode SwapIntervalZeroKey = Input::KeyCode_F2;
			const Input::KeyCode TogglePerformanceOverlayKey = Input::KeyCode_F3;
		} keyBindings;

		std::unique_ptr<Tests::ITestTask> currentTestTask = nullptr;
	};
}
