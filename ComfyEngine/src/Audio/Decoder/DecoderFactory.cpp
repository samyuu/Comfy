#include "DecoderFactory.h"
#include "Detail/Decoders.h"
#include "Audio/Core/AudioEngine.h"
#include "Audio/Core/Resample.h"
#include "Audio/SampleProvider/MemorySampleProvider.h"
#include "IO/File.h"
#include "IO/Path.h"
#include "Misc/StringUtil.h"
#include "Core/Logger.h"

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <stdio.h>
#include <mferror.h>
#include <wrl.h>
#include "Core/Win32LeanWindowsHeader.h"

#pragma comment(lib, "Mfplat.lib")
#pragma comment(lib, "Mfuuid.lib")
#pragma comment(lib, "Mfreadwrite.lib")

using Microsoft::WRL::ComPtr;

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

			return DecodeAndProcessFileContentUsingDecoder(*decoder, fileContent.get(), fileSize);
		}

		// NOTE: The MediaFoundation fallback decoder only works with file paths for now 
		//		 but that should be fine as usually only ComfyData and FArc files are loaded from memory which are already in known format
		DecoderOutputData mediaFoundationOutputData = {};
		if (TryDecodeAndParseFileUsingMediaFoundation(filePath, mediaFoundationOutputData) == DecoderResult::Success)
			return ProcessDecoderOutputDataToMemorySampleProvider(mediaFoundationOutputData);

		Logger::LogErrorLine(__FUNCTION__"(): No compatible IDecoder found for the input file %.*s", filePath.size(), filePath.data());
		return nullptr;
	}

	std::unique_ptr<ISampleProvider> DecoderFactory::DecodeFileContent(std::string_view fileName, const void* fileContent, size_t fileSize)
	{
		if (fileContent == nullptr || fileSize == 0)
			return nullptr;

		const auto extension = IO::Path::GetExtension(fileName);
		for (auto& decoder : availableDecoders)
		{
			if (IO::Path::DoesAnyPackedExtensionMatch(extension, decoder->GetFileExtensions()))
				return DecodeAndProcessFileContentUsingDecoder(*decoder, fileContent, fileSize);
		}

		Logger::LogErrorLine(__FUNCTION__"(): No compatible IDecoder found for the input file %.*s", fileName.size(), fileName.data());
		return nullptr;
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

	std::unique_ptr<ISampleProvider> DecoderFactory::DecodeAndProcessFileContentUsingDecoder(IDecoder& decoder, const void* fileContent, size_t fileSize)
	{
		DecoderOutputData outputData = {};
		if (decoder.DecodeParseAudio(fileContent, fileSize, outputData) == DecoderResult::Failure)
			return nullptr;

		return ProcessDecoderOutputDataToMemorySampleProvider(outputData);
	}

	std::unique_ptr<ISampleProvider> DecoderFactory::ProcessDecoderOutputDataToMemorySampleProvider(DecoderOutputData& outputData)
	{
		// TODO: Implement high quality resampling using MediaFoundation
		if (outputData.SampleRate != AudioEngine::OutputSampleRate)
			Resample<i16>(outputData.SampleData, outputData.SampleCount, outputData.SampleRate, AudioEngine::OutputSampleRate, outputData.ChannelCount);

		auto outSampleProvider = std::make_unique<MemorySampleProvider>();
		outSampleProvider->channelCount = outputData.ChannelCount;
		outSampleProvider->sampleRate = outputData.SampleRate;
		outSampleProvider->sampleCount = outputData.SampleCount;
		outSampleProvider->sampleData = std::move(outputData.SampleData);
		return outSampleProvider;
	}

	DecoderResult DecoderFactory::TryDecodeAndParseFileUsingMediaFoundation(std::string_view filePath, DecoderOutputData& outputData)
	{
		HRESULT hr = S_OK;
		Win32ThreadLocalCoInitializeOnce();
		Win32ThreadLocalMFStartupOnce();
		
		constexpr auto audioStreamIndex = MF_SOURCE_READER_FIRST_AUDIO_STREAM;

		ComPtr<IMFSourceReader> sourceReader = nullptr;
		hr = ::MFCreateSourceReaderFromURL(UTF8::WideArg(filePath).c_str(), nullptr, &sourceReader);
		if (FAILED(hr))
			return DecoderResult::Failure;

		hr = sourceReader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, false);
		hr = sourceReader->SetStreamSelection(audioStreamIndex, true);

		ComPtr<IMFMediaType> mediaTypeAudioPCM = nullptr;
		hr = ::MFCreateMediaType(&mediaTypeAudioPCM);
		if (FAILED(hr))
			return DecoderResult::Failure;

		hr = mediaTypeAudioPCM->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
		hr = mediaTypeAudioPCM->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
		hr = sourceReader->SetCurrentMediaType(audioStreamIndex, nullptr, mediaTypeAudioPCM.Get());
		if (FAILED(hr))
			return DecoderResult::Failure;

		ComPtr<IMFMediaType> mediaTypeAudioUncompressed = nullptr;
		hr = sourceReader->GetCurrentMediaType(audioStreamIndex, &mediaTypeAudioUncompressed);
		if (FAILED(hr))
			return DecoderResult::Failure;

		hr = sourceReader->SetStreamSelection(audioStreamIndex, true);
		if (FAILED(hr))
			return DecoderResult::Failure;

		UINT32 channelCount = {}, samplesPerSecond = {}, bitsPerSample = {}, blockAlignment = {};
		hr = mediaTypeAudioUncompressed->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &channelCount);
		hr = mediaTypeAudioUncompressed->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &samplesPerSecond);
		hr = mediaTypeAudioUncompressed->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &bitsPerSample);
		hr = mediaTypeAudioUncompressed->GetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, &blockAlignment);

		if (channelCount == 0 || samplesPerSecond == 0 || bitsPerSample != 16)
			return DecoderResult::Failure;

		// TODO: Somehow ask total number of samples first..? But none of the "duration" GUIDs seem to be valid
		std::vector<BYTE> allSampleBytes;

		// HACK: Just to somewhat keep the number of reallocations lower
		const size_t arbritraryInitialCapacityInSeconds = 60;
		const size_t arbritraryInitialCapacityInBytes = (samplesPerSecond * arbritraryInitialCapacityInSeconds * channelCount) * sizeof(i16);
		allSampleBytes.reserve(arbritraryInitialCapacityInBytes);

		while (true)
		{
			DWORD outSampleFlags = {};
			ComPtr<IMFSample> outSample = nullptr;
			hr = sourceReader->ReadSample(audioStreamIndex, 0, nullptr, &outSampleFlags, nullptr, &outSample);
			if (FAILED(hr))
				break;

			if (outSampleFlags & MF_SOURCE_READERF_ENDOFSTREAM)
				break;

			ComPtr<IMFMediaBuffer> outContiguousBuffer = nullptr;
			hr = outSample->ConvertToContiguousBuffer(&outContiguousBuffer);
			if (FAILED(hr))
				break;

			BYTE* outRawAudioData = nullptr;
			DWORD outRawAudioDataSize = {};
			if (SUCCEEDED(hr = outContiguousBuffer->Lock(&outRawAudioData, nullptr, &outRawAudioDataSize)))
			{
				const size_t totalSamplesByteSizeSoFar = allSampleBytes.size();
				allSampleBytes.resize(totalSamplesByteSizeSoFar + outRawAudioDataSize);
				memcpy(allSampleBytes.data() + totalSamplesByteSizeSoFar, outRawAudioData, outRawAudioDataSize);

				hr = outContiguousBuffer->Unlock();
			}
		}

		const size_t totalSampleCount = (allSampleBytes.size() / sizeof(i16));

		outputData.ChannelCount = channelCount;
		outputData.SampleRate = samplesPerSecond;
		outputData.SampleCount = totalSampleCount;
		outputData.SampleData = std::make_unique<i16[]>(totalSampleCount);
		memcpy(outputData.SampleData.get(), allSampleBytes.data(), allSampleBytes.size());

		return DecoderResult::Success;
	}
}
