#pragma once
#include "Audio/Decoder/IAudioDecoder.h"

namespace Audio
{
	class FlacDecoder : public IAudioDecoder
	{
	public:
		const char* GetFileExtensions() const override;
		AudioDecoderResult DecodeParseAudio(const void* fileData, size_t fileSize, AudioDecoderOutputData* outputData) override;
	};

	class HevagDecoder : public IAudioDecoder
	{
	public:
		const char* GetFileExtensions() const override;
		AudioDecoderResult DecodeParseAudio(const void* fileData, size_t fileSize, AudioDecoderOutputData* outputData) override;
	};

	class Mp3Decoder : public IAudioDecoder
	{
	public:
		const char* GetFileExtensions() const override;
		AudioDecoderResult DecodeParseAudio(const void* fileData, size_t fileSize, AudioDecoderOutputData* outputData) override;
	};

	class VorbisDecoder : public IAudioDecoder
	{
	public:
		const char* GetFileExtensions() const override;
		AudioDecoderResult DecodeParseAudio(const void* fileData, size_t fileSize, AudioDecoderOutputData* outputData) override;
	};

	class WavDecoder : public IAudioDecoder
	{
	public:
		const char* GetFileExtensions() const override;
		AudioDecoderResult DecodeParseAudio(const void* fileData, size_t fileSize, AudioDecoderOutputData* outputData) override;
	};
}