#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "CoreMacros.h"

namespace Comfy::IO
{
	namespace Path
	{
		constexpr char DirectorySeparator = '/';
		constexpr char DirectorySeparatorAlt = '\\';
		constexpr const char* DirectorySeparators = "/\\";

		constexpr std::array InvalidPathCharacters = { '\"', '<', '>', '|', '\0', };
		constexpr std::array InvalidFileNameCharacters = { '\"', '<', '>', '|', ':', '*', '?', '\\', '/', '\0', };

		// NOTE: Includes extension '.' character
		COMFY_NODISCARD constexpr std::string_view GetExtension(std::string_view filePath)
		{
			const auto lastIndex = filePath.find_last_of('.');
			return (lastIndex == std::string_view::npos) ? "" : filePath.substr(lastIndex);
		}

		COMFY_NODISCARD constexpr std::string_view TrimExtension(std::string_view filePath)
		{
			const auto extension = GetExtension(filePath);
			return filePath.substr(0, filePath.size() - extension.size());
		}

		COMFY_NODISCARD constexpr std::string_view TrimTrailingPathSeparators(std::string_view filePath)
		{
			const auto lastNotIndex = filePath.find_last_not_of(DirectorySeparators);
			return (lastNotIndex == std::string_view::npos) ? filePath : filePath.substr(0, lastNotIndex + 1);
		}

		COMFY_NODISCARD constexpr std::string_view GetFileName(std::string_view filePath, bool includeExtension = true)
		{
			const auto lastIndex = filePath.find_last_of(DirectorySeparators);
			const auto fileName = (lastIndex == std::string_view::npos) ? filePath : filePath.substr(lastIndex + 1);
			return (includeExtension) ? fileName : TrimExtension(fileName);
		}

		COMFY_NODISCARD constexpr std::string_view GetDirectoryName(std::string_view filePath)
		{
			const auto fileName = GetFileName(filePath);
			return fileName.empty() ? filePath : filePath.substr(0, filePath.size() - fileName.size() - 1);
		}

		COMFY_NODISCARD bool IsRelative(std::string_view filePath);

		// NOTE: New extension should contain a '.' character
		COMFY_NODISCARD std::string ChangeExtension(std::string_view filePath, std::string_view newExtension);

		COMFY_NODISCARD std::string Combine(std::string_view basePath, std::string_view fileOrDirectory);

		// NOTE: Replace '\\' -> '/' etc.
		COMFY_NODISCARD std::string Normalize(std::string_view filePath);

		// NOTE: Replace '/' -> '\\' etc.
		COMFY_NODISCARD std::string NormalizeWin32(std::string_view filePath);
	}
}
