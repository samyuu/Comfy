#include "Window/ApplicationHost.h"
#include "ImGui/Gui.h"
#include "Audio/Audio.h"
#include "Input/Input.h"

namespace Comfy
{
	void Main()
	{
		ApplicationHost::ConstructionParam param = {};
		param.WindowTitle = "YEP COCK";
		param.IconHandle = nullptr;

		auto host = ApplicationHost(param);
		host.SetSwapInterval(0);

		const auto buttonSoundSourceHandle = Audio::Engine::GetInstance().LoadAudioSource("dev_ram/sound/button/01_button1.wav");
		Audio::Engine::GetInstance().SetAudioAPI(Audio::Engine::AudioAPI::ASIO);
		Audio::Engine::GetInstance().OpenStream();
		Audio::Engine::GetInstance().StartStream();

		host.EnterProgramLoop([&]
		{
			static int i = 0;
			char titleBuffer[64];
			sprintf_s(titleBuffer, "Test Title : Frame %d : FPS %.3f", i++, Gui::GetIO().Framerate);
			host.SetWindowTitle(titleBuffer);

			const ImGuiViewport* viewport = Gui::GetMainViewport();
			Gui::SetNextWindowPos(viewport->Pos);
			Gui::SetNextWindowSize(viewport->Size);
			Gui::SetNextWindowViewport(viewport->ID);

			ImGuiWindowFlags fullscreenFlags = ImGuiWindowFlags_None;
			fullscreenFlags |= ImGuiWindowFlags_NoDocking;
			fullscreenFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			fullscreenFlags |= ImGuiWindowFlags_NoNavFocus;
			fullscreenFlags |= ImGuiWindowFlags_NoSavedSettings;

			if (Gui::Begin("Test", nullptr, fullscreenFlags))
			{
				Gui::GetWindowDrawList()->AddText(Gui::GetMousePos(), IM_COL32_WHITE, titleBuffer);

				if (Gui::BeginPopupContextWindow())
				{
					Gui::MenuItem("Test 0");
					Gui::MenuItem("Test 1");
					Gui::End();
				}
			}
			Gui::End();

			if (Gui::IsKeyPressed(Input::KeyCode_K, false))
				Audio::Engine::GetInstance().PlaySound(buttonSoundSourceHandle, "TestSound");
		});
	}
}

int main(int argc, const char* argv[])
{
	Comfy::Main();
}
