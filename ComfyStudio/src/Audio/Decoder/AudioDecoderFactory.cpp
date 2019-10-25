#include "AudioDecoderFactory.h"
#include "Detail/Decoders.h"
#include "FileSystem/FileHelper.h"
#include "Misc/FileExtensionHelper.h"
#include "Misc/StringHelper.h"
#include "Core/Logger.h"

namespace Audio
{
	UniquePtr<AudioDecoderFactory> AudioDecoderFactory::instance = nullptr;

	AudioDecoderFactory* AudioDecoderFactory::GetInstance()
	{
		if (instance == nullptr)
			instance.reset(new AudioDecoderFactory());

		return instance.get();
	}

	RefPtr<MemorySampleProvider> AudioDecoderFactory::DecodeFile(const std::string& filePath)
	{
		const std::wstring widePath = Utf8ToUtf16(filePath);
		if (!FileSystem::FileExists(widePath))
		{
			Logger::LogErrorLine(__FUNCTION__"(): Input file %s not found", filePath.c_str());
			return nullptr;
		}

		const std::string extension = FileSystem::GetFileExtension(filePath);

		for (auto& decoder : availableDecoders)
		{
			const char* decoderExtensions = decoder->GetFileExtensions();
			if (!FileExtensionHelper::DoesAnyExtensionMatch(extension.c_str(), decoderExtensions))
				continue;

			std::vector<uint8_t> fileContent;
			if (!FileSystem::FileReader::ReadEntireFile(widePath, &fileContent))
			{
				Logger::LogErrorLine(__FUNCTION__"(): Unable to read input file %s", filePath.c_str());
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

		Logger::LogErrorLine(__FUNCTION__"(): No compatible IAudioDecoder found for the input file %s", filePath.c_str());
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