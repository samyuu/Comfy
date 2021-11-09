#include "Decoders.h"
#include "Misc/EndianHelper.h"

namespace Comfy::Audio
{
	namespace
	{
		constexpr i16 HevagCoefficients[128][4] =
		{
				{      0,     0,     0,     0 }, {   7680,     0,     0,     0 }, {  14720, -6656,     0,     0 }, {  12544, -7040,     0,     0 },
				{  15616, -7680,     0,     0 }, {  14731, -7059,     0,     0 }, {  14507, -7366,     0,     0 }, {  13920, -7522,     0,     0 },
				{  13133, -7680,     0,     0 }, {  12028, -7680,     0,     0 }, {  10764, -7680,     0,     0 }, {   9359, -7680,     0,     0 },
				{   7832, -7680,     0,     0 }, {   6201, -7680,     0,     0 }, {   4488, -7680,     0,     0 }, {   2717, -7680,     0,     0 },
				{    910, -7680,     0,     0 }, {   -910, -7680,     0,     0 }, {  -2717, -7680,     0,     0 }, {  -4488, -7680,     0,     0 },
				{  -6201, -7680,     0,     0 }, {  -7832, -7680,     0,     0 }, {  -9359, -7680,     0,     0 }, { -10764, -7680,     0,     0 },
				{ -12028, -7680,     0,     0 }, { -13133, -7680,     0,     0 }, { -13920, -7522,     0,     0 }, { -14507, -7366,     0,     0 },
				{ -14731, -7059,     0,     0 }, {   5376, -9216,  3328, -3072 }, {  -6400, -7168, -3328, -2304 }, { -10496, -7424, -3584, -1024 },
				{   -167, -2722,  -494,  -541 }, {  -7430, -2221, -2298,   424 }, {  -8001, -3166, -2814,   289 }, {   6018, -4750,  2649, -1298 },
				{   3798, -6946,  3875, -1216 }, {  -8237, -2596, -2071,   227 }, {   9199,  1982, -1382, -2316 }, {  13021, -3044, -3792,  1267 },
				{  13112, -4487, -2250,  1665 }, {  -1668, -3744, -6456,   840 }, {   7819, -4328,  2111,  -506 }, {   9571, -1336,  -757,   487 },
				{  10032, -2562,   300,   199 }, {  -4745, -4122, -5486, -1493 }, {  -5896,  2378, -4787, -6947 }, {  -1193, -9117, -1237, -3114 },
				{   2783, -7108, -1575, -1447 }, {  -7334, -2062, -2212,   446 }, {   6127, -2577,  -315,   -18 }, {   9457, -1858,   102,   258 },
				{   7876, -4483,  2126,  -538 }, {  -7172, -1795, -2069,   482 }, {  -7358, -2102, -2233,   440 }, {  -9170, -3509, -2674,  -391 },
				{  -2638, -2647, -1929, -1637 }, {   1873,  9183,  1860, -5746 }, {   9214,  1859, -1124, -2427 }, {  13204, -3012, -4139,  1370 },
				{  12437, -4792,  -256,   622 }, {  -2653, -1144, -3182, -6878 }, {   9331, -1048,  -828,   507 }, {   1642,  -620,  -946, -4229 },
				{   4246, -7585,  -533, -2259 }, {  -8988, -3891, -2807,    44 }, {  -2562, -2735, -1730, -1899 }, {   3182,  -483,  -714, -1421 },
				{   7937, -3844,  2821, -1019 }, {  10069, -2609,   314,   195 }, {   8400, -3297,  1551,  -155 }, {  -8529, -2775, -2432,  -336 },
				{   9477, -1882,   108,   256 }, {     75, -2241,  -298, -6937 }, {  -9143, -4160, -2963,     5 }, {  -7270, -1958, -2156,   460 },
				{  -2740,  3745,  5936, -1089 }, {   8993,  1948,  -683, -2704 }, {  13101, -2835, -3854,  1055 }, {   9543, -1961,   130,   250 },
				{   5272, -4270,  3124, -3157 }, {  -7696, -3383, -2907,  -456 }, {   7309,  2523,   434, -2461 }, {  10275, -2867,   391,   172 },
				{  10940, -3721,   665,    97 }, {     24,  -310, -1262,   320 }, {  -8122, -2411, -2311,  -271 }, {  -8511, -3067, -2337,   163 },
				{    326, -3846,   419,  -933 }, {   8895,  2194,  -541, -2880 }, {  12073, -1876, -2017,  -601 }, {   8729, -3423,  1674,  -169 },
				{  12950, -3847, -3007,  1946 }, {  10038, -2570,   302,   198 }, {   9385, -2757,  1008,    41 }, {  -4720, -5006, -2852, -1161 },
				{   7869, -4326,  2135,  -501 }, {   2450, -8597,  1299, -2780 }, {  10192, -2763,   360,   181 }, {  11313, -4213,   833,    53 },
				{  10154, -2716,   345,   185 }, {   9638, -1417,  -737,   482 }, {   3854, -4554,  2843, -3397 }, {   6699, -5659,  2249, -1074 },
				{  11082, -3908,   728,    80 }, {  -1026, -9810,  -805, -3462 }, {  10396, -3746,  1367,   -96 }, {  10287,   988, -1915, -1437 },
				{   7953,  3878,  -764, -3263 }, {  12689, -3375, -3354,  2079 }, {   6641,  3166,   231, -2089 }, {  -2348, -7354, -1944, -4122 },
				{   9290, -4039,  1885,  -246 }, {   4633, -6403,  1748, -1619 }, {  11247, -4125,   802,    61 }, {   9807, -2284,   219,   222 },
				{   9736, -1536,  -706,   473 }, {   8440, -3436,  1562,  -176 }, {   9307, -1021,  -835,   509 }, {   1698, -9025,   688, -3037 },
				{  10214, -2791,   368,   179 }, {   8390,  3248,  -758, -2989 }, {   7201,  3316,    46, -2614 }, {    -88, -7809,  -538, -4571 },
				{   6193, -5189,  2760, -1245 }, {  12325, -1290, -3284,   253 }, {  13064, -4075, -2824,  1877 }, {   5333,  2999,   775, -1132 },
		};

		constexpr i32 HighNibbleI32(u8 byte) { return ((byte & 0x70) - (byte & 0x80)) >> 4; }
		constexpr i32 LowNibbleI32(u8 byte) { return (byte & 7) - (byte & 8); }

		struct ADPCMDataBlock
		{
			u8 DecodingCoefficient;
			u8 LoopInformation;
			u8 SoundData[14];
		};

		constexpr size_t MinHevagChannelCount = 1;
		constexpr size_t MaxHevagChannelCount = 8;
		constexpr size_t ADPCMDataBlockByteSize = sizeof(ADPCMDataBlock);
		constexpr size_t SamplesPerADPCMDataBlock = 28;

		void DecodeSingleADPCMBlock(const ADPCMDataBlock& adpcmBlock, std::array<i32, 4>& inOutADPCMHistory, i16 outSamples[SamplesPerADPCMDataBlock])
		{
			i32 coefIndex = ((adpcmBlock.LoopInformation >> 0) & 0xF0) | ((adpcmBlock.DecodingCoefficient >> 4) & 0xF);
			if (coefIndex > 127) coefIndex = 127;

			i32 shiftFactor = (adpcmBlock.DecodingCoefficient >> 0) & 0xF;
			if (shiftFactor > 12) shiftFactor = 9;
			shiftFactor = (20 - shiftFactor);

			i32 flag = (adpcmBlock.LoopInformation >> 0) & 0xF;

			for (size_t sample = 0; sample < SamplesPerADPCMDataBlock; sample++)
			{
				i32 decodedSample = 0;

				if (flag < 7)
				{
					decodedSample = (((
						inOutADPCMHistory[0] * HevagCoefficients[coefIndex][0] +
						inOutADPCMHistory[1] * HevagCoefficients[coefIndex][1] +
						inOutADPCMHistory[2] * HevagCoefficients[coefIndex][2] +
						inOutADPCMHistory[3] * HevagCoefficients[coefIndex][3]) >> 5) +
						((sample & 1 ? HighNibbleI32(adpcmBlock.SoundData[sample / 2]) : LowNibbleI32(adpcmBlock.SoundData[sample / 2])) << shiftFactor)) >> 8;
				}

				outSamples[sample] = Clamp<i32>(decodedSample, std::numeric_limits<i16>::min(), std::numeric_limits<i16>::max());

				inOutADPCMHistory[3] = inOutADPCMHistory[2];
				inOutADPCMHistory[2] = inOutADPCMHistory[1];
				inOutADPCMHistory[1] = inOutADPCMHistory[0];
				inOutADPCMHistory[0] = decodedSample;
			}
		}

		void DecodeAllADPCMBlocks(i16* outSamples, u32 channelCount, size_t blockCount, const ADPCMDataBlock* adpcmBlocks)
		{
			std::array<std::array<i32, 4>, MaxHevagChannelCount> perChannelHistory = {};
			std::array<std::array<i16, SamplesPerADPCMDataBlock>, MaxHevagChannelCount> perChannelSampleBuffer = {};

			i16* outWriteHead = outSamples;
			for (size_t channelBlock = 0; channelBlock < (blockCount / channelCount); channelBlock++)
			{
				for (size_t c = 0; c < channelCount; c++)
					DecodeSingleADPCMBlock(adpcmBlocks[(channelBlock * channelCount) + c], perChannelHistory[c], perChannelSampleBuffer[c].data());

				for (size_t sample = 0; sample < SamplesPerADPCMDataBlock; sample++)
				{
					for (size_t c = 0; c < channelCount; c++)
					{
						*outWriteHead = perChannelSampleBuffer[c][sample];
						outWriteHead++;
					}
				}
			}
		}
	}

	const char* HevagDecoder::GetFileExtensions() const
	{
		return ".vag";
	}

	DecoderResult HevagDecoder::DecodeParseAudio(const void* fileData, size_t fileSize, DecoderOutputData& outputData)
	{
		constexpr size_t headerByteSize = 48;
		if (fileSize < headerByteSize)
			return DecoderResult::Failure;

		auto fileStream = static_cast<const u8*>(fileData);
		const auto signature = Util::ByteSwapU32(*reinterpret_cast<const u32*>(fileStream)); fileStream += sizeof(u32);
		const auto version = Util::ByteSwapU32(*reinterpret_cast<const u32*>(fileStream)); fileStream += sizeof(u32);
		const auto loopStart = Util::ByteSwapU32(*reinterpret_cast<const u32*>(fileStream)); fileStream += sizeof(u32);
		const auto waveformDataSize = Util::ByteSwapU32(*reinterpret_cast<const u32*>(fileStream)); fileStream += sizeof(u32);
		const auto sampleRate = Util::ByteSwapU32(*reinterpret_cast<const u32*>(fileStream)); fileStream += sizeof(u32);
		const auto loopEnd = Util::ByteSwapU32(*reinterpret_cast<const u32*>(fileStream)); fileStream += sizeof(u32);
		fileStream += sizeof(u8) * 6;
		const auto channelCount = Clamp<u8>(*reinterpret_cast<const u8*>(fileStream), MinHevagChannelCount, MaxHevagChannelCount); fileStream += sizeof(u8);
		fileStream += sizeof(u8) * 1;
		char waveformDataName[16];
		memcpy(waveformDataName, fileStream, sizeof(waveformDataName)); fileStream += sizeof(waveformDataName);

		if (version >= 0x30000)
		{
			const auto dataWordSize = Util::ByteSwapU32(*reinterpret_cast<const u32*>(fileStream)); fileStream += sizeof(u32);
			const auto dataFormat = Util::ByteSwapU32(*reinterpret_cast<const u8*>(fileStream)); fileStream += sizeof(u8);
			fileStream += sizeof(u8) * 7;
		}

		if (signature != 'VAGp' || version != 0x00020001)
			return DecoderResult::Failure;

		auto sampleCount = (waveformDataSize / ADPCMDataBlockByteSize) * SamplesPerADPCMDataBlock;
		auto samples = std::make_unique<i16[]>(sampleCount);

		DecodeAllADPCMBlocks(samples.get(), channelCount, (waveformDataSize / sizeof(ADPCMDataBlock)), reinterpret_cast<const ADPCMDataBlock*>(fileStream));

		outputData.ChannelCount = channelCount;
		outputData.SampleRate = sampleRate;
		outputData.SampleCount = sampleCount;
		outputData.SampleData = std::move(samples);

		return DecoderResult::Success;
	}
}
