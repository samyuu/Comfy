#include "FileViewer.h"
#include "IO/Directory.h"
#include "IO/File.h"
#include "IO/Path.h"
#include "IO/Shell.h"
#include "Misc/StringUtil.h"
#include <FontIcons.h>
#include <filesystem>

using namespace Comfy;

namespace ImGui
{
	FileViewer::FileViewer(std::string_view directory)
	{
		SetDirectory(directory);
	}

	bool FileViewer::DrawGui()
	{
		const bool parentFocused = IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && IsWindowHovered(ImGuiFocusedFlags_RootAndChildWindows);
		const auto windowSize = vec2(GetWindowWidth(), 0.0f);

		PushID(this);
		PushStyleVar(ImGuiStyleVar_ButtonTextAlign, vec2(0.0f, 0.5f));

		BeginChild("FileDirectoryChild##FileViewer", vec2(windowSize.x, 24.0f));
		{
			PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(0.0f, 0.0f));
			{
				const bool isAsyncBusy = IsAsyncDirectoryInfoBusy();
				const bool backClicked = parentFocused && IsMouseReleased(3);

				if (isAsyncBusy)
					PushStyleColor(ImGuiCol_Text, GetStyleColorVec4(ImGuiCol_TextDisabled));

				if (ArrowButton("PreviousDirectoryButton::FileViewer", ImGuiDir_Up) || backClicked)
				{
					if (!isAsyncBusy)
						SetParentDirectory(currentDirectoryOrArchive);
				}

				SameLine();

				constexpr float searchBarWidth = 0.25f;

				PushItemWidth(windowSize.x * (1.0f - searchBarWidth));
				if (InputText("##DirectoryInputText::FileViewer", currentDirectoryBuffer, sizeof(currentDirectoryBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
				{
					if (!isAsyncBusy)
						SetDirectory(currentDirectoryBuffer);
				}
				PopItemWidth();

				if (isAsyncBusy)
					PopStyleColor();

				PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(1.0f, 0.0f));
				SameLine();
				fileFilter.Draw("##FileFilter::FileViewer", ICON_FA_SEARCH, windowSize.x * searchBarWidth);
				PopStyleVar();
			}
			PopStyleVar();
		}
		EndChild();
		Separator();

		bool fileClicked = false;

		// NOTE: Always draw vertical scrollbar to avoid constant size changes while navigating between directories with different file counts
		BeginChild("FileListChild##FileViewer", vec2(0.0f, 0.0f), false, ImGuiWindowFlags_AlwaysVerticalScrollbar);
		{
			CheckAsyncUpdateDirectoryInfo();

			if (auto clickedInfo = DrawFileListGui(); clickedInfo != nullptr)
			{
				if (clickedInfo->IsDirectory || IO::Archive::IsValidPath(clickedInfo->FullPath))
				{
					SetDirectory(IO::Path::Combine(currentDirectoryOrArchive, clickedInfo->ChildName));
				}
				else
				{
					if (clickedInfo->FileType == FileType::Link)
					{
						if (const auto resolvedPath = IO::Shell::ResolveFileLink(clickedInfo->FullPath); IO::Path::IsDirectory(resolvedPath))
						{
							SetDirectory(resolvedPath);
						}
						else
						{
							fileToOpen = resolvedPath;
							fileClicked = true;
						}
					}
					else
					{
						fileToOpen = clickedInfo->FullPath;
						fileClicked = true;
					}
				}
			}

			WindowContextMenu("ContextMenu##FileViewer", [this]()
			{
				const bool fileItemSelected = contextMenuFilePathInfo != nullptr;
				const bool isDirectory = fileItemSelected && contextMenuFilePathInfo->IsDirectory;

				if (MenuItem("Open", nullptr, nullptr, fileItemSelected && !isDirectory && !currentDirectoryIsArchive))
					OpenContextItemDefaultProgram();

				if (MenuItem("Open in Explorer..."))
					OpenDirectoryInExplorer();

				if (MenuItem("Refresh", nullptr, nullptr, !IsAsyncDirectoryInfoBusy()))
					SetDirectory(currentDirectoryOrArchive);

				if (MenuItem("Resize Columns"))
					resizeColumns = true;

				Separator();
				if (MenuItem("Properties", nullptr, nullptr, fileItemSelected && !currentDirectoryIsArchive))
					OpenContextItemProperties();
			});
		}
		EndChild();

		PopStyleVar(1);
		PopID();

		return fileClicked;
	}

	std::string_view FileViewer::GetDirectory() const
	{
		return currentDirectoryOrArchive;
	}

	void FileViewer::SetDirectory(std::string_view newDirectory)
	{
		previousDirectoryOrArchive = currentDirectoryOrArchive;
		currentDirectoryOrArchive = IO::Path::TrimTrailingPathSeparators(IO::Path::Normalize(IO::Path::TrimQuotes(newDirectory)));

		if (currentDirectoryOrArchive.size() > std::size(currentDirectoryBuffer) + 2)
			currentDirectoryOrArchive = currentDirectoryOrArchive.substr(0, std::size(currentDirectoryBuffer) - 2);

		currentDirectoryIsArchive = IO::Archive::IsValidPath(currentDirectoryOrArchive);

		std::memcpy(currentDirectoryBuffer, currentDirectoryOrArchive.data(), currentDirectoryOrArchive.size());
		if (!currentDirectoryIsArchive)
		{
			currentDirectoryBuffer[currentDirectoryOrArchive.size() + 0] = IO::Path::DirectorySeparator;
			currentDirectoryBuffer[currentDirectoryOrArchive.size() + 1] = '\0';
		}
		else
		{
			currentDirectoryBuffer[currentDirectoryOrArchive.size()] = '\0';
		}

		fileFilter.Clear();
		StartAsyncUpdateDirectoryInfo();
	}

	std::string_view FileViewer::GetFileToOpen() const
	{
		return fileToOpen;
	}

	bool FileViewer::GetIsReadOnly() const
	{
		return isReadOnly;
	}

	void FileViewer::SetIsReadOnly(bool value)
	{
		isReadOnly = value;
	}

	FileViewer::FilePathInfo* FileViewer::DrawFileListGui()
	{
		FilePathInfo* clickedInfo = nullptr;
		bool anyContextMenuClicked = false;

		Columns(3, "FileListColumns##FileViewer");

		if (lastWindowWidth != GetWindowWidth())
			resizeColumns = true;

		lastWindowWidth = GetWindowWidth();

		if (resizeColumns)
		{
			if (lastWindowWidth > (fileColumnWidth + typeColumnWidth + sizeColumnWidth))
			{
				SetColumnOffset(1, lastWindowWidth - typeColumnWidth - sizeColumnWidth);
				SetColumnOffset(2, lastWindowWidth - typeColumnWidth);
			}

			resizeColumns = false;
		}

		Text("Name"); NextColumn();
		Text("Size"); NextColumn();
		Text("Type"); NextColumn();
		Separator();
		if (currentDirectoryInfo.empty() && IsAsyncDirectoryInfoBusy())
		{
			asyncLoadingAnimation.Update();
			Selectable(asyncLoadingAnimation.GetText(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_Disabled);
		}
		else
		{
			if (currentDirectoryInfo.empty())
				Selectable("This folder is empty.", false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_Disabled);

			char displayNameBuffer[_MAX_PATH];
			for (auto& info : currentDirectoryInfo)
			{
				if (!fileFilter.PassFilter(info.ChildName.c_str()))
					continue;

				ImGuiSelectableFlags selectableFlags = ImGuiSelectableFlags_SpanAllColumns;
				if (isReadOnly && !info.IsDirectory && info.FileType != FileType::Link)
					selectableFlags |= ImGuiSelectableFlags_Disabled;

				sprintf_s(displayNameBuffer, GetFileInfoFormatString(info), info.ChildName.c_str());
				if (Selectable(displayNameBuffer, info.IsDirectory, selectableFlags))
					clickedInfo = &info;

				info.IsHovered = IsItemHovered();

				if (IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) && IsMouseClicked(1))
				{
					contextMenuFilePathInfo = &info;
					anyContextMenuClicked = true;
				}

				NextColumn();
				if (!info.IsDirectory)
					Text(info.ReadableFileSize.c_str());
				NextColumn();

				Text(info.IsDirectory ? "File Folder" : FormatFileType(info.FileType));
				NextColumn();
			}
		}
		Columns(1);

		if (!anyContextMenuClicked && IsMouseClicked(1))
			contextMenuFilePathInfo = nullptr;

		return clickedInfo;
	}

	void FileViewer::StartAsyncUpdateDirectoryInfo()
	{
		asyncLoadingAnimation.Reset();
		currentDirectoryInfo.clear();

		updateDirectoryInfoFuture = std::async(std::launch::async, [&, inputPath = currentDirectoryOrArchive]()
		{
			tempAsyncDirectoryInfo.clear();

			if (IO::Archive::IsValidPath(inputPath))
			{
				if (!IO::File::Exists(inputPath))
					return;

				const auto filePaths = IO::Archive::Detail::GetFileEntries(inputPath);
				for (const auto& fileEntry : filePaths)
				{
					const auto archivePath = IO::Archive::ParsePath(fileEntry.FullPath);

					auto& info = tempAsyncDirectoryInfo.emplace_back();
					info.FullPath = IO::Path::Normalize(fileEntry.FullPath);
					info.ChildName = archivePath.FileName;
					info.IsDirectory = false;
					info.FileSize = fileEntry.FileSize;
					info.FileType = useFileTypeIcons ? GetFileType(info.ChildName) : FileType::Default;
					FormatReadableFileSize(info.ReadableFileSize, info.FileSize);
				}
			}
			else
			{
				if (!IO::Directory::Exists(inputPath))
					return;

				for (const auto& file : std::filesystem::directory_iterator(UTF8::Widen(inputPath)))
				{
					if (!file.is_regular_file() && !file.is_directory())
						continue;

					auto& info = tempAsyncDirectoryInfo.emplace_back();
					const auto path = file.path();
					info.FullPath = IO::Path::Normalize(path.u8string());
					info.ChildName = path.filename().u8string();
					info.IsDirectory = file.is_directory();
					info.FileSize = file.file_size();
					FormatReadableFileSize(info.ReadableFileSize, info.FileSize);
					info.FileType = (!info.IsDirectory && useFileTypeIcons) ? GetFileType(info.ChildName) : FileType::Default;

					if (info.IsDirectory && appendDirectoryChildNameSlash)
						info.ChildName += '/';
				}
			}
		});
	}

	void FileViewer::CheckAsyncUpdateDirectoryInfo()
	{
		if (!updateDirectoryInfoFuture.valid() || !updateDirectoryInfoFuture._Is_ready())
			return;

		updateDirectoryInfoFuture.get();
		currentDirectoryInfo.reserve(tempAsyncDirectoryInfo.size());

		for (const auto& info : tempAsyncDirectoryInfo)
			if (info.IsDirectory)
				currentDirectoryInfo.emplace_back(std::move(info));

		for (const auto& info : tempAsyncDirectoryInfo)
			if (!info.IsDirectory)
				currentDirectoryInfo.emplace_back(std::move(info));
	}

	bool FileViewer::IsAsyncDirectoryInfoBusy() const
	{
		return (updateDirectoryInfoFuture.valid() && !updateDirectoryInfoFuture._Is_ready());
	}

	void FileViewer::SetParentDirectory(const std::string& directory)
	{
		if (Util::EndsWith(directory, IO::Path::DirectorySeparator) || Util::EndsWith(directory, IO::Path::DirectorySeparatorAlt))
			SetDirectory(IO::Path::GetDirectoryName(directory.substr(0, directory.length() - 1)));
		else
			SetDirectory(IO::Path::GetDirectoryName(directory));
	}

	void FileViewer::OpenDirectoryInExplorer()
	{
		if (currentDirectoryIsArchive)
			IO::Shell::OpenInExplorer(IO::Path::GetDirectoryName(currentDirectoryOrArchive));
		else
			IO::Shell::OpenInExplorer(currentDirectoryOrArchive);
	}

	void FileViewer::OpenContextItemDefaultProgram()
	{
		if (contextMenuFilePathInfo != nullptr)
			IO::Shell::OpenWithDefaultProgram(IO::Path::NormalizeWin32(contextMenuFilePathInfo->FullPath));
	}

	void FileViewer::OpenContextItemProperties()
	{
		std::string_view filePath = contextMenuFilePathInfo != nullptr ? contextMenuFilePathInfo->FullPath : currentDirectoryOrArchive;
		IO::Shell::OpenExplorerProperties(IO::Path::NormalizeWin32(filePath));
	}

	FileType FileViewer::GetFileType(const std::string_view fileName)
	{
		const auto inputExtension = IO::Path::GetExtension(fileName);
		if (!inputExtension.empty())
		{
			for (const auto&[fileType, packedExtensions] : fileTypeDictionary)
			{
				if (IO::Path::DoesAnyPackedExtensionMatch(inputExtension, packedExtensions))
					return fileType;
			}
		}
		return FileType::Default;
	}

	const char* FileViewer::GetFileInfoFormatString(const FilePathInfo& info)
	{
#define FILE_ICON_FORMAT_STRING(iconName) ("  " iconName "  %s")
#define CASE_FILE_ICON(caseValue, icon) case caseValue: return FILE_ICON_FORMAT_STRING(icon)

		if (info.IsDirectory)
			return info.IsHovered ? FILE_ICON_FORMAT_STRING(ICON_FA_FOLDER_OPEN) : FILE_ICON_FORMAT_STRING(ICON_FA_FOLDER);

		switch (info.FileType)
		{
			CASE_FILE_ICON(FileType::Default, ICON_FA_FILE);
			CASE_FILE_ICON(FileType::Link, ICON_FA_EXTERNAL_LINK_ALT);
			CASE_FILE_ICON(FileType::Text, ICON_FA_FILE_ALT);
			CASE_FILE_ICON(FileType::Config, ICON_FA_FILE_ALT);
			CASE_FILE_ICON(FileType::Binary, ICON_FA_FILE);
			CASE_FILE_ICON(FileType::Image, ICON_FA_FILE_IMAGE);
			CASE_FILE_ICON(FileType::Code, ICON_FA_FILE_CODE);
			CASE_FILE_ICON(FileType::Archive, ICON_FA_FILE_ARCHIVE);
			CASE_FILE_ICON(FileType::Video, ICON_FA_FILE_VIDEO);
			CASE_FILE_ICON(FileType::Audio, ICON_FA_FILE_AUDIO);
			CASE_FILE_ICON(FileType::Application, ICON_FA_FILE);

		default:
			return FILE_ICON_FORMAT_STRING(ICON_FA_FILE);
		}

#undef CASE_FILE_ICON
#undef FILE_ICON_FORMAT_STRING
	}

	const char* FileViewer::FormatFileType(FileType type)
	{
		switch (type)
		{
		case FileType::Default:
			return "File";
		case FileType::Text:
			return "Text";
		case FileType::Link:
			return "Link";
		case FileType::Config:
			return "Config";
		case FileType::Binary:
			return "BIN";
		case FileType::Image:
			return "Image";
		case FileType::Code:
			return "Source File";
		case FileType::Archive:
			return "Archive";
		case FileType::Video:
			return "Video";
		case FileType::Audio:
			return "Audio";
		case FileType::Application:
			return "Application";
		default:
			assert(false);
			return "Unknown";
		}
	}

	void FileViewer::FormatReadableFileSize(std::string& value, u64 fileSize)
	{
		if (fileSize <= 0)
			return;

		constexpr u64 unitFactor = 1024;
		constexpr const char* narrowUnitsString = "B  KB MB GB TB PB EB ZB YB";

		int unitIndex = 0;
		while (fileSize > unitFactor && unitIndex < 8)
		{
			fileSize /= unitFactor;
			unitIndex++;
		}

		const char* unitOffset = &narrowUnitsString[unitIndex * 3];

		value.reserve(16);
		snprintf(const_cast<char*>(value.c_str()), value.capacity(), "%llu %c%c", fileSize, unitOffset[0], unitOffset[1]);
	}
}