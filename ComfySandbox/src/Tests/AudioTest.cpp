#include "TestTask.h"

namespace Comfy::Sandbox::Tests
{
	class AudioTest : public ITestTask
	{
	public:
		AudioTest()
		{
			// Audio::AudioEngine::GetInstance().SetAudioBackend(Audio::AudioBackend::WASAPIExclusive);
			Audio::AudioEngine::GetInstance().OpenStartStream();
		}

		void Update() override
		{
			for (const auto keyCode : std::array {
				Input::KeyCode_W, Input::KeyCode_A, Input::KeyCode_S, Input::KeyCode_D,
				Input::KeyCode_I, Input::KeyCode_J, Input::KeyCode_K, Input::KeyCode_L, })
				if (Gui::IsKeyPressed(keyCode, false))
					Audio::AudioEngine::GetInstance().PlaySound(buttonSound, "TestSound");

			if (Gui::Begin("Test Audio Control"))
			{
				if (Gui::Button("WASAPI (Shared)")) Audio::AudioEngine::GetInstance().SetAudioBackend(Audio::AudioBackend::WASAPIShared);
				if (Gui::Button("WASAPI (Exclusive)")) Audio::AudioEngine::GetInstance().SetAudioBackend(Audio::AudioBackend::WASAPIExclusive);
			}
			Gui::End();
		}

	private:
		Audio::SourceHandle buttonSound = Audio::AudioEngine::GetInstance().LoadAudioSource("dev_ram/sound/button/01_button1.wav");
	};
}
