#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "CoreMacros.h"
#include "Misc/UTF8.h"
#include <filesystem>

namespace Comfy::IO
{
	namespace Directory
	{
		namespace Detail
		{
			template <bool IterateFiles, bool IterateDirectories, bool IterateRecursive, typename Func>
			void Iterate(std::string_view directoryPath, Func func)
			{
				auto iterateGeneric = [&](const auto directoryIterator)
				{
					for (const auto& it : directoryIterator)
					{
						bool validPath = false;

						if constexpr (IterateFiles)
							validPath |= it.is_regular_file();
						if constexpr (IterateDirectories)
							validPath |= it.is_directory();

						if (validPath)
							func(it.path().u8string());
					}
				};

				if constexpr (IterateRecursive)
					iterateGeneric(std::filesystem::recursive_directory_iterator(UTF8::WideArg(directoryPath).c_str()));
				else
					iterateGeneric(std::filesystem::directory_iterator(UTF8::WideArg(directoryPath).c_str()));
			}
		}

		COMFY_NODISCARD bool Exists(std::string_view directoryPath);

		void Create(std::string_view directoryPath);

		COMFY_NODISCARD std::string GetWorkingDirectory();
		void SetWorkingDirectory(std::string_view directoryPath);

		template <typename Func>
		void IterateFiles(std::string_view directoryPath, Func func) { Detail::Iterate<true, false, false>(directoryPath, func); }

		template <typename Func>
		void IterateDirectories(std::string_view directoryPath, Func func) { Detail::Iterate<false, true, false>(directoryPath, func); }

		template <typename Func>
		void IterateFilesRecursive(std::string_view directoryPath, Func func) { Detail::Iterate<true, false, true>(directoryPath, func); }

		template <typename Func>
		void IterateDirectoriesRecursive(std::string_view directoryPath, Func func) { Detail::Iterate<false, true, true>(directoryPath, func); }
	}
}
