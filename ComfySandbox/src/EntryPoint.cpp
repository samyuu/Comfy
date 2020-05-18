#include "Types.h"
#include "CoreTypes.h"
#include "CoreMacros.h"
#include "Window/ApplicationHost.h"
#include "Tests/TestTask.h"

namespace Comfy::Sandbox
{
	void Main()
	{
		ApplicationHost::ConstructionParam param = {};
		param.WindowTitle = "YEP COCK";
		param.IconHandle = nullptr;

		auto host = ApplicationHost(param);
		host.SetSwapInterval(1);

		std::unique_ptr<Tests::ITestTask> currentTestTask = nullptr; // std::make_unique<Tests::AetRendererTest>();
		host.EnterProgramLoop([&]
		{
			constexpr auto returnKey = Input::KeyCode_Escape;
			if (Gui::IsKeyPressed(returnKey, false))
				currentTestTask = nullptr;

			if (Gui::IsKeyPressed(Input::KeyCode_F1))
				host.SetSwapInterval(1);
			if (Gui::IsKeyPressed(Input::KeyCode_F2))
				host.SetSwapInterval(0);

			if (currentTestTask == nullptr)
			{
				const auto viewport = Gui::GetMainViewport();
				Gui::SetNextWindowPos(viewport->Pos);
				Gui::SetNextWindowSize(viewport->Size);
				Gui::SetNextWindowViewport(viewport->ID);

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
