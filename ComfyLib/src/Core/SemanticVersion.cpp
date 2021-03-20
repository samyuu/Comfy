#include "SemanticVersion.h"

namespace Comfy
{
	constexpr const char* SemanticVersionFormatString = "%u.%u.%u";

	std::string SemanticVersion::ToString() const
	{
		char buffer[40];
		return std::string(buffer, buffer + sprintf_s(buffer, SemanticVersionFormatString, Major, Minor, Patch));
	}

	std::optional<SemanticVersion> SemanticVersion::FromString(const std::string& versionString)
	{
		if (versionString.empty())
			return std::nullopt;

		SemanticVersion result = {};
		sscanf_s(versionString.c_str(), SemanticVersionFormatString, &result.Major, &result.Minor, &result.Patch);
		return result;
	}
}
