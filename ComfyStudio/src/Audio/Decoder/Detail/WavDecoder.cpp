#include "Decoders.h"
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

namespace Comfy::Audio
{
	const char* WavDecoder::GetFileExtensions() const
	{
		return ".wav";
	}

	AudioDecoderResult WavDecoder::DecodeParseAudio(const void* fileData, size_t fileSize, AudioDecoderOutputData* outputData)
	{
		u32 channels, sampleRate;
		u64 totalSampleCount;

		// drwav_open_memory_and_read_s16_into_vector(fileData, fileSize, &channels, &sampleRate, &totalSampleCount, outputData->SampleData);
		i16* data = drwav_open_memory_and_read_s16(fileData, fileSize, &channels, &sampleRate, &totalSampleCount);

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
