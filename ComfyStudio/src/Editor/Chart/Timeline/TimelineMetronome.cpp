#include "TimelineMetronome.h"
#include "System/ComfyData.h"

namespace Comfy::Studio::Editor
{
	namespace
	{
		Audio::SourceHandle LoadWAVFromComfyData(std::string_view filePath)
		{
			const auto* file = System::Data.FindFile(filePath);
			if (file == nullptr)
				return Audio::SourceHandle::Invalid;

			auto fileContent = std::make_unique<u8[]>(file->Size);
			if (!System::Data.ReadFileIntoBuffer(file, fileContent.get()))
				return Audio::SourceHandle::Invalid;

			return Audio::AudioEngine::GetInstance().LoadAudioSourceFromWAV(fileContent.get(), file->Size);
		}
	}

	TimelineMetronome::TimelineMetronome()
	{
		loadFuture = std::async(std::launch::async, [this]
		{
			auto& audioEngine = Audio::AudioEngine::GetInstance();

			char buffer[64];
			for (size_t i = 0; i < voicePool.size(); i++)
			{
				voicePool[i] = audioEngine.AddVoice(Audio::SourceHandle::Invalid, std::string_view(buffer, sprintf_s(buffer, "TimelineMetronome::VoicePool[%zu]", i)), false, volume);
				voicePool[i].SetPauseOnEnd(true);
			}

			beatSource = LoadWAVFromComfyData("sound/metronome_beat.wav");
			barSource = LoadWAVFromComfyData("sound/metronome_bar.wav");
			sourcesLoaded = true;
		});
	}

	TimelineMetronome::~TimelineMetronome()
	{
		if (loadFuture.valid())
			loadFuture.get();

		auto& audioEngine = Audio::AudioEngine::GetInstance();
		for (auto& voice : voicePool)
			audioEngine.RemoveVoice(voice);

		audioEngine.UnloadSource(beatSource);
		audioEngine.UnloadSource(barSource);
	}

	void TimelineMetronome::UpdatePlaySounds(const Chart& chart, TimeSpan timeThisFrame, TimeSpan timeLastFrame, TimeSpan futureOffset)
	{
		if (timeLastFrame != lastProvidedFrameTime)
			lastPlayedBeatTime = TimeSpan::FromSeconds(std::numeric_limits<f64>::min());
		lastProvidedFrameTime = timeThisFrame;

		const auto cursorTick = chart.TimelineMap.GetTickAt(timeThisFrame);
		const auto cursorEndTick = cursorTick + TimelineTick::FromBars(1);

		chart.TempoMap.ForEachBeatBar([&](const TimelineTick beatTick, const size_t barIndex, const bool isBar)
		{
			if (beatTick >= cursorEndTick)
				return true;

			const auto beatTime = chart.TimelineMap.GetTimeAt(beatTick);
			const auto offsetBeatTime = (beatTime - futureOffset);

			if (offsetBeatTime >= timeLastFrame && offsetBeatTime <= timeThisFrame)
			{
				// NOTE: Otherwise identical beats occasionally get played twice, possibly due to rounding errors (?)
				if (lastPlayedBeatTime == beatTime)
					return true;

				const auto startTime = std::min((timeThisFrame - beatTime), TimeSpan::Zero());

				PlayTickSound(startTime, isBar);
				lastPlayedBeatTime = beatTime;
				return true;
			}

			return false;
		});
	}

	f32 TimelineMetronome::GetVolume() const
	{
		return volume;
	}

	void TimelineMetronome::SetVolume(f32 value)
	{
		for (auto& voice : voicePool)
			voice.SetVolume(value);

		volume = value;
	}

	void TimelineMetronome::PlayTickSound(TimeSpan startTime, bool onBar)
	{
		if (!sourcesLoaded)
			return;

		auto& voice = voicePool[voicePoolRingIndex];
		if (++voicePoolRingIndex >= voicePool.size())
			voicePoolRingIndex = 0;

		voice.SetSource(onBar ? barSource : beatSource);
		voice.SetPosition(startTime);
		voice.SetIsPlaying(true);
	}
}
