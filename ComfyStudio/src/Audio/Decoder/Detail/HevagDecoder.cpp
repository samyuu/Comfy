#include "Decoders.h"
#include "Misc/EndianHelper.h"

namespace Audio
{
	const char* HevagDecoder::GetFileExtensions() const
	{
		return ".vag";
	}

	AudioDecoderResult HevagDecoder::DecodeParseAudio(const void* fileData, size_t fileSize, AudioDecoderOutputData* outputData)
	{
		return AudioDecoderResult::Failure;
	}
}