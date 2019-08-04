#pragma once
#include "ICallbackReceiver.h"
#include <RtAudio.h>
#include <stdint.h>
#include <algorithm>
#include <vector>
#include <array>
#include <memory>

typedef RtAudio::StreamParameters StreamParameters;

class AudioInstance;
class ISampleProvider;
namespace DataTest { class AudioTestWindow; }

enum class AudioApi : int32_t
{
	Invalid = -1,
	ASIO = 0,
	WASAPI = 1,
	Count,
};

enum class AudioCallbackResult
{
	Continue = 0,
	Stop = 1,
};

constexpr float MIN_VOLUME = 0.0f;
constexpr float MAX_VOLUME = 1.0f;

constexpr uint32_t DEFAULT_BUFFER_SIZE = 64;
constexpr uint32_t MAX_BUFFER_SIZE = 0x2000;

class AudioEngine
{
	friend DataTest::AudioTestWindow;

public:
	// ----------------------
	void Initialize();
	void Dispose();

	void SetAudioApi(AudioApi audioApi);
	void OpenStream();
	void CloseStream();

	void StartStream();
	void StopStream();
	// ----------------------

	// ----------------------
	size_t GetDeviceCount();
	RtAudio::DeviceInfo GetDeviceInfo(uint32_t device);

	void SetBufferSize(uint32_t bufferSize);
	void AddAudioInstance(std::shared_ptr<AudioInstance> audioInstance);
	void PlaySound(ISampleProvider* sampleProvider, float volume = MAX_VOLUME, const char* name = nullptr);
	void ShowControlPanel();
	void AddCallbackReceiver(ICallbackReceiver* callbackReceiver);
	void RemoveCallbackReceiver(ICallbackReceiver* callbackReceiver);

	inline RtAudio* GetRtAudio() { return rtAudio; };
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
	static inline AudioEngine* GetInstance() { return engineInstance; };
	// ----------------------

private:
	AudioEngine();
	~AudioEngine();

	std::vector<std::shared_ptr<AudioInstance>> audioInstances;
	std::vector<ICallbackReceiver*> callbackReceivers;

	int16_t* tempOutputBuffer;
	uint32_t bufferSize = DEFAULT_BUFFER_SIZE;

	bool isStreamOpen = false, isStreamRunning = false;
	float masterVolume = MAX_VOLUME;

	bool callbackRunning = false;
	double callbackLatency;
	double callbackStreamTime, lastCallbackStreamTime;

	AudioApi audioApi = AudioApi::Invalid;
	RtAudio* rtAudio = nullptr;

	StreamParameters streamOutputParameter;

	int16_t MixSamples(int16_t sampleA, int16_t sampleB);
	RtAudio::Api GetRtAudioApi(AudioApi audioApi);

	std::array<RtAudio::Api, static_cast<size_t>(AudioApi::Count)> audioApis =
	{
		RtAudio::WINDOWS_ASIO,		// AudioApi::ASIO
		RtAudio::WINDOWS_WASAPI,	// AudioApi::WASAPI
	};

	AudioCallbackResult InternalAudioCallback(int16_t* outputBuffer, uint32_t bufferFrameCount, double streamTime);

	uint32_t GetDeviceId();
	StreamParameters* GetStreamOutputParameters();
	StreamParameters* GetStreamInputParameters();

	static AudioEngine* engineInstance;

	static int InternalStaticAudioCallback(void*, void*, uint32_t, double, RtAudioStreamStatus, void*);
};
