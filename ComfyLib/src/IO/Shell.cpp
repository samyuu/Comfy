#include "Shell.h"
#include "Directory.h"
#include "Path.h"
#include "Misc/UTF8.h"
#include <shlwapi.h>
#include <shobjidl.h>

namespace Comfy::IO
{
	namespace Shell
	{
		namespace
		{
			std::wstring FileDialogFilterFilterVectorToString(const std::vector<std::string>& filterVector)
			{
				assert(filterVector.size() % 2 == 0);

				std::wstring filterString;
				for (size_t i = 0; i + 1 < filterVector.size(); i++)
				{
					filterString += UTF8::Widen(filterVector[i]);
					filterString += L'\0';
				}
				filterString += L'\0';

				return filterString;
			}

			bool FileDialogBase(std::string& outFilePath, const char* title, const char* directory, const std::vector<std::string>& filter, bool openFileDialog)
			{
				assert(!filter.empty());

				auto const previousWorkingDirectory = Directory::GetWorkingDirectory();
				wchar_t filePathBuffer[MAX_PATH]; filePathBuffer[0] = '\0';

				const auto filterString = FileDialogFilterFilterVectorToString(filter);
				const auto directoryString = (directory != nullptr) ? UTF8::Widen(directory) : L"";
				const auto titleString = (title != nullptr) ? UTF8::Widen(title) : L"";

				OPENFILENAMEW openFileName = {};
				openFileName.lStructSize = sizeof(OPENFILENAMEW);
				openFileName.hwndOwner = NULL;
				openFileName.lpstrFilter = filterString.c_str();
				openFileName.lpstrFile = filePathBuffer;
				openFileName.nMaxFile = MAX_PATH;
				openFileName.lpstrInitialDir = (directory != nullptr) ? directoryString.c_str() : nullptr;
				openFileName.lpstrTitle = (title != nullptr) ? titleString.c_str() : nullptr;
				openFileName.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
				openFileName.lpstrDefExt = L"";

				const bool fileSelected = (openFileDialog) ? ::GetOpenFileNameW(&openFileName) : ::GetSaveFileNameW(&openFileName);
				if (fileSelected)
					outFilePath = UTF8::Narrow(filePathBuffer);

				Directory::SetWorkingDirectory(previousWorkingDirectory);
				return fileSelected;
			}
		}

		std::string ResolveFileLink(std::string_view lnkFilePath)
		{
#if 0
			::CoInitialize(nullptr);
			COMFY_SCOPE_EXIT([] { ::CoUninitialize(); });
#endif

			IShellLinkW* shellLink;
			if (FAILED(::CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLinkW, reinterpret_cast<LPVOID*>(&shellLink))))
				return "";
			COMFY_SCOPE_EXIT([&] { shellLink->Release(); });

			IPersistFile* persistFile;
			if (FAILED(shellLink->QueryInterface(IID_IPersistFile, reinterpret_cast<LPVOID*>(&persistFile))))
				return "";
			COMFY_SCOPE_EXIT([&] { persistFile->Release(); });

			if (FAILED(persistFile->Load(UTF8::WideArg(lnkFilePath).c_str(), STGM_READ)))
				return "";

			if (FAILED(shellLink->Resolve(NULL, 0)))
				return "";

			WCHAR pathBuffer[MAX_PATH];
			WIN32_FIND_DATAW findData;

			if (SUCCEEDED(shellLink->GetPath(pathBuffer, MAX_PATH, &findData, SLGP_SHORTPATH)))
				return UTF8::Narrow(pathBuffer);

			return "";
		}

		void OpenInExplorer(std::string_view filePath)
		{
			if (Path::IsRelative(filePath))
			{
				const auto absolutePath = Path::Combine(Directory::GetWorkingDirectory(), filePath);
				::ShellExecuteW(NULL, L"open", UTF8::WideArg(absolutePath).c_str(), NULL, NULL, SW_SHOWDEFAULT);
			}
			else
			{
				::ShellExecuteW(NULL, L"open", UTF8::WideArg(filePath).c_str(), NULL, NULL, SW_SHOWDEFAULT);
			}
		}

		void OpenExplorerProperties(std::string_view filePath)
		{
			const auto filePathArg = UTF8::WideArg(filePath);

			SHELLEXECUTEINFOW info = {};
			info.cbSize = sizeof(info);
			info.lpFile = filePathArg.c_str();
			info.nShow = SW_SHOW;
			info.fMask = SEE_MASK_INVOKEIDLIST;
			info.lpVerb = L"properties";
			::ShellExecuteExW(&info);
		}

		void OpenWithDefaultProgram(std::string_view filePath)
		{
			::ShellExecuteW(NULL, L"open", UTF8::WideArg(filePath).c_str(), NULL, NULL, SW_SHOW);
		}

		bool CreateOpenFileDialog(std::string& outFilePath, const char* title, const char* directory, const std::vector<std::string>& filter)
		{
			return FileDialogBase(outFilePath, title, directory, filter, true);
		}

		bool CreateSaveFileDialog(std::string& outFilePath, const char* title, const char* directory, const std::vector<std::string>& filter)
		{
			return FileDialogBase(outFilePath, title, directory, filter, false);
		}
	}
}
