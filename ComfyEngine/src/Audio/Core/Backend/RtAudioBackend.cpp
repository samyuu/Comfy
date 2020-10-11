#include "RtAudioBackend.h"

namespace Comfy::Audio
{
	RtAudioBackend::RtAudioBackend(RtAudio::Api rtAPI) : rtAPI(rtAPI)
	{
	}

	bool RtAudioBackend::OpenStartStream(const StreamParameters& param, RenderCallbackFunc callback)
	{
		if (!callback)
			return false;

		renderCallback = std::move(callback);
		if (context == nullptr)
			context = std::make_unique<RtAudio>(rtAPI);

		outputParameters.deviceId = context->getDefaultOutputDevice();
		outputParameters.nChannels = param.ChannelCount;
		outputParameters.firstChannel = 0;

		auto inOutBufferFrames = static_cast<unsigned int>(param.DesiredFrameCount);
		void* userData = this;

		context->openStream(&outputParameters, inputParameters, RTAUDIO_SINT16, param.SampleRate, &inOutBufferFrames, &StaticAudioCallback, userData);
		context->startStream();

		return true;
	}

	bool RtAudioBackend::StopCloseStream()
	{
		if (context == nullptr)
			return true;

		context->closeStream();
		return true;
	}

	bool RtAudioBackend::IsOpenRunning() const
	{
		return (context == nullptr) ? false : (context->isStreamOpen() && context->isStreamRunning());
	}

	int RtAudioBackend::StaticAudioCallback(void* outputBuffer, void* inputBuffer, u32 bufferFrames, double streamTime, RtAudioStreamStatus status, void* userData)
	{
		const auto thisPointer = reinterpret_cast<RtAudioBackend*>(userData);
		return thisPointer->MemberAudioCallback(outputBuffer, inputBuffer, bufferFrames, streamTime, status);
	}

	int RtAudioBackend::MemberAudioCallback(void* outputBuffer, void* inputBuffer, u32 bufferFrames, double streamTime, RtAudioStreamStatus status)
	{
		auto outSamples = static_cast<i16*>(outputBuffer);
		const auto outFrameCount = bufferFrames;
		const auto outChannelCount = static_cast<u32>(outputParameters.nChannels);

		renderCallback(outSamples, outFrameCount, outChannelCount);
		return 0;
	}
}
