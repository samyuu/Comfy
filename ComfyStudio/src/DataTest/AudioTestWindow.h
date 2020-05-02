#pragma once
#include "CoreTypes.h"
#include "Core/BaseWindow.h"
#include "Audio/Core/AudioEngine.h"
#include "Audio/SampleProvider/MemorySampleProvider.h"

namespace Comfy::DataTest
{
	class AudioTestWindow : public BaseWindow
	{
	public:
		AudioTestWindow(Application*);
		~AudioTestWindow();

		void DrawGui() override;
		const char* GetGuiName() const override;
		ImGuiWindowFlags GetWindowFlags() const override;

	private:
		static constexpr const float audioInstancesChildHeight = 240;
		static constexpr const char* testSongPath = "dev_ram/sound/song/sngtst.flac";
		static constexpr const char* testButtonSoundPath = "dev_ram/sound/button/01_button1.wav";

	private:
		struct ExtendedDeviceInfo
		{
			RtAudio::DeviceInfo Info;
			std::string SampleRatesString;
			std::string NativeFormatsString;
		};

		std::vector<ExtendedDeviceInfo> deviceInfoList;

		std::shared_ptr<Audio::MemorySampleProvider> songTestStream = nullptr;
		std::shared_ptr<Audio::MemorySampleProvider> buttonTestStream = nullptr;
		std::shared_ptr<Audio::AudioInstance> songAudioInstance = nullptr;

		float testButtonVolume = Audio::AudioEngine::MaxVolume;

		Audio::AudioApi selectedAudioApi = Audio::AudioApi::Invalid;
		Audio::ChannelMixer::MixingBehavior selectedMixingBehavior = static_cast<Audio::ChannelMixer::MixingBehavior>(-1);

		int newBufferSize = -1;

		static constexpr std::array<const char*, static_cast<size_t>(Audio::AudioApi::Count)> audioApiNames =
		{
			"AudioApi::ASIO",
			"AudioApi::WASAPI",
		};

		static constexpr std::array<const char*, static_cast<size_t>(Audio::ChannelMixer::MixingBehavior::Count)> mixingBehaviorNames =
		{
			"ChannelMixer::MixingBehavior::Ignore",
			"ChannelMixer::MixingBehavior::Mix",
		};

		static constexpr std::array<const char*, 8> deviceInfoFieldNames =
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

		static constexpr std::array<std::pair<RtAudioFormat, const char*>, 6> audioFormatDescriptions =
		{
			std::make_pair(RTAUDIO_SINT8,	"SINT8"),
			std::make_pair(RTAUDIO_SINT16,	"SINT16"),
			std::make_pair(RTAUDIO_SINT24,	"SINT24"),
			std::make_pair(RTAUDIO_SINT32,	"SINT32"),
			std::make_pair(RTAUDIO_FLOAT32,	"FLOAT32"),
			std::make_pair(RTAUDIO_FLOAT64,	"FLOAT64"),
		};

		void RefreshDeviceInfoList();
		void ShowDeviceInfoProperties(const ExtendedDeviceInfo& deviceInfo, int uid);
	};
}
