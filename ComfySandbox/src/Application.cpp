#include "Application.h"

// NOTE: Make sure *not* to include these inline classes in the project
#include "Tests/AetRendererTest.cpp"
#include "Tests/AudioTest.cpp"
#include "Tests/FontRendererTest.cpp"
#include "Tests/MenuTest.cpp"
#include "Tests/Renderer2DTest.cpp"
#include "Tests/Renderer3DTest.cpp"

namespace Comfy::Sandbox::Tests
{
	std::vector<TestTaskInitializer> RegisterAllTaskInitializers()
	{
		return
		{
			TestTaskInitializer::Create<AetRendererTest>("Comfy::Sandbox::Tests::AetRendererTest"),
			TestTaskInitializer::Create<AudioTest>("Comfy::Sandbox::Tests::AudioTest"),
			TestTaskInitializer::Create<FontRendererTest>("Comfy::Sandbox::Tests::FontRendererTest"),
			TestTaskInitializer::Create<MenuTest>("Comfy::Sandbox::Tests::MenuTest"),
			TestTaskInitializer::Create<Renderer2DTest>("Comfy::Sandbox::Tests::Renderer2DTest"),
			TestTaskInitializer::Create<Renderer3DTest>("Comfy::Sandbox::Tests::Renderer3DTest"),
		};
	}
}

namespace Comfy::Sandbox
{
	void Application::Run()
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

	void Application::SetStartupTestTask()
	{
		constexpr std::string_view startupTaskName = "Comfy::Sandbox::Tests::AetRendererTest";

		for (const auto& initializer : registeredTaskInitializers)
		{
			if (initializer.Name == startupTaskName)
				SetActiveTestTask(initializer);
		}
	}

	void Application::SetActiveTestTask(const Tests::TestTaskInitializer& initializer)
	{
		std::string formattedTitle;
		formattedTitle.reserve(ComfySandboxWindowTitle.size() + initializer.Name.size() + 3);
		formattedTitle += ComfySandboxWindowTitle;
		formattedTitle += " - ";
		formattedTitle += initializer.Name;

		host->SetWindowTitle(formattedTitle);
		activeTestTask = initializer.Function();
	}

	void Application::Gui()
	{
		if (showMenuBar)
			GuiMainMenuBar();

		host->GuiMainDockspace(showMenuBar);

		CheckGlobalKeyBindings();

		if (activeTestTask == nullptr)
			GuiTestTaskSelection();
		else
			activeTestTask->Update();

		if (showPerformanceOverlay)
			performanceOverlay.Gui(showPerformanceOverlay);

		// Gui::DEBUG_NOSAVE_WINDOW("ShowStyleEditor", [&] { Gui::ShowStyleEditor(); }, ImGuiWindowFlags_None);
		// Gui::DEBUG_NOSAVE_WINDOW("ShowDemoWindow", [&] { Gui::ShowDemoWindow(); }, ImGuiWindowFlags_None);
	}

	void Application::GuiMainMenuBar()
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

	void Application::CheckGlobalKeyBindings()
	{
		if (Gui::IsKeyPressed(keyBindings.TestSelectionReturnKey, false))
			activeTestTask = nullptr;

		if (Gui::IsKeyPressed(keyBindings.SwapIntervalOneKey))
			host->SetSwapInterval(1);

		if (Gui::IsKeyPressed(keyBindings.SwapIntervalZeroKey))
			host->SetSwapInterval(0);

		if (Gui::IsKeyPressed(keyBindings.TogglePerformanceOverlayKey))
			showPerformanceOverlay ^= true;
	}

	void Application::GuiTestTaskSelection()
	{
		const auto fullscreenWindowFlags = (ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoSavedSettings);
		if (Gui::Begin("TestTaskSelection", nullptr, fullscreenWindowFlags))
		{
			Gui::TextUnformatted("Available ITestTasks:");
			Gui::SameLine();
			Gui::TextDisabled("(Return to TestTaskSelection by pressing '%s')", Input::GetKeyCodeName(keyBindings.TestSelectionReturnKey));
			Gui::Separator();

			for (const auto& initializer : registeredTaskInitializers)
			{
				if (Gui::Selectable(initializer.Name.data()))
					SetActiveTestTask(initializer);
			}
		}
		Gui::End();
	}
}
