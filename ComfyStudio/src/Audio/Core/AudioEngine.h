#pragma once
#include "Types.h"
#include "ICallbackReceiver.h"
#include "Detail/ChannelMixer.h"
#include "Detail/SampleMixer.h"
#include "Audio/SampleProvider/ISampleProvider.h"
#include "Audio/SampleProvider/MemorySampleProvider.h"
#include <RtAudio.h>
#include <algorithm>
#include <mutex>

namespace Comfy::DataTest { class AudioTestWindow; }

namespace Comfy::Audio
{
	enum class AudioApi : i32
	{
		Invalid = -1,
		ASIO = 0,
		WASAPI = 1,
		Count,
	};

	using StreamParameters = RtAudio::StreamParameters;

	class AudioInstance;

	class AudioEngine : NonCopyable
	{
		friend DataTest::AudioTestWindow;

	private:
		enum class AudioCallbackResult
		{
			Continue = 0, Stop = 1,
		};

	public:
		static constexpr float MinVolume = 0.0f;
		static constexpr float MaxVolume = 1.0f;

		static constexpr u32 DEFAULT_BUFFER_SIZE = 64;
		static constexpr u32 MAX_BUFFER_SIZE = 0x2000;

	public:
		~AudioEngine();

		// ----------------------
		void Initialize();
		void Dispose();

		void SetAudioApi(AudioApi value);
		void OpenStream();
		void CloseStream();

		void StartStream();
		void StopStream();
		// ----------------------

		// ----------------------
		size_t GetDeviceCount();
		RtAudio::DeviceInfo GetDeviceInfo(u32 device);

		void SetBufferSize(u32 bufferSize);
		void AddAudioInstance(const std::shared_ptr<AudioInstance>& audioInstance);
		void PlaySound(const std::shared_ptr<ISampleProvider>& sampleProvider, float volume = MaxVolume, const char* name = nullptr);
		void ShowControlPanel();
		void AddCallbackReceiver(ICallbackReceiver* callbackReceiver);
		void RemoveCallbackReceiver(ICallbackReceiver* callbackReceiver);

		std::shared_ptr<MemorySampleProvider> LoadAudioFile(std::string_view filePath);

		inline RtAudio* GetRtAudio() { return rtAudio.get(); }
		inline u32 GetChannelCount() { return 2; }
		inline u32 GetSampleRate() { return 44100; }
		inline u32 GetBufferSize() { return bufferSize; }
		inline RtAudioFormat GetStreamFormat() { return RTAUDIO_SINT16; }

		double GetStreamTime();
		void SetStreamTime(double value);
		double GetCallbackLatency();

		inline AudioApi GetDefaultAudioApi() { return AudioApi::WASAPI; }
		inline AudioApi GetActiveAudioApi() { return audioApi; }

		inline bool GetIsStreamOpen() { return isStreamOpen; }
		inline bool GetIsStreamRunning() { return isStreamRunning; }

		float GetMasterVolume();
		void SetMasterVolume(float value);
		bool GetIsExclusiveMode();

		static void CreateInstance();
		static void InitializeInstance();
		static void DisposeInstance();
		static void DeleteInstance();
		static inline AudioEngine* GetInstance() { return engineInstance.get(); }
		// ----------------------

	private:
		AudioEngine();

		std::mutex audioInstancesMutex;

		std::vector<std::shared_ptr<AudioInstance>> audioInstances;
		std::vector<ICallbackReceiver*> callbackReceivers;

		std::vector<i16> tempOutputBuffer;
		u32 bufferSize = DEFAULT_BUFFER_SIZE;

		bool isStreamOpen = false, isStreamRunning = false;
		float masterVolume = AudioEngine::MaxVolume;

		double callbackLatency;
		double callbackStreamTime, lastCallbackStreamTime;

		AudioApi audioApi = AudioApi::Invalid;
		std::unique_ptr<RtAudio> rtAudio = nullptr;

		StreamParameters streamOutputParameter;
		ChannelMixer channelMixer;

		static RtAudio::Api GetRtAudioApi(AudioApi audioApi);

		static constexpr std::array<RtAudio::Api, static_cast<size_t>(AudioApi::Count)> audioApis =
		{
			RtAudio::WINDOWS_ASIO,		// AudioApi::ASIO
			RtAudio::WINDOWS_WASAPI,	// AudioApi::WASAPI
		};

		AudioCallbackResult InternalAudioCallback(i16* outputBuffer, u32 bufferFrameCount, double streamTime);

		u32 GetDeviceId();
		StreamParameters* GetStreamOutputParameters();
		StreamParameters* GetStreamInputParameters();

		static std::unique_ptr<AudioEngine> engineInstance;

		static int InternalStaticAudioCallback(void*, void*, u32, double, RtAudioStreamStatus, void*);
	};
}
