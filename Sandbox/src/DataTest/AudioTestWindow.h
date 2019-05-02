#pragma once
#include "../BaseWindow.h"
#include "../Audio/AudioEngine.h"
#include <vector>

class AudioTestWindow : public BaseWindow
{
public:
	AudioTestWindow(Application*);
	~AudioTestWindow();

	virtual void DrawGui() override;
	virtual const char* GetGuiName() override;
	virtual ImGuiWindowFlags GetWindowFlags() override;

private:
	struct ExtendedDeviceInfo
	{
		RtAudio::DeviceInfo Info;
		std::string SampleRatesString;
		std::string NativeFormatsString;
	};

	std::vector<ExtendedDeviceInfo> deviceInfoList;
	AudioApi selectedAudioApi = AUDIO_API_INVALID;

	const char* audioApiNames[AUDIO_API_COUNT] =
	{ 
		"AUDIO_API_ASIO", 
		"AUDIO_API_WASAPI", 
	};

	const char* deviceInfoFieldNames[8] =
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

	struct { RtAudioFormat format; const char* name; } audioFormatAndNames[6] =
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