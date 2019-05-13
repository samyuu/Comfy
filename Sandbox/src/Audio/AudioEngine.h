#pragma once
#include <RtAudio.h>
#include <stdint.h>
#include <algorithm>
#include <vector>
#include <memory>

typedef uint8_t byte;
typedef RtAudio::StreamParameters StreamParameters;

class AudioInstance;
class ISampleProvider;
class AudioTestWindow;

enum AudioApi
{
	AUDIO_API_INVALID = -1,
	AUDIO_API_ASIO,
	AUDIO_API_WASAPI,
	AUDIO_API_COUNT,
};

enum AudioCallbackResult
{
	AUDIO_CALLBACK_CONTINUE = 0,
	AUDIO_CALLBACK_STOP = 1,
};

constexpr float MIN_VOLUME = 0.0f;
constexpr float MAX_VOLUME = 1.0f;

constexpr uint32_t DEFAULT_BUFFER_SIZE = 64;
constexpr uint32_t MAX_BUFFER_SIZE = 0x2000;

class AudioEngine
{
	friend AudioTestWindow;

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

	inline RtAudio* GetRtAudio() { return rtAudio; };
	inline uint32_t GetChannelCount() { return 2; };
	inline uint32_t GetSampleRate() { return 44100; };
	inline uint32_t GetBufferSize() { return bufferSize; };
	inline RtAudioFormat GetStreamFormat() { return RTAUDIO_SINT16; };

	inline double GetStreamTime() { return GetRtAudio() && GetIsStreamOpen() ? GetRtAudio()->getStreamTime() : 0.0; };
	inline void SetStreamTime(double value) { if (GetRtAudio()) { GetRtAudio()->setStreamTime(value); } };
	inline double GetCallbackLatency() { return callbackLatency; };

	inline AudioApi GetDefaultAudioApi() { return AUDIO_API_WASAPI; };
	inline AudioApi GetActiveAudioApi() { return audioApi; };

	inline bool GetIsStreamOpen() { return isStreamOpen; };
	inline bool GetIsStreamRunning() { return isStreamRunning; };

	inline float GetMasterVolume() { return masterVolume; };
	inline void SetMasterVolume(float value) { masterVolume = std::clamp(value, MIN_VOLUME, MAX_VOLUME); };
	inline bool GetIsExclusiveMode() { return GetActiveAudioApi() == AUDIO_API_ASIO; };

	static inline void CreateInstance() { engineInstance = new AudioEngine(); };
	static inline void InitializeInstance() { GetInstance()->Initialize(); };
	static inline void DisposeInstance() { GetInstance()->Dispose(); };
	static inline void DeleteInstance() { delete GetInstance(); engineInstance = nullptr; };
	static inline AudioEngine* GetInstance() { return engineInstance; };
	// ----------------------

private:
	AudioEngine();
	~AudioEngine();

	//int16_t currentSampleBuffer[64 * 2];
	std::vector<std::shared_ptr<AudioInstance>> audioInstances;

	int16_t* tempOutputBuffer;
	uint32_t bufferSize = DEFAULT_BUFFER_SIZE;

	bool isStreamOpen = false, isStreamRunning = false;
	float masterVolume = MAX_VOLUME;

	bool callbackRunning = false;
	double callbackLatency;
	double callbackStreamTime, lastCallbackStreamTime;

	AudioApi audioApi = AUDIO_API_INVALID;
	RtAudio* rtAudio = nullptr;

	StreamParameters streamOutputParameter;

	inline int16_t MixSamples(int16_t sample1, int16_t sample2)
	{
		const int32_t result(static_cast<int32_t>(sample1) + static_cast<int32_t>(sample2));
		typedef std::numeric_limits<short int> Range;

		if (Range::max() < result)
			return Range::max();
		else if (Range::min() > result)
			return Range::min();
		else
			return result;
	}

	inline RtAudio::Api GetRtAudioApi(AudioApi audioApi)
	{
		return (audioApi < AUDIO_API_COUNT) ? audioApis[audioApi] : RtAudio::UNSPECIFIED;
	};

	RtAudio::Api audioApis[AUDIO_API_COUNT] =
	{
		RtAudio::WINDOWS_ASIO,
		RtAudio::WINDOWS_WASAPI,
	};

	AudioCallbackResult InternalAudioCallback(int16_t* outputBuffer, uint32_t bufferFrameCount, double streamTime);

	uint32_t GetDeviceId();
	StreamParameters* GetStreamOutputParameters();
	StreamParameters* GetStreamInputParameters();

	static AudioEngine* engineInstance;

	static int InternalStaticAudioCallback(void*, void*, uint32_t, double, RtAudioStreamStatus, void*);
};
