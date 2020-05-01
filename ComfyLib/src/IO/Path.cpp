#include "Path.h"
#include "Misc/UTF8.h"
#include <Shlwapi.h>

namespace Comfy::IO
{
	namespace Path
	{
		bool IsRelative(std::string_view filePath)
		{
			return ::PathIsRelativeW(UTF8::WideArg(filePath).c_str());
		}

		std::string ChangeExtension(std::string_view filePath, std::string_view newExtension)
		{
			const auto oldExtension = GetExtension(filePath);

			std::string newPath;
			newPath.reserve(filePath.size() - oldExtension.size() + newExtension.size());
			newPath += filePath.substr(0, filePath.size() - oldExtension.size());
			newPath += newExtension;
			return newPath;
		}

		std::string Combine(std::string_view basePath, std::string_view fileOrDirectory)
		{
			const auto trimmedBase = TrimTrailingPathSeparators(basePath);
			std::string combinedPath;
			combinedPath.reserve(trimmedBase.size() + fileOrDirectory.size() + 1);
			combinedPath += trimmedBase;
			combinedPath += DirectorySeparator;
			combinedPath += fileOrDirectory;
			return combinedPath;
		}

		std::string Normalize(std::string_view filePath)
		{
			auto normalized = std::string(filePath);
			std::replace(normalized.begin(), normalized.end(), DirectorySeparatorAlt, DirectorySeparator);
			return normalized;
		}

		std::string NormalizeWin32(std::string_view filePath)
		{
			auto normalized = std::string(filePath);
			std::replace(normalized.begin(), normalized.end(), DirectorySeparator, DirectorySeparatorAlt);
			return normalized;
		}
	}
}
