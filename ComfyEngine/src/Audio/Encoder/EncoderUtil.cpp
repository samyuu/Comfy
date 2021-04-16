#include "EncoderUtil.h"
#include <vorbis/codec.h>
#include <vorbis/vorbisenc.h>
#include <time.h>

namespace Comfy::Audio
{
	constexpr i64 EncoderBufferFrames = 0x4000; // 0x400;

	EncoderErrorCode EncodeOggVorbis(const EncoderInput& input, const EncoderOutput& output, const EncoderOptions& options, const EncoderCallbacks& callbacks)
	{
		// NOTE: Take physical pages, weld into a logical stream of packets
		ogg_stream_state oggStreamState;
		// NOTE: One Ogg bitstream page.  Vorbis packets are inside
		ogg_page         oggPage;
		// NOTE: One raw packet of data for decode
		ogg_packet       oggPacket;
		// NOTE: Struct that stores all the static vorbis bitstream settings
		vorbis_info      vorbisInfo;
		// NOTE: Struct that stores all the user comments
		vorbis_comment   vorbisComment;
		// NOTE: Central working state for the packet->PCM decoder
		vorbis_dsp_state vorbisDspState;
		// NOTE: Local working space for packet->PCM decode
		vorbis_block     vorbisBlock;

		bool eos = false;

		// NOTE: Encode setup
		vorbis_info_init(&vorbisInfo);

		const int encoderInitResult = vorbis_encode_init_vbr(&vorbisInfo, static_cast<long>(input.ChannelCount), static_cast<long>(input.SampleRate), options.VBRQuality);
		if (encoderInitResult != 0)
		{
			// NOTE: Do not continue if setup failed; this can happen if we ask for a mode that libVorbis does not support (eg, too low a bitrate, etc, will return 'OV_EIMPL')
			assert(encoderInitResult == 0);
			return EncoderErrorCode::FailedToInitialize;
		}

		vorbis_comment_init(&vorbisComment);
		{
			std::string commentBuffer;
			auto addCommentTag = [&](std::string_view tag, std::string_view content)
			{
				// TODO: Scan for and replace illegal characters in input strings
				commentBuffer.reserve(tag.size() + 1 + content.size());
				commentBuffer += tag;
				commentBuffer += '=';
				commentBuffer += content;
				vorbis_comment_add(&vorbisComment, commentBuffer.c_str());
				commentBuffer.clear();
			};

			addCommentTag("Encoder", "ComfyStudio");
			for (const auto& comment : options.Comments)
				addCommentTag(comment.Tag, comment.Content);
		}

		// NOTE: Set up the analysis state and auxiliary encoding storage
		vorbis_analysis_init(&vorbisDspState, &vorbisInfo);
		vorbis_block_init(&vorbisDspState, &vorbisBlock);

		// NOTE: Set up our packet->stream encoder
		// NOTE: Pick a random serial number; that way we can more likely build chained streams just by concatenation
		srand(static_cast<int>(time(nullptr)));
		ogg_stream_init(&oggStreamState, rand());

		{
			// NOTE: Vorbis streams begin with three headers; the initial header (with most of the codec setup parameters) which is mandated by the Ogg bitstream spec.
			//		 The second header holds any comment fields. The third header holds the bitstream codebook.
			//		 We merely need to make the headers, then pass them to libvorbis one at a time; libvorbis handles the additional Ogg bitstream constraints
			ogg_packet oggHeaderPacket;
			ogg_packet oggHeaderCommentPacket;
			ogg_packet oggHeaderCodebookPacket;

			vorbis_analysis_headerout(&vorbisDspState, &vorbisComment, &oggHeaderPacket, &oggHeaderCommentPacket, &oggHeaderCodebookPacket);
			// NOTE: Automatically placed in its own page
			ogg_stream_packetin(&oggStreamState, &oggHeaderPacket);
			ogg_stream_packetin(&oggStreamState, &oggHeaderCommentPacket);
			ogg_stream_packetin(&oggStreamState, &oggHeaderCodebookPacket);

			// NOTE: This ensures the actual audio data will start on a new page, as per spec
			while (!eos)
			{
				const int result = ogg_stream_flush(&oggStreamState, &oggPage);
				if (result == 0)
					break;

				output.WriteFileBytes(oggPage.header_len, oggPage.header);
				output.WriteFileBytes(oggPage.body_len, oggPage.body);
			}
		}

		auto interleavedReadSampleBuffer = std::make_unique<i16[]>(EncoderBufferFrames * input.ChannelCount);

		i64 totalFramesReadSoFar = 0;

		while (!eos)
		{
			const i64 framesToProcess = std::min<i64>(EncoderBufferFrames, input.TotalFrameCount - totalFramesReadSoFar);

			if (framesToProcess <= 0)
			{
				// NOTE: End of file. This can be done implicitly in the mainline, but it's easier to see here in non-clever fashion. 
				//		 Tell the library we're at end of stream so that it can handle the last frame and mark end of stream in the output properly
				vorbis_analysis_wrote(&vorbisDspState, 0);
			}
			else
			{
				auto i16ToF32 = [](i16 v) -> f32 { return static_cast<f32>(v) / static_cast<f32>(std::numeric_limits<i16>::max()); };

				// NOTE: Data to encode. Expose the buffer to submit data
				input.ReadRawSamples(framesToProcess, interleavedReadSampleBuffer.get());
				totalFramesReadSoFar += framesToProcess;

				float** perChannelBuffer = vorbis_analysis_buffer(&vorbisDspState, static_cast<int>(framesToProcess));

				// NOTE: Uninterleave samples
				if (input.ChannelCount == 1)
				{
					for (i64 frame = 0; frame < framesToProcess; frame++)
						perChannelBuffer[0][frame] = i16ToF32(interleavedReadSampleBuffer[frame]);
				}
				else if (input.ChannelCount == 2)
				{
					for (i64 frame = 0; frame < framesToProcess; frame++)
						perChannelBuffer[0][frame] = i16ToF32(interleavedReadSampleBuffer[(frame * 2) + 0]);
					for (i64 frame = 0; frame < framesToProcess; frame++)
						perChannelBuffer[1][frame] = i16ToF32(interleavedReadSampleBuffer[(frame * 2) + 1]);
				}
				else
				{
					for (u32 channel = 0; channel < input.ChannelCount; channel++)
					{
						for (i64 frame = 0; frame < framesToProcess; frame++)
							perChannelBuffer[channel][frame] = i16ToF32(interleavedReadSampleBuffer[(frame * input.ChannelCount) + channel]);
					}
				}

				// NOTE: Tell the library how much we actually submitted
				vorbis_analysis_wrote(&vorbisDspState, static_cast<int>(framesToProcess));

				if (callbacks.OnSamplesEncoded)
				{
					EncoderCallbackProgressStatus progress;
					progress.FramesEncodedSoFar = totalFramesReadSoFar;
					progress.TotalFramesToEncode = input.TotalFrameCount;
					const auto callbackResponse = callbacks.OnSamplesEncoded(progress);

					if (callbackResponse == EncoderCallbackResponse::Abort)
					{
						// TODO: Handle graceful exit...
					}
				}
			}

			// NOTE: Vorbis does some data preanalysis, then divvies up blocks for more involved (potentially parallel) processing. Get a single block for encoding now
			while (vorbis_analysis_blockout(&vorbisDspState, &vorbisBlock) == 1)
			{
				// NOTE: Analysis, assume we want to use bitrate management
				vorbis_analysis(&vorbisBlock, nullptr);
				vorbis_bitrate_addblock(&vorbisBlock);

				while (vorbis_bitrate_flushpacket(&vorbisDspState, &oggPacket))
				{
					// NOTE: Weld the packet into the bitstream
					ogg_stream_packetin(&oggStreamState, &oggPacket);

					// NOTE: Write out pages (if any)
					while (!eos)
					{
						int result = ogg_stream_pageout(&oggStreamState, &oggPage);
						if (result == 0)
							break;

						output.WriteFileBytes(oggPage.header_len, oggPage.header);
						output.WriteFileBytes(oggPage.body_len, oggPage.body);

						// NOTE: This could be set above, but for illustrative purposes, I do it here (to show that vorbis does know where the stream ends)
						if (ogg_page_eos(&oggPage))
							eos = true;
					}
				}
			}
		}

		// NOTE: Clean up and exit. vorbis_info_clear() must be called last
		ogg_stream_clear(&oggStreamState);
		vorbis_block_clear(&vorbisBlock);
		vorbis_dsp_clear(&vorbisDspState);
		vorbis_comment_clear(&vorbisComment);
		vorbis_info_clear(&vorbisInfo);

		return EncoderErrorCode::Success;
	}
}
