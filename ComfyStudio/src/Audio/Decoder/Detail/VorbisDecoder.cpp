#include "Decoders.h"

namespace Audio
{
	const char* VorbisDecoder::GetFileExtensions() const
	{
		return ".ogg";
	}

	AudioDecoderResult VorbisDecoder::DecodeParseAudio(const void* fileData, size_t fileSize, AudioDecoderOutputData* outputData)
	{
		return AudioDecoderResult::Failure;
	}
}