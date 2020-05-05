#include "Decoders.h"
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

namespace Comfy::Audio
{
	const char* WavDecoder::GetFileExtensions() const
	{
		return ".wav";
	}

	DecoderResult WavDecoder::DecodeParseAudio(const void* fileData, size_t fileSize, DecoderOutputData* outputData)
	{
		u32 channels, sampleRate;
		u64 totalSampleCount;

		// drwav_open_memory_and_read_s16_into_vector(fileData, fileSize, &channels, &sampleRate, &totalSampleCount, outputData->SampleData);
		i16* data = drwav_open_memory_and_read_s16(fileData, fileSize, &channels, &sampleRate, &totalSampleCount);
		COMFY_SCOPE_EXIT([&] { drwav_free(data); });

		if (data == nullptr)
			return DecoderResult::Failure;

		*outputData->ChannelCount = channels;
		*outputData->SampleRate = sampleRate;

		const auto sampleCount = (totalSampleCount * channels);
		*outputData->SampleCount = sampleCount;

		// TEMP:
		{
			*outputData->SampleData = std::make_unique<i16[]>(sampleCount);
			std::copy(data, data + sampleCount, outputData->SampleData->get());
		}

		return DecoderResult::Success;
	}
}
