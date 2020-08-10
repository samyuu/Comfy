#include "ButtonSoundController.h"

namespace Comfy::Studio::Editor
{
	ButtonSoundController::ButtonSoundController()
	{
		auto& audioEngine = Audio::AudioEngine::GetInstance();

		for (size_t i = 0; i < buttonSoundVoicePool.size(); i++)
		{
			char nameBuffer[64];
			sprintf_s(nameBuffer, "ButtonSoundController::ButtonSoundVoices[%02zu]", i);
			buttonSoundVoicePool[i] = audioEngine.AddVoice(Audio::SourceHandle::Invalid, nameBuffer, false);
			buttonSoundVoicePool[i].SetPauseOnEnd(true);
		}

		// NOTE: Only load the default button sound for now
		loadedSoundSources.reserve(1);
		loadedSoundSources.push_back(AsyncSoundSource { Audio::SourceHandle::Invalid, audioEngine.LoadAudioSourceAsync(buttonSoundPath) });

		buttonSoundIndex = 0;
	}

	ButtonSoundController::~ButtonSoundController()
	{
		for (auto& voice : buttonSoundVoicePool)
			Audio::AudioEngine::GetInstance().RemoveVoice(voice);

		for (auto& source : loadedSoundSources)
			Audio::AudioEngine::GetInstance().UnloadSource(source.Get());
	}

	void ButtonSoundController::PlayButtonSound(TimeSpan startTime, std::optional<TimeSpan> externalClock)
	{
		if (buttonSoundIndex < 0)
			return;

		lastButtonSoundTime = buttonSoundTime;
		buttonSoundTime = externalClock.value_or(TimeSpan::GetTimeNow());
		timeSinceLastButtonSound = (buttonSoundTime - lastButtonSoundTime);

		if (const auto voiceToUse = FindIdleOrLongestRunningVoice(); voiceToUse != nullptr)
			PlayButtonSoundUsingVoice(*voiceToUse, startTime);
	}

	void ButtonSoundController::PauseAllNegativeVoices()
	{
		for (auto& voice : buttonSoundVoicePool)
		{
			if (voice.GetIsPlaying() && voice.GetPosition() < TimeSpan::Zero())
			{
				voice.SetIsPlaying(false);
				voice.SetPosition(voice.GetDuration());
			}
		}

		lastButtonSoundTime = buttonSoundTime = timeSinceLastButtonSound = {};
	}

	Audio::SourceHandle ButtonSoundController::GetButtonSoundSource(int index)
	{
		return loadedSoundSources[buttonSoundIndex].Get();
	}

	Audio::Voice* ButtonSoundController::FindIdleOrLongestRunningVoice()
	{
		Audio::Voice* longestRunningVoice = nullptr;
		for (auto& voice : buttonSoundVoicePool)
		{
			const auto voicePosition = voice.GetPosition();

			if (!voice.GetIsPlaying() || (voicePosition >= voice.GetDuration()))
				return &voice;

			if (longestRunningVoice == nullptr || voicePosition > longestRunningVoice->GetPosition())
				longestRunningVoice = &voice;
		}

		return longestRunningVoice;
	}

	void ButtonSoundController::PlayButtonSoundUsingVoice(Audio::Voice voice, TimeSpan startTime)
	{
		Audio::AudioEngine::GetInstance().EnsureStreamRunning();
		voice.SetSource(GetButtonSoundSource(buttonSoundIndex));
		voice.SetVolume(buttonSoundVolume * GetLastButtonSoundTimeVolumeFactor());
		voice.SetPosition(startTime);
		voice.SetIsPlaying(true);
	}

	f32 ButtonSoundController::GetLastButtonSoundTimeVolumeFactor() const
	{
		constexpr auto threshold = 35.0;
		const auto timeSinceSound = timeSinceLastButtonSound.TotalMilliseconds();

		if (timeSinceSound >= threshold)
			return 1.0f;

		// NOTE: All of this is far from perfect
		const auto delta = static_cast<f32>(timeSinceSound / threshold);
		const auto factor = std::clamp((delta * delta), 0.0f, 1.0f);

		return factor;
	}

	Audio::SourceHandle ButtonSoundController::AsyncSoundSource::Get()
	{
		if (FutureHandle.valid())
			Handle = FutureHandle.get();

		return Handle;
	}
}
