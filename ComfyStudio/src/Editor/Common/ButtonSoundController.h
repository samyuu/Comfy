#pragma once
#include "Time/TimeSpan.h"
#include "Audio/Audio.h"

namespace Comfy::Studio::Editor
{
	class ButtonSoundController
	{
	public:
		ButtonSoundController();
		~ButtonSoundController();

	public:
		void PlayButtonSound();

		Audio::SourceHandle GetButtonSoundSource(int index);

	private:
		static constexpr const char* buttonSoundPath = "dev_rom/sound/button/01_button1.wav";

		TimeSpan buttonSoundTime, lastButtonSoundTime, timeSinceLastButtonSound;
		float buttonSoundVolume = 0.75f;

		int buttonSoundIndex = -1;
		std::vector<Audio::SourceHandle> buttonSoundSources;
		std::array<Audio::Voice, 16> buttonSoundVoicePool;

		void PlayButtonSound(Audio::Voice voice);
	};
}
