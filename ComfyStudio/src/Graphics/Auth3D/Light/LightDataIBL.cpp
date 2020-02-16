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
		: Version(), Character(), Stage(), Sun(), Reflect(), Shadow(), CharacterColor(), CharacterF(), Projection()
	{

	}

	LightData* LightDataIBL::GetLightData(LightTargetType type)
	{
		switch (type)
		{
		case LightTargetType::Character:
			return &Character;
		case LightTargetType::Stage:
			return &Stage;
		case LightTargetType::Sun:
			return &Sun;
		case LightTargetType::Reflect:
			return &Reflect;
		case LightTargetType::Shadow:
			return &Shadow;
		case LightTargetType::CharacterColor:
			return &CharacterColor;
		case LightTargetType::CharacterF:
			return &CharacterF;
		case LightTargetType::Projection:
			return &Projection;

		default:
			assert(false);
			return nullptr;
		}
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

			auto targetType = static_cast<LightTargetType>(StringParsing::ParseType<uint32_t>(StringParsing::GetLineAdvanceToNonCommentLine(textBuffer)));
			LightData* lightData = GetLightData(targetType);

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

				if (lightData != nullptr)
				{
					lightData->LightMap.Format = lightMapFormat;
					lightData->LightMap.Size = { size[0], size[1] };
				}
			}
		}

		const uint8_t* binaryBuffer = reinterpret_cast<const uint8_t*>(textBuffer);

		for (size_t i = 0; i < static_cast<size_t>(LightTargetType::Count); i++)
		{
			auto& lightMap = GetLightData(static_cast<LightTargetType>(i))->LightMap;
			const bool valid = (lightMap.Size.x >= 1 && lightMap.Size.y >= 1);

			lightMap.DataPointers = {};
			for (size_t i = 0; i < lightMap.DataPointers.size(); i++)
			{
				lightMap.DataPointers[i][0] = valid ? binaryBuffer : nullptr;
				binaryBuffer += GetLightMapFaceByteSize(lightMap.Size, lightMap.Format);
			}
		}

		// TODO: Calculate mipmaps used for rendering self shadows
	}

	void LightDataIBL::UploadAll()
	{
		for (size_t i = 0; i < static_cast<size_t>(LightTargetType::Count); i++)
		{
			auto& lightMap = GetLightData(static_cast<LightTargetType>(i))->LightMap;

			if (lightMap.Size.x >= 1 && lightMap.Size.y >= 1)
			{
				lightMap.CubeMap = MakeUnique<D3D_CubeMap>(lightMap);
				D3D_SetObjectDebugName(lightMap.CubeMap->GetTexture(), "LightMap IBL: %s", LightTargetTypeNames[i]);
			}
		}
	}
}
