#include "ButtonSoundController.h"

namespace Comfy::Studio::Editor
{
	ButtonSoundController::ButtonSoundController()
	{
		auto& audioEngine = Audio::Engine::GetInstance();

		for (size_t i = 0; i < buttonSoundVoicePool.size(); i++)
		{
			char nameBuffer[64];
			sprintf_s(nameBuffer, "ButtonSoundController::ButtonSoundVoices[%02zu]", i);
			buttonSoundVoicePool[i] = audioEngine.AddVoice(Audio::SourceHandle::Invalid, nameBuffer, false);
		}

		// NOTE: Only load the default button sound for now
		buttonSoundSources.reserve(1);
		buttonSoundSources.push_back(audioEngine.LoadAudioSource(buttonSoundPath));

		buttonSoundIndex = 0;
	}

	ButtonSoundController::~ButtonSoundController()
	{
		for (auto& voice : buttonSoundVoicePool)
			Audio::Engine::GetInstance().RemoveVoice(voice);

		for (auto& source : buttonSoundSources)
			Audio::Engine::GetInstance().UnloadSource(source);
	}

	void ButtonSoundController::PlayButtonSound(TimeSpan startTime, std::optional<TimeSpan> externalClock)
	{
		if (buttonSoundIndex < 0)
			return;

		lastButtonSoundTime = buttonSoundTime;
		buttonSoundTime = externalClock.value_or(TimeSpan::GetTimeNow());
		timeSinceLastButtonSound = (buttonSoundTime - lastButtonSoundTime);

		Audio::Voice* longestRunningVoice = nullptr;
		for (auto& voice : buttonSoundVoicePool)
		{
			const auto voicePosition = voice.GetPosition();

			if (!voice.GetIsPlaying() || (voicePosition >= voice.GetDuration()))
			{
				PlayButtonSound(voice, startTime);
				return;
			}

			if (longestRunningVoice == nullptr || voicePosition > longestRunningVoice->GetPosition())
				longestRunningVoice = &voice;
		}

		if (longestRunningVoice != nullptr)
			PlayButtonSound(*longestRunningVoice, startTime);
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
		return buttonSoundSources[buttonSoundIndex];
	}

	void ButtonSoundController::PlayButtonSound(Audio::Voice voice, TimeSpan startTime)
	{
		Audio::Engine::GetInstance().EnsureStreamRunning();
		voice.SetVolume(buttonSoundVolume * GetVolumeFactor());
		voice.SetSource(GetButtonSoundSource(buttonSoundIndex));
		voice.SetPosition(startTime);
		voice.SetIsPlaying(true);
	}

	f32 ButtonSoundController::GetVolumeFactor() const
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
}
