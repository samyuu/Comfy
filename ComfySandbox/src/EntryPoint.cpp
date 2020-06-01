#include "Types.h"
#include "CoreTypes.h"
#include "CoreMacros.h"
#include "Window/ApplicationHost.h"
#include "PerformanceOverlay.h"
#include "Tests/TestTask.h"
#include "System/ComfyData.h"

namespace Comfy::Sandbox
{
	void Main()
	{
		System::MountComfyData();
		System::LoadComfyConfig();

		ApplicationHost::ConstructionParam param;
		param.StartupWindowState.Title = "ComfySandbox - YEP COCK";

		auto host = ApplicationHost(param);
		host.SetSwapInterval(1);

		bool showPerformanceOverlay = true;
		PerformanceOverlay performanceOverlay = {};

		std::unique_ptr<Tests::ITestTask> currentTestTask = nullptr;

#if 1
		Tests::TestTaskInitializer::IterateRegistered([&](const auto& initializer)
		{
			if (std::strcmp(initializer.DerivedName, "Comfy::Sandbox::Tests::MenuTest") == 0)
				currentTestTask = initializer.Function();
		});
#endif

		host.EnterProgramLoop([&]
		{
			bool showMenuBar = false;

			if (showMenuBar && Gui::BeginMainMenuBar())
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

			host.GuiMainDockspace(showMenuBar);

			constexpr auto returnKey = Input::KeyCode_F4;
			if (Gui::IsKeyPressed(returnKey, false))
				currentTestTask = nullptr;

			if (Gui::IsKeyPressed(Input::KeyCode_F1))
				host.SetSwapInterval(1);
			if (Gui::IsKeyPressed(Input::KeyCode_F2))
				host.SetSwapInterval(0);
			if (Gui::IsKeyPressed(Input::KeyCode_F3))
				showPerformanceOverlay ^= true;

			if (currentTestTask == nullptr)
			{
				const auto fullscreenWindowFlags = (ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoSavedSettings);
				if (Gui::Begin("TestTaskSelection", nullptr, fullscreenWindowFlags))
				{
					Gui::TextUnformatted("Available ITestTasks:");
					Gui::SameLine();
					Gui::TextDisabled("(Return to TestTaskSelection by pressing '%s')", Input::GetKeyCodeName(returnKey));
					Gui::Separator();
					Tests::TestTaskInitializer::IterateRegistered([&](const auto& initializer)
					{
						if (Gui::Selectable(initializer.DerivedName))
							currentTestTask = initializer.Function();
					});
				}
				Gui::End();
			}
			else
			{
				currentTestTask->Update();
			}

			if (showPerformanceOverlay)
				performanceOverlay.Gui(showPerformanceOverlay);

			// Gui::DEBUG_NOSAVE_WINDOW("ShowStyleEditor", [&] { Gui::ShowStyleEditor(); }, ImGuiWindowFlags_None);
			// Gui::DEBUG_NOSAVE_WINDOW("ShowDemoWindow", [&] { Gui::ShowDemoWindow(); }, ImGuiWindowFlags_None);
		});
	}
}

int main(int argc, const char* argv[])
{
	Comfy::Sandbox::Main();
	return EXIT_SUCCESS;
}
