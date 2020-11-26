#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "CoreMacros.h"
#include "File.h"

namespace Comfy::IO
{
	namespace Path
	{
		constexpr char ExtensionSeparator = '.';

		constexpr char DirectorySeparator = '/', DirectorySeparatorAlt = '\\';
		constexpr const char* DirectorySeparators = "/\\";

		constexpr std::array InvalidPathCharacters = { '\"', '<', '>', '|', '\0', };
		constexpr std::array InvalidFileNameCharacters = { '\"', '<', '>', '|', ':', '*', '?', '\\', '/', '\0', };

		namespace Detail
		{
			// NOTE: Includes extension '.' separator character
			COMFY_NODISCARD constexpr std::string_view GetExtension(std::string_view filePath)
			{
				const auto lastDirectoryIndex = filePath.find_last_of(DirectorySeparators);
				const auto directoryTrimmed = (lastDirectoryIndex == std::string_view::npos) ? filePath : filePath.substr(lastDirectoryIndex);

				const auto lastIndex = directoryTrimmed.find_last_of(ExtensionSeparator);
				return (lastIndex == std::string_view::npos) ? "" : directoryTrimmed.substr(lastIndex);
			}
		}

		COMFY_NODISCARD constexpr std::string_view GetExtension(std::string_view filePath)
		{
			if (const auto archivePath = Archive::ParsePath(filePath); !archivePath.FileName.empty())
				return Detail::GetExtension(archivePath.FileName);

			return Detail::GetExtension(filePath);
		}

		// NOTE: Example packed extensions: ".wav;.flac;.ogg;.mp3"
		COMFY_NODISCARD bool DoesAnyPackedExtensionMatch(std::string_view extensionToCheck, std::string_view packedExtensions, char packedSeparator = ';');

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

		COMFY_NODISCARD constexpr std::string_view TrimQuotes(std::string_view filePath)
		{
			if (filePath.size() >= 2 && filePath.front() == '\"' && filePath.back() == '\"')
				return filePath.substr(1, filePath.size() - 2);
			return filePath;
		}

		COMFY_NODISCARD constexpr std::string_view GetFileName(std::string_view filePath, bool includeExtension = true)
		{
			if (const auto archivePath = Archive::ParsePath(filePath); !archivePath.FileName.empty())
				return (includeExtension) ? archivePath.FileName : TrimExtension(archivePath.FileName);

			const auto lastIndex = filePath.find_last_of(DirectorySeparators);
			const auto fileName = (lastIndex == std::string_view::npos) ? filePath : filePath.substr(lastIndex + 1);
			return (includeExtension) ? fileName : TrimExtension(fileName);
		}

		COMFY_NODISCARD constexpr std::string_view GetDirectoryName(std::string_view filePath)
		{
			if (const auto archivePath = Archive::ParsePath(filePath); !archivePath.FileName.empty())
				filePath = archivePath.BasePath;

			const auto fileName = GetFileName(filePath);
			return fileName.empty() ? filePath : filePath.substr(0, filePath.size() - fileName.size() - 1);
		}

		COMFY_NODISCARD bool IsRelative(std::string_view filePath);

		COMFY_NODISCARD bool IsDirectory(std::string_view filePath);

		COMFY_NODISCARD std::string ResolveRelative(std::string_view relativePath);
		COMFY_NODISCARD std::string ResolveRelativeTo(std::string_view relativePath, std::string_view baseFileOrDirectory);
		
		COMFY_NODISCARD std::string TryMakeRelative(std::string_view absolutePath, std::string_view baseFileOrDirectory);

		// NOTE: New extension should contain a '.' character
		COMFY_NODISCARD std::string ChangeExtension(std::string_view filePath, std::string_view newExtension);

		COMFY_NODISCARD std::string Combine(std::string_view basePath, std::string_view fileOrDirectory);

		// NOTE: Replace '\\' -> '/' etc.
		COMFY_NODISCARD std::string Normalize(std::string_view filePath);

		// NOTE: Replace '/' -> '\\' etc.
		COMFY_NODISCARD std::string NormalizeWin32(std::string_view filePath);
	}
}
