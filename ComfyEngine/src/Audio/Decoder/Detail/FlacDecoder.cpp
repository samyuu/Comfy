#include "Decoders.h"
#define DR_FLAC_IMPLEMENTATION
#include "dr_flac.h"

namespace Comfy::Audio
{
	const char* FlacDecoder::GetFileExtensions() const
	{
		return ".flac";
	}

	AudioDecoderResult FlacDecoder::DecodeParseAudio(const void* fileData, size_t fileSize, AudioDecoderOutputData* outputData)
	{
		u32 channels, sampleRate;
		u64 totalSampleCount;

		i16* data = drflac_open_memory_and_read_pcm_frames_s16(fileData, fileSize, &channels, &sampleRate, &totalSampleCount);

		if (data == nullptr)
			return AudioDecoderResult::Failure;

		*outputData->ChannelCount = channels;
		*outputData->SampleRate = sampleRate;

		// TEMP:
		{
			outputData->SampleData->resize(totalSampleCount * channels);
			std::copy(data, data + outputData->SampleData->size(), outputData->SampleData->data());
			drflac_free(data);
		}

		return AudioDecoderResult::Success;
	}
}
