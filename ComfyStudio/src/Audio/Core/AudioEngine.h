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

namespace DataTest { class AudioTestWindow; }

namespace Audio
{
	enum class AudioApi : int32_t
	{
		Invalid = -1,
		ASIO = 0,
		WASAPI = 1,
		Count,
	};

	using StreamParameters = RtAudio::StreamParameters;

	class AudioInstance;

	class AudioEngine
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

		static constexpr uint32_t DEFAULT_BUFFER_SIZE = 64;
		static constexpr uint32_t MAX_BUFFER_SIZE = 0x2000;

	public:
		AudioEngine(const AudioEngine&) = delete;
		~AudioEngine();

		const AudioEngine& operator=(const AudioEngine&) = delete;

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
		RtAudio::DeviceInfo GetDeviceInfo(uint32_t device);

		void SetBufferSize(uint32_t bufferSize);
		void AddAudioInstance(const RefPtr<AudioInstance>& audioInstance);
		void PlaySound(const RefPtr<ISampleProvider>& sampleProvider, float volume = MaxVolume, const char* name = nullptr);
		void ShowControlPanel();
		void AddCallbackReceiver(ICallbackReceiver* callbackReceiver);
		void RemoveCallbackReceiver(ICallbackReceiver* callbackReceiver);

		RefPtr<MemorySampleProvider> LoadAudioFile(const std::string& filePath);

		inline RtAudio* GetRtAudio() { return rtAudio.get(); };
		inline uint32_t GetChannelCount() { return 2; };
		inline uint32_t GetSampleRate() { return 44100; };
		inline uint32_t GetBufferSize() { return bufferSize; };
		inline RtAudioFormat GetStreamFormat() { return RTAUDIO_SINT16; };

		double GetStreamTime();
		void SetStreamTime(double value);
		double GetCallbackLatency();

		inline AudioApi GetDefaultAudioApi() { return AudioApi::WASAPI; };
		inline AudioApi GetActiveAudioApi() { return audioApi; };

		inline bool GetIsStreamOpen() { return isStreamOpen; };
		inline bool GetIsStreamRunning() { return isStreamRunning; };

		float GetMasterVolume();
		void SetMasterVolume(float value);
		bool GetIsExclusiveMode();

		static void CreateInstance();
		static void InitializeInstance();
		static void DisposeInstance();
		static void DeleteInstance();
		static inline AudioEngine* GetInstance() { return engineInstance.get(); };
		// ----------------------

	private:
		AudioEngine();

		std::mutex audioInstancesMutex;

		std::vector<RefPtr<AudioInstance>> audioInstances;
		std::vector<ICallbackReceiver*> callbackReceivers;

		std::vector<int16_t> tempOutputBuffer;
		uint32_t bufferSize = DEFAULT_BUFFER_SIZE;

		bool isStreamOpen = false, isStreamRunning = false;
		float masterVolume = AudioEngine::MaxVolume;

		double callbackLatency;
		double callbackStreamTime, lastCallbackStreamTime;

		AudioApi audioApi = AudioApi::Invalid;
		UniquePtr<RtAudio> rtAudio = nullptr;

		StreamParameters streamOutputParameter;
		ChannelMixer channelMixer;

		static RtAudio::Api GetRtAudioApi(AudioApi audioApi);

		static constexpr std::array<RtAudio::Api, static_cast<size_t>(AudioApi::Count)> audioApis =
		{
			RtAudio::WINDOWS_ASIO,		// AudioApi::ASIO
			RtAudio::WINDOWS_WASAPI,	// AudioApi::WASAPI
		};

		AudioCallbackResult InternalAudioCallback(int16_t* outputBuffer, uint32_t bufferFrameCount, double streamTime);

		uint32_t GetDeviceId();
		StreamParameters* GetStreamOutputParameters();
		StreamParameters* GetStreamInputParameters();

		static UniquePtr<AudioEngine> engineInstance;

		static int InternalStaticAudioCallback(void*, void*, uint32_t, double, RtAudioStreamStatus, void*);
	};
}