#pragma once
#include "Core/TimeSpan.h"
#include "Audio/Core/AudioInstance.h"
#include "Audio/SampleProvider/MemorySampleProvider.h"
#include <array>

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

		const RefPtr<Audio::MemorySampleProvider>& GetButtonSoundSource(int index);

	private:
		const char* buttonSoundPath = u8"rom/sound/button/01_button1.wav";

		TimeSpan buttonSoundTime, lastButtonSoundTime, timeSinceLastButtonSound;
		float buttonSoundVolume = 0.75f;
		
		int buttonSoundIndex = -1;
		std::vector<RefPtr<Audio::MemorySampleProvider>> buttonSoundSources;
		std::array<RefPtr<Audio::AudioInstance>, 16> buttonSoundInstancePool;

		void PlayButtonSound(Audio::AudioInstance* audioInstance);
	};
}