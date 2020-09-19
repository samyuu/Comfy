#include "Shell.h"
#include "Directory.h"
#include "Path.h"
#include "Misc/StringUtil.h"
#include "Misc/UTF8.h"
#include <shlwapi.h>
#include <shobjidl.h>

namespace Comfy::IO
{
	namespace Shell
	{
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
			return CreateOpenShowDialog(false);
		}

		bool FileDialog::OpenSave()
		{
			return CreateOpenShowDialog(true);
		}

		bool FileDialog::CreateOpenShowDialog(bool openSave)
		{
			HRESULT result = S_OK;
			IFileDialog* fileDialog = nullptr;

			if (result = ::CoCreateInstance(openSave ? CLSID_FileSaveDialog : CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&fileDialog)); !SUCCEEDED(result))
				return false;

			DWORD eventCookie = 0;
			IFileDialogEvents* dialogEvents = nullptr;
			IFileDialogCustomize* dialogCustomize = nullptr;

			if (result = DialogEventHandler::CreateInstance(IID_PPV_ARGS(&dialogEvents)); SUCCEEDED(result))
			{
				if (result = fileDialog->Advise(dialogEvents, &eventCookie); SUCCEEDED(result))
				{
					if (result = fileDialog->QueryInterface(IID_PPV_ARGS(&dialogCustomize)); SUCCEEDED(result))
						PlaceCustomDialogItems(CustomizeItems, *dialogCustomize);

					DWORD existingOptionsFlags = 0;
					result = fileDialog->GetOptions(&existingOptionsFlags);
					result = fileDialog->SetOptions(existingOptionsFlags | FOS_FORCEFILESYSTEM);

					if (!Title.empty())
						result = fileDialog->SetTitle(UTF8::WideArg(Title).c_str());

					if (!FileName.empty())
						result = fileDialog->SetFileName(UTF8::WideArg(FileName).c_str());

					if (!DefaultExtension.empty())
						result = fileDialog->SetDefaultExtension(UTF8::WideArg(Util::StripPrefix(DefaultExtension, ".")).c_str());

					if (!Filters.empty())
					{
						std::vector<std::wstring> owningWideStrings;
						owningWideStrings.reserve(Filters.size() * 2);

						std::vector<COMDLG_FILTERSPEC> convertedFilters;
						convertedFilters.reserve(Filters.size());

						for (const auto& inputFilter : Filters)
						{
							convertedFilters.push_back(COMDLG_FILTERSPEC
								{
									owningWideStrings.emplace_back(std::move(UTF8::Widen(inputFilter.Name))).c_str(),
									owningWideStrings.emplace_back(std::move(UTF8::Widen(inputFilter.Spec))).c_str(),
								});
						}

						result = fileDialog->SetFileTypes(static_cast<UINT>(convertedFilters.size()), convertedFilters.data());
						result = fileDialog->SetFileTypeIndex(FilterIndex);
					}
				}

				dialogEvents->Release();
			}

			if (SUCCEEDED(result))
			{
				if (result = fileDialog->Show(reinterpret_cast<HWND>(ParentWindowHandle)); SUCCEEDED(result))
				{
					IShellItem* itemResult = nullptr;
					if (result = fileDialog->GetResult(&itemResult); SUCCEEDED(result))
					{
						wchar_t* filePath = nullptr;
						if (result = itemResult->GetDisplayName(SIGDN_FILESYSPATH, &filePath); SUCCEEDED(result))
						{
							result = fileDialog->GetFileTypeIndex(&FilterIndex);
							OutFilePath = UTF8::Narrow(filePath);

							if (dialogCustomize != nullptr)
								ReadCustomDialogItems(CustomizeItems, *dialogCustomize);

							::CoTaskMemFree(filePath);
						}

						itemResult->Release();
					}
				}
			}

			if (dialogCustomize != nullptr)
				dialogCustomize->Release();

			if (fileDialog != nullptr)
			{
				fileDialog->Unadvise(eventCookie);
				fileDialog->Release();
			}

			return SUCCEEDED(result) && !OutFilePath.empty();
		}
	}
}
