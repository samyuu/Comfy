#include "AudioController.h"
#include "Audio/Core/AudioEngine.h"
#include "Audio/Decoder/AudioDecoderFactory.h"
#include <assert.h>

namespace Editor
{
	AudioController::AudioController()
	{
	}

	AudioController::~AudioController()
	{
		for (const auto &instance : buttonSoundInstancePool)
		{
			if (instance == nullptr)
				continue;
			
			instance->SetIsPlaying(false);
			instance->SetAppendRemove(true);
		}
	}

	void AudioController::Initialize()
	{
		auto audioEngine = Audio::AudioEngine::GetInstance();

		for (auto& instance : buttonSoundInstancePool)
		{
			instance = MakeRefPtr<Audio::AudioInstance>(nullptr, false, "AudioController::ButtonSoundInstance");
			audioEngine->AddAudioInstance(instance);
		}

		// only load the default button sound for now
		buttonSoundSources.push_back(audioEngine->LoadAudioFile(buttonSoundPath));
		buttonSoundIndex = 0;
	}

	void AudioController::PlayButtonSound()
	{
		if (buttonSoundIndex < 0)
			return;

		lastButtonSoundTime = buttonSoundTime;
		buttonSoundTime = TimeSpan::GetTimeNow();
		timeSinceLastButtonSound = buttonSoundTime - lastButtonSoundTime;

		Audio::AudioInstance* longestRunningInstance = nullptr;
		for (const auto &instance : buttonSoundInstancePool)
		{
			if (instance == nullptr)
				continue;

			if (!instance->GetIsPlaying() || instance->GetHasReachedEnd())
			{
				PlayButtonSound(instance.get());
				return;
			}

			if (longestRunningInstance == nullptr || instance->GetPosition() > longestRunningInstance->GetPosition())
				longestRunningInstance = instance.get();
		}

		if (longestRunningInstance != nullptr)
			PlayButtonSound(longestRunningInstance);
	}

	const RefPtr<Audio::MemorySampleProvider>& AudioController::GetButtonSoundSource(int index)
	{
		return buttonSoundSources[buttonSoundIndex];
	}

	void AudioController::PlayButtonSound(Audio::AudioInstance* audioInstance)
	{
		float volume = buttonSoundVolume;

		// subject to change
		float threshold = 50.0f;
		if (timeSinceLastButtonSound.TotalMilliseconds() < threshold)
		{
			float elapsed = static_cast<float>(timeSinceLastButtonSound.TotalMilliseconds() / threshold);
			volume *= (0.0f + elapsed * (1.0f - 0.0f));
		}
		// -----------------

		audioInstance->SetVolume(volume);
		audioInstance->SetSampleProvider(GetButtonSoundSource(buttonSoundIndex));
		audioInstance->Restart();
		audioInstance->SetIsPlaying(true);
	}
}