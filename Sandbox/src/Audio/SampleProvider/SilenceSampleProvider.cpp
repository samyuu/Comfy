#include "SilenceSampleProvider.h"
#include "Audio/Core/AudioEngine.h"

namespace Audio
{
	int64_t SilenceSampleProvider::ReadSamples(int16_t bufferToFill[], int64_t frameOffset, int64_t framesToRead, uint32_t channelsToFill)
	{
		std::fill(bufferToFill, bufferToFill + (framesToRead * channelsToFill), 0);
		return framesToRead;
	}

	int64_t SilenceSampleProvider::GetFrameCount() const
	{
		return 0;
	};

	uint32_t SilenceSampleProvider::GetChannelCount() const
	{
		return AudioEngine::GetInstance()->GetChannelCount();
	}

	uint32_t SilenceSampleProvider::GetSampleRate() const
	{
		return AudioEngine::GetInstance()->GetSampleRate();
	}
}