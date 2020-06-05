#include "FogParameter.h"
#include "Misc/StringUtil.h"
#include "Misc/StringParseHelper.h"

namespace Comfy::Graphics
{
	namespace
	{
		constexpr std::string_view EndOfFileTag = "EOF";
		constexpr std::string_view GroupStartTag = "group_start";
		constexpr std::string_view GroupEndTag = "group_end";
		constexpr std::string_view TypeTag = "type";
		constexpr std::string_view DensityTag = "density";
		constexpr std::string_view LinearTag = "linear";
		constexpr std::string_view ColorTag = "color";

		constexpr Fog DefaultDepthFog =
		{
			FogType::Linear,
			0.0f,
			10.0f, 1000.0f,
			vec3(1.0f, 1.0f, 1.0f),
		};

		constexpr Fog DefaultHeightFog =
		{
			FogType::Linear,
			0.0f,
			0.0f, 10.0f,
			vec3(1.0f, 1.0f, 1.0f),
		};

		constexpr Fog DefaultBumpFog =
		{
			FogType::Linear,
			0.0f,
			1.0f, 1000.0f,
			vec3(1.0f, 1.0f, 1.0f),
		};
	}

	FogParameter::FogParameter() :
		Depth(DefaultDepthFog),
		Height(DefaultHeightFog),
		Bump(DefaultBumpFog)
	{
	}
	
	void FogParameter::Parse(const u8* buffer, size_t bufferSize)
	{
		const char* textBuffer = reinterpret_cast<const char*>(buffer);
		const char* endOfTextBuffer = reinterpret_cast<const char*>(buffer + bufferSize);

		Fog* currentFog = nullptr;

		while (textBuffer < endOfTextBuffer)
		{
			auto line = Util::StringParsing::GetLineAdvanceToNextLine(textBuffer);

			if (Util::StartsWith(line, EndOfFileTag) || line.empty())
				break;

			const auto tag = Util::StringParsing::GetWord(line.data());
			const auto tagData = line.substr(tag.size() + 1);

			if (tag == GroupStartTag || tag == GroupEndTag)
			{
				const auto groupID = Util::StringParsing::ParseType<u32>(tagData);
				currentFog = (groupID <= 3) ? (&Depth + groupID) : nullptr;
			}
			else if (tag == GroupEndTag)
			{
				currentFog = nullptr;
			}
			else if (currentFog == nullptr)
			{
				continue;
			}
			else if (tag == TypeTag)
			{
				currentFog->Type = static_cast<FogType>(Util::StringParsing::ParseType<u32>(tagData));
			}
			else if (tag == DensityTag)
			{
				currentFog->Density = Util::StringParsing::ParseType<float>(tagData);
			}
			else if (tag == LinearTag)
			{
				const auto data = Util::StringParsing::ParseTypeArray<float, 2>(tagData);
				currentFog->Start = data[0];
				currentFog->End = data[1];
			}
			else if (tag == ColorTag)
			{
				const auto data = Util::StringParsing::ParseTypeArray<float, 4>(tagData);
				currentFog->Color = { data[0], data[1], data[2] };
			}
		}
	}
}
