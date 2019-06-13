#pragma once
#include "../BaseWindow.h"
#include "../Audio/AudioEngine.h"
#include "../Audio/MemoryAudioStream.h"
#include <vector>
#include <array>

class AudioTestWindow : public BaseWindow
{
public:
	AudioTestWindow(Application*);
	~AudioTestWindow();

	virtual void DrawGui() override;
	virtual const char* GetGuiName() const override;
	virtual ImGuiWindowFlags GetWindowFlags() const override;

private:
	const float audioInstancesChildHeight = 0; // 128;
	const char* testSongPath = "rom/sound/sngtst.flac";

	MemoryAudioStream songTestStream;
	std::shared_ptr<AudioInstance> songAudioInstance = nullptr;

	MemoryAudioStream testButtonSound;
	float testButtonVolume = MAX_VOLUME;

	struct ExtendedDeviceInfo
	{
		RtAudio::DeviceInfo Info;
		std::string SampleRatesString;
		std::string NativeFormatsString;
	};

	std::vector<ExtendedDeviceInfo> deviceInfoList;
	AudioApi selectedAudioApi = AUDIO_API_INVALID;
	int newBufferSize = -1;

	std::array<const char*, AUDIO_API_COUNT> audioApiNames =
	{ 
		"AUDIO_API_ASIO",
		"AUDIO_API_WASAPI",
	};

	std::array<const char*, 8> deviceInfoFieldNames =
	{
		"Name",
		"Output Channels",
		"Input Channels",
		"Duplex Channels",
		"Default Output",
		"Default Input",
		"Sample Rates",
		"Native Formats",
	};

	struct { RtAudioFormat Format; const char* Name; } audioFormatDescriptions[6] =
	{
		{ RTAUDIO_SINT8,	"SINT8"   },
		{ RTAUDIO_SINT16,	"SINT16"  },
		{ RTAUDIO_SINT24,	"SINT24"  },
		{ RTAUDIO_SINT32,	"SINT32"  },
		{ RTAUDIO_FLOAT32,	"FLOAT32" },
		{ RTAUDIO_FLOAT64,	"FLOAT64" },
	};

	void RefreshDeviceInfoList();
	void ShowDeviceInfoProperties(ExtendedDeviceInfo& deviceInfo, int uid);
};