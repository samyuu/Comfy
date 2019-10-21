#include "AudioDecoderFactory.h"
#include "Detail/Decoders.h"
#include "FileSystem/FileHelper.h"
#include "Misc/StringHelper.h"
#include "Core/Logger.h"

namespace Audio
{
	class FileExtensionHelper
	{
	public:
		static int GetNumberOfExtenions(const char* packedExtensions)
		{
			int numberOfExtensions = 0;
			{
				const char* currentCharacter = packedExtensions;
				while (*currentCharacter != '\0')
				{
					if (*currentCharacter == '.')
						numberOfExtensions++;
					currentCharacter++;
				}
			}
			return numberOfExtensions;
		}

		static bool DoesExtensionMatch(const char* extensionA, int extensionLengthA, const char* extensionB, int extensionLengthB)
		{
			if (extensionLengthA != extensionLengthB)
				return false;

			for (int i = 0; i < extensionLengthA; i++)
			{
				if (tolower(extensionA[i]) != tolower(extensionB[i]))
					return false;
			}

			return true;
		}

		static bool DoesAnyExtensionMatch(const char* inputExtension, const char* packedExtensions)
		{
			int inputExtensionLength = static_cast<int>(strlen(inputExtension));
			int numberOfExtensions = GetNumberOfExtenions(packedExtensions);

			const char* packedExtensionOffset = packedExtensions;
			for (int i = 0; i < numberOfExtensions; i++)
			{
				const char* thisPackedExtensionOffset = packedExtensionOffset;

				do
				{
					packedExtensionOffset++;
				} while (*packedExtensionOffset != '\0' && *packedExtensionOffset != '.');

				int extensionLength = static_cast<int>(packedExtensionOffset - thisPackedExtensionOffset);
				if (DoesExtensionMatch(inputExtension, inputExtensionLength, thisPackedExtensionOffset, extensionLength))
					return true;
			}

			return false;
		}
	};

	UniquePtr<AudioDecoderFactory> AudioDecoderFactory::instance = nullptr;

	AudioDecoderFactory* AudioDecoderFactory::GetInstance()
	{
		if (instance == nullptr)
			instance.reset(new AudioDecoderFactory());

		return instance.get();
	}

	RefPtr<MemorySampleProvider> AudioDecoderFactory::DecodeFile(const std::string& filePath)
	{
		std::wstring widePath = Utf8ToUtf16(filePath);
		if (!FileSystem::FileExists(widePath))
		{
			Logger::LogErrorLine(__FUNCTION__"(): Input file %s not found", filePath.c_str());
			return nullptr;
		}

		std::string extension = FileSystem::GetFileExtension(filePath);

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