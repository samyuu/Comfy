#include "LightDataIBL.h"
#include "Graphics/GraphicTypesNames.h"
#include "Misc/StringParseHelper.h"

namespace Graphics
{
	using namespace Utilities;

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
			auto getBytesPerPixel = [](LightMapFormat format)
			{
				switch (format)
				{
				case LightMapFormat::RGBA8_CUBE:
					return 4;
				case LightMapFormat::RGBA16F_CUBE:
					return 8;
				case LightMapFormat::RGBA32F_CUBE:
					return 16;
				default:
					return 0;
				}
			};

			const size_t bytesPerPixel = getBytesPerPixel(lightMapFormat);
			return lightMapSize.x * lightMapSize.y * bytesPerPixel;
		}
	}

	LightDataIBL::LightDataIBL()
		: Version(), Lights(), LightMaps()
	{

	}

	void LightDataIBL::Parse(const uint8_t* buffer, size_t bufferSize)
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
			Version = StringParsing::ParseType<uint32_t>(versionLine);
		}

		while (textBuffer < endOfTextBuffer)
		{
			auto tag = StringParsing::GetLineAdvanceToNonCommentLine(textBuffer);

			if (tag == BinaryDataTag || tag.empty())
				break;

			auto lightIndex = StringParsing::ParseType<uint32_t>(StringParsing::GetLineAdvanceToNonCommentLine(textBuffer));
			LightData* lightData = (lightIndex < Lights.size()) ? &Lights[lightIndex] : nullptr;

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
				auto size = StringParsing::ParseTypeArray<int32_t, 2>(StringParsing::GetLineAdvanceToNonCommentLine(textBuffer));

				// NOTE: Special light map that is stored as a mipmap of the first
				if (lightIndex == 1)
				{
					const auto& parentLightMap = LightMaps[0];
					assert(lightMapFormat == parentLightMap.Format && ivec2(size[0], size[1]) == (parentLightMap.Size / 2));
					continue;
				}

				// NOTE: Adjust for the mipmap lightmap that has been skipped
				if (lightIndex > 1)
					lightIndex--;

				if (LightMap* lightMap = (lightIndex < LightMaps.size()) ? (&LightMaps[lightIndex]) : nullptr; lightMap != nullptr)
				{
					lightMap->Format = lightMapFormat;
					lightMap->Size = { size[0], size[1] };
				}
			}
		}

		const uint8_t* binaryBuffer = reinterpret_cast<const uint8_t*>(textBuffer);

		for (auto& lightMap : LightMaps)
		{
			const bool valid = (lightMap.Size.x >= 1 && lightMap.Size.y >= 1);

			lightMap.DataPointers = {};
			for (size_t cubeFace = 0; cubeFace < lightMap.DataPointers.size(); cubeFace++)
			{
				lightMap.DataPointers[cubeFace][0] = valid ? binaryBuffer : nullptr;
				binaryBuffer += GetLightMapFaceByteSize(lightMap.Size, lightMap.Format);
			}

			// NOTE: Special combined mipmap case
			if (&lightMap == &LightMaps.front())
			{
				for (size_t cubeFace = 0; cubeFace < lightMap.DataPointers.size(); cubeFace++)
				{
					lightMap.DataPointers[cubeFace][1] = valid ? binaryBuffer : nullptr;
					binaryBuffer += GetLightMapFaceByteSize(lightMap.Size / 2, lightMap.Format);
				}
			}
		}
	}

	void LightDataIBL::UploadAll()
	{
		for (size_t i = 0; i < LightMaps.size(); i++)
		{
			if (auto& lightMap = LightMaps[i]; lightMap.Size.x >= 1 && lightMap.Size.y >= 1)
			{
				lightMap.D3D_CubeMap = MakeUnique<D3D_CubeMap>(lightMap);
				D3D_SetObjectDebugName(lightMap.D3D_CubeMap->GetTexture(), "LightMap IBL [%zu]", i);
			}
		}
	}
}
