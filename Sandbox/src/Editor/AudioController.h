#pragma once
#include <array>
#include <memory>
#include "../TimeSpan.h"
#include "../Audio/AudioInstance.h"
#include "../Audio/MemoryAudioStream.h"

namespace Editor
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

		MemoryAudioStream* GetButtonSoundSource(int index);

	private:
		const char* buttonSoundPath = u8"rom/sound/button/01_button1.wav";

		TimeSpan buttonSoundTime, lastButtonSoundTime, timeSinceLastButtonSound;
		float buttonSoundVolume = 0.75f;
		
		int buttonSoundIndex = -1;
		std::vector<MemoryAudioStream> buttonSoundSources;
		std::array<std::shared_ptr<AudioInstance>, 16> buttonSoundInstancePool;

		void PlayButtonSound(AudioInstance* audioInstance);
	};
}