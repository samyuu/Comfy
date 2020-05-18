#include "TestTask.h"

namespace Comfy::Sandbox::Tests
{
	class AudioTest : public ITestTask
	{
	public:
		COMFY_REGISTER_TEST_TASK(AudioTest);

		AudioTest()
		{
			Audio::Engine::GetInstance().OpenStream();
			Audio::Engine::GetInstance().StartStream();
			// Audio::Engine::GetInstance().SetAudioAPI(Audio::Engine::AudioAPI::ASIO);
		}

		void Update() override
		{
			for (const auto keyCode : std::array {
				Input::KeyCode_W, Input::KeyCode_A, Input::KeyCode_S, Input::KeyCode_D,
				Input::KeyCode_I, Input::KeyCode_J, Input::KeyCode_K, Input::KeyCode_L, })
				if (Gui::IsKeyPressed(keyCode, false))
					Audio::Engine::GetInstance().PlaySound(buttonSound, "TestSound");
		}

	private:
		Audio::SourceHandle buttonSound = Audio::Engine::GetInstance().LoadAudioSource("dev_ram/sound/button/01_button1.wav");
	};
}
