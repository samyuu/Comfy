#include "Decoders.h"

namespace Comfy::Audio
{
	const char* VorbisDecoder::GetFileExtensions() const
	{
		return ".ogg";
	}

	DecoderResult VorbisDecoder::DecodeParseAudio(const void* fileData, size_t fileSize, DecoderOutputData* outputData)
	{
		return DecoderResult::Failure;
	}
}
