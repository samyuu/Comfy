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

	class ButtonSoundController
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

	private:
		void PlayButtonSoundType(ButtonSoundType type, TimeSpan startTime, std::optional<TimeSpan> externalClock);
		void PlayButtonSoundTypeUsingVoice(Audio::Voice voice, ButtonSoundType type, TimeSpan startTime);

		void PlayChainSoundType(ButtonSoundType type, ChainSoundSlot slot, TimeSpan startTime, std::optional<TimeSpan> externalClock);

		f32 GetLastButtonSoundTimeVolumeFactor(ButtonSoundType type) const;

	private:
		struct AsyncSoundSource
		{
			Audio::SourceHandle Handle;
			std::future<Audio::SourceHandle> FutureHandle;
			f32 Volume;

			AsyncSoundSource(std::string_view filePath, f32 volume = 1.0f);
			Audio::SourceHandle Get();
		};

		std::array<std::vector<AsyncSoundSource>, EnumCount<ButtonSoundType>()> loadedSources;

		float masterSoundVolume = 1.0f;

		struct SoundTypeTimeData { TimeSpan This, Last, SinceLast; };
		SoundTypeTimeData buttonSoundTime = {}, slideSoundTime = {};

		TimeSpan chainFadeOutDuration = TimeSpan::FromMilliseconds(200.0);

		size_t buttonPoolRingIndex = 0;
		std::array<size_t, EnumCount<ChainSoundSlot>()> chainStartPoolRingIndices = {};
		std::array<size_t, EnumCount<ChainSoundSlot>()> chainEndPoolRingIndices = {};

		std::array<Audio::Voice, ButtonVoicePoolSize> buttonVoicePool;
		std::array<std::array<Audio::Voice, PerSlotChainVoicePoolSize>, EnumCount<ChainSoundSlot>()> chainStartVoicePools, chainEndVoicePools;
	};
}
