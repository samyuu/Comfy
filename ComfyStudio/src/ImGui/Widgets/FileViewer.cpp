#include "FileViewer.h"
#include "FileSystem/FileHelper.h"
#include "Misc/FileExtensionHelper.h"
#include "Misc/StringHelper.h"
#include <filesystem>

using namespace Comfy;

namespace ImGui
{
	FileViewer::FileViewer(const std::string_view directory)
	{
		SetDirectory(std::string(directory));
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
					SetParentDirectory(directory);

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

		bool filedClicked = false;
		BeginChild("FileListChild##FileViewer");
		{
			FilePathInfo* clickedInfo = DrawFileListGui();
			if (clickedInfo != nullptr)
			{
				if (clickedInfo->IsDirectory)
				{
					SetDirectory(FileSystem::Combine(directory, clickedInfo->ChildName));
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
						filedClicked = true;
					}
				}
			}

			WindowContextMenu("ContextMenu##FileViewer", [this]()
			{
				bool fileItemSelected = contextMenuFilePathInfo != nullptr;
				bool isDirectory = fileItemSelected && contextMenuFilePathInfo->IsDirectory;

				if (MenuItem("Open", nullptr, nullptr, fileItemSelected && !isDirectory))
					OpenContextItemDefaultProgram();

				if (MenuItem("Open in Explorer..."))
					OpenDirectoryInExplorer();

				if (MenuItem("Refresh"))
					SetDirectory(directory);

				Separator();
				if (MenuItem("Properties", nullptr, nullptr, fileItemSelected))
					OpenContextItemProperties();
			});
		}
		EndChild();

		PopStyleVar(1);
		PopID();

		return filedClicked;
	}

	void FileViewer::SetDirectory(std::string directory)
	{
		FileSystem::SanitizePath(directory);

		bool endingSlash = EndsWith(directory, '/');
		auto adjustedDirectory = endingSlash ? directory.substr(0, directory.length() - 1) : directory;

		strcpy_s(currentDirectoryBuffer, directory.c_str());
		if (!endingSlash)
		{
			char* destination = (char*)(currentDirectoryBuffer + directory.size());
			size_t size = (size_t)sizeof(currentDirectoryBuffer - directory.size());
			strcpy_s(destination, size, "/");
		}

		SetDirectoryInternal(adjustedDirectory);
	}

	const std::string& FileViewer::GetDirectory() const
	{
		return directory;
	}

	const std::string& FileViewer::GetFileToOpen() const
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
				SetColumnOffset(1, GetWindowWidth() * .75f);
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
		std::wstring widePath = Utf8ToUtf16(directory);
		if (!FileSystem::DirectoryExists(widePath))
			return;

		fileFilter.Clear();

		std::vector<FilePathInfo> newDirectoryInfo;
		newDirectoryInfo.reserve(8);

		for (const auto& file : std::filesystem::directory_iterator(widePath))
		{
			if (!file.is_regular_file() && !file.is_directory())
				continue;

			newDirectoryInfo.emplace_back();
			FilePathInfo& info = newDirectoryInfo.back();

			const auto path = file.path();
			info.FullPath = path.u8string();
			info.ChildName = path.filename().u8string();
			info.IsDirectory = file.is_directory();
			info.FileSize = file.file_size();

			FileSystem::SanitizePath(info.FullPath);
			FormatReadableFileSize(info.ReadableFileSize, info.FileSize);

			if (info.IsDirectory)
			{
				if (appendDirectorySlash)
					info.ChildName += '/';
			}
			else
			{
				info.FileType = useFileTypeIcons ? GetFileType(info.ChildName) : FileType::Default;
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
		previousDirectory = directory;
		directory = newDirectory;

		UpdateDirectoryInformation();
	}

	void FileViewer::SetParentDirectory(const std::string& directory)
	{
		if (EndsWith(directory, '/') || EndsWith(directory, '\\'))
			SetDirectory(std::string(FileSystem::GetDirectory(directory.substr(0, directory.length() - 1))));
		else
			SetDirectory(std::string(FileSystem::GetDirectory(directory)));
	}

	void FileViewer::SetResolveFileLinke(const FilePathInfo& info)
	{
		std::wstring wideLinkPath = Utf8ToUtf16(info.FullPath);
		
		std::wstring wideResolvedPath = FileSystem::ResolveFileLink(wideLinkPath);
		std::string resolvedPath = Utf16ToUtf8(wideResolvedPath);

		SetDirectory(resolvedPath);
	}

	void FileViewer::OpenDirectoryInExplorer()
	{
		FileSystem::OpenInExplorer(Utf8ToUtf16(directory));
	}

	void FileViewer::OpenContextItemDefaultProgram()
	{
		if (contextMenuFilePathInfo != nullptr)
		{
			std::string filePath = contextMenuFilePathInfo->FullPath;
			FileSystem::FuckUpWindowsPath(filePath);
			FileSystem::OpenWithDefaultProgram(Utf8ToUtf16(filePath));
		}
	}

	void FileViewer::OpenContextItemProperties()
	{
		std::string filePath = contextMenuFilePathInfo != nullptr ? contextMenuFilePathInfo->FullPath : directory;
		FileSystem::FuckUpWindowsPath(filePath);
		FileSystem::OpenExplorerProperties(Utf8ToUtf16(filePath));
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

	void FileViewer::FormatReadableFileSize(std::string& value, uint64_t fileSize)
	{
		if (fileSize <= 0)
			return;

		constexpr uint64_t unitFactor = 1024;
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