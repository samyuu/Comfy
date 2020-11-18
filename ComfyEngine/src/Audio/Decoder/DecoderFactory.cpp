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
	std::unique_ptr<DecoderFactory> DecoderFactoryInstance = std::make_unique<DecoderFactory>();

	DecoderFactory::DecoderFactory()
	{
		RegisterAllDecoders();
	}

	DecoderFactory& DecoderFactory::GetInstance()
	{
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

			return DecodeAndProcess(*decoder, fileContent.get(), fileSize);
		}

		Logger::LogErrorLine(__FUNCTION__"(): No compatible IDecoder found for the input file %.*s", filePath.size(), filePath.data());
		return nullptr;
	}

	std::unique_ptr<ISampleProvider> DecoderFactory::DecodeFileContentWAV(const void* fileContent, size_t fileSize)
	{
		assert(wavDecoder != nullptr);
		assert(fileContent != nullptr && fileSize > 0);

		return DecodeAndProcess(*wavDecoder, fileContent, fileSize);
	}

	template <typename T>
	IDecoder* DecoderFactory::RegisterDecoder()
	{
		static_assert(std::is_base_of_v<IDecoder, T>, "T must inherit from IAudioDecoder");
		return availableDecoders.emplace_back(std::make_unique<T>()).get();
	}

	void DecoderFactory::RegisterAllDecoders()
	{
		availableDecoders.reserve(5);
		RegisterDecoder<FlacDecoder>();
		RegisterDecoder<HevagDecoder>();
		RegisterDecoder<Mp3Decoder>();
		RegisterDecoder<VorbisDecoder>();
		wavDecoder = RegisterDecoder<WavDecoder>();
	}

	std::unique_ptr<ISampleProvider> DecoderFactory::DecodeAndProcess(IDecoder& decoder, const void* fileContent, size_t fileSize)
	{
		DecoderOutputData outputData = {};
		if (decoder.DecodeParseAudio(fileContent, fileSize, outputData) == DecoderResult::Failure)
			return nullptr;

		if (outputData.SampleRate != AudioEngine::OutputSampleRate)
			Resample<i16>(outputData.SampleData, outputData.SampleCount, outputData.SampleRate, AudioEngine::OutputSampleRate, outputData.ChannelCount);

		auto resultSampleProvider = std::make_unique<MemorySampleProvider>();
		resultSampleProvider->channelCount = outputData.ChannelCount;
		resultSampleProvider->sampleRate = outputData.SampleRate;
		resultSampleProvider->sampleCount = outputData.SampleCount;
		resultSampleProvider->sampleData = std::move(outputData.SampleData);
		return resultSampleProvider;
	}
}
