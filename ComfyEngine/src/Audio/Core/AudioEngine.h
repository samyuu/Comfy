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

	constexpr f32 ConvertSampleI16ToF32(i16 v) { return static_cast<f32>(v) / static_cast<f32>(std::numeric_limits<i16>::max()); }
	constexpr i16 ConvertSampleF32ToI16(f32 v) { return static_cast<i16>(v * static_cast<f32>(std::numeric_limits<i16>::max())); }

	inline f32 ClampLinearVolume(f32 linear) { return glm::clamp(linear, 0.0f, 1.0f); }
	inline f32 DecibelToLinearVolume(f32 decibel) { return ::powf(10.0f, 0.05f * decibel); }
	inline f32 LinearVolumeToDecibel(f32 linear) { return 20.0f * ::log10f(linear); }
	inline f32 LinearVolumeToSquare(f32 linear) { return ::sqrtf(linear); }
	inline f32 SquareToLinearVolume(f32 power) { return (power * power); }

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

		f32 GetPlaybackSpeed() const;
		void SetPlaybackSpeed(f32 value);

		TimeSpan GetPosition() const;
		TimeSpan GetPositionSmooth() const;
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
		WASAPIShared,
		WASAPIExclusive,
		Count,
		Default = WASAPIExclusive, // WASAPIShared,
	};

	constexpr std::array<const char*, EnumCount<AudioBackend>()> AudioBackendNames =
	{
		"WASAPI (Shared)",
		"WASAPI (Exclusive)",
	};

	class AudioEngine : NonCopyable
	{
		friend Voice;
		friend CallbackReceiver;

	public:
		static constexpr f32 MinVolume = 0.0f, MaxVolume = 1.0f;
		static constexpr size_t MaxSimultaneousVoices = 128;

		static constexpr u32 OutputChannelCount = 2;
		static constexpr u32 OutputSampleRate = 44100;

		static constexpr u32 DefaultBufferFrameCount = 64;
		static constexpr u32 MinBufferFrameCount = 8;
		static constexpr u32 MaxBufferFrameCount = OutputSampleRate;

		static constexpr size_t CallbackDurationRingBufferSize = 64;
		static constexpr size_t LastPlayedSamplesRingBufferFrameCount = MaxBufferFrameCount;

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
		COMFY_NODISCARD std::future<SourceHandle> LoadSourceAsync(std::string_view filePath);
		COMFY_NODISCARD SourceHandle LoadSource(std::string_view filePath);
		COMFY_NODISCARD SourceHandle LoadSource(std::string_view fileName, const void* fileContent, size_t fileSize);
		COMFY_NODISCARD SourceHandle RegisterSource(std::shared_ptr<ISampleProvider> sampleProvider, std::string_view name);
		void UnloadSource(SourceHandle source);

		// NOTE: Add a voice and keep a handle to it
		COMFY_NODISCARD VoiceHandle AddVoice(SourceHandle source, std::string_view name, bool playing, f32 volume = MaxVolume, bool playPastEnd = false);
		void RemoveVoice(VoiceHandle voice);

		// NOTE: Add a voice, play it once then discard
		void PlayOneShotSound(SourceHandle source, std::string_view name, f32 volume = MaxVolume);

		COMFY_NODISCARD std::shared_ptr<ISampleProvider> GetSharedSource(SourceHandle source);

		f32 GetSourceBaseVolume(SourceHandle source);
		void SetSourceBaseVolume(SourceHandle source, f32 value);

		void GetSourceName(SourceHandle source, std::string& outName);
		void SetSourceName(SourceHandle source, std::string_view newName);

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

		i64 DebugGetTotalRenderedFrames() const;
		TimeSpan DebugGetTotalRenderTime() const;

		void DebugShowControlPanel() const;

		// NOTE: Has to be large enough to store at least Engine::MaxSimultaneousVoices
		void DebugGetAllVoices(Voice* outputVoices, size_t* outputVoiceCount);

		size_t DebugGetMaxSourceCount();

		std::array<TimeSpan, CallbackDurationRingBufferSize> DebugGetCallbackDurations();
		std::array<std::array<i16, LastPlayedSamplesRingBufferFrameCount>, OutputChannelCount> DebugGetLastPlayedSamples();

		bool DebugGetEnableOutputCapture() const;
		void DebugSetEnableOutputCapture(bool value);
		void DebugFlushCaptureDiscard();
		void DebugFlushCaptureToWaveFile(std::string_view filePath);

	private:
		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}
