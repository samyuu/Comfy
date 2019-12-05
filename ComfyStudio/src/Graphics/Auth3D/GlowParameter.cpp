#include "GlowParameter.h"
#include "Misc/StringParseHelper.h"

namespace Graphics
{
	using namespace Utilities;

	namespace
	{
		constexpr std::string_view EndOfFileTag = "EOF";
		constexpr std::string_view ExposureTag = "exposure";
		constexpr std::string_view GammaTag = "gamma";
		constexpr std::string_view SaturatePowerTag = "saturate_power";
		constexpr std::string_view SaturateCoefficientTag = "saturate_coef";
		constexpr std::string_view FlareTag = "flare";
		constexpr std::string_view SigmaTag = "sigma";
		constexpr std::string_view IntensityTag = "intensity";
		constexpr std::string_view AutoExposureTag = "auto_exposure";
		constexpr std::string_view ToneMapMethodTag = "tone_map_method";
		constexpr std::string_view FadeColorTag = "fade_color";
		constexpr std::string_view ToneTransformTag = "tone_transform";
	}

	GlowParameter::GlowParameter() : 
		Exposure(2.0f),
		Gamma(1.0f),
		SaturatePower(1),
		SaturateCoefficient(1.0f),
		FlareA(1.0f),
		ShaftA(1.0f),
		GhostA(0.5f),
		Sigma(1.0f, 1.0f, 1.0f),
		Intensity(1.0f, 1.0f, 1.0f),
		AutoExposure(false),
		ToneMapMethod(ToneMapMethod::YCC_Exponent),
		FadeColor(0.0f, 0.0f, 0.0f, 0.0f),
		ToneTransform(0.0f, 0.0f, 0.0f, 0.0f)
	{
	}

	void GlowParameter::Parse(const uint8_t * buffer)
	{
		const char* textBuffer = reinterpret_cast<const char*>(buffer);

		while (true)
		{
			auto line = StringParsing::GetLineAdvanceToNextLine(textBuffer);

			if (line == EndOfFileTag || line.empty())
				break;

			auto tag = StringParsing::GetWord(line.data());
			auto tagData = line.substr(tag.size() + 1);

			if (tag == ExposureTag)
			{
				Exposure = StringParsing::ParseType<float>(tagData);
			}
			else if (tag == GammaTag)
			{
				Gamma = StringParsing::ParseType<float>(tagData);
			}
			else if (tag == SaturatePowerTag)
			{
				SaturatePower = StringParsing::ParseType<int32_t>(tagData);
			}
			else if (tag == SaturateCoefficientTag)
			{
				SaturateCoefficient = StringParsing::ParseType<float>(tagData);
			}
			else if (tag == FlareTag)
			{
				auto data = StringParsing::ParseTypeArray<float, 3>(tagData);
				FlareA = data[0];
				ShaftA = data[1];
				GhostA = data[2];
			}
			else if (tag == SigmaTag)
			{
				auto data = StringParsing::ParseTypeArray<float, 3>(tagData);
				Sigma = { data[0], data[1], data[2], };
			}
			else if (tag == IntensityTag)
			{
				auto data = StringParsing::ParseTypeArray<float, 3>(tagData);
				Intensity = { data[0], data[1], data[2], };
			}
			else if (tag == AutoExposureTag)
			{
				AutoExposure = StringParsing::ParseType<int32_t>(tagData);
			}
			else if (tag == ToneMapMethodTag)
			{
				ToneMapMethod = static_cast<Graphics::ToneMapMethod>(StringParsing::ParseType<uint32_t>(tagData));
			}
			else if (tag == FadeColorTag)
			{
				auto data = StringParsing::ParseTypeArray<float, 4>(tagData);
				FadeColor = { data[0], data[1], data[2], data[3] };
			}
			else if (tag == ToneTransformTag)
			{
				// TODO: ... (?)
				auto data = StringParsing::ParseTypeArray<float, 6>(tagData);
				ToneTransform = { data[0], data[1], data[2], data[3], };
			}
		}
	}
}
