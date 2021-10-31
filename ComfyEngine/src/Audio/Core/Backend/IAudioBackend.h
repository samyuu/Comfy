#pragma once
#include "Types.h"
#include <functional>

namespace Comfy::Audio
{
	enum class StreamShareMode
	{
		Shared,
		Exclusive,
	};

	struct StreamParameters
	{
		u32 SampleRate;
		u32 ChannelCount;
		u32 DesiredFrameCount;
		StreamShareMode Mode;
	};

	using RenderCallbackFunc = std::function<void(i16* outputBuffer, const u32 bufferFrameCount, const u32 bufferChannelCount)>;

	class IAudioBackend
	{
	public:
		virtual ~IAudioBackend() = default;

	public:
		virtual bool OpenStartStream(const StreamParameters& param, RenderCallbackFunc callback) = 0;
		virtual bool StopCloseStream() = 0;

	public:
		virtual bool IsOpenRunning() const = 0;
	};
}
