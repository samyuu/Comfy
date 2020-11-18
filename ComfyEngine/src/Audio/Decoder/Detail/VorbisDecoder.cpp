#include "Decoders.h"
#include <stb_vorbis.h>

namespace Comfy::Audio
{
	const char* VorbisDecoder::GetFileExtensions() const
	{
		return ".ogg";
	}

	DecoderResult VorbisDecoder::DecodeParseAudio(const void* fileData, size_t fileSize, DecoderOutputData& outputData)
	{
		i32 channels, sampleRate;
		i16* sampleData;

		const i32 frameCount = stb_vorbis_decode_memory(static_cast<const uint8*>(fileData), static_cast<int>(fileSize), &channels, &sampleRate, &sampleData);
		COMFY_SCOPE_EXIT([&] { free(sampleData); });

		if (sampleData == nullptr || frameCount < 0)
			return DecoderResult::Failure;

		const i32 sampleCount = frameCount * channels;
		outputData.ChannelCount = channels;
		outputData.SampleRate = sampleRate;
		outputData.SampleCount = sampleCount;

		// TEMP:
		{
			outputData.SampleData = std::make_unique<i16[]>(sampleCount);
			std::copy(sampleData, sampleData + sampleCount, outputData.SampleData.get());
		}

		return DecoderResult::Success;
	}
}
