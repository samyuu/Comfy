#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Core/BaseWindow.h"
#include "Audio/Audio.h"

namespace Comfy::Studio::DataTest
{
	class AudioTestWindow : public BaseWindow
	{
	public:
		AudioTestWindow(Application&);
		~AudioTestWindow();

	public:
		const char* GetName() const override;
		ImGuiWindowFlags GetFlags() const override;
		void Gui() override;

	private:
		void AudioEngineGui();
		void ActiveVoicesGui();
		void LoadedSourcesGui();

		void StartSourcePreview(Audio::SourceHandle source, TimeSpan startTime = TimeSpan::Zero());
		void StopSourcePreview();

	private:
		bool previewVoiceAdded = false;
		Audio::Voice sourcePreviewVoice = Audio::VoiceHandle::Invalid;

		std::string voiceFlagsBuffer, sourceNameBuffer;
		u32 newBufferFrameCount = Audio::AudioEngine::DefaultBufferFrameCount;
	};
}
