#include "Decoders.h"
#include "Misc/EndianHelper.h"

namespace Comfy::Audio
{
	const char* HevagDecoder::GetFileExtensions() const
	{
		return ".vag";
	}

	DecoderResult HevagDecoder::DecodeParseAudio(const void* fileData, size_t fileSize, DecoderOutputData* outputData)
	{
		return DecoderResult::Failure;
	}
}
