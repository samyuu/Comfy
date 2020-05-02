#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "CoreMacros.h"
#include "Misc/UTF8.h"
#include <filesystem>

namespace Comfy::IO
{
	namespace IteratorFlags
	{
		using Type = u32;
		enum Enum : Type
		{
			None = 0,
			Recursive = 1 << 0,

			Files = 1 << 1,
			Directories = 1 << 2,

			RecursiveFiles = Recursive | Files,
			RecursiveDirectories = Recursive | Directories,
		};
	}

	namespace Directory
	{
		COMFY_NODISCARD bool Exists(std::string_view directoryPath);

		void Create(std::string_view directoryPath);

		COMFY_NODISCARD std::string GetWorkingDirectory();
		void SetWorkingDirectory(std::string_view directoryPath);

		template <IteratorFlags::Type Flags = IteratorFlags::Files, typename Func>
		void Iterate(std::string_view directoryPath, Func func)
		{
			auto iterateGeneric = [&](const auto directoryIterator)
			{
				for (const auto& it : directoryIterator)
				{
					bool validPath = false;

					if constexpr (Flags & IteratorFlags::Files)
						validPath |= it.is_regular_file();
					if constexpr (Flags & IteratorFlags::Directories)
						validPath |= it.is_directory();

					if (validPath)
						func(it.path().u8string());
				}
			};

			if constexpr (Flags & IteratorFlags::Recursive)
				iterateGeneric(std::filesystem::recursive_directory_iterator(UTF8::WideArg(directoryPath).c_str()));
			else
				iterateGeneric(std::filesystem::directory_iterator(UTF8::WideArg(directoryPath).c_str()));
		}
	}
}
