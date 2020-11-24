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

	ButtonSoundController::ButtonSoundController(SoundEffectManager& soundEffectManager) : soundEffectManager(soundEffectManager)
	{
		InitializeVoicePools();
	}

	ButtonSoundController::~ButtonSoundController()
	{
		UnloadVoicePools();
	}

	void ButtonSoundController::SetIDs(u32 buttonID, u32 slideID, u32 chainSlideID, u32 sliderTouchID)
	{
		buttonIDs.Button = buttonID;
		buttonIDs.Slide = slideID;
		buttonIDs.ChainSlide = chainSlideID;
		buttonIDs.SliderTouch = sliderTouchID;
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

		auto& subVoice = perSlotChainSubVoices[slotIndex];
		if (subVoice.GetPosition() < TimeSpan::Zero())
		{
			subVoice.SetIsPlaying(false);
		}
		else
		{
			subVoice.SetIsLooping(false);
			subVoice.SetPauseOnEnd(true);
		}
	}

	void ButtonSoundController::PauseAllChainSounds()
	{
		for (auto& voicePool : chainStartVoicePools)
		{
			for (auto& voice : voicePool)
				voice.SetIsPlaying(false);
		}

		for (auto& voice : perSlotChainSubVoices)
			voice.SetIsPlaying(false);
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

	f32 ButtonSoundController::GetMasterVolume() const
	{
		return masterVolume;
	}

	void ButtonSoundController::SetMasterVolume(f32 value)
	{
		masterVolume = value;
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

		for (size_t slotIndex = 0; slotIndex < EnumCount<ChainSoundSlot>(); slotIndex++)
		{
			auto& voice = perSlotChainSubVoices[slotIndex];
			const auto nameView = std::string_view(nameBuffer, sprintf_s(nameBuffer, "ButtonSoundController::ChainSubVoice[%c]", getSlotCharID(slotIndex)));
			voice = audioEngine.AddVoice(Audio::SourceHandle::Invalid, nameView, false);
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

		for (auto& voice : perSlotChainSubVoices)
			audioEngine.RemoveVoice(voice);
	}

	void ButtonSoundController::PlayButtonSoundType(ButtonSoundType type, ChainSoundSlot slot, TimeSpan startTime, std::optional<TimeSpan> externalClock)
	{
		const auto typeIndex = static_cast<u8>(type);
		const auto slotIndex = static_cast<u8>(slot);

		soundTimings[typeIndex].Update(externalClock);

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
		voice->SetSource(GetSource(type));
		voice->SetVolume(masterVolume * soundTimings[typeIndex].GetVolumeFactor());
		voice->SetPosition(startTime);
		voice->SetIsPlaying(true);

		if (type == ButtonSoundType::ChainSlideFirst)
		{
			// NOTE: Or maybe instead of a fixed loop these should be played every n-th chain fragment past the end of the first chain sound (?)
			auto& subVoice = perSlotChainSubVoices[slotIndex];
			subVoice.SetSource(GetSource(ButtonSoundType::ChainSlideSub));
			subVoice.SetVolume(masterVolume);
			subVoice.SetPosition(startTime - voice->GetDuration());
			subVoice.SetIsPlaying(true);
			subVoice.SetIsLooping(true);
			subVoice.SetPauseOnEnd(false);
		}
	}

	Audio::SourceHandle ButtonSoundController::GetSource(ButtonSoundType type, i32 sliderTouchIndex) const
	{
		if (!soundEffectManager.IsAsyncLoaded())
			return Audio::SourceHandle::Invalid;

		switch (type)
		{
		case ButtonSoundType::Button:
			return soundEffectManager.GetButtonSound(buttonIDs.Button);
		case ButtonSoundType::Slide:
			return soundEffectManager.GetSlideSound(buttonIDs.Slide);
		case ButtonSoundType::ChainSlideFirst:
			return soundEffectManager.GetChainSlideSound(buttonIDs.ChainSlide)[0];
		case ButtonSoundType::ChainSlideSub:
			return soundEffectManager.GetChainSlideSound(buttonIDs.ChainSlide)[1];
		case ButtonSoundType::ChainSlideSuccess:
			return soundEffectManager.GetChainSlideSound(buttonIDs.ChainSlide)[2];
		case ButtonSoundType::ChainSlideFailure:
			return soundEffectManager.GetChainSlideSound(buttonIDs.ChainSlide)[3];
		case ButtonSoundType::SlideTouch:
			return soundEffectManager.GetSliderTouchSound(buttonIDs.SliderTouch)[std::clamp(sliderTouchIndex, 0, 31)];
		}

		return Audio::SourceHandle::Invalid;
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
