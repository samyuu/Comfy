#include "SilenceSampleProvider.h"
#include "Audio/Core/AudioEngine.h"

namespace Comfy::Audio
{
	i64 SilenceSampleProvider::ReadSamples(i16 bufferToFill[], i64 frameOffset, i64 framesToRead)
	{
		std::fill(bufferToFill, bufferToFill + (framesToRead * AudioEngine::OutputChannelCount), 0);
		return framesToRead;
	}

	i64 SilenceSampleProvider::GetFrameCount() const
	{
		return 0;
	}

	u32 SilenceSampleProvider::GetChannelCount() const
	{
		return AudioEngine::OutputChannelCount;
	}

	u32 SilenceSampleProvider::GetSampleRate() const
	{
		return AudioEngine::OutputSampleRate;
	}

	const i16* SilenceSampleProvider::GetRawSampleView() const
	{
		return nullptr;
	}
}
