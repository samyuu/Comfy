#include "LightDataIBL.h"
#include "LightParameters.h"
#include <charconv>

namespace Graphics
{
	namespace
	{
		constexpr std::string_view IblFileTag = "VF5_IBL";
		constexpr std::string_view VersionTag = "VERSION";
		constexpr std::string_view LightDirectionTag = "LIT_DIR";
		constexpr std::string_view LightColorTag = "LIT_COL";
		constexpr std::string_view LightIrradianceTag = "DIFF_COEF";
		constexpr std::string_view LightMapTag = "LIGHT_MAP";
		constexpr std::string_view BinaryDataTag = "BINARY";

		std::string_view GetLine(const char* textBuffer)
		{
			const char* startOfLine = textBuffer;
			const char* endOfLine = textBuffer;

			while (*endOfLine != '\0' && *endOfLine != '\r' && *endOfLine != '\n')
				endOfLine++;

			const size_t lineLength = endOfLine - startOfLine;
			return std::string_view(textBuffer, lineLength);
		}

		std::string_view GetWord(const char* textBuffer)
		{
			const char* startOfWord = textBuffer;
			const char* endOfWord = textBuffer;

			while (*endOfWord != '\0' && *endOfWord != ' ' && *endOfWord != '\r' && *endOfWord != '\n')
				endOfWord++;

			const size_t lineLength = endOfWord - startOfWord;
			return std::string_view(textBuffer, lineLength);
		}

		bool IsComment(std::string_view line)
		{
			return !line.empty() && line.front() == '#';
		}

		void AdvanceToNextLine(const char*& textBuffer)
		{
			while (*textBuffer != '\0')
			{
				if (*textBuffer == '\r')
				{
					textBuffer++;

					if (*textBuffer == '\n')
						textBuffer++;

					return;
				}

				if (*textBuffer == '\n')
				{
					textBuffer++;
					return;
				}

				textBuffer++;
			}
		}

		std::string_view GetLineAdvanceToNonCommentLine(const char*& textBuffer)
		{
			auto line = GetLine(textBuffer);
			AdvanceToNextLine(textBuffer);

			while (IsComment(line))
			{
				line = GetLine(textBuffer);
				AdvanceToNextLine(textBuffer);
			}

			return line;
		}

		template <typename T>
		T ParseType(std::string_view string)
		{
			T value = {};
			auto result = std::from_chars(string.data(), string.data() + string.size(), value);
			return value;
		}

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

		template <typename T, size_t Size>
		std::array<T, Size> ParseTypeArray(std::string_view string)
		{
			std::array<T, Size> value = {};

			for (size_t i = 0; i < Size; i++)
			{
				auto word = GetWord(string.data());
				value[i] = ParseType<T>(word);

				if (i + 1 < Size)
					string = string.substr(word.size() + 1);
			}

			return value;
		}

		constexpr LightData* GetLightData(LightDataIBL* ibl, LightTargetType lightType)
		{
			switch (lightType)
			{
			case Character:
				return &ibl->Character;
			case Stage:
				return &ibl->Stage;
			case Sun:
				return &ibl->Sun;
			case Reflect:
				return &ibl->Reflect;
			case Shadow:
				return &ibl->Shadow;
			case CharacterColor:
				return &ibl->CharacterColor;
			case CharacterF:
				return &ibl->CharacterF;
			case Projection:
				return &ibl->Projection;

			default:
				assert(false);
				return nullptr;
			}
		}

		constexpr size_t GetLightMapByteSize(LightMap& lightMap)
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
			
			const size_t bytesPerPixel = getBytesPerPixel(lightMap.Format);
			return lightMap.Size.x * lightMap.Size.y * bytesPerPixel;
		}
	}

	LightDataIBL::LightDataIBL() 
		: Version(), Character(), Stage(), Sun(), Reflect(), Shadow(), CharacterColor(), CharacterF(), Projection()
	{

	}

	void LightDataIBL::Parse(const uint8_t* buffer)
	{
		const char* textBuffer = reinterpret_cast<const char*>(buffer);

		auto signatureTag = GetLineAdvanceToNonCommentLine(textBuffer);
		if (signatureTag != IblFileTag)
		{
			assert(false);
			return;
		}

		auto versionTag = GetLineAdvanceToNonCommentLine(textBuffer);
		if (versionTag == VersionTag)
		{
			auto versionLine = GetLineAdvanceToNonCommentLine(textBuffer);
			Version = ParseType<uint32_t>(versionLine);
		}

		while (true)
		{
			auto tag = GetLineAdvanceToNonCommentLine(textBuffer);

			if (tag == BinaryDataTag || tag.empty())
				break;

			auto targetType = static_cast<LightTargetType>(ParseType<uint32_t>(GetLineAdvanceToNonCommentLine(textBuffer)));
			LightData* lightData = GetLightData(this, targetType);

			if (tag == LightDirectionTag)
			{
				auto direction = ParseTypeArray<float, 3>(GetLineAdvanceToNonCommentLine(textBuffer));

				if (lightData != nullptr)
					lightData->LightDirection = { direction[0], direction[1], direction[2] };
			}
			else if (tag == LightColorTag)
			{
				auto color = ParseTypeArray<float, 3>(GetLineAdvanceToNonCommentLine(textBuffer));

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
						auto row = ParseTypeArray<float, mat4::length()>(GetLineAdvanceToNonCommentLine(textBuffer));
						if (lightData != nullptr)
							lightData->IrradianceRGB[i][r] = { row[0], row[1], row[2], row[3] };
					}
				}
			}
			else if (tag == LightMapTag)
			{
				auto lightMapFormat = ParseLightMapFormat(GetLineAdvanceToNonCommentLine(textBuffer));
				auto size = ParseTypeArray<int32_t, 2>(GetLineAdvanceToNonCommentLine(textBuffer));

				if (lightData != nullptr)
				{
					lightData->LightMap.Format = lightMapFormat;
					lightData->LightMap.Size = { size[0], size[1] };
				}
			}
		}

		const uint8_t* binaryBuffer = reinterpret_cast<const uint8_t*>(textBuffer);

		for (size_t i = 0; i <= LightTargetType::Projection; i++)
		{
			auto& lightMap = GetLightData(this, static_cast<LightTargetType>(i))->LightMap;

			bool valid = (lightMap.Size.x >= 1 && lightMap.Size.y >= 1);
			for (size_t i = 0; i < lightMap.DataPointers.size(); i++)
			{
				lightMap.DataPointers[i] = valid ? binaryBuffer : nullptr;
				binaryBuffer += GetLightMapByteSize(lightMap);
			}
		}
	}

	void LightDataIBL::UploadAll()
	{
		constexpr std::array<const char*, 8> lightTargetTypeNames = 
		{
			"Character",
			"Stage",
			"Sun",
			"Reflect",
			"Shadow",
			"Character Color",
			"Character F",
			"Projection",
		};

		for (size_t i = 0; i <= LightTargetType::Projection; i++)
		{
			auto& lightMap = GetLightData(this, static_cast<LightTargetType>(i))->LightMap;

			if (lightMap.Size.x >= 1 && lightMap.Size.y >= 1)
			{
				lightMap.CubeMap = MakeUnique<D3D_CubeMap>(lightMap);
				D3D_SetObjectDebugName(lightMap.CubeMap->GetTexture(), "LightMap IBL: %s", lightTargetTypeNames[i]);
			}
		}
	}
}
