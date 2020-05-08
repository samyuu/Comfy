#pragma once
#include "Types.h"
#include "CoreTypes.h"

namespace Comfy::Audio
{
	enum class DecoderResult
	{
		Success,
		Failure,
	};

	struct DecoderOutputData
	{
		u32* ChannelCount;
		u32* SampleRate;
		size_t* SampleCount;
		std::unique_ptr<i16[]>* SampleData;
	};

	class IDecoder
	{
	public:
		virtual ~IDecoder() = default;

		virtual const char* GetFileExtensions() const = 0;
		virtual DecoderResult DecodeParseAudio(const void* fileData, size_t fileSize, DecoderOutputData* outputData) = 0;
	};
}
