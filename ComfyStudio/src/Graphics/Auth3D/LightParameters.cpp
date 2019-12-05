#include "LightParameters.h"
#include "Misc/StringParseHelper.h"

namespace Graphics
{
	using namespace Utilities;

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
	}

	LightParameter::LightParameter()
		: Character(), Stage(), Sun(), Reflect(), Shadow(), CharacterColor(), CharacterF(), Projection()
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

	void LightParameter::Parse(const uint8_t* buffer)
	{
		const char* textBuffer = reinterpret_cast<const char*>(buffer);

		Light* currentLight = nullptr;

		while (true)
		{
			auto line = StringParsing::GetLineAdvanceToNextLine(textBuffer);

			if (line == EndOfFileTag || line.empty())
				break;

			auto tag = StringParsing::GetWord(line.data());
			auto tagData = line.substr(tag.size() + 1);

			if (tag == GroupStartTag || tag == GroupEndTag)
			{
				// TODO: ... (?)
				auto groupID = StringParsing::ParseType<uint32_t>(tagData);
			}
			else if (tag == IDStartTag)
			{
				auto lightType = static_cast<LightTargetType>(StringParsing::ParseType<uint32_t>(tagData));
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
				currentLight->Type = static_cast<LightSourceType>(StringParsing::ParseType<uint32_t>(tagData));
			}
			else if (tag == AmbientTag)
			{
				auto vec4Data = StringParsing::ParseTypeArray<float, 4>(tagData);
				currentLight->Ambient = { vec4Data[0], vec4Data[1], vec4Data[2] };
			}
			else if (tag == DiffuseTag)
			{
				auto vec4Data = StringParsing::ParseTypeArray<float, 4>(tagData);
				currentLight->Diffuse = { vec4Data[0], vec4Data[1], vec4Data[2] };
			}
			else if (tag == SpecularTag)
			{
				auto vec4Data = StringParsing::ParseTypeArray<float, 4>(tagData);
				currentLight->Specular = { vec4Data[0], vec4Data[1], vec4Data[2] };
			}
			else if (tag == PositionTag)
			{
				auto vec4Data = StringParsing::ParseTypeArray<float, 4>(tagData);
				currentLight->Position = { vec4Data[0], vec4Data[1], vec4Data[2] };
			}
			else if (tag == ToneCurveTag)
			{
				auto vec3Data = StringParsing::ParseTypeArray<float, 3>(tagData);
				currentLight->ToneCurveBegin = vec3Data[0];
				currentLight->ToneCurveEnd = vec3Data[1];
				currentLight->ToneCurveBlendRate = vec3Data[2];
			}
			else if (tag == SpotDirectionTag)
			{
				auto vec3Data = StringParsing::ParseTypeArray<float, 3>(tagData);
				currentLight->SpotDirection = { vec3Data[0], vec3Data[1], vec3Data[2] };
			}
			else if (tag == SpotExponentTag)
			{
				currentLight->SpotExponent = StringParsing::ParseType<float>(tagData);
			}
			else if (tag == SpotCutoffTag)
			{
				currentLight->SpotCuttoff = StringParsing::ParseType<float>(tagData);
			}
			else if (tag == AttenuationTag)
			{
				auto vec3Data = StringParsing::ParseTypeArray<float, 3>(tagData);
				currentLight->AttenuationConstant = vec3Data[0];
				currentLight->AttenuationLinear = vec3Data[1];
				currentLight->AttenuationQuadratic = vec3Data[2];
			}
			else if (tag == ClipPlaneTag)
			{
				// TODO: ... (?)
				auto vec4Data = StringParsing::ParseTypeArray<int, 4>(tagData);
			}
		}
	}
}
