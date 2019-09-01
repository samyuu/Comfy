#pragma once
#include "Core/CoreTypes.h"

namespace Audio
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
		Vector<int16_t>* SampleData;
	};

	class IAudioDecoder
	{
	public:
		virtual const char* GetFileExtensions() const = 0;
		virtual AudioDecoderResult DecodeParseAudio(void* fileData, size_t fileSize, AudioDecoderOutputData* outputData) = 0;
	};
}