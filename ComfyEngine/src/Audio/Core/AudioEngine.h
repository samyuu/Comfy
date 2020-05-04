#pragma once
#include "Types.h"
#include "ICallbackReceiver.h"
#include "Detail/ChannelMixer.h"
#include "Detail/SampleMixer.h"
#include "Audio/SampleProvider/ISampleProvider.h"
#include "Audio/SampleProvider/MemorySampleProvider.h"
#include "Time/TimeSpan.h"
#include <algorithm>
#include <mutex>

namespace Comfy::Audio
{
#if 0
	inline void TestUsage()
	{
		auto& audioEngine = AudioEngine::GetInstance();
		SourceHandle source = audioEngine.LoadAudioSource("Test.wav");
		// audioEngine.UnloadSource(source);
		audioEngine.AddVoice(source, "test", true, 1.0f, false);
	}
#endif

	// NOTE: Sample: Raw PCM
	// NOTE: Frame: Pair of samples for each channel
	// NOTE: Source: ISampleProvider
	// NOTE: Voice: Instance of a source

	// NOTE: Opaque types for referncing data stored in the AudioEngine, internally interpreted as in index
	using HandleBaseType = u16;
	enum class VoiceHandle : HandleBaseType { Invalid = 0xFFFF };
	enum class SourceHandle : HandleBaseType { Invalid = 0xFFFF };

	// NOTE: Lightweight and safe wrapper around a VoiceHandle providing a convenient OOP interface
	struct Voice
	{
		Voice() : Handle(VoiceHandle::Invalid) {}
		Voice(VoiceHandle handle) : Handle(handle) {}

		VoiceHandle Handle;

	public:
		f32 GetVoiceVolume(VoiceHandle handle);
		void SetVoiceVolume(VoiceHandle handle, f32 value);

		TimeSpan GetVoicePosition(VoiceHandle handle);
		void SetVoicePosition(VoiceHandle handle, TimeSpan value);

		SourceHandle GetSource(VoiceHandle handle);
		void SetSource(VoiceHandle handle, SourceHandle value);

		TimeSpan GetVoiceDuration(VoiceHandle handle);

		bool GetVoiceIsPlaying(VoiceHandle handle);
		void SetVoiceIsPlaying(VoiceHandle handle, bool value);

		bool GetVoiceIsLooping(VoiceHandle handle);
		void SetVoiceIsLooping(VoiceHandle handle, bool value);

		bool GetVoicePlayPastEnd(VoiceHandle handle);
		void SetVoicePlayPastEnd(VoiceHandle handle, bool value);

		bool GetVoiceRemoveOnEnd(VoiceHandle handle);
		void SetVoiceRemoveOnEnd(VoiceHandle handle, bool value);

		std::string_view GetVoiceName(VoiceHandle voice);
	};

	class AudioEngine : NonCopyable
	{
		friend Voice;

	public:
		static constexpr f32 MinVolume = 0.0f, MaxVolume = 1.0f;
		static constexpr size_t MaxSimultaneousVoices = 64;

		static constexpr u32 OutputChannelCount = 2;
		static constexpr u32 OutputSampleRate = 44100;

		static constexpr u32 DefaultSampleBufferSize = 64;
		static constexpr u32 MaxSampleBufferSize = 0x2000;

		enum class AudioAPI : u32
		{
			Invalid,
			ASIO,
			WASAPI,
			Default = WASAPI,
		};

	public:
		AudioEngine();
		~AudioEngine();

	public:
		static void CreateInstance();
		static void DeleteInstance();
		static AudioEngine& GetInstance();

	public:
		void OpenStream();
		void CloseStream();

		void StartStream();
		void StopStream();

	public:
		COMFY_NODISCARD SourceHandle LoadAudioSource(std::string_view filePath);
		COMFY_NODISCARD SourceHandle LoadAudioSource(std::unique_ptr<ISampleProvider> sampleProvider);
		void UnloadSource(SourceHandle source);

		VoiceHandle AddVoice(SourceHandle source, std::string_view name, bool playing, f32 volume = MaxVolume, bool playPastEnd = false);
		void RemoveVoice(VoiceHandle voice);

	public:
		void RegisterCallbackReceiver(ICallbackReceiver* callbackReceiver);
		void UnregisterCallbackReceiver(ICallbackReceiver* callbackReceiver);

	public:
		AudioAPI GetAudioAPI() const;
		void SetAudioAPI(AudioAPI value);

		bool GetIsStreamOpen() const;
		bool GetIsStreamRunning() const;

		f32 GetMasterVolume() const;
		void SetMasterVolume(f32 value);

		u32 GetChannelCount() const;
		u32 GetSampleRate() const;

		u32 GetBufferFrameSize() const;
		void SetBufferFrameSize(u32 bufferFrameSize);

		TimeSpan GetStreamTime() const;
		void SetStreamTime(TimeSpan value);

		bool GetIsExclusiveMode() const;
		TimeSpan GetCallbackLatency() const;

		ChannelMixer& GetChannelMixer();

	public:
		// size_t GetDeviceCount() const;
		// RtAudio::DeviceInfo GetDeviceInfo(u32 device);

		void DebugShowControlPanel() const;

		/*
		// NOTE: Only use for displaying debug info such as in the AudioTestWindow
		template <typename Func>
		void DebugIterateAudioInstances(Func func)
		{
			const auto lock = std::scoped_lock(GetAudioInstancesMutex());
			for (const auto& instance : GetAudioInstances())
				func(instance);
		}
		*/

	private:
		struct Impl;
		std::unique_ptr<Impl> impl;

	private:
		// std::mutex& GetAudioInstancesMutex();
		// std::vector<std::shared_ptr<AudioInstance>>& GetAudioInstances();
	};
}
