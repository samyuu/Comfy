#pragma once
#include "Types.h"
#include "Audio/Decoder/IDecoder.h"

namespace Comfy::Audio
{
	class FlacDecoder : public IDecoder
	{
	public:
		const char* GetFileExtensions() const override;
		DecoderResult DecodeParseAudio(const void* fileData, size_t fileSize, DecoderOutputData* outputData) override;
	};

	class HevagDecoder : public IDecoder
	{
	public:
		const char* GetFileExtensions() const override;
		DecoderResult DecodeParseAudio(const void* fileData, size_t fileSize, DecoderOutputData* outputData) override;
	};

	class Mp3Decoder : public IDecoder
	{
	public:
		const char* GetFileExtensions() const override;
		DecoderResult DecodeParseAudio(const void* fileData, size_t fileSize, DecoderOutputData* outputData) override;
	};

	class VorbisDecoder : public IDecoder
	{
	public:
		const char* GetFileExtensions() const override;
		DecoderResult DecodeParseAudio(const void* fileData, size_t fileSize, DecoderOutputData* outputData) override;
	};

	class WavDecoder : public IDecoder
	{
	public:
		const char* GetFileExtensions() const override;
		DecoderResult DecodeParseAudio(const void* fileData, size_t fileSize, DecoderOutputData* outputData) override;
	};
}
