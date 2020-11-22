#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Time/TimeSpan.h"
#include "Audio/Audio.h"
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
		static constexpr size_t PerSlotChainVoicePoolSize = 12;

	public:
		ButtonSoundController();
		~ButtonSoundController();

	public:
		void PlayButtonSound(TimeSpan startTime = TimeSpan::Zero(), std::optional<TimeSpan> externalClock = {});
		void PlaySlideSound(TimeSpan startTime = TimeSpan::Zero(), std::optional<TimeSpan> externalClock = {});

		void PlayChainSoundStart(ChainSoundSlot slot, TimeSpan startTime = TimeSpan::Zero(), std::optional<TimeSpan> externalClock = {});
		void PlayChainSoundSuccess(ChainSoundSlot slot, TimeSpan startTime = TimeSpan::Zero(), std::optional<TimeSpan> externalClock = {});
		void PlayChainSoundFailure(ChainSoundSlot slot, TimeSpan startTime = TimeSpan::Zero(), std::optional<TimeSpan> externalClock = {});
		void FadeOutLastChainSound(ChainSoundSlot slot);

		void PauseAllChainSounds();
		void PauseAllNegativeVoices();

		f32 GetVolume() const;
		void SetVolume(f32 value);

	private:
		void InitializeVoicePools();
		void UnloadVoicePools();
		void InitializeSoundSources();

		void PlayButtonSoundType(ButtonSoundType type, ChainSoundSlot slot, TimeSpan startTime, std::optional<TimeSpan> externalClock);

	private:
		f32 masterSoundVolume = 1.0f;

		struct AsyncSoundSource
		{
			Audio::SourceHandle Handle;
			std::future<Audio::SourceHandle> FutureHandle;
			f32 Volume;

			AsyncSoundSource(std::string_view filePath, f32 volume = 1.0f);
			Audio::SourceHandle Get();
		};
		std::array<std::vector<AsyncSoundSource>, EnumCount<ButtonSoundType>()> loadedSources;

		struct SoundTypeTimeData
		{
			TimeSpan This, Last, SinceLast;

			void Update(std::optional<TimeSpan> externalClock);
			f32 GetVolumeFactor() const;
		};
		std::array<SoundTypeTimeData, EnumCount<ButtonSoundType>()> soundTimings = {};

		const TimeSpan chainFadeOutDuration = TimeSpan::FromMilliseconds(200.0);

		size_t buttonPoolRingIndex = 0;
		std::array<size_t, EnumCount<ChainSoundSlot>()> chainStartPoolRingIndices = {};
		std::array<size_t, EnumCount<ChainSoundSlot>()> chainEndPoolRingIndices = {};

		std::array<Audio::Voice, ButtonVoicePoolSize> buttonVoicePool;
		std::array<std::array<Audio::Voice, PerSlotChainVoicePoolSize>, EnumCount<ChainSoundSlot>()> chainStartVoicePools, chainEndVoicePools;
		std::array<Audio::Voice, EnumCount<ChainSoundSlot>()> perSlotChainSubVoices;
	};
}
