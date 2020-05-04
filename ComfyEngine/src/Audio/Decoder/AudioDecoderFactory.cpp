#include "AudioDecoderFactory.h"
#include "Detail/Decoders.h"
#include "IO/File.h"
#include "IO/Path.h"
#include "Misc/FileExtensionHelper.h"
#include "Misc/StringHelper.h"
#include "Core/Logger.h"

namespace Comfy::Audio
{
	std::unique_ptr<AudioDecoderFactory> AudioDecoderFactoryInstance = nullptr;

	AudioDecoderFactory::AudioDecoderFactory()
	{
		RegisterAllDecoders();
	}

	AudioDecoderFactory& AudioDecoderFactory::GetInstance()
	{
		if (AudioDecoderFactoryInstance == nullptr)
			AudioDecoderFactoryInstance = std::make_unique<AudioDecoderFactory>();

		return *AudioDecoderFactoryInstance;
	}

	std::unique_ptr<ISampleProvider> AudioDecoderFactory::DecodeFile(std::string_view filePath)
	{
		if (!IO::File::Exists(filePath))
		{
			Logger::LogErrorLine(__FUNCTION__"(): Input file %.*s not found", filePath.size(), filePath.data());
			return nullptr;
		}

		const auto[basePath, internalFile] = IO::FolderFile::ParsePath(filePath);
		const auto extension = IO::Path::GetExtension(basePath);

		for (auto& decoder : availableDecoders)
		{
			if (!FileExtensionHelper::DoesAnyExtensionMatch(extension, decoder->GetFileExtensions()))
				continue;

			const auto[fileContent, fileSize] = IO::File::ReadAllBytes(filePath);
			if (fileContent == nullptr)
			{
				Logger::LogErrorLine(__FUNCTION__"(): Unable to read input file %.*s", filePath.size(), filePath.data());
				return nullptr;
			}

			auto resultSampleProvider = std::make_unique<MemorySampleProvider>();

			AudioDecoderOutputData outputData;
			outputData.ChannelCount = &resultSampleProvider->channelCount;
			outputData.SampleRate = &resultSampleProvider->sampleRate;
			outputData.SampleData = &resultSampleProvider->sampleData;

			if (decoder->DecodeParseAudio(fileContent.get(), fileSize, &outputData) == AudioDecoderResult::Failure)
				continue;

			return resultSampleProvider;
		}

		Logger::LogErrorLine(__FUNCTION__"(): No compatible IAudioDecoder found for the input file %.*s", filePath.size(), filePath.data());
		return nullptr;
	}

	void AudioDecoderFactory::RegisterAllDecoders()
	{
		availableDecoders.reserve(5);
		RegisterDecoder<FlacDecoder>();
		RegisterDecoder<HevagDecoder>();
		RegisterDecoder<Mp3Decoder>();
		RegisterDecoder<VorbisDecoder>();
		RegisterDecoder<WavDecoder>();
	}
}
