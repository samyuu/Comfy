#include "Directory.h"
#include "Core/Win32/ComfyWindows.h"

namespace Comfy::IO
{
	namespace Directory
	{
		bool Exists(std::string_view directoryPath)
		{
			const auto attributes = ::GetFileAttributesW(UTF8::WideArg(directoryPath).c_str());
			return (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY));
		}

		void Create(std::string_view directoryPath)
		{
			::CreateDirectoryW(UTF8::WideArg(directoryPath).c_str(), 0);
		}

		std::string GetWorkingDirectory()
		{
			wchar_t buffer[MAX_PATH];
			::GetCurrentDirectoryW(MAX_PATH, buffer);
			
			return UTF8::Narrow(buffer);
		}

		void SetWorkingDirectory(std::string_view directoryPath)
		{
			::SetCurrentDirectoryW(UTF8::WideArg(directoryPath).c_str());
		}

		std::string GetExecutableDirectory()
		{
			wchar_t buffer[MAX_PATH];
			::GetModuleFileNameW(NULL, buffer, MAX_PATH);

			std::string result = UTF8::Narrow(buffer);
			result = Path::GetDirectoryName(result);
			return result;
		}
	}
}
