#pragma once
#include "Time/TimeSpan.h"
#include "Audio/Audio.h"
#include <optional>

namespace Comfy::Studio::Editor
{
	class ButtonSoundController
	{
	public:
		ButtonSoundController();
		~ButtonSoundController();

	public:
		void PlayButtonSound(TimeSpan startTime = TimeSpan::Zero(), std::optional<TimeSpan> externalClock = {});
		void PauseAllNegativeVoices();

		Audio::SourceHandle GetButtonSoundSource(int index);

	private:
		Audio::Voice* FindEmptyOrLongestRunningVoice();

		void PlayButtonSoundUsingVoice(Audio::Voice voice, TimeSpan startTime);
		f32 GetLastButtonSoundTimeVolumeFactor() const;

	private:
		static constexpr std::string_view buttonSoundPath = "dev_rom/sound/button/01_button1.wav";

		struct AsyncSoundSource
		{
			Audio::SourceHandle Handle;
			std::future<Audio::SourceHandle> FutureHandle;

			Audio::SourceHandle Get();
		};

		std::vector<AsyncSoundSource> loadedSoundSources;

		TimeSpan buttonSoundTime = {}, lastButtonSoundTime = {}, timeSinceLastButtonSound = {};
		float buttonSoundVolume = 0.75f;

		int buttonSoundIndex = -1;
		std::array<Audio::Voice, 32> buttonSoundVoicePool;
	};
}
