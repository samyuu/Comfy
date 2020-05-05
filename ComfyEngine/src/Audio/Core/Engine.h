#pragma once
#include "Types.h"
#include "ICallbackReceiver.h"
#include "Detail/ChannelMixer.h"
#include "Audio/SampleProvider/ISampleProvider.h"
#include "Time/TimeSpan.h"

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

		std::string_view GetName() const;
	};

	// TODO:
	struct DeviceInfo
	{
	};

	class Engine : NonCopyable
	{
		friend Voice;

	public:
		static constexpr f32 MinVolume = 0.0f, MaxVolume = 1.0f;
		static constexpr size_t MaxSimultaneousVoices = 64;

		static constexpr u32 OutputChannelCount = 2;
		static constexpr u32 OutputSampleRate = 44100;

		static constexpr u32 DefaultSampleBufferSize = 64;
		static constexpr u32 MaxSampleBufferSize = 0x2000;

		static constexpr size_t CallbackDurationRingBufferSize = 64;

		enum class AudioAPI : u32
		{
			Invalid,
			ASIO,
			WASAPI,
			Default = WASAPI,
		};

	public:
		Engine();
		~Engine();

	public:
		static void CreateInstance();
		static void DeleteInstance();

		static bool InstanceValid();
		static Engine& GetInstance();

	public:
		void OpenStream();
		void CloseStream();

		void StartStream();
		void StopStream();

		// NOTE: Little helper to easily delay opening and starting of the stream until it's necessary
		void EnsureStreamRunning();

	public:
		COMFY_NODISCARD SourceHandle LoadAudioSource(std::string_view filePath);
		COMFY_NODISCARD SourceHandle LoadAudioSource(std::unique_ptr<ISampleProvider> sampleProvider);
		void UnloadSource(SourceHandle source);

		// NOTE: Add a voice and keep a handle to it
		COMFY_NODISCARD VoiceHandle AddVoice(SourceHandle source, std::string_view name, bool playing, f32 volume = MaxVolume, bool playPastEnd = false);
		void RemoveVoice(VoiceHandle voice);

		// NOTE: Add a voice, play it once then discard
		void PlaySound(SourceHandle source, std::string_view name, f32 volume = MaxVolume);

		// NOTE: Underlying source pointer may be deleted at any time using UnloadSource or be invalided using LoadAudioSource
		ISampleProvider* GetRawSource(SourceHandle handle);

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
		TimeSpan GetCallbackFrequency() const;

		ChannelMixer& GetChannelMixer();

	public:
		// size_t GetDeviceCount() const;
		// DeviceInfo GetDeviceInfo(u32 device);

		void DebugShowControlPanel() const;

		// NOTE: Has to be large enough to store Engine::MaxSimultaneousVoices
		void DebugGetAllVoices(Voice* outputVoices, size_t* outputVoiceCount);

		// NOTE: Has to be large enough to store Engine::CallbackDurationRingBufferSize
		void DebugGetCallbackDurations(TimeSpan* outputDurations);

	private:
		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}
