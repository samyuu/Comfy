#pragma once
#include "Types.h"
#include "Editor/Chart/Chart.h"
#include "Audio/Audio.h"
#include "Time/TimeSpan.h"

namespace Comfy::Studio::Editor
{
	class TimelineMetronome : NonCopyable
	{
	public:
		TimelineMetronome();
		~TimelineMetronome();

	public:
		void UpdatePlaySounds(const Chart& chart, TimeSpan timeThisFrame, TimeSpan timeLastFrame, TimeSpan futureOffset);
		void PauseAllNegativeVoices();

		f32 GetVolume() const;
		void SetVolume(f32 value);

		void PlayTickSound(TimeSpan startTime, bool onBar);

	private:
		f32 volume = 1.0f;

		TimeSpan lastPlayedBeatTime = TimeSpan::FromSeconds(std::numeric_limits<f64>::min());
		TimeSpan lastProvidedFrameTime = TimeSpan::FromSeconds(std::numeric_limits<f64>::min());

		std::future<void> loadFuture;
		std::atomic<bool> sourcesLoaded;

		// NOTE: Should hold up even at the highest supported BPM value
		size_t voicePoolRingIndex = 0;
		std::array<Audio::Voice, 4> voicePool = {};

		Audio::SourceHandle beatSource, barSource;
	};
}
