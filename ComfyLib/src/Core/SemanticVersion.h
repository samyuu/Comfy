#pragma once
#include "Types.h"
#include <string>
#include <optional>

namespace Comfy
{
	struct SemanticVersion
	{
		u32 Major;
		u32 Minor;
		u32 Patch;

		constexpr SemanticVersion() : Major(0), Minor(0), Patch(0) {}
		constexpr SemanticVersion(u32 major, u32 minor = 0, u32 patch = 0) : Major(major), Minor(minor), Patch(patch) {}

		std::string ToString() const;
		static std::optional<SemanticVersion> FromString(const std::string& versionString);
	};
}
