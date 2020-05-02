#include "FileViewer.h"
#include "IO/Directory.h"
#include "IO/File.h"
#include "IO/Path.h"
#include "IO/Shell.h"
#include "Misc/FileExtensionHelper.h"
#include "Misc/StringHelper.h"
#include <filesystem>

using namespace Comfy;

namespace ImGui
{
	FileViewer::FileViewer(std::string_view directory)
	{
		SetDirectory(directory);
	}

	FileViewer::~FileViewer()
	{
	}

	bool FileViewer::DrawGui()
	{
		bool parentFocused = IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && IsWindowHovered(ImGuiFocusedFlags_RootAndChildWindows);
		vec2 windowSize = vec2(GetWindowWidth(), 0.0f);

		PushID(this);
		PushStyleVar(ImGuiStyleVar_ButtonTextAlign, vec2(0.0f, 0.5f));

		BeginChild("FileDirectoryChild##FileViewer", vec2(windowSize.x, 24.0f));
		{
			PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(0.0f, 0.0f));
			{
				bool backClicked = parentFocused && IsMouseReleased(3);
				if (ArrowButton("PreviousDirectoryButton::FileViewer", ImGuiDir_Up) || backClicked)
					SetParentDirectory(currentDirectoryOrArchive);

				SameLine();

				constexpr float searchBarWidth = 0.25f;

				PushItemWidth(windowSize.x * (1.0f - searchBarWidth));
				if (InputText("##DirectoryInputText::FileViewer", currentDirectoryBuffer, sizeof(currentDirectoryBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
					SetDirectory(currentDirectoryBuffer);
				PopItemWidth();

				PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(1.0f, 0.0f));
				SameLine();
				fileFilter.Draw("##FileFilter::FileViewer", ICON_FA_SEARCH, windowSize.x * searchBarWidth);
			}
			PopStyleVar(2);
		}
		EndChild();
		Separator();

		bool fileClicked = false;
		BeginChild("FileListChild##FileViewer");
		{
			if (FilePathInfo* clickedInfo = DrawFileListGui(); clickedInfo != nullptr)
			{
				if (clickedInfo->IsDirectory || IO::FolderFile::IsValidFolderFile(clickedInfo->FullPath))
				{
					SetDirectory(IO::Path::Combine(currentDirectoryOrArchive, clickedInfo->ChildName));
				}
				else
				{
					if (clickedInfo->FileType == FileType::Link)
					{
						SetResolveFileLinke(*clickedInfo);
					}
					else
					{
						fileToOpen = std::string(clickedInfo->FullPath);
						fileClicked = true;
					}
				}
			}

			WindowContextMenu("ContextMenu##FileViewer", [this]()
			{
				const bool fileItemSelected = contextMenuFilePathInfo != nullptr;
				const bool isDirectory = fileItemSelected && contextMenuFilePathInfo->IsDirectory;

				if (MenuItem("Open", nullptr, nullptr, fileItemSelected && !isDirectory))
					OpenContextItemDefaultProgram();

				if (MenuItem("Open in Explorer..."))
					OpenDirectoryInExplorer();

				if (MenuItem("Refresh"))
					SetDirectory(currentDirectoryOrArchive);

				Separator();
				if (MenuItem("Properties", nullptr, nullptr, fileItemSelected))
					OpenContextItemProperties();
			});
		}
		EndChild();

		PopStyleVar(1);
		PopID();

		return fileClicked;
	}

	void FileViewer::SetDirectory(std::string_view newDirectory)
	{
		currentDirectoryOrArchive = IO::Path::TrimTrailingPathSeparators(IO::Path::Normalize(newDirectory));

		if (currentDirectoryOrArchive.size() > std::size(currentDirectoryBuffer) + 2)
			currentDirectoryOrArchive = currentDirectoryOrArchive.substr(0, std::size(currentDirectoryBuffer) - 2);

		std::memcpy(currentDirectoryBuffer, currentDirectoryOrArchive.data(), currentDirectoryOrArchive.size());
		if (!IO::FolderFile::IsValidFolderFile(currentDirectoryOrArchive))
		{
			currentDirectoryBuffer[currentDirectoryOrArchive.size() + 0] = IO::Path::DirectorySeparator;
			currentDirectoryBuffer[currentDirectoryOrArchive.size() + 1] = '\0';
		}
		else
		{
			currentDirectoryBuffer[currentDirectoryOrArchive.size()] = '\0';
		}

		SetDirectoryInternal(currentDirectoryOrArchive);
	}

	std::string_view FileViewer::GetDirectory() const
	{
		return currentDirectoryOrArchive;
	}

	std::string_view FileViewer::GetFileToOpen() const
	{
		return fileToOpen;
	}

	FileViewer::FilePathInfo* FileViewer::DrawFileListGui()
	{
		FilePathInfo* clickedInfo = nullptr;
		bool anyContextMenuClicked = false;

		Columns(3, "FileListColumns##FileViewer");
		Text("Name"); NextColumn();
		Text("Size"); NextColumn();
		Text("Type"); NextColumn();
		Separator();
		{
			if (resizeColumns)
			{
				SetColumnOffset(1, GetWindowWidth() * 0.75f);
				resizeColumns = false;
			}

			char displayNameBuffer[_MAX_PATH];
			for (auto& info : directoryInfo)
			{
				if (!fileFilter.PassFilter(info.ChildName.c_str()))
					continue;

				sprintf_s(displayNameBuffer, GetFileInfoFormatString(info), info.ChildName.c_str());
				if (Selectable(displayNameBuffer, info.IsDirectory, ImGuiSelectableFlags_SpanAllColumns))
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

	void FileViewer::UpdateDirectoryInformation()
	{
		fileFilter.Clear();

		const bool isArchiveDirectory = IO::FolderFile::IsValidFolderFile(currentDirectoryOrArchive);
		if (isArchiveDirectory)
		{
			if (!IO::File::Exists(currentDirectoryOrArchive))
				return;
		}
		else
		{
			if (!IO::Directory::Exists(currentDirectoryOrArchive))
				return;
		}

		std::vector<FilePathInfo> newDirectoryInfo;
		newDirectoryInfo.reserve(8);

		if (isArchiveDirectory)
		{
			auto filePaths = IO::FolderFile::GetFileEntries(currentDirectoryOrArchive);
			for (const auto& fileEntry : filePaths)
			{
				const auto[basePath, internalFile] = IO::FolderFile::ParsePath(fileEntry.FullPath);

				FilePathInfo& info = newDirectoryInfo.emplace_back();
				info.FullPath = IO::Path::Normalize(fileEntry.FullPath);
				info.ChildName = internalFile;
				info.IsDirectory = false;
				info.FileSize = fileEntry.FileSize;
				info.FileType = useFileTypeIcons ? GetFileType(info.ChildName) : FileType::Default;
				FormatReadableFileSize(info.ReadableFileSize, info.FileSize);
			}
		}
		else
		{
			for (const auto& file : std::filesystem::directory_iterator(UTF8::Widen(currentDirectoryOrArchive)))
			{
				if (!file.is_regular_file() && !file.is_directory())
					continue;

				FilePathInfo& info = newDirectoryInfo.emplace_back();
				const auto path = file.path();
				info.FullPath = IO::Path::Normalize(path.u8string());
				info.ChildName = path.filename().u8string();
				info.IsDirectory = file.is_directory();
				info.FileSize = file.file_size();
				FormatReadableFileSize(info.ReadableFileSize, info.FileSize);

				if (info.IsDirectory)
				{
					if (appendDirectorySlash) info.ChildName += '/';
				}
				else
				{
					info.FileType = useFileTypeIcons ? GetFileType(info.ChildName) : FileType::Default;
				}
			}
		}

		directoryInfo.clear();
		directoryInfo.reserve(newDirectoryInfo.size());

		for (const auto& info : newDirectoryInfo)
			if (info.IsDirectory)
				directoryInfo.push_back(info);

		for (const auto& info : newDirectoryInfo)
			if (!info.IsDirectory)
				directoryInfo.push_back(info);
	}

	void FileViewer::SetDirectoryInternal(const std::string& newDirectory)
	{
		previousDirectory = currentDirectoryOrArchive;
		currentDirectoryOrArchive = newDirectory;

		UpdateDirectoryInformation();
	}

	void FileViewer::SetParentDirectory(const std::string& directory)
	{
		if (EndsWith(directory, IO::Path::DirectorySeparator) || EndsWith(directory, IO::Path::DirectorySeparatorAlt))
			SetDirectory(IO::Path::GetDirectoryName(directory.substr(0, directory.length() - 1)));
		else
			SetDirectory(IO::Path::GetDirectoryName(directory));
	}

	void FileViewer::SetResolveFileLinke(const FilePathInfo& info)
	{
		const auto resolvedPath = IO::Shell::ResolveFileLink(info.FullPath);
		SetDirectory(resolvedPath);
	}

	void FileViewer::OpenDirectoryInExplorer()
	{
		IO::Shell::OpenInExplorer(currentDirectoryOrArchive);
	}

	void FileViewer::OpenContextItemDefaultProgram()
	{
		if (contextMenuFilePathInfo != nullptr)
		{
			std::string_view filePath = contextMenuFilePathInfo->FullPath;
			IO::Shell::OpenWithDefaultProgram(IO::Path::NormalizeWin32(filePath));
		}
	}

	void FileViewer::OpenContextItemProperties()
	{
		std::string_view filePath = contextMenuFilePathInfo != nullptr ? contextMenuFilePathInfo->FullPath : currentDirectoryOrArchive;
		IO::Shell::OpenExplorerProperties(IO::Path::NormalizeWin32(filePath));
	}

	FileType FileViewer::GetFileType(const std::string_view fileName)
	{
		const auto inputExtension = FileExtensionHelper::GetExtensionSubstring(fileName);
		if (!inputExtension.empty())
		{
			for (const auto&[fileType, packedExtensions] : fileTypeDictionary)
			{
				if (FileExtensionHelper::DoesAnyExtensionMatch(inputExtension, packedExtensions))
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