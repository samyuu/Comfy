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
			const auto fileName = (lastIndex == std::string_view::npos) ? filePath : filePath.substr(lastIndex);
			return (includeExtension) ? fileName : TrimExtension(fileName);
		}

		COMFY_NODISCARD constexpr std::string_view GetDirectoryName(std::string_view filePath)
		{
			const auto fileName = GetFileName(filePath);
			return fileName.empty() ? filePath : filePath.substr(0, filePath.size() - fileName.size());
		}

		COMFY_NODISCARD bool IsRelative(std::string_view filePath);

		// NOTE: New extension should contain a '.' character
		COMFY_NODISCARD std::string ChangeExtension(std::string_view filePath, std::string_view newExtension);

		COMFY_NODISCARD std::string Combine(std::string_view basePath, std::string_view fileOrDirectory);

		// NOTE: Replace '\\' -> '/' etc.
		COMFY_NODISCARD std::string Normalize(std::string_view filePath);

		// NOTE: Replace '/' -> '\\' etc.
		COMFY_NODISCARD std::string NormalizeWin32(std::string_view filePath);

		namespace ConstexprTest
		{
			/*
			static_assert(GetExtension("C:/MyDir/MySubDir/MyFile.ext") == ".ext");
			static_assert(GetExtension("C:/MyDir/MySubDir/MyFile") == "");
			static_assert(GetExtension("C:/MyDir/MySubDir") == "");
			static_assert(GetExtension("C:/MyDir/MyFile.ext") == ".ext");
			static_assert(GetExtension("C:/MyDir/MyFile.") == ".");
			static_assert(GetExtension("C:/MyFile.ext") == ".ext");
			static_assert(GetExtension("MyFile.ext") == ".ext");
			static_assert(GetExtension("MyFile") == "");
			static_assert(GetExtension("C:/.ext") == ".ext");
			static_assert(GetExtension("C:/") == "");
			static_assert(GetExtension("") == "");

			static_assert(TrimTrailingPathSeparators("C:/MyDir/MySubDir/") == "C:/MyDir/MySubDir");
			static_assert(TrimTrailingPathSeparators("C:/MyDir//") == "C:/MyDir");
			static_assert(TrimTrailingPathSeparators("C:/MyDir") == "C:/MyDir");
			static_assert(TrimTrailingPathSeparators("") == "");

			static_assert(GetDirectoryName("C:/MyDir/MySubDir/MyFile.ext") == "C:/MyDir/MySubDir");
			static_assert(GetDirectoryName("C:/MyDir/MySubDir") == "C:/MyDir");
			static_assert(GetDirectoryName("C:/MyDir/") == "C:/MyDir");
			static_assert(GetDirectoryName("C:/MyDir") == "C:");
			static_assert(GetDirectoryName("C:/") == "C:");
			*/
		}
	}
}
