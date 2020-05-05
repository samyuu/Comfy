#pragma once
#include "CoreTypes.h"
#include "Core/BaseWindow.h"
#include "Audio/Audio.h"

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
			Audio::DeviceInfo Info;
			std::string SampleRatesString;
			std::string NativeFormatsString;
		};

		std::vector<ExtendedDeviceInfo> deviceInfoList;

		Audio::SourceHandle testSongSource = Audio::SourceHandle::Invalid;
		Audio::SourceHandle buttonTestSource = Audio::SourceHandle::Invalid;
		Audio::Voice testSongVoice = Audio::VoiceHandle::Invalid;

		float testButtonVolume = Audio::Engine::MaxVolume;

		Audio::Engine::AudioAPI selectedAudioApi = Audio::Engine::AudioAPI::Invalid;
		Audio::ChannelMixer::MixingBehavior selectedMixingBehavior = static_cast<Audio::ChannelMixer::MixingBehavior>(-1);

		u32 newBufferSize = 4;

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

		void RefreshDeviceInfoList();
		void ShowDeviceInfoProperties(const ExtendedDeviceInfo& deviceInfo, int uid);
	};
}
