#pragma once
#include "Types.h"
#include "IDecoder.h"
#include "Audio/SampleProvider/ISampleProvider.h"

namespace Comfy::Audio
{
	class DecoderFactory : NonCopyable
	{
	public:
		DecoderFactory();
		~DecoderFactory() = default;

	public:
		std::unique_ptr<ISampleProvider> DecodeFile(std::string_view filePath);
		std::unique_ptr<ISampleProvider> DecodeFileContent(std::string_view fileName, const void* fileContent, size_t fileSize);

	public:
		static DecoderFactory& GetInstance();

	private:
		template <typename T>
		IDecoder* RegisterDecoder();

		void RegisterAllDecoders();

		std::unique_ptr<ISampleProvider> DecodeAndProcessFileContentUsingDecoder(IDecoder& decoder, const void* fileContent, size_t fileSize);
		std::unique_ptr<ISampleProvider> ProcessDecoderOutputDataToMemorySampleProvider(DecoderOutputData& outputData);
		DecoderResult TryDecodeAndParseFileUsingMediaFoundation(std::string_view filePath, DecoderOutputData& outputData);

	private:
		std::vector<std::unique_ptr<IDecoder>> availableDecoders;
		IDecoder* wavDecoder = nullptr;
	};
}
