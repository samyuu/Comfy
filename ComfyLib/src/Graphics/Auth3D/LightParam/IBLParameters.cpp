#include "IBLParameters.h"
#include "Misc/StringParseHelper.h"

namespace Comfy::Graphics
{
	using namespace Util;

	namespace
	{
		constexpr std::string_view IblFileTag = "VF5_IBL";
		constexpr std::string_view VersionTag = "VERSION";
		constexpr std::string_view LightDirectionTag = "LIT_DIR";
		constexpr std::string_view LightColorTag = "LIT_COL";
		constexpr std::string_view LightIrradianceTag = "DIFF_COEF";
		constexpr std::string_view LightMapTag = "LIGHT_MAP";
		constexpr std::string_view BinaryDataTag = "BINARY";

		LightMapFormat ParseLightMapFormat(std::string_view string)
		{
			if (string == "RGBA16F_CUBE")
				return LightMapFormat::RGBA16F_CUBE;
			else if (string == "RGBA8_CUBE")
				return LightMapFormat::RGBA8_CUBE;
			else if (string == "RGBA32F_CUBE")
				return LightMapFormat::RGBA32F_CUBE;

			assert(false);
			return {};
		}

		constexpr size_t GetLightMapFaceByteSize(ivec2 lightMapSize, LightMapFormat lightMapFormat)
		{
			auto getBytesPerComponent = [](LightMapFormat format)
			{
				switch (format)
				{
				case LightMapFormat::RGBA8_CUBE:
					return sizeof(u8);
				case LightMapFormat::RGBA16F_CUBE:
					return sizeof(u16);
				case LightMapFormat::RGBA32F_CUBE:
					return sizeof(float);
				default:
					return static_cast<size_t>(0);
				}
			};

			const size_t bytesPerPixel = (getBytesPerComponent(lightMapFormat) * 4);
			return lightMapSize.x * lightMapSize.y * bytesPerPixel;
		}
	}

	IBLParameters::IBLParameters()
		: Version(), Lights(), LightMaps()
	{
	}

	void IBLParameters::Parse(const u8* buffer, size_t bufferSize)
	{
		const char* textBuffer = reinterpret_cast<const char*>(buffer);
		const char* endOfTextBuffer = reinterpret_cast<const char*>(buffer + bufferSize);

		auto signatureTag = StringParsing::GetLineAdvanceToNonCommentLine(textBuffer);
		if (signatureTag != IblFileTag)
		{
			assert(false);
			return;
		}

		auto versionTag = StringParsing::GetLineAdvanceToNonCommentLine(textBuffer);
		if (versionTag == VersionTag)
		{
			auto versionLine = StringParsing::GetLineAdvanceToNonCommentLine(textBuffer);
			Version = StringParsing::ParseType<u32>(versionLine);
		}

		while (textBuffer < endOfTextBuffer)
		{
			auto tag = StringParsing::GetLineAdvanceToNonCommentLine(textBuffer);

			if (tag == BinaryDataTag || tag.empty())
				break;

			auto lightIndex = StringParsing::ParseType<u32>(StringParsing::GetLineAdvanceToNonCommentLine(textBuffer));
			LightDataIBL* lightData = (lightIndex < Lights.size()) ? &Lights[lightIndex] : nullptr;

			if (tag == LightDirectionTag)
			{
				auto direction = StringParsing::ParseTypeArray<float, 3>(StringParsing::GetLineAdvanceToNonCommentLine(textBuffer));

				if (lightData != nullptr)
					lightData->LightDirection = { direction[0], direction[1], direction[2] };
			}
			else if (tag == LightColorTag)
			{
				auto color = StringParsing::ParseTypeArray<float, 3>(StringParsing::GetLineAdvanceToNonCommentLine(textBuffer));

				if (lightData != nullptr)
					lightData->LightColor = { color[0], color[1], color[2] };
			}
			else if (tag == LightIrradianceTag)
			{
				constexpr int rgbComponents = 3;

				for (int i = 0; i < rgbComponents; i++)
				{
					for (int r = 0; r < mat4::length(); r++)
					{
						auto row = StringParsing::ParseTypeArray<float, mat4::length()>(StringParsing::GetLineAdvanceToNonCommentLine(textBuffer));
						if (lightData != nullptr)
							lightData->IrradianceRGB[i][r] = { row[0], row[1], row[2], row[3] };
					}
				}
			}
			else if (tag == LightMapTag)
			{
				auto lightMapFormat = ParseLightMapFormat(StringParsing::GetLineAdvanceToNonCommentLine(textBuffer));
				auto size = StringParsing::ParseTypeArray<i32, 2>(StringParsing::GetLineAdvanceToNonCommentLine(textBuffer));

				// NOTE: Skip odd LightMaps as they will be stored as mipmaps instead
				if (lightIndex % 2 != 0)
				{
					const auto& parentLightMap = LightMaps[lightIndex / 2];
					assert(lightMapFormat == parentLightMap.Format && ivec2(size[0], size[1]) == (parentLightMap.Size / 2));
					continue;
				}

				lightIndex /= 2;

				if (lightIndex < LightMaps.size())
				{
					LightMaps[lightIndex].Format = lightMapFormat;
					LightMaps[lightIndex].Size = { size[0], size[1] };
				}
			}
		}

		const size_t remainingBytes = static_cast<ptrdiff_t>(endOfTextBuffer - textBuffer);
		assert(remainingBytes < bufferSize);

		LightMapBinaryData = std::make_unique<u8[]>(remainingBytes);
		std::memcpy(LightMapBinaryData.get(), textBuffer, remainingBytes);

		const u8* binaryBuffer = LightMapBinaryData.get();
		for (auto& lightMap : LightMaps)
		{
			for (int mipMap = 0; mipMap < LightMapIBL::MipMaps; mipMap++)
			{
				for (int cubeFace = 0; cubeFace < LightMapIBL::Faces; cubeFace++)
				{
					lightMap.DataPointers[cubeFace][mipMap] = binaryBuffer;
					binaryBuffer += GetLightMapFaceByteSize(lightMap.Size / (mipMap + 1), lightMap.Format);
				}
			}
		}
	}

	/* // TODO: Move upload responsibility to Comfy::Render
	void IBLParameters::UploadAll()
	{
		for (size_t i = 0; i < LightMaps.size(); i++)
		{
			if (auto& lightMap = LightMaps[i]; lightMap.Size.x >= 1 && lightMap.Size.y >= 1)
			{
				const char* debugName = nullptr;

#if COMFY_D3D11_DEBUG_NAMES
				char debugNameBuffer[64];
				sprintf_s(debugNameBuffer, "LightMap IBL [%zu]", i);
				debugName = debugNameBuffer;
#endif

				lightMap.GPU_CubeMap = GPU::MakeCubeMap(lightMap, debugName);
			}
		}
	}
	*/
}
