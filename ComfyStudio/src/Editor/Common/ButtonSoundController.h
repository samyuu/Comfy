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
		static constexpr std::string_view buttonSoundPath = "dev_rom/sound/button/01_button1.wav";

		TimeSpan buttonSoundTime, lastButtonSoundTime, timeSinceLastButtonSound;
		float buttonSoundVolume = 0.75f;

		int buttonSoundIndex = -1;
		std::vector<Audio::SourceHandle> buttonSoundSources;
		std::array<Audio::Voice, 32> buttonSoundVoicePool;

		void PlayButtonSound(Audio::Voice voice, TimeSpan startTime);
		f32 GetVolumeFactor() const;
	};
}
