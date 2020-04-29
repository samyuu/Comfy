#include "SilenceSampleProvider.h"
#include "Audio/Core/AudioEngine.h"

namespace Comfy::Audio
{
	i64 SilenceSampleProvider::ReadSamples(i16 bufferToFill[], i64 frameOffset, i64 framesToRead, u32 channelsToFill)
	{
		std::fill(bufferToFill, bufferToFill + (framesToRead * channelsToFill), 0);
		return framesToRead;
	}

	i64 SilenceSampleProvider::GetFrameCount() const
	{
		return 0;
	};

	u32 SilenceSampleProvider::GetChannelCount() const
	{
		return AudioEngine::GetInstance()->GetChannelCount();
	}

	u32 SilenceSampleProvider::GetSampleRate() const
	{
		return AudioEngine::GetInstance()->GetSampleRate();
	}
}
