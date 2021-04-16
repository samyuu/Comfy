#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include <functional>

namespace Comfy::Audio
{
	constexpr f32 VorbisVBRQualityMin = 0.1f;
	constexpr f32 VorbisVBRQualityMax = 1.0f;

	struct VorbisComment
	{
		std::string Tag;
		std::string Content;
	};

	enum class EncoderCallbackResponse : u8
	{
		Continue,
		Abort,
	};

	struct EncoderCallbackProgressStatus
	{
		i64 FramesEncodedSoFar;
		i64 TotalFramesToEncode;
	};

	struct EncoderCallbacks
	{
		std::function<EncoderCallbackResponse(const EncoderCallbackProgressStatus&)> OnSamplesEncoded;
	};

	struct EncoderInput
	{
		u32 ChannelCount;
		u32 SampleRate;
		i64 TotalFrameCount;
		std::function<void(i64 framesToRead, i16* bufferToFill)> ReadRawSamples;
	};

	struct EncoderOptions
	{
		f32 VBRQuality;
		std::vector<VorbisComment> Comments;
	};

	struct EncoderOutput
	{
		std::function<void(size_t byteSize, const u8* bytesToWrite)> WriteFileBytes;
	};

	enum class EncoderErrorCode : i32
	{
		Success,
		FailedToInitialize,
	};

	EncoderErrorCode EncodeOggVorbis(const EncoderInput& input, const EncoderOutput& output, const EncoderOptions& options, const EncoderCallbacks& callbacks);
}
