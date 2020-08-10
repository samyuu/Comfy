#include "DecoderFactory.h"
#include "Detail/Decoders.h"
#include "Audio/Core/AudioEngine.h"
#include "Audio/Core/Resample.h"
#include "Audio/SampleProvider/MemorySampleProvider.h"
#include "IO/File.h"
#include "IO/Path.h"
#include "Misc/StringUtil.h"
#include "Core/Logger.h"

namespace Comfy::Audio
{
	std::unique_ptr<DecoderFactory> DecoderFactoryInstance = nullptr;

	DecoderFactory::DecoderFactory()
	{
		RegisterAllDecoders();
	}

	DecoderFactory& DecoderFactory::GetInstance()
	{
		if (DecoderFactoryInstance == nullptr)
			DecoderFactoryInstance = std::make_unique<DecoderFactory>();

		return *DecoderFactoryInstance;
	}

	std::unique_ptr<ISampleProvider> DecoderFactory::DecodeFile(std::string_view filePath)
	{
		if (!IO::File::Exists(filePath))
		{
			Logger::LogErrorLine(__FUNCTION__"(): Input file %.*s not found", filePath.size(), filePath.data());
			return nullptr;
		}

		const auto extension = IO::Path::GetExtension(filePath);

		for (auto& decoder : availableDecoders)
		{
			if (!IO::Path::DoesAnyPackedExtensionMatch(extension, decoder->GetFileExtensions()))
				continue;

			const auto[fileContent, fileSize] = IO::File::ReadAllBytes(filePath);
			if (fileContent == nullptr)
			{
				Logger::LogErrorLine(__FUNCTION__"(): Unable to read input file %.*s", filePath.size(), filePath.data());
				return nullptr;
			}

			auto resultSampleProvider = std::make_unique<MemorySampleProvider>();

			DecoderOutputData outputData;
			outputData.ChannelCount = &resultSampleProvider->channelCount;
			outputData.SampleRate = &resultSampleProvider->sampleRate;
			outputData.SampleCount = &resultSampleProvider->sampleCount;
			outputData.SampleData = &resultSampleProvider->sampleData;

			if (decoder->DecodeParseAudio(fileContent.get(), fileSize, &outputData) == DecoderResult::Failure)
				continue;

			if (*outputData.SampleRate != AudioEngine::OutputSampleRate)
				Resample<i16>(*outputData.SampleData, *outputData.SampleCount, *outputData.SampleRate, AudioEngine::OutputSampleRate, *outputData.ChannelCount);

			return resultSampleProvider;
		}

		Logger::LogErrorLine(__FUNCTION__"(): No compatible IDecoder found for the input file %.*s", filePath.size(), filePath.data());
		return nullptr;
	}

	void DecoderFactory::RegisterAllDecoders()
	{
		availableDecoders.reserve(5);
		RegisterDecoder<FlacDecoder>();
		RegisterDecoder<HevagDecoder>();
		RegisterDecoder<Mp3Decoder>();
		RegisterDecoder<VorbisDecoder>();
		RegisterDecoder<WavDecoder>();
	}
}
