#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "CoreMacros.h"
#include "Window/ApplicationHost.h"
#include "PerformanceOverlay.h"
#include "Tests/TestTask.h"
#include "System/ComfyData.h"

namespace Comfy::Sandbox::Tests
{
	std::vector<TestTaskInitializer> RegisterAllTaskInitializers();
}

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
		void Run();

	private:
		void SetStartupTestTask();
		void SetActiveTestTask(const Tests::TestTaskInitializer& initializer);

	private:
		void Gui();
		void GuiMainMenuBar();
		void CheckGlobalKeyBindings();
		void GuiTestTaskSelection();

	private:

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

		std::vector<Tests::TestTaskInitializer> registeredTaskInitializers = Tests::RegisterAllTaskInitializers();
		std::unique_ptr<Tests::ITestTask> activeTestTask = nullptr;
	};
}
