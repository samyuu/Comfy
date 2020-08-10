#include "Decoders.h"
#include "Audio/Core/AudioEngine.h"
#define DR_MP3_IMPLEMENTATION
#include <dr_mp3.h>

namespace Comfy::Audio
{
	const char* Mp3Decoder::GetFileExtensions() const
	{
		return ".mp3";
	}

	DecoderResult Mp3Decoder::DecodeParseAudio(const void* fileData, size_t fileSize, DecoderOutputData* outputData)
	{
		drmp3_config config;
		config.outputChannels = AudioEngine::GetInstance().GetChannelCount();
		config.outputSampleRate = AudioEngine::GetInstance().GetSampleRate();

		u64 frameCount;
		i16* data = drmp3_open_memory_and_read_s16(fileData, fileSize, &config, &frameCount);
		COMFY_SCOPE_EXIT([&] { drmp3_free(data); });

		if (data == nullptr)
			return DecoderResult::Failure;

		*outputData->ChannelCount = config.outputChannels;
		*outputData->SampleRate = config.outputSampleRate;

		const auto sampleCount = (frameCount * config.outputChannels);
		*outputData->SampleCount = sampleCount;

		// TEMP:
		{
			*outputData->SampleData = std::make_unique<i16[]>(sampleCount);
			std::copy(data, data + sampleCount, outputData->SampleData->get());
		}

		return DecoderResult::Success;
	}
}
