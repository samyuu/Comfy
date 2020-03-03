#include "Decoders.h"
#include "Audio/Core/AudioEngine.h"
#include <dr_mp3.h>

namespace Comfy::Audio
{
	const char* Mp3Decoder::GetFileExtensions() const
	{
		return ".mp3";
	}

	AudioDecoderResult Mp3Decoder::DecodeParseAudio(const void* fileData, size_t fileSize, AudioDecoderOutputData* outputData)
	{
		drmp3_config config;
		config.outputChannels = AudioEngine::GetInstance()->GetChannelCount();
		config.outputSampleRate = AudioEngine::GetInstance()->GetSampleRate();

		uint64_t frameCount;
		int16_t* data = drmp3_open_memory_and_read_s16(fileData, fileSize, &config, &frameCount);

		if (data == nullptr)
			return AudioDecoderResult::Failure;

		*outputData->ChannelCount = config.outputChannels;
		*outputData->SampleRate = config.outputSampleRate;

		// TEMP:
		{
			outputData->SampleData->resize(frameCount * config.outputChannels);
			std::copy(data, data + outputData->SampleData->size(), outputData->SampleData->data());
			drmp3_free(data);
		}

		return AudioDecoderResult::Success;
	}
}
