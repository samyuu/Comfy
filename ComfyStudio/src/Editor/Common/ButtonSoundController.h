#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Time/TimeSpan.h"
#include "SoundEffectManager.h"
#include <optional>

namespace Comfy::Studio::Editor
{
	enum class ButtonSoundType : u8
	{
		Button,
		Slide,
		ChainSlideFirst,
		ChainSlideSub,
		ChainSlideSuccess,
		ChainSlideFailure,
		SlideTouch,
		Count,
	};

	enum class ChainSoundSlot : u8
	{
		Left,
		Right,
		Count,
	};

	class ButtonSoundController : NonCopyable
	{
	public:
		static constexpr size_t ButtonVoicePoolSize = 24;
		static constexpr size_t SliderTouchVoicePoolSize = 24;
		static constexpr size_t PerSlotChainVoicePoolSize = 12;

	public:
		ButtonSoundController(SoundEffectManager& soundEffectManager);
		~ButtonSoundController();

	public:
		void SetIDs(u32 buttonID, u32 slideID, u32 chainSlideID, u32 sliderTouchID);

		void PlayButtonSound(TimeSpan startTime = TimeSpan::Zero(), std::optional<TimeSpan> externalClock = {});
		void PlaySlideSound(TimeSpan startTime = TimeSpan::Zero(), std::optional<TimeSpan> externalClock = {});

		void PlayChainSoundStart(ChainSoundSlot slot, TimeSpan startTime = TimeSpan::Zero(), std::optional<TimeSpan> externalClock = {});
		void PlayChainSoundSuccess(ChainSoundSlot slot, TimeSpan startTime = TimeSpan::Zero(), std::optional<TimeSpan> externalClock = {});
		void PlayChainSoundFailure(ChainSoundSlot slot, TimeSpan startTime = TimeSpan::Zero(), std::optional<TimeSpan> externalClock = {});
		void FadeOutLastChainSound(ChainSoundSlot slot, TimeSpan startTime = TimeSpan::Zero());

		void PlaySliderTouch(i32 sliderTouchIndex, f32 baseVolume = 1.0f);

		void PauseAllChainSounds();
		void PauseAllNegativeVoices();

		f32 GetMasterVolume() const;
		void SetMasterVolume(f32 value);

	private:
		void InitializeVoicePools();
		void UnloadVoicePools();

		void PlayButtonSoundType(ButtonSoundType type, ChainSoundSlot slot, TimeSpan startTime, std::optional<TimeSpan> externalClock);

		Audio::SourceHandle GetSource(ButtonSoundType type, i32 sliderTouchIndex = 0) const;

	private:
		SoundEffectManager& soundEffectManager;

		f32 masterVolume = 1.0f;

		struct ButtonSoundIDs
		{
			u32 Button, Slide, ChainSlide, SliderTouch;
		} buttonIDs = {};

		struct SoundTypeTimeData
		{
			TimeSpan This, Last, SinceLast;

			void Update(std::optional<TimeSpan> externalClock);
			f32 GetVolumeFactor() const;
		};
		std::array<SoundTypeTimeData, EnumCount<ButtonSoundType>()> soundTimings = {};

		const TimeSpan chainFadeOutDuration = TimeSpan::FromMilliseconds(200.0);

		size_t buttonPoolRingIndex = 0, sliderTouchPoolRingIndex = 0;
		std::array<size_t, EnumCount<ChainSoundSlot>()> chainStartPoolRingIndices = {};
		std::array<size_t, EnumCount<ChainSoundSlot>()> chainEndPoolRingIndices = {};

		std::array<Audio::Voice, ButtonVoicePoolSize> buttonVoicePool;
		std::array<Audio::Voice, SliderTouchVoicePoolSize> sliderTouchVoicePool;
		std::array<std::array<Audio::Voice, PerSlotChainVoicePoolSize>, EnumCount<ChainSoundSlot>()> chainStartVoicePools, chainEndVoicePools;
		std::array<Audio::Voice, EnumCount<ChainSoundSlot>()> perSlotChainSubVoices;
	};
}
