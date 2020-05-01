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
		using IteratorFlags = u32;
		enum IteratorFlags_Enum : IteratorFlags
		{
			IteratorFlags_Files,
			IteratorFlags_Directories,
			IteratorFlags_Recursive,
		};

		COMFY_NODISCARD bool Exists(std::string_view directoryPath);

		void Create(std::string_view directoryPath);

		COMFY_NODISCARD std::string GetWorkingDirectory();
		void SetWorkingDirectory(std::string_view directoryPath);

		template <typename Func>
		void Iterate(std::string_view directoryPath, Func func, IteratorFlags flags = IteratorFlags_Files)
		{
			auto iterateGeneric = [&](const auto directoryIterator)
			{
				for (const auto& it : directoryIterator)
				{
					if (((flags & IteratorFlags_Files) && it.is_regular_file()) || (flags & IteratorFlags_Directories) && it.is_directory())
						func(it.path().u8string());
				}
			};

			if (flags & IteratorFlags_Recursive)
				iterateGeneric(std::filesystem::recursive_directory_iterator(UTF8::WideArg(directoryPath).c_str()));
			else
				iterateGeneric(std::filesystem::directory_iterator(UTF8::WideArg(directoryPath).c_str()));
		}
	}
}
