#pragma once
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
		uint32_t* ChannelCount;
		uint32_t* SampleRate;
		std::vector<int16_t>* SampleData;
	};

	class IAudioDecoder
	{
	public:
		virtual const char* GetFileExtensions() const = 0;
		virtual AudioDecoderResult DecodeParseAudio(const void* fileData, size_t fileSize, AudioDecoderOutputData* outputData) = 0;
	};
}
