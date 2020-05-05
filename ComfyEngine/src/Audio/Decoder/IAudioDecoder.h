#pragma once
#include "Types.h"
#include "CoreTypes.h"

namespace Comfy::Audio
{
	enum class AudioDecoderResult
	{
		Success,
		Failure,
	};

	struct AudioDecoderOutputData
	{
		u32* ChannelCount;
		u32* SampleRate;
		size_t* SampleCount;
		std::unique_ptr<i16[]>* SampleData;
	};

	class IAudioDecoder
	{
	public:
		virtual const char* GetFileExtensions() const = 0;
		virtual AudioDecoderResult DecodeParseAudio(const void* fileData, size_t fileSize, AudioDecoderOutputData* outputData) = 0;
	};
}
