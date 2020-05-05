#include "AudioController.h"
#include "Audio/Core/AudioEngine.h"
#include "Audio/Decoder/AudioDecoderFactory.h"

namespace Comfy::Editor
{
	AudioController::AudioController()
	{
	}

	AudioController::~AudioController()
	{
		for (auto& voice : buttonSoundVoicePool)
			Audio::AudioEngine::GetInstance().RemoveVoice(voice);
	}

	void AudioController::Initialize()
	{
		auto& audioEngine = Audio::AudioEngine::GetInstance();

		for (auto& voice : buttonSoundVoicePool)
			voice = audioEngine.AddVoice(Audio::SourceHandle::Invalid, "AudioController::ButtonSoundVoice", false);

		// NOTE: Only load the default button sound for now
		buttonSoundSources.push_back(audioEngine.LoadAudioSource(buttonSoundPath));
		buttonSoundIndex = 0;
	}

	void AudioController::PlayButtonSound()
	{
		if (buttonSoundIndex < 0)
			return;

		lastButtonSoundTime = buttonSoundTime;
		buttonSoundTime = TimeSpan::GetTimeNow();
		timeSinceLastButtonSound = buttonSoundTime - lastButtonSoundTime;

		Audio::Voice* longestRunningVoice = nullptr;
		for (auto& voice : buttonSoundVoicePool)
		{
			const auto voicePosition = voice.GetPosition();

			if (!voice.GetIsPlaying() || (voicePosition >= voice.GetDuration()))
			{
				PlayButtonSound(voice);
				return;
			}

			if (longestRunningVoice == nullptr || voicePosition > longestRunningVoice->GetPosition())
				longestRunningVoice = &voice;
		}

		if (longestRunningVoice != nullptr)
			PlayButtonSound(*longestRunningVoice);
	}

	Audio::SourceHandle AudioController::GetButtonSoundSource(int index)
	{
		return buttonSoundSources[buttonSoundIndex];
	}

	void AudioController::PlayButtonSound(Audio::Voice voice)
	{
		float volume = buttonSoundVolume;

		// NOTE: Subject to change
		constexpr float threshold = 50.0f;
		if (timeSinceLastButtonSound.TotalMilliseconds() < threshold)
		{
			const float elapsed = static_cast<float>(timeSinceLastButtonSound.TotalMilliseconds() / threshold);
			volume *= (0.0f + elapsed * (1.0f - 0.0f));
		}

		voice.SetVolume(volume);
		voice.SetSource(GetButtonSoundSource(buttonSoundIndex));
		voice.SetPosition(TimeSpan::Zero());
		voice.SetIsPlaying(true);
	}
}
