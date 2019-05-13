#pragma once
#include <array>
#include <memory>
#include "../TimeSpan.h"
#include "../Audio/AudioInstance.h"
#include "../Audio/MemoryAudioStream.h"

namespace Editor
{
	class AudioController
	{
	public:
		AudioController();
		~AudioController();

		void Initialize();

		// need a way to play button sounds (adjust volume based on last sound)
		// pushing back new AudioInstances on a separate thread is unsafe 
		// so instead this class should have a pool of button sound instances and control their playback / positions
		void PlayButtonSound();

		// check button times on separate thread
	
	private:
		const char* buttonSoundPath = u8"rom/sound/button/01_button1.wav";

		TimeSpan buttonSoundTime, lastButtonSoundTime, timeSinceLastButtonSound;
		float buttonSoundVolume = MAX_VOLUME * 0.95f;
		int buttonSoundIndex = -1;
		std::vector<MemoryAudioStream> buttonSoundSources;
		std::array<std::shared_ptr<AudioInstance>, 16> buttonSoundInstancePool;

		void PlayButtonSound(AudioInstance* audioInstance);
	};
}