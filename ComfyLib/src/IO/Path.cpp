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

// #define COMFY_STATIC_ASSERT_STRCMP(stringA, stringB) static_assert((stringA) == (stringB))
#define COMFY_STATIC_ASSERT_STRCMP(stringA, stringB) static_assert(ConstexprStrCmp((stringA), (stringB)))

namespace Comfy::IO::Path::ConstexprTest
{
	constexpr bool ConstexprStrCmp(std::string_view stringA, std::string_view stringB)
	{
		if (stringA.size() != stringB.size())
			return false;

		for (size_t i = 0; i < stringA.size(); i++)
			if (stringA[i] != stringB[i])
				return false;

		return true;
	}

	COMFY_STATIC_ASSERT_STRCMP(GetExtension("C:/MyDir/MySubDir/MyFile"), "");
	COMFY_STATIC_ASSERT_STRCMP(GetExtension("C:/MyDir/MySubDir"), "");
	COMFY_STATIC_ASSERT_STRCMP(GetExtension("C:/MyDir/MyFile.ext"), ".ext");
	COMFY_STATIC_ASSERT_STRCMP(GetExtension("C:/MyDir/MyFile."), ".");
	COMFY_STATIC_ASSERT_STRCMP(GetExtension("C:/MyFile.ext"), ".ext");
	COMFY_STATIC_ASSERT_STRCMP(GetExtension("MyFile.ext"), ".ext");
	COMFY_STATIC_ASSERT_STRCMP(GetExtension("MyFile"), "");
	COMFY_STATIC_ASSERT_STRCMP(GetExtension("C:/.ext"), ".ext");
	COMFY_STATIC_ASSERT_STRCMP(GetExtension("C:/"), "");
	COMFY_STATIC_ASSERT_STRCMP(GetExtension(""), "");

	COMFY_STATIC_ASSERT_STRCMP(GetFileName("C:/MyDir/MySubDir/MyFile.ext"), "MyFile.ext");

	COMFY_STATIC_ASSERT_STRCMP(TrimTrailingPathSeparators("C:/MyDir/MySubDir/"), "C:/MyDir/MySubDir");
	COMFY_STATIC_ASSERT_STRCMP(TrimTrailingPathSeparators("C:/MyDir//"), "C:/MyDir");
	COMFY_STATIC_ASSERT_STRCMP(TrimTrailingPathSeparators("C:/MyDir"), "C:/MyDir");
	COMFY_STATIC_ASSERT_STRCMP(TrimTrailingPathSeparators(""), "");

	COMFY_STATIC_ASSERT_STRCMP(GetDirectoryName("C:/MyDir/MySubDir/MyFile.ext"), "C:/MyDir/MySubDir");
	COMFY_STATIC_ASSERT_STRCMP(GetDirectoryName("C:/MyDir/MySubDir"), "C:/MyDir");
	COMFY_STATIC_ASSERT_STRCMP(GetDirectoryName("C:/MyDir/"), "C:/MyDir/");
	COMFY_STATIC_ASSERT_STRCMP(GetDirectoryName("C:/MyDir"), "C:");
	COMFY_STATIC_ASSERT_STRCMP(GetDirectoryName("C:/"), "C:/");
}
