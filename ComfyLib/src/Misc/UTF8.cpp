#include "UTF8.h"
#include "Core/Win32/ComfyWindows.h"

namespace Comfy::UTF8
{
	std::string Narrow(std::wstring_view inputString)
	{
		std::string utf8String;
		const int utf8Length = ::WideCharToMultiByte(CP_UTF8, 0, inputString.data(), static_cast<int>(inputString.size() + 1), nullptr, 0, nullptr, nullptr) - 1;

		if (utf8Length > 0)
		{
			utf8String.resize(utf8Length);
			::WideCharToMultiByte(CP_UTF8, 0, inputString.data(), static_cast<int>(inputString.size()), utf8String.data(), utf8Length, nullptr, nullptr);
		}

		return utf8String;
	}

	std::wstring Widen(std::string_view inputString)
	{
		std::wstring utf16String;
		const int utf16Length = ::MultiByteToWideChar(CP_UTF8, 0, inputString.data(), static_cast<int>(inputString.size() + 1), nullptr, 0) - 1;

		if (utf16Length > 0)
		{
			utf16String.resize(utf16Length);
			::MultiByteToWideChar(CP_UTF8, 0, inputString.data(), static_cast<int>(inputString.size()), utf16String.data(), utf16Length);
		}

		return utf16String;
	}

	WideArg::WideArg(std::string_view inputString)
	{
		// NOTE: Length **without** null terminator
		convertedLength = ::MultiByteToWideChar(CP_UTF8, 0, inputString.data(), static_cast<int>(inputString.size() + 1), nullptr, 0) - 1;

		if (convertedLength <= 0)
		{
			stackBuffer[0] = L'\0';
			return;
		}

		if (convertedLength < stackBuffer.size())
		{
			::MultiByteToWideChar(CP_UTF8, 0, inputString.data(), static_cast<int>(inputString.size()), stackBuffer.data(), convertedLength);
			stackBuffer[convertedLength] = L'\0';
		}
		else
		{
			heapBuffer = MakeUnique<wchar_t[]>(convertedLength + 1);
			::MultiByteToWideChar(CP_UTF8, 0, inputString.data(), static_cast<int>(inputString.size()), heapBuffer.get(), convertedLength);
			heapBuffer[convertedLength] = L'\0';
		}
	}

	const wchar_t* WideArg::c_str() const
	{
		return (convertedLength < stackBuffer.size()) ? stackBuffer.data() : heapBuffer.get();
	}
}
