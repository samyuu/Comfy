#include "Decoders.h"
#define DR_FLAC_IMPLEMENTATION
#include <dr_flac.h>

namespace Comfy::Audio
{
	const char* FlacDecoder::GetFileExtensions() const
	{
		return ".flac";
	}

	DecoderResult FlacDecoder::DecodeParseAudio(const void* fileData, size_t fileSize, DecoderOutputData* outputData)
	{
		u32 channels, sampleRate;
		u64 totalFrameCount;

		i16* data = drflac_open_memory_and_read_pcm_frames_s16(fileData, fileSize, &channels, &sampleRate, &totalFrameCount);
		COMFY_SCOPE_EXIT([&] { drflac_free(data); });

		if (data == nullptr)
			return DecoderResult::Failure;

		*outputData->ChannelCount = channels;
		*outputData->SampleRate = sampleRate;

		const auto sampleCount = (totalFrameCount * channels);
		*outputData->SampleCount = sampleCount;

		// TEMP:
		{
			*outputData->SampleData = std::make_unique<i16[]>(sampleCount);
			std::copy(data, data + sampleCount, outputData->SampleData->get());
		}

		return DecoderResult::Success;
	}
}
