#pragma once
#include "Time/TimeSpan.h"
#include "Audio/Audio.h"

namespace Comfy::Studio::Editor
{
	// This class stores a pool of button sound instances.
	// It controls their volume levels and exposes a public play method.
	class AudioController
	{
	public:
		AudioController();
		~AudioController();

		void Initialize();
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
