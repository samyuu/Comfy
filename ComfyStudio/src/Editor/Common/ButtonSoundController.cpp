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
		InitializeVoicePools();
		InitializeSoundSources();
	}

	ButtonSoundController::~ButtonSoundController()
	{
		UnloadVoicePools();
	}

	void ButtonSoundController::PlayButtonSound(TimeSpan startTime, std::optional<TimeSpan> externalClock)
	{
		PlayButtonSoundType(ButtonSoundType::Button, {}, startTime, externalClock);
	}

	void ButtonSoundController::PlaySlideSound(TimeSpan startTime, std::optional<TimeSpan> externalClock)
	{
		PlayButtonSoundType(ButtonSoundType::Slide, {}, startTime, externalClock);
	}

	void ButtonSoundController::PlayChainSoundStart(ChainSoundSlot slot, TimeSpan startTime, std::optional<TimeSpan> externalClock)
	{
		PlayButtonSoundType(ButtonSoundType::ChainSlideFirst, slot, startTime, externalClock);
	}

	void ButtonSoundController::PlayChainSoundSuccess(ChainSoundSlot slot, TimeSpan startTime, std::optional<TimeSpan> externalClock)
	{
		PlayButtonSoundType(ButtonSoundType::ChainSlideSuccess, slot, startTime, externalClock);
	}

	void ButtonSoundController::PlayChainSoundFailure(ChainSoundSlot slot, TimeSpan startTime, std::optional<TimeSpan> externalClock)
	{
		PlayButtonSoundType(ButtonSoundType::ChainSlideFailure, slot, startTime, externalClock);
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

		soundTimings = {};
	}

	f32 ButtonSoundController::GetVolume() const
	{
		return masterSoundVolume;
	}

	void ButtonSoundController::SetVolume(f32 value)
	{
		masterSoundVolume = value;
	}

	void ButtonSoundController::InitializeVoicePools()
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
	}

	void ButtonSoundController::UnloadVoicePools()
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

	void ButtonSoundController::InitializeSoundSources()
	{
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

	void ButtonSoundController::PlayButtonSoundType(ButtonSoundType type, ChainSoundSlot slot, TimeSpan startTime, std::optional<TimeSpan> externalClock)
	{
		const auto typeIndex = static_cast<u8>(type);
		const auto slotIndex = static_cast<u8>(slot);

		soundTimings[typeIndex].Update(externalClock);
		auto& source = loadedSources[typeIndex][0];

		Audio::Voice* voice = nullptr;
		if (type == ButtonSoundType::Button || type == ButtonSoundType::Slide)
		{
			voice = &buttonVoicePool[buttonPoolRingIndex];
			buttonPoolRingIndex = IncrementRingIndex<ButtonVoicePoolSize>(buttonPoolRingIndex);
		}
		else if (type == ButtonSoundType::ChainSlideFirst)
		{
			voice = &chainStartVoicePools[slotIndex][chainStartPoolRingIndices[slotIndex]];
			chainStartPoolRingIndices[slotIndex] = IncrementRingIndex<PerSlotChainVoicePoolSize>(chainStartPoolRingIndices[slotIndex]);

			voice->ResetVolumeMap();
		}
		else if (type == ButtonSoundType::ChainSlideSuccess || type == ButtonSoundType::ChainSlideFailure)
		{
			voice = &chainEndVoicePools[slotIndex][chainEndPoolRingIndices[slotIndex]];
			chainEndPoolRingIndices[slotIndex] = IncrementRingIndex<PerSlotChainVoicePoolSize>(chainEndPoolRingIndices[slotIndex]);
		}
		else
		{
			assert(false);
			return;
		}

		Audio::AudioEngine::GetInstance().EnsureStreamRunning();
		voice->SetSource(source.Get());
		voice->SetVolume(masterSoundVolume * source.Volume * soundTimings[typeIndex].GetVolumeFactor());
		voice->SetPosition(startTime);
		voice->SetIsPlaying(true);
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

	void ButtonSoundController::SoundTypeTimeData::Update(std::optional<TimeSpan> externalClock)
	{
		Last = This;
		This = externalClock.value_or(TimeSpan::GetTimeNow());
		SinceLast = (This - Last);
	}

	f32 ButtonSoundController::SoundTypeTimeData::GetVolumeFactor() const
	{
		constexpr auto thresholdMS = 35.0;
		const auto timeSinceMS = SinceLast.TotalMilliseconds();

		if (timeSinceMS >= thresholdMS)
			return 1.0f;

		// NOTE: All of this is far from perfect
		const auto delta = static_cast<f32>(timeSinceMS / thresholdMS);
		const auto factor = std::clamp((delta * delta), 0.0f, 1.0f);

		return factor;
	}
}
