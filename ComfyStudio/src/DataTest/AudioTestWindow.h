#pragma once
#include "CoreTypes.h"
#include "Core/BaseWindow.h"
#include "Audio/Audio.h"

namespace Comfy::Studio::DataTest
{
	class AudioTestWindow : public BaseWindow
	{
	public:
		AudioTestWindow(Application&);
		~AudioTestWindow() = default;

	public:
		const char* GetName() const override;
		ImGuiWindowFlags GetFlags() const override;
		void Gui() override;

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

		float testButtonVolume = Audio::AudioEngine::MaxVolume;

		Audio::AudioBackend selectedAudioBackend = Audio::AudioBackend::Invalid;
		Audio::ChannelMixer::MixingBehavior selectedMixingBehavior = static_cast<Audio::ChannelMixer::MixingBehavior>(-1);

		u32 newBufferFrameCount = Audio::AudioEngine::DefaultBufferFrameCount;

		static constexpr std::array<const char*, EnumCount<Audio::ChannelMixer::MixingBehavior>()> mixingBehaviorNames =
		{
			"ChannelMixer::MixingBehavior::Ignore",
			"ChannelMixer::MixingBehavior::Combine",
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
