#include "AudioDecoderFactory.h"
#include "Detail/Decoders.h"
#include "IO/FileHelper.h"
#include "Misc/FileExtensionHelper.h"
#include "Misc/StringHelper.h"
#include "Core/Logger.h"

namespace Comfy::Audio
{
	UniquePtr<AudioDecoderFactory> AudioDecoderFactory::instance = nullptr;

	AudioDecoderFactory* AudioDecoderFactory::GetInstance()
	{
		if (instance == nullptr)
			instance.reset(new AudioDecoderFactory());

		return instance.get();
	}

	RefPtr<MemorySampleProvider> AudioDecoderFactory::DecodeFile(std::string_view filePath)
	{
		if (!IO::FileExists(filePath))
		{
			Logger::LogErrorLine(__FUNCTION__"(): Input file %.*s not found", filePath.size(), filePath.data());
			return nullptr;
		}

		auto extension = IO::GetFileExtension(filePath);

		for (auto& decoder : availableDecoders)
		{
			const char* decoderExtensions = decoder->GetFileExtensions();
			if (!FileExtensionHelper::DoesAnyExtensionMatch(extension, decoderExtensions))
				continue;

			std::vector<u8> fileContent;
			if (!IO::FileReader::ReadEntireFile(filePath, &fileContent))
			{
				Logger::LogErrorLine(__FUNCTION__"(): Unable to read input file %.*s", filePath.size(), filePath.data());
				return nullptr;
			}

			RefPtr<MemorySampleProvider> sampleProvider = MakeRef<MemorySampleProvider>();

			AudioDecoderOutputData outputData;
			outputData.ChannelCount = &sampleProvider->channelCount;
			outputData.SampleRate = &sampleProvider->sampleRate;
			outputData.SampleData = &sampleProvider->sampleData;

			if (decoder->DecodeParseAudio(fileContent.data(), fileContent.size(), &outputData) == AudioDecoderResult::Failure)
				continue;

			return sampleProvider;
		}

		Logger::LogErrorLine(__FUNCTION__"(): No compatible IAudioDecoder found for the input file %.*s", filePath.size(), filePath.data());
		return nullptr;
	}

	AudioDecoderFactory::AudioDecoderFactory()
	{
		RegisterAllDecoders();
	}

	AudioDecoderFactory::~AudioDecoderFactory()
	{
	}

	void AudioDecoderFactory::RegisterAllDecoders()
	{
		RegisterDecoder<FlacDecoder>();
		RegisterDecoder<HevagDecoder>();
		RegisterDecoder<Mp3Decoder>();
		RegisterDecoder<VorbisDecoder>();
		RegisterDecoder<WavDecoder>();
	}
}
