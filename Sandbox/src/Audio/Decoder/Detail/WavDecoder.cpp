#include "Decoders.h"
#include <dr_wav.h>

namespace Audio
{
	const char* WavDecoder::GetFileExtensions() const
	{
		return ".wav";
	}

	AudioDecoderResult WavDecoder::DecodeParseAudio(void* fileData, size_t fileSize, AudioDecoderOutputData* outputData)
	{
		uint32_t channels, sampleRate;
		uint64_t totalSampleCount;

		// drwav_open_memory_and_read_s16_into_vector(fileData, fileSize, &channels, &sampleRate, &totalSampleCount, outputData->SampleData);
		int16_t* data = drwav_open_memory_and_read_s16(fileData, fileSize, &channels, &sampleRate, &totalSampleCount);

		if (data == nullptr)
			return AudioDecoderResult::Failure;

		*outputData->ChannelCount = channels;
		*outputData->SampleRate = sampleRate;

		// TEMP:
		{
			outputData->SampleData->resize(totalSampleCount * channels);
			std::copy(data, data + outputData->SampleData->size(), outputData->SampleData->data());
			drwav_free(data);
		}

		return AudioDecoderResult::Success;
	}
}