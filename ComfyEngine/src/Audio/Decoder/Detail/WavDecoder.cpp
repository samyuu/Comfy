#include "Decoders.h"
#define DR_WAV_IMPLEMENTATION
#include <dr_wav.h>

namespace Comfy::Audio
{
	const char* WavDecoder::GetFileExtensions() const
	{
		return ".wav";
	}

	DecoderResult WavDecoder::DecodeParseAudio(const void* fileData, size_t fileSize, DecoderOutputData& outputData)
	{
		u32 channels, sampleRate;
		u64 sampleCount;

		// drwav_open_memory_and_read_s16_into_vector(fileData, fileSize, &channels, &sampleRate, &sampleCount, outputData->SampleData);
		i16* data = drwav_open_memory_and_read_s16(fileData, fileSize, &channels, &sampleRate, &sampleCount);
		defer { drwav_free(data); };

		if (data == nullptr)
			return DecoderResult::Failure;

		outputData.ChannelCount = channels;
		outputData.SampleRate = sampleRate;
		outputData.SampleCount = sampleCount;

		// TEMP:
		{
			outputData.SampleData = std::make_unique<i16[]>(sampleCount);
			std::copy(data, data + sampleCount, outputData.SampleData.get());
		}

		return DecoderResult::Success;
	}

	std::pair<std::unique_ptr<u8[]>, size_t> WavDecoder::Encode(const i16* samples, size_t sampleCount, u32 sampleRate, u32 channelCount)
	{
		drwav_data_format format;
		format.container = drwav_container_riff;
		format.format = DR_WAVE_FORMAT_PCM;
		format.channels = channelCount;
		format.sampleRate = sampleRate;
		format.bitsPerSample = sizeof(i16) * CHAR_BIT;

		void* memoryData = nullptr;
		size_t memoryDataSize = 0;

		drwav* wavContext = drwav_open_memory_write_sequential(&memoryData, &memoryDataSize, &format, sampleCount);
		defer { drwav_close(wavContext); drwav_free(memoryData); };

		if (wavContext == nullptr)
			return std::make_pair(nullptr, 0);

		const auto samplesWritten = drwav_write(wavContext, sampleCount, samples);
		if (samplesWritten != sampleCount)
			return std::make_pair(nullptr, 0);

		auto outFileContent = std::make_unique<u8[]>(memoryDataSize);
		if (outFileContent == nullptr)
			return std::make_pair(nullptr, 0);

		std::memcpy(outFileContent.get(), memoryData, memoryDataSize);

		return std::make_pair(std::move(outFileContent), memoryDataSize);
	}
}
