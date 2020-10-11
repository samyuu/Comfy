#pragma once
#include "IAudioBackend.h"
#include <RtAudio.h>

namespace Comfy::Audio
{
	class RtAudioBackend : public IAudioBackend, NonCopyable
	{
	public:
		RtAudioBackend(RtAudio::Api rtAPI = RtAudio::WINDOWS_WASAPI);
		~RtAudioBackend() = default;

	public:
		bool OpenStartStream(const StreamParameters& param, RenderCallbackFunc callback) override;
		bool StopCloseStream() override;

	public:
		bool IsOpenRunning() const override;

	private:
		static int StaticAudioCallback(void* outputBuffer, void* inputBuffer, u32 bufferFrames, double streamTime, RtAudioStreamStatus status, void* userData);
		int MemberAudioCallback(void* outputBuffer, void* inputBuffer, u32 bufferFrames, double streamTime, RtAudioStreamStatus status);

	private:
		RenderCallbackFunc renderCallback;

		RtAudio::Api rtAPI;
		std::unique_ptr<RtAudio> context = nullptr;
		RtAudio::StreamParameters outputParameters = {};
		RtAudio::StreamParameters* inputParameters = nullptr;
	};
}
