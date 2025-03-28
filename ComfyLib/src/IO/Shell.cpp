#include "Shell.h"
#include "Directory.h"
#include "Path.h"
#include "Misc/StringUtil.h"
#include "Misc/UTF8.h"
#include "Time/Stopwatch.h"
#include <shlwapi.h>
#include <shobjidl.h>
#include <wrl.h>

#include "Core/Win32LeanWindowsHeader.h"

using Microsoft::WRL::ComPtr;

namespace Comfy::IO
{
	namespace Shell
	{
		namespace
		{
			// HACK: Run inside a separate thread with a timeout so that worst case the program at least won't freeze indefinitely.
			//		 Only to be used for "heavy" operations for which the overhead of creating a thread isn't a big deal
			template <typename Func>
			void CreateRunAndWaitOnComThreadWithTimeout(Func threadFunc, TimeSpan timeoutDuration = TimeSpan::FromSeconds(6.0))
			{
				std::atomic<bool> threadIsFinished = false;
				auto comThread = std::thread([&]()
				{
					Win32ThreadLocalCoInitializeOnce();
					threadFunc();
					Win32ThreadLocalCoUnInitializeIfLast();
					threadIsFinished = true;
				});

				auto timeoutStopwatch = Stopwatch::StartNew();
				while (true)
				{
					if (threadIsFinished && comThread.joinable())
					{
						comThread.join();
						break;
					}
					else if (timeoutStopwatch.GetElapsed() >= timeoutDuration)
					{
						// NOTE: In this case just let thread leak as *anything* is probably still much better 
						//		 than locking up the entire program indefinitely and potentially lossing unsaved progress
						assert(false);
						if (comThread.joinable())
							comThread.detach();
						break;
					}
				}
			}

			// HACK: Fuck it, I don't wanna deal with this any more. All the COM stuff is causing too many multi threading issues... 
			//		 Very hacky but showing / opening files this way should at least work reliably regardless of the current program state
			bool LaunchDetachedExplorerExeProcessWithPathArg(std::string_view singlePathArg)
			{
				const auto explorerExePath = IO::Path::TrySearchSystemSearchPath("explorer", ".exe");
				const auto explorerExePathW = UTF8::WideArg(explorerExePath);
				if (explorerExePath.empty())
					return false;

				std::wstring commandLineW;
				commandLineW += L'"';
				commandLineW += explorerExePathW.c_str();
				commandLineW += L'"';
				if (!singlePathArg.empty())
				{
					commandLineW += L' ';
					commandLineW += L'"';
					commandLineW += UTF8::WideArg(IO::Path::NormalizeWin32(singlePathArg)).c_str();
					commandLineW += L'"';
				}

				STARTUPINFOW inStartupInfo = {};
				inStartupInfo.cb = sizeof(inStartupInfo);

				PROCESS_INFORMATION outProcessInfo = {};
				const BOOL result = ::CreateProcessW(explorerExePathW.c_str(), commandLineW.data(), nullptr, nullptr, false, DETACHED_PROCESS, nullptr, nullptr, &inStartupInfo, &outProcessInfo);
				if (!result)
					return false;

				::CloseHandle(outProcessInfo.hProcess);
				::CloseHandle(outProcessInfo.hThread);
				return true;
			}
		}

		bool IsFileLink(std::string filePath)
		{
			const auto extension = Path::GetExtension(filePath);
			return Util::MatchesInsensitive(extension, FileLinkExtension);
		}

		std::string ResolveFileLink(std::string_view lnkFilePath)
		{
			Win32ThreadLocalCoInitializeOnce();

			ComPtr<IShellLinkW> shellLink = nullptr;
			if (FAILED(::CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, __uuidof(shellLink), &shellLink)))
				return "";

			ComPtr<IPersistFile> persistFile = nullptr;
			if (FAILED(shellLink->QueryInterface(__uuidof(persistFile), &persistFile)))
				return "";

			if (FAILED(persistFile->Load(UTF8::WideArg(lnkFilePath).c_str(), STGM_READ)))
				return "";

			if (FAILED(shellLink->Resolve(NULL, 0)))
				return "";

			WCHAR pathBuffer[MAX_PATH] = {};
			WIN32_FIND_DATAW findData = {};
			if (SUCCEEDED(shellLink->GetPath(pathBuffer, MAX_PATH, &findData, SLGP_SHORTPATH)))
				return UTF8::Narrow(pathBuffer);

			return "";
		}

		void OpenInExplorer(std::string_view filePath)
		{
#if 0
			// HACK: Just to be sure...
			CreateRunAndWaitOnComThreadWithTimeout([path = std::string(filePath)]()
			{
				if (Path::IsRelative(path))
				{
					const auto absolutePath = Path::Combine(Directory::GetWorkingDirectory(), path);
					::ShellExecuteW(NULL, L"open", UTF8::WideArg(absolutePath).c_str(), NULL, NULL, SW_SHOWDEFAULT);
				}
				else
				{
					::ShellExecuteW(NULL, L"open", UTF8::WideArg(path).c_str(), NULL, NULL, SW_SHOWDEFAULT);
				}
			});
#else 
			LaunchDetachedExplorerExeProcessWithPathArg(IO::Path::ResolveRelative(filePath));
#endif
		}

		void OpenExplorerProperties(std::string_view filePath)
		{
			// HACK: Just to be sure...
			CreateRunAndWaitOnComThreadWithTimeout([path = std::string(filePath)]()
			{
				const auto filePathArg = UTF8::WideArg(path);

				SHELLEXECUTEINFOW info = {};
				info.cbSize = sizeof(info);
				info.lpFile = filePathArg.c_str();
				info.nShow = SW_SHOW;
				info.fMask = SEE_MASK_INVOKEIDLIST;
				info.lpVerb = L"properties";
				::ShellExecuteExW(&info);
			});
		}

		void OpenWithDefaultProgram(std::string_view filePath)
		{
#if 0
			// BUG: Can't reliably reproduce it but under certain conditions it seems if this is called after the MoviePlayer has been initialized
			//		then these can completely lock up the program. Probably related to multi threading / COM threading apartment...
			CreateRunAndWaitOnComThreadWithTimeout([path = std::string(filePath)]()
			{
				if (Path::IsRelative(path))
				{
					const auto absolutePath = Path::Combine(Directory::GetWorkingDirectory(), path);
					::ShellExecuteW(NULL, L"open", UTF8::WideArg(absolutePath).c_str(), NULL, NULL, SW_SHOW);
				}
				else
				{
					::ShellExecuteW(NULL, L"open", UTF8::WideArg(path).c_str(), NULL, NULL, SW_SHOW);
				}
			});
#else
			LaunchDetachedExplorerExeProcessWithPathArg(IO::Path::ResolveRelative(filePath));
#endif
		}

		MessageBoxResult ShowMessageBox(std::string_view message, std::string_view title, MessageBoxButtons buttons, MessageBoxIcon icon, void* parentWindowHandle)
		{
			UINT flags = 0;

			switch (buttons)
			{
			case MessageBoxButtons::AbortRetryIgnore: flags |= MB_ABORTRETRYIGNORE; break;
			case MessageBoxButtons::CancelTryContinue: flags |= MB_CANCELTRYCONTINUE; break;
			case MessageBoxButtons::OK: flags |= MB_OK; break;
			case MessageBoxButtons::OKCancel: flags |= MB_OKCANCEL; break;
			case MessageBoxButtons::RetryCancel: flags |= MB_RETRYCANCEL; break;
			case MessageBoxButtons::YesNo: flags |= MB_YESNO; break;
			case MessageBoxButtons::YesNoCancel: flags |= MB_YESNOCANCEL; break;
			default: break;
			}

			switch (icon)
			{
			case MessageBoxIcon::Asterisk: flags |= MB_ICONASTERISK; break;
			case MessageBoxIcon::Error: flags |= MB_ICONERROR; break;
			case MessageBoxIcon::Exclamation: flags |= MB_ICONEXCLAMATION; break;
			case MessageBoxIcon::Hand: flags |= MB_ICONHAND; break;
			case MessageBoxIcon::Information: flags |= MB_ICONINFORMATION; break;
			case MessageBoxIcon::None: break;
			case MessageBoxIcon::Question: flags |= MB_ICONQUESTION; break;
			case MessageBoxIcon::Stop: flags |= MB_ICONSTOP; break;
			case MessageBoxIcon::Warning: flags |= MB_ICONWARNING; break;
			default: break;
			}

			const WORD languageID = 0;
			const int result = ::MessageBoxExW(reinterpret_cast<HWND>(parentWindowHandle), UTF8::WideArg(message).c_str(), title.empty() ? nullptr : UTF8::WideArg(title).c_str(), flags, languageID);

			switch (result)
			{
			case IDABORT: return MessageBoxResult::Abort;
			case IDCANCEL: return MessageBoxResult::Cancel;
			case IDCONTINUE: return MessageBoxResult::Continue;
			case IDIGNORE: return MessageBoxResult::Ignore;
			case IDNO: return MessageBoxResult::No;
			case IDOK: return MessageBoxResult::OK;
			case IDRETRY: return MessageBoxResult::Retry;
			case IDTRYAGAIN: return MessageBoxResult::TryAgain;
			case IDYES: return MessageBoxResult::Yes;
			default: return MessageBoxResult::None;
			}
		}

		namespace
		{
			class DialogEventHandler : public IFileDialogEvents, public IFileDialogControlEvents
			{
			public:
				DialogEventHandler() : refCount(1) {}
				~DialogEventHandler() = default;

			public:
#pragma region IUnknown
				IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv)
				{
					static const QITAB searchTable[] =
					{
						QITABENT(DialogEventHandler, IFileDialogEvents),
						QITABENT(DialogEventHandler, IFileDialogControlEvents),
						{ nullptr, 0 },
					};
					return QISearch(this, searchTable, riid, ppv);
				}

				IFACEMETHODIMP_(ULONG) AddRef()
				{
					return InterlockedIncrement(&refCount);
				}

				IFACEMETHODIMP_(ULONG) Release()
				{
					long cRef = InterlockedDecrement(&refCount);
					if (!cRef)
						delete this;
					return cRef;
				}
#pragma endregion IUnknown

#pragma region IFileDialogEvents
				IFACEMETHODIMP OnFileOk(IFileDialog*) { return S_OK; }
				IFACEMETHODIMP OnFolderChange(IFileDialog*) { return S_OK; }
				IFACEMETHODIMP OnFolderChanging(IFileDialog*, IShellItem*) { return S_OK; }
				IFACEMETHODIMP OnHelp(IFileDialog*) { return S_OK; }
				IFACEMETHODIMP OnSelectionChange(IFileDialog*) { return S_OK; }
				IFACEMETHODIMP OnShareViolation(IFileDialog*, IShellItem*, FDE_SHAREVIOLATION_RESPONSE*) { return S_OK; }
				IFACEMETHODIMP OnTypeChange(IFileDialog* pfd) { return S_OK; }
				IFACEMETHODIMP OnOverwrite(IFileDialog*, IShellItem*, FDE_OVERWRITE_RESPONSE*) { return S_OK; }

				// This method gets called when an dialog control item selection happens (radio-button selection. etc).
				// For sample sake, let's react to this event by changing the dialog title.
				IFACEMETHODIMP OnItemSelected(IFileDialogCustomize* pfdc, DWORD dwIDCtl, DWORD dwIDItem)
				{
					HRESULT result = S_OK;
					IFileDialog* fileDialog = nullptr;

					if (result = pfdc->QueryInterface(&fileDialog); !SUCCEEDED(result))
						return result;

#if 0 // NOTE: Add future item check logic here if needed
					switch (dwIDItem)
					{
					default:
						break;
					}
#endif

					fileDialog->Release();
					return result;
				}

				IFACEMETHODIMP OnButtonClicked(IFileDialogCustomize*, DWORD) { return S_OK; }
				IFACEMETHODIMP OnCheckButtonToggled(IFileDialogCustomize*, DWORD, BOOL) { return S_OK; }
				IFACEMETHODIMP OnControlActivating(IFileDialogCustomize*, DWORD) { return S_OK; }
#pragma endregion IFileDialogEvents

			private:
				long refCount;

			public:
				static HRESULT CreateInstance(REFIID riid, void** ppv)
				{
					*ppv = nullptr;
					if (DialogEventHandler* pDialogEventHandler = new DialogEventHandler(); pDialogEventHandler != nullptr)
					{
						HRESULT result = pDialogEventHandler->QueryInterface(riid, ppv);
						pDialogEventHandler->Release();
						return S_OK;
					}

					return E_OUTOFMEMORY;
				}
			};

			void PlaceCustomDialogItems(std::vector<Custom::Item>& customizeItems, IFileDialogCustomize& dialogCustomize)
			{
				HRESULT result = S_OK;

				for (size_t i = 0; i < customizeItems.size(); i++)
				{
					const auto& item = customizeItems[i];
					const auto itemID = static_cast<DWORD>(Custom::ItemBaseID + i);

					if (item.Type == Custom::ItemType::VisualGroupStart)
						result = dialogCustomize.StartVisualGroup(itemID, UTF8::WideArg(item.Label).c_str());
					else if (item.Type == Custom::ItemType::VisualGroupEnd)
						result = dialogCustomize.EndVisualGroup();
					else if (item.Type == Custom::ItemType::Checkbox)
					{
						result = dialogCustomize.AddCheckButton(itemID, UTF8::WideArg(item.Label).c_str(), (item.Data.CheckboxChecked != nullptr) ? *item.Data.CheckboxChecked : false);

						if (item.Data.CheckboxChecked == nullptr)
							result = dialogCustomize.SetControlState(itemID, CDCS_VISIBLE);
					}
				}
			}

			void ReadCustomDialogItems(std::vector<Custom::Item>& customizeItems, IFileDialogCustomize& dialogCustomize)
			{
				HRESULT result = S_OK;

				for (size_t i = 0; i < customizeItems.size(); i++)
				{
					auto& item = customizeItems[i];
					const auto itemID = static_cast<DWORD>(Custom::ItemBaseID + i);

					if (item.Type == Custom::ItemType::Checkbox)
					{
						BOOL checked;
						result = dialogCustomize.GetCheckButtonState(itemID, &checked);
						if (item.Data.CheckboxChecked != nullptr)
							*item.Data.CheckboxChecked = checked;
					}
				}
			}
		}

		bool FileDialog::OpenRead()
		{
			return InternalCreateAndShowDialog(false, false);
		}

		bool FileDialog::OpenSave()
		{
			return InternalCreateAndShowDialog(true, false);
		}

		bool FileDialog::OpenSelectFolder()
		{
			return InternalCreateAndShowDialog(false, true);
		}

		// NOTE: Can't use CreateRunAndWaitOnComThreadWithTimeout() here because the parent window still needs to respond to window messages (?) 
		bool FileDialog::InternalCreateAndShowDialog(bool save, bool selectFolder)
		{
			for (auto& filter : Filters)
			{
				// NOTE: Shouldn't have a file spec suffix
				assert(!filter.Name.empty() && filter.Name.back() != ')');
				// NOTE: Don't forget the "*." prefix
				assert(!filter.Spec.empty() && filter.Spec[0] == '*' && filter.Spec[1] == '.' && filter.Spec.back() != ';');
			}

			HRESULT hr = S_OK;
			ComPtr<IFileDialog> fileDialog = nullptr;

			hr = Win32ThreadLocalCoInitializeOnce();
			if (hr = ::CoCreateInstance(save ? CLSID_FileSaveDialog : CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, __uuidof(fileDialog), &fileDialog); !SUCCEEDED(hr))
				return false;

			DWORD eventCookie = 0;
			ComPtr<IFileDialogEvents> dialogEvents = nullptr;
			ComPtr<IFileDialogCustomize> dialogCustomize = nullptr;

			if (hr = DialogEventHandler::CreateInstance(__uuidof(dialogEvents), &dialogEvents); SUCCEEDED(hr))
			{
				if (hr = fileDialog->Advise(dialogEvents.Get(), &eventCookie); SUCCEEDED(hr))
				{
					if (hr = fileDialog->QueryInterface(__uuidof(dialogCustomize), &dialogCustomize); SUCCEEDED(hr))
						PlaceCustomDialogItems(CustomizeItems, *dialogCustomize.Get());

					DWORD existingOptionsFlags = 0;
					hr = fileDialog->GetOptions(&existingOptionsFlags);
					hr = fileDialog->SetOptions(existingOptionsFlags | FOS_FORCEFILESYSTEM | (selectFolder ? FOS_PICKFOLDERS : 0));

					if (!Title.empty())
						hr = fileDialog->SetTitle(UTF8::WideArg(Title).c_str());

					if (!FileName.empty())
						hr = fileDialog->SetFileName(UTF8::WideArg(FileName).c_str());

					if (!DefaultExtension.empty())
						hr = fileDialog->SetDefaultExtension(UTF8::WideArg(Util::StripPrefix(DefaultExtension, ".")).c_str());

					if (!selectFolder && !Filters.empty())
					{
						// NOTE: Important to reserve so the c_str()s won't get invalidated
						std::vector<std::wstring> owningWideStrings;
						owningWideStrings.reserve(Filters.size() * 2);

						std::vector<COMDLG_FILTERSPEC> convertedFilters;
						convertedFilters.reserve(Filters.size());

						std::string nameWithSpecSuffix;
						for (const auto& inputFilter : Filters)
						{
							nameWithSpecSuffix.clear();
							nameWithSpecSuffix += inputFilter.Name;
							nameWithSpecSuffix += " (";
							nameWithSpecSuffix += inputFilter.Spec;
							nameWithSpecSuffix += ')';

							convertedFilters.push_back(COMDLG_FILTERSPEC
								{
									owningWideStrings.emplace_back(std::move(UTF8::Widen(nameWithSpecSuffix))).c_str(),
									owningWideStrings.emplace_back(std::move(UTF8::Widen(inputFilter.Spec))).c_str(),
								});
						}

						hr = fileDialog->SetFileTypes(static_cast<UINT>(convertedFilters.size()), convertedFilters.data());
						hr = fileDialog->SetFileTypeIndex(FilterIndex);
					}
				}
			}

			if (SUCCEEDED(hr))
			{
				// BUG: It's possible for this to never return if one too many (even a single?) CoInitialize*Ex*() has been called (?) ... probably related to the threading model (?)
				if (hr = fileDialog->Show(reinterpret_cast<HWND>(ParentWindowHandle)); SUCCEEDED(hr))
				{
					ComPtr<IShellItem> itemResult = nullptr;
					if (hr = fileDialog->GetResult(&itemResult); SUCCEEDED(hr))
					{
						wchar_t* filePath = nullptr;
						if (hr = itemResult->GetDisplayName(SIGDN_FILESYSPATH, &filePath); SUCCEEDED(hr))
						{
							hr = fileDialog->GetFileTypeIndex(&FilterIndex);
							OutFilePath = UTF8::Narrow(filePath);

							if (dialogCustomize != nullptr)
								ReadCustomDialogItems(CustomizeItems, *dialogCustomize.Get());

							::CoTaskMemFree(filePath);
						}
					}
				}
			}

			if (fileDialog != nullptr)
				fileDialog->Unadvise(eventCookie);

			return SUCCEEDED(hr) && !OutFilePath.empty();
		}
	}
}
