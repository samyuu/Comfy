#pragma once
#include "Types.h"
#include "Time/TimeSpan.h"
#include "Time/Stopwatch.h"

namespace Comfy::Studio::Editor
{
	class BPMTapCalculator
	{
	public:
		static constexpr auto DefaultAutoResetInterval = TimeSpan::FromSeconds(2.0);

	public:
		inline void Tap()
		{
			autoResetStopwatch.Restart();

			if (!tapStopwatch.IsRunning())
				tapStopwatch.Restart();

			bpmOnLastTap = CalculateBPMElapsedTaps(++tapCount, tapStopwatch.GetElapsed());
			bpmOnLastTapMin = (tapCount <= 2) ? bpmOnLastTap : std::min(bpmOnLastTap, bpmOnLastTapMin);
			bpmOnLastTapMax = (tapCount <= 2) ? bpmOnLastTap : std::max(bpmOnLastTap, bpmOnLastTapMax);
		}

		inline void Reset()
		{
			tapStopwatch.Stop();

			tapCount = 0;
			bpmOnLastTap = bpmOnLastTapMin = bpmOnLastTapMax = 0.0;
		}

		inline void Update()
		{
			if (tapStopwatch.IsRunning() && autoResetInterval > TimeSpan::Zero() && autoResetStopwatch.GetElapsed() >= autoResetInterval)
				Reset();
		}

		inline i32 GetTapCount() const { return tapCount; }

		inline f32 GetBPMOnLastTap() const { return bpmOnLastTap; }
		inline f32 GetBPMOnLastTapRound() const { return glm::round(GetBPMOnLastTap()); }

		inline f32 GetBPMOnLastTapMin() const { return bpmOnLastTapMin; }
		inline f32 GetBPMOnLastTapMinRound() const { return glm::round(GetBPMOnLastTapMin()); }

		inline f32 GetBPMOnLastTapMax() const { return bpmOnLastTapMax; }
		inline f32 GetBPMOnLastTapMaxRound() const { return glm::round(GetBPMOnLastTapMax()); }

		inline f32 GetRunningBPM() const { return CalculateBPMElapsedTaps(tapCount, tapStopwatch.GetElapsed()); }
		inline f32 GetRunningBPMRound() const { return glm::round(GetRunningBPM()); }

		inline TimeSpan GetTimeSinceLastTap() const { return autoResetStopwatch.GetElapsed(); }

		inline TimeSpan GetAutoResetInterval() const { return autoResetInterval; }
		inline void SetAutoResetInterval(TimeSpan value) { autoResetInterval = value; }

	private:
		static constexpr f32 CalculateBPMElapsedTaps(i32 tapCount, TimeSpan elapsed)
		{
			return (tapCount <= 1) ? 0.0f : static_cast<f32>(60.0 * (tapCount - 1) / elapsed.TotalSeconds());
		}

	private:
		i32 tapCount = 0;
		f32 bpmOnLastTap = 0.0f;
		f32 bpmOnLastTapMin = 0.0f, bpmOnLastTapMax = 0.0f;

		Stopwatch tapStopwatch = Stopwatch::StartNew();
		Stopwatch autoResetStopwatch = Stopwatch::StartNew();

		TimeSpan autoResetInterval = DefaultAutoResetInterval;
	};
}
