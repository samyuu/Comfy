#include "LightParameters.h"
#include "Misc/StringUtil.h"
#include "Misc/StringParseHelper.h"

namespace Comfy::Graphics
{
	namespace
	{
		constexpr std::string_view EndOfFileTag = "EOF";
		constexpr std::string_view GroupStartTag = "group_start";
		constexpr std::string_view GroupEndTag = "group_end";
		constexpr std::string_view IDStartTag = "id_start";
		constexpr std::string_view IDEndTag = "id_end";
		constexpr std::string_view TypeTag = "type";
		constexpr std::string_view AmbientTag = "ambient";
		constexpr std::string_view DiffuseTag = "diffuse";
		constexpr std::string_view SpecularTag = "specular";
		constexpr std::string_view PositionTag = "position";
		constexpr std::string_view ToneCurveTag = "tonecurve";
		constexpr std::string_view SpotDirectionTag = "spot_direction";
		constexpr std::string_view SpotExponentTag = "spot_exponent";
		constexpr std::string_view SpotCutoffTag = "spot_cutoff";
		constexpr std::string_view AttenuationTag = "attenuation";
		constexpr std::string_view ClipPlaneTag = "clipplane";
	
		constexpr Light DefaultLight =
		{
			LightSourceType::Parallel,
			vec3(0.0f, 0.0f, 0.0f),
			vec3(1.0f, 1.0f, 1.0f),
			vec4(1.0f, 1.0f, 1.0f, 1.0f),
			vec3(-0.594598f, 0.392729f, 0.701582f),

			0.0f,
			0.0f,
			0.0f,

			vec3(0.0f, 0.0f, 0.0f),
			0.0f,
			0.0f,
			0.0f,
			0.0f,
			0.0f,
		};

		constexpr Light DefaultDisabledLight =
		{
			LightSourceType::None,
			vec3(0.0f, 0.0f, 0.0f),
			vec3(1.0f, 1.0f, 1.0f),
			vec4(1.0f, 1.0f, 1.0f, 1.0f),
			vec3(0.0f, 0.0f, 0.0f),

			0.0f,
			0.0f,
			0.0f,

			vec3(0.0f, 0.0f, 0.0f),
			0.0f,
			0.0f,
			0.0f,
			0.0f,
			0.0f,
		};
	}

	LightParameter::LightParameter() : 
		Character(DefaultLight),
		Stage(DefaultLight),
		Sun(DefaultDisabledLight),
		Reflect(DefaultDisabledLight),
		Shadow(DefaultDisabledLight),
		CharacterColor(DefaultDisabledLight),
		CharacterF(DefaultDisabledLight),
		Projection(DefaultDisabledLight)
	{
	}

	Light* LightParameter::GetLight(LightTargetType type)
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

	void LightParameter::Parse(const u8* buffer, size_t bufferSize)
	{
		const char* textBuffer = reinterpret_cast<const char*>(buffer);
		const char* endOfTextBuffer = reinterpret_cast<const char*>(buffer + bufferSize);
		
		Light* currentLight = nullptr;

		while (textBuffer < endOfTextBuffer)
		{
			auto line = Util::StringParsing::GetLineAdvanceToNextLine(textBuffer);

			if (Util::StartsWith(line, EndOfFileTag) || line.empty())
				break;

			auto tag = Util::StringParsing::GetWord(line.data());
			auto tagData = line.substr(tag.size() + 1);

			if (tag == GroupStartTag || tag == GroupEndTag)
			{
				// TODO: ... (?)
				auto groupID = Util::StringParsing::ParseType<u32>(tagData);
			}
			else if (tag == IDStartTag)
			{
				auto lightType = static_cast<LightTargetType>(Util::StringParsing::ParseType<u32>(tagData));
				currentLight = GetLight(lightType);
			}
			else if (tag == IDEndTag)
			{
				currentLight = nullptr;
			}
			else if (currentLight == nullptr)
			{
				continue;
			}
			else if (tag == TypeTag)
			{
				currentLight->Type = static_cast<LightSourceType>(Util::StringParsing::ParseType<u32>(tagData));
			}
			else if (tag == AmbientTag)
			{
				auto data = Util::StringParsing::ParseTypeArray<float, 4>(tagData);
				currentLight->Ambient = { data[0], data[1], data[2] };
			}
			else if (tag == DiffuseTag)
			{
				auto data = Util::StringParsing::ParseTypeArray<float, 4>(tagData);
				currentLight->Diffuse = { data[0], data[1], data[2] };
			}
			else if (tag == SpecularTag)
			{
				auto data = Util::StringParsing::ParseTypeArray<float, 4>(tagData);
				currentLight->Specular = { data[0], data[1], data[2], data[3] };
			}
			else if (tag == PositionTag)
			{
				auto data = Util::StringParsing::ParseTypeArray<float, 4>(tagData);
				currentLight->Position = { data[0], data[1], data[2] };
			}
			else if (tag == ToneCurveTag)
			{
				auto data = Util::StringParsing::ParseTypeArray<float, 3>(tagData);
				currentLight->ToneCurveBegin = data[0];
				currentLight->ToneCurveEnd = data[1];
				currentLight->ToneCurveBlendRate = data[2];
			}
			else if (tag == SpotDirectionTag)
			{
				auto data = Util::StringParsing::ParseTypeArray<float, 3>(tagData);
				currentLight->SpotDirection = { data[0], data[1], data[2] };
			}
			else if (tag == SpotExponentTag)
			{
				currentLight->SpotExponent = Util::StringParsing::ParseType<float>(tagData);
			}
			else if (tag == SpotCutoffTag)
			{
				currentLight->SpotCuttoff = Util::StringParsing::ParseType<float>(tagData);
			}
			else if (tag == AttenuationTag)
			{
				auto data = Util::StringParsing::ParseTypeArray<float, 3>(tagData);
				currentLight->AttenuationConstant = data[0];
				currentLight->AttenuationLinear = data[1];
				currentLight->AttenuationQuadratic = data[2];
			}
			else if (tag == ClipPlaneTag)
			{
				// TODO: ... (?)
				auto data = Util::StringParsing::ParseTypeArray<int, 4>(tagData);
			}
		}
	}
}
