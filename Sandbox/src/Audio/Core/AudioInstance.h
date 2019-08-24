#pragma once
#include "Audio/SampleProvider/ISampleProvider.h"
#include "AudioEngine.h"
#include "Core/TimeSpan.h"

namespace Audio
{
	enum class AudioFinishedAction
	{
		None,
		Remove,
		Count,
	};

	class AudioInstance
	{
	public:
		friend class AudioEngine;

		// Constructors
		// ------------
		AudioInstance(RefPtr<ISampleProvider> sampleProvider, bool playing, const char* name = nullptr);
		AudioInstance(RefPtr<ISampleProvider> sampleProvider, bool playing, AudioFinishedAction finishedAction, float volume = AudioEngine::MaxVolume, const char* name = nullptr);

		AudioInstance(const AudioInstance& other) = delete;

		// Destructors
		// -----------
		~AudioInstance();

		// Sample Provider
		const RefPtr<ISampleProvider>& GetSampleProvider() const;
		void SetSampleProvider(const RefPtr<ISampleProvider>& value);
		bool HasSampleProvider();

		// Position
		TimeSpan GetPosition() const;
		void SetPosition(TimeSpan value);
		void Restart();

		// Duration
		TimeSpan GetDuration() const;

		// Name
		const char* GetName() const;
		void SetName(const char* value);

		// Volume
		float GetVolume() const;
		void SetVolume(float value);

		// IsPlaying
		bool GetIsPlaying() const;
		void SetIsPlaying(bool value);

		// IsLooping
		bool GetIsLooping() const;
		void SetIsLooping(bool value);

		// PlayPastEnd
		bool GetPlayPastEnd() const;
		void SetPlayPastEnd(bool value);

		// AppendRemove
		bool GetAppendRemove() const;
		void SetAppendRemove(bool value);

		// HasBeenRemoved
		bool GetHasBeenRemoved() const;
		bool GetHasReachedEnd() const;

		// OnFinishedAction
		AudioFinishedAction GetOnFinishedAction() const;
		void SetOnFinishedAction(AudioFinishedAction value);

		// FramePosition
		int64_t GetFramePosition() const;
		void SetFramePosition(int64_t value);

		int64_t GetFrameCount() const;
		uint32_t GetSampleRate() const;
		uint32_t GetChannelCount() const;

	public:
		AudioInstance& operator= (AudioInstance& other) = delete;

	private:
		// Members Variables
		// -----------------
		const char* name = "No-Name";

		float volume = AudioEngine::MaxVolume;

		bool isPlaying = false;
		bool isLooping = false;
		bool playPastEnd = false;
		bool hasBeenRemoved = false;
		bool appendRemove = false;

		int64_t framePosition = 0;
		AudioFinishedAction onFinishedAction = AudioFinishedAction::None;
		RefPtr<ISampleProvider> sampleProvider = nullptr;
		// -----------------

	protected:
		// Used by AudioEngine
		// -------------------
		inline void SetHasBeenRemoved(bool value) { hasBeenRemoved = value; };
		inline void IncrementFramePosition(int64_t value) { SetFramePosition(GetFramePosition() + value); };

	public:
		// Conversion Helper Methods
		// -------------------------
		TimeSpan FramesToTimeSpan(double frames) const;
		int64_t TimeSpanToFrames(TimeSpan time) const;

	protected:
		// Conversion Helper Functions
		// ---------------------------
		static TimeSpan FramesToTimeSpan(double frames, double sampleRate);
		static int64_t TimeSpanToFrames(TimeSpan time, double sampleRate);
	};
}