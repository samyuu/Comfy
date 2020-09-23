#include "ButtonSoundController.h"

namespace Comfy::Studio::Editor
{
	namespace
	{
		template <size_t Size>
		constexpr size_t IncrementRingIndex(size_t index)
		{
			static_assert(Size > 0);
			return (++index % Size);
		}

		template <size_t Size>
		constexpr size_t DecrementRingIndex(size_t index)
		{
			static_assert(Size > 0);
			return (index == 0) ? (Size - 1) : (--index % Size);
		}
	}

	ButtonSoundController::ButtonSoundController()
	{
		auto& audioEngine = Audio::AudioEngine::GetInstance();
		char nameBuffer[64];

		for (size_t i = 0; i < ButtonVoicePoolSize; i++)
		{
			const auto nameView = std::string_view(nameBuffer, sprintf_s(nameBuffer, "ButtonSoundController::ButtonVoicePool[%02zu]", i));
			buttonVoicePool[i] = audioEngine.AddVoice(Audio::SourceHandle::Invalid, nameView, false);
			buttonVoicePool[i].SetPauseOnEnd(true);
		}

		auto getSlotCharID = [](size_t slot) -> char { assert(EnumCount<ChainSoundSlot>() == 2 && slot < 2); return std::array { 'L', 'R' }[slot]; };

		for (size_t slotIndex = 0; slotIndex < EnumCount<ChainSoundSlot>(); slotIndex++)
		{
			auto& slotVoicePool = chainStartVoicePools[slotIndex];
			for (size_t i = 0; i < PerSlotChainVoicePoolSize; i++)
			{
				const auto nameView = std::string_view(nameBuffer, sprintf_s(nameBuffer, "ButtonSoundController::ChainStartVoicePool[%c][%02zu]", getSlotCharID(slotIndex), i));
				slotVoicePool[i] = audioEngine.AddVoice(Audio::SourceHandle::Invalid, nameView, false);
				slotVoicePool[i].SetPauseOnEnd(true);
			}
		}

		for (size_t slotIndex = 0; slotIndex < EnumCount<ChainSoundSlot>(); slotIndex++)
		{
			auto& slotVoicePool = chainEndVoicePools[slotIndex];
			for (size_t i = 0; i < PerSlotChainVoicePoolSize; i++)
			{
				const auto nameView = std::string_view(nameBuffer, sprintf_s(nameBuffer, "ButtonSoundController::ChainEndVoicePool[%c][%02zu]", getSlotCharID(slotIndex), i));
				slotVoicePool[i] = audioEngine.AddVoice(Audio::SourceHandle::Invalid, nameView, false);
				slotVoicePool[i].SetPauseOnEnd(true);
			}
		}

		// NOTE: Only load the default sounds for now

		auto& buttonSources = loadedSources[static_cast<u8>(ButtonSoundType::Button)];
		buttonSources.reserve(1);
		buttonSources.emplace_back("dev_rom/sound/button/01_button1.wav", 0.6f);

		auto& slideSources = loadedSources[static_cast<u8>(ButtonSoundType::Slide)];
		slideSources.reserve(1);
		slideSources.emplace_back("dev_rom/sound/slide_se/slide_se13.wav", 0.26f);

		auto& chainFirstSources = loadedSources[static_cast<u8>(ButtonSoundType::ChainSlideFirst)];
		chainFirstSources.reserve(1);
		chainFirstSources.emplace_back("dev_rom/sound/slide_long/slide_long02a.wav", 0.62f);

		auto& chainSubSources = loadedSources[static_cast<u8>(ButtonSoundType::ChainSlideSub)];
		chainSubSources.reserve(1);
		chainSubSources.emplace_back("dev_rom/sound/slide_long/slide_button08.wav", 0.09f);

		auto& chainSuccessSources = loadedSources[static_cast<u8>(ButtonSoundType::ChainSlideSuccess)];
		chainSuccessSources.reserve(1);
		chainSuccessSources.emplace_back("dev_rom/sound/slide_long/slide_ok03.wav", 0.7f);

		auto& chainFailureSources = loadedSources[static_cast<u8>(ButtonSoundType::ChainSlideFailure)];
		chainFailureSources.reserve(1);
		chainFailureSources.emplace_back("dev_rom/sound/slide_long/slide_ng03.wav", 0.44f);
	}

	ButtonSoundController::~ButtonSoundController()
	{
		auto& audioEngine = Audio::AudioEngine::GetInstance();

		for (auto& voice : buttonVoicePool)
			audioEngine.RemoveVoice(voice);

		for (auto& voices : chainStartVoicePools)
		{
			for (auto& voice : voices)
				audioEngine.RemoveVoice(voice);
		}

		for (auto& voices : chainEndVoicePools)
		{
			for (auto& voice : voices)
				audioEngine.RemoveVoice(voice);
		}

		for (auto& sources : loadedSources)
		{
			for (auto& source : sources)
				audioEngine.UnloadSource(source.Get());
		}
	}

	void ButtonSoundController::PlayButtonSound(TimeSpan startTime, std::optional<TimeSpan> externalClock)
	{
		PlayButtonSoundType(ButtonSoundType::Button, startTime, externalClock);
	}

	void ButtonSoundController::PlaySlideSound(TimeSpan startTime, std::optional<TimeSpan> externalClock)
	{
		PlayButtonSoundType(ButtonSoundType::Slide, startTime, externalClock);
	}

	void ButtonSoundController::PlayChainSoundStart(ChainSoundSlot slot, TimeSpan startTime, std::optional<TimeSpan> externalClock)
	{
		PlayChainSoundType(ButtonSoundType::ChainSlideFirst, slot, startTime, externalClock);
	}

	void ButtonSoundController::PlayChainSoundSuccess(ChainSoundSlot slot, TimeSpan startTime, std::optional<TimeSpan> externalClock)
	{
		PlayChainSoundType(ButtonSoundType::ChainSlideSuccess, slot, startTime, externalClock);
	}

	void ButtonSoundController::PlayChainSoundFailure(ChainSoundSlot slot, TimeSpan startTime, std::optional<TimeSpan> externalClock)
	{
		PlayChainSoundType(ButtonSoundType::ChainSlideFailure, slot, startTime, externalClock);
	}

	void ButtonSoundController::FadeOutLastChainSound(ChainSoundSlot slot)
	{
		const auto slotIndex = static_cast<u8>(slot);
		auto& voice = chainStartVoicePools[slotIndex][DecrementRingIndex<PerSlotChainVoicePoolSize>(chainStartPoolRingIndices[slotIndex])];

		const auto voicePosition = voice.GetPosition();
		voice.SetVolumeMap(voicePosition, voicePosition + chainFadeOutDuration, 1.0f, 0.0f);
	}

	void ButtonSoundController::PauseAllChainSounds()
	{
		for (auto& voicePool : chainStartVoicePools)
		{
			for (auto& voice : voicePool)
				voice.SetIsPlaying(false);
		}
	}

	void ButtonSoundController::PauseAllNegativeVoices()
	{
		for (auto& voice : buttonVoicePool)
		{
			if (voice.GetIsPlaying() && voice.GetPosition() < TimeSpan::Zero())
			{
				voice.SetIsPlaying(false);
				voice.SetPosition(voice.GetDuration());
			}
		}

		lastButtonSoundTime = buttonSoundTime = timeSinceLastButtonSound = {};
	}

	void ButtonSoundController::PlayButtonSoundType(ButtonSoundType type, TimeSpan startTime, std::optional<TimeSpan> externalClock)
	{
		lastButtonSoundTime = buttonSoundTime;
		buttonSoundTime = externalClock.value_or(TimeSpan::GetTimeNow());
		timeSinceLastButtonSound = (buttonSoundTime - lastButtonSoundTime);

		auto& voice = buttonVoicePool[buttonPoolRingIndex];
		buttonPoolRingIndex = IncrementRingIndex<ButtonVoicePoolSize>(buttonPoolRingIndex);

		PlayButtonSoundTypeUsingVoice(voice, type, startTime);
	}

	void ButtonSoundController::PlayButtonSoundTypeUsingVoice(Audio::Voice voice, ButtonSoundType type, TimeSpan startTime)
	{
		auto& source = loadedSources[static_cast<u8>(type)][0];

		Audio::AudioEngine::GetInstance().EnsureStreamRunning();
		voice.SetSource(source.Get());
		voice.SetVolume(masterSoundVolume * source.Volume * GetLastButtonSoundTimeVolumeFactor());
		voice.SetPosition(startTime);
		voice.SetIsPlaying(true);
	}

	void ButtonSoundController::PlayChainSoundType(ButtonSoundType type, ChainSoundSlot slot, TimeSpan startTime, std::optional<TimeSpan> externalClock)
	{
		const auto slotIndex = static_cast<u8>(slot);

		auto& source = loadedSources[static_cast<u8>(type)][0];
		Audio::AudioEngine::GetInstance().EnsureStreamRunning();

		if (type == ButtonSoundType::ChainSlideFirst)
		{
			auto& voice = chainStartVoicePools[slotIndex][chainStartPoolRingIndices[slotIndex]];
			chainStartPoolRingIndices[slotIndex] = IncrementRingIndex<PerSlotChainVoicePoolSize>(chainStartPoolRingIndices[slotIndex]);

			voice.SetSource(source.Get());
			voice.SetVolume(masterSoundVolume * source.Volume);
			voice.SetPosition(startTime);
			voice.SetIsPlaying(true);
			voice.ResetVolumeMap();
		}
		else
		{
			auto& voice = chainEndVoicePools[slotIndex][chainEndPoolRingIndices[slotIndex]];
			chainEndPoolRingIndices[slotIndex] = IncrementRingIndex<PerSlotChainVoicePoolSize>(chainEndPoolRingIndices[slotIndex]);

			voice.SetSource(source.Get());
			voice.SetVolume(masterSoundVolume * source.Volume);
			voice.SetPosition(startTime);
			voice.SetIsPlaying(true);
		}
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

	ButtonSoundController::AsyncSoundSource::AsyncSoundSource(std::string_view filePath, f32 volume) :
		Handle(Audio::SourceHandle::Invalid), FutureHandle(Audio::AudioEngine::GetInstance().LoadAudioSourceAsync(filePath)), Volume(volume)
	{
	}

	Audio::SourceHandle ButtonSoundController::AsyncSoundSource::Get()
	{
		if (FutureHandle.valid())
			Handle = FutureHandle.get();

		return Handle;
	}
}
