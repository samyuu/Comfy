#pragma once
#include "Core/BaseWindow.h"
#include "Audio/Core/AudioEngine.h"
#include "Audio/SampleProvider/MemorySampleProvider.h"
#include <vector>
#include <array>

namespace DataTest
{
	class AudioTestWindow : public BaseWindow
	{
	public:
		AudioTestWindow(Application*);
		~AudioTestWindow();

		virtual void DrawGui() override;
		virtual const char* GetGuiName() const override;
		virtual ImGuiWindowFlags GetWindowFlags() const override;

	private:
		const float audioInstancesChildHeight = 240;
		const char* testSongPath = "dev_ram/sound/song/sngtst.flac";
		const char* testButtonSoundPath = "rom/sound/button/01_button1.wav";

		RefPtr<Audio::MemorySampleProvider> songTestStream = nullptr;
		RefPtr<Audio::MemorySampleProvider> buttonTestStream = nullptr;
		RefPtr<Audio::AudioInstance> songAudioInstance = nullptr;

		float testButtonVolume = Audio::AudioEngine::MaxVolume;

		struct ExtendedDeviceInfo
		{
			RtAudio::DeviceInfo Info;
			std::string SampleRatesString;
			std::string NativeFormatsString;
		};

		std::vector<ExtendedDeviceInfo> deviceInfoList;
		Audio::AudioApi selectedAudioApi = Audio::AudioApi::Invalid;
		Audio::ChannelMixer::MixingBehavior selectedMixingBehavior = static_cast<Audio::ChannelMixer::MixingBehavior>(-1);
		int newBufferSize = -1;

		std::array<const char*, static_cast<size_t>(Audio::AudioApi::Count)> audioApiNames =
		{
			"AudioApi::ASIO",
			"AudioApi::WASAPI",
		};

		std::array<const char*, static_cast<size_t>(Audio::ChannelMixer::MixingBehavior::Count)> mixingBehaviorNames =
		{
			"ChannelMixer::MixingBehavior::Ignore",
			"ChannelMixer::MixingBehavior::Mix",
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
}