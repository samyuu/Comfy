#pragma once
#include <RtAudio.h>
#include <stdint.h>
#include <algorithm>
#include <vector>
//#include <sndfile.h>

typedef uint8_t byte;

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

class AudioEngine
{
public:
	void Initialize();
	void Dispose();

	void SetAudioApi(AudioApi audioApi);
	void OpenAccess();
	void CloseAccess();

	void StartStream();
	void StopStream();

	size_t GetDeviceCount();
	RtAudio::DeviceInfo GetDeviceInfo(uint32_t device);

	inline RtAudio* GetRtAudio() { return rtAudio; };
	inline uint32_t GetChannelCount() { return 2; };
	inline uint32_t GetSampleRate() { return 44100; };
	inline uint32_t GetBufferSize() { return 64; };
	inline RtAudioFormat GetStreamFormat() { return RTAUDIO_SINT16; };

	inline AudioApi GetDefaultAudioApi() { return AUDIO_API_WASAPI; };
	inline AudioApi GetActiveAudioApi() { return audioApi; };

	inline bool GetIsStreamOpen() { return isStreamOpen; };
	inline bool GetIsStreamRunning() { return isStreamRunning; };

	inline float GetMasterVolume() { return masterVolume; };
	inline float SetMasterVolume(float value) { masterVolume = std::clamp(value, MIN_VOLUME, MAX_VOLUME); };
	inline float* GetMasterVolumePtr() { return &masterVolume; };

	static inline void CreateInstance() { engineInstance = new AudioEngine(); };
	static inline void InitializeInstance() { GetInstance()->Initialize(); };
	static inline void DisposeInstance() { GetInstance()->Dispose(); };
	static inline void DeleteInstance() { delete GetInstance(); engineInstance = nullptr; };
	static inline AudioEngine* GetInstance() { return engineInstance; };

private:
	AudioEngine();
	~AudioEngine();

	//std::vector<ISampleProvider>
	//std::vector<IAudioSource>
	//int16_t currentSampleBuffer[64 * 2];

	bool isStreamOpen = false, isStreamRunning = false;
	float masterVolume = MAX_VOLUME;
	
	AudioApi audioApi = AUDIO_API_INVALID;
	RtAudio* rtAudio = nullptr;

	inline int16_t MixSamples(int16_t a, int16_t b)
	{
		// If both samples are negative, mixed signal must have an amplitude between the lesser of A and B, and the minimum permissible negative amplitude
		// If both samples are positive, mixed signal must have an amplitude between the greater of A and B, and the maximum permissible positive amplitude
		// If samples are on opposite sides of the 0-crossing, mixed signal should reflect that samples cancel each other out somewhat

		return
			a < 0 && b < 0 ?
			((int)a + (int)b) - (((int)a * (int)b) / INT16_MIN) :
			(a > 0 && b > 0 ?
			((int)a + (int)b) - (((int)a * (int)b) / INT16_MAX) : 
				a + b);
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

	AudioCallbackResult InternalAudioCallback(int16_t* outputBuffer, uint32_t bufferFrameCount);
	RtAudio::StreamParameters GetStreamOutputParameters();
	RtAudio::StreamParameters* GetStreamInputParameters();

	static AudioEngine* engineInstance;

	static int InternalStaticAudioCallback(void*, void*, uint32_t, double, RtAudioStreamStatus, void*);
};
