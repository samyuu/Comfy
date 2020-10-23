#pragma once
#include "Types.h"
#include "ChannelMixer.h"
#include "Audio/SampleProvider/ISampleProvider.h"
#include "Time/TimeSpan.h"
#include <functional>
#include <future>

namespace Comfy::Audio
{
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
	public:
		Voice() : Handle(VoiceHandle::Invalid) {}
		Voice(VoiceHandle handle) : Handle(handle) {}

		VoiceHandle Handle;

		inline operator VoiceHandle() const { return Handle; }

	public:
		bool IsValid() const;

		f32 GetVolume() const;
		void SetVolume(f32 value);

		TimeSpan GetPosition() const;
		void SetPosition(TimeSpan value);

		SourceHandle GetSource() const;
		void SetSource(SourceHandle value);

		TimeSpan GetDuration() const;

		bool GetIsPlaying() const;
		void SetIsPlaying(bool value);

		bool GetIsLooping() const;
		void SetIsLooping(bool value);

		bool GetPlayPastEnd() const;
		void SetPlayPastEnd(bool value);

		bool GetRemoveOnEnd() const;
		void SetRemoveOnEnd(bool value);
		
		bool GetPauseOnEnd() const;
		void SetPauseOnEnd(bool value);

		std::string_view GetName() const;

		void ResetVolumeMap();
		void SetVolumeMap(TimeSpan startTime, TimeSpan endTime, f32 startVolume, f32 endVolume);

	private:
		bool GetInternalFlag(u16 flag) const;
		void SetInternalFlag(u16 flag, bool value);
	};

	struct CallbackReceiver : NonCopyable
	{
		CallbackReceiver(std::function<void(void)> callback);
		~CallbackReceiver();

		std::function<void(void)> OnAudioCallback;
	};

	// TODO:
	struct DeviceInfo
	{
	};

	enum class AudioBackend : u32
	{
		Invalid,
		WASAPIShared,
		WASAPIExclusive,
		Count,
		Default = WASAPIExclusive, // WASAPIShared,
	};

	class AudioEngine : NonCopyable
	{
		friend Voice;
		friend CallbackReceiver;

	public:
		static constexpr f32 MinVolume = 0.0f, MaxVolume = 1.0f;
		static constexpr size_t MaxSimultaneousVoices = 88;

		static constexpr u32 OutputChannelCount = 2;
		static constexpr u32 OutputSampleRate = 44100;

		static constexpr u32 DefaultBufferFrameCount = 64;
		static constexpr u32 MinBufferSampleCount = 8;
		static constexpr u32 MinBufferFrameCount = (MinBufferSampleCount / OutputChannelCount);
		static constexpr u32 MaxBufferSampleCount = 2048;
		static constexpr u32 MaxBufferFrameCount = (MaxBufferSampleCount / OutputChannelCount);

		static constexpr size_t CallbackDurationRingBufferSize = 64;
		static constexpr size_t LastPlayedSamplesRingBufferFrameCount = MaxBufferSampleCount;

	public:
		AudioEngine();
		~AudioEngine();

	public:
		static void CreateInstance();
		static void DeleteInstance();

		static bool InstanceValid();
		static AudioEngine& GetInstance();

	public:
		void OpenStartStream();
		void StopCloseStream();

		// NOTE: Little helper to easily delay opening and starting of the stream until it's necessary
		void EnsureStreamRunning();

	public:
		COMFY_NODISCARD std::future<SourceHandle> LoadAudioSourceAsync(std::string_view filePath);
		COMFY_NODISCARD SourceHandle LoadAudioSource(std::string_view filePath);
		COMFY_NODISCARD SourceHandle LoadAudioSource(std::shared_ptr<ISampleProvider> sampleProvider);
		COMFY_NODISCARD SourceHandle LoadAudioSourceFromWAV(const void* fileContent, size_t fileSize);
		void UnloadSource(SourceHandle source);

		// NOTE: Add a voice and keep a handle to it
		COMFY_NODISCARD VoiceHandle AddVoice(SourceHandle source, std::string_view name, bool playing, f32 volume = MaxVolume, bool playPastEnd = false);
		void RemoveVoice(VoiceHandle voice);

		// NOTE: Add a voice, play it once then discard
		void PlaySound(SourceHandle source, std::string_view name, f32 volume = MaxVolume);

		COMFY_NODISCARD std::shared_ptr<ISampleProvider> GetSharedSource(SourceHandle handle);

	public:
		AudioBackend GetAudioBackend() const;
		void SetAudioBackend(AudioBackend value);

		bool GetIsStreamOpenRunning() const;
		bool GetAllVoicesAreIdle() const;

		f32 GetMasterVolume() const;
		void SetMasterVolume(f32 value);

		u32 GetChannelCount() const;
		u32 GetSampleRate() const;

		u32 GetBufferFrameSize() const;
		void SetBufferFrameSize(u32 bufferFrameSize);

		TimeSpan GetCallbackFrequency() const;

		ChannelMixer& GetChannelMixer();

	public:
		// size_t GetDeviceCount() const;
		// DeviceInfo GetDeviceInfo(u32 device);

		void DebugShowControlPanel() const;

		// NOTE: Has to be large enough to store at least Engine::MaxSimultaneousVoices
		void DebugGetAllVoices(Voice* outputVoices, size_t* outputVoiceCount);

		std::array<TimeSpan, CallbackDurationRingBufferSize> DebugGetCallbackDurations();
		std::array<std::array<i16, LastPlayedSamplesRingBufferFrameCount>, OutputChannelCount> DebugGetLastPlayedSamples();

		bool DebugGetEnableOutputCapture() const;
		void DebugSetEnableOutputCapture(bool value);
		void DebugFlushCaptureToWaveFile(std::string_view filePath);

	private:
		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}
