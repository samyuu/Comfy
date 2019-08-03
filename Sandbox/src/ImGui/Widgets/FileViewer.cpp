#include "FileViewer.h"
#include "ImGui/imgui_extensions.h"
#include "FileSystem/FileHelper.h"
#include "Misc/StringHelper.h"
#include <filesystem>

#define FILE_ICON_FORMAT_STRING(iconName) ("  " iconName "  %s")
#define CASE_FILE_ICON(caseValue, icon) case caseValue: return FILE_ICON_FORMAT_STRING(icon)

using FileSystemPath = std::filesystem::path;
using DirectoryIterator = std::filesystem::directory_iterator;

namespace ImGui
{
	std::array<FileTypeDefinition, static_cast<size_t>(FileType::Count)> FileViewer::fileTypeDictionary =
	{
		FileTypeDefinition(FileType::Default, { "." }),
		FileTypeDefinition(FileType::Text, { ".txt" }),
		FileTypeDefinition(FileType::Config, { ".ini", ".xml" }),
		FileTypeDefinition(FileType::Binary, { ".bin" }),
		FileTypeDefinition(FileType::Image, { ".png", ".jpg", ".jpeg", ".gif", ".dds", ".bmp", ".psd" }),
		FileTypeDefinition(FileType::Code, { ".c", ".cpp", ".cs", ".h", ".glsl" }),
		FileTypeDefinition(FileType::Archive, { ".farc", ".7z", ".zip" }),
		FileTypeDefinition(FileType::Video, { ".mp4", ".wmv" }),
		FileTypeDefinition(FileType::Audio, { ".wav", ".flac", ".ogg", ".mp3" }),
		FileTypeDefinition(FileType::Application, { ".exe", ".dll" }),
	};

	FileViewer::FileViewer(const std::string& directory)
	{
		SetDirectory(directory);
	}

	FileViewer::~FileViewer()
	{
	}

	bool FileViewer::DrawGui()
	{
		bool parentFocused = IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && IsWindowHovered(ImGuiFocusedFlags_RootAndChildWindows);
		ImVec2 windowSize(ImGui::GetWindowWidth(), 0);

		PushID(this);
		PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.5f));

		BeginChild("FileDirectoryChild##FileViewer", ImVec2(windowSize.x, 24));
		{
			PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
			{
				bool backClicked = parentFocused && IsMouseReleased(3);
				if (ArrowButton("PreviousDirectoryButton##FileViewer", ImGuiDir_Up) || backClicked)
					SetParentDirectory(directory);

				SameLine();

				PushItemWidth(windowSize.x);
				if (InputText("##DirectoryInputText##FileViewer", currentDirectoryBuffer, sizeof(currentDirectoryBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
					SetDirectory(currentDirectoryBuffer);
				PopItemWidth();
			}
			PopStyleVar(1);
		}
		EndChild();
		ImGui::Separator();

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
					fileToOpen = std::string(clickedInfo->FullPath);
					filedClicked = true;
				}
			}

			ImGui::WindowContextMenu("ContextMenu##FileViewer", [this]()
			{
				if (MenuItem("Open in Explorer..."))
					OpenDirectoryInExplorer();

				if (contextMenuFilePathInfo != nullptr)
				{
					Separator();
					if (MenuItem("Properties"))
						OpenContextItemProperties();
				}
			});
		}
		EndChild();

		PopStyleVar(1);
		PopID();

		return filedClicked;
	}

	void FileViewer::SetDirectory(std::string directory)
	{
		std::replace(directory.begin(), directory.end(), '\\', '/');

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

	const std::string& FileViewer::GetFileToOpen() const
	{
		return fileToOpen;
	}

	FileViewer::FilePathInfo* FileViewer::DrawFileListGui()
	{
		FilePathInfo* clickedInfo = nullptr;
		bool anyContextMenuClicked = false;

		ImGui::Columns(3, "FileListColumns##FileViewer");
		ImGui::Text("Name"); ImGui::NextColumn();
		ImGui::Text("Size"); ImGui::NextColumn();
		ImGui::Text("Type"); ImGui::NextColumn();
		ImGui::Separator();
		{
			if (resizeColumns)
			{
				SetColumnOffset(1, ImGui::GetWindowWidth() * .75f);
				resizeColumns = false;
			}

			char displayNameBuffer[_MAX_PATH];
			for (auto& info : directoryInfo)
			{
				sprintf_s(displayNameBuffer, GetFileInfoFormatString(info), info.ChildName.c_str());
				if (Selectable(displayNameBuffer, info.IsDirectory, ImGuiSelectableFlags_SpanAllColumns))
					clickedInfo = &info;

				info.IsHovered = IsItemHovered();

				if (IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) && IsMouseClicked(1))
				{
					contextMenuFilePathInfo = &info;
					anyContextMenuClicked = true;
				}

				ImGui::NextColumn();
				if (!info.IsDirectory)
					ImGui::Text(info.ReadableFileSize.c_str());
				ImGui::NextColumn();

				ImGui::Text(info.IsDirectory ? "File Folder" : FormatFileType(info.FileType));
				ImGui::NextColumn();
			}
		}
		ImGui::Columns(1);

		if (!anyContextMenuClicked && IsMouseClicked(1))
			contextMenuFilePathInfo = nullptr;

		return clickedInfo;
	}

	void FileViewer::UpdateDirectoryInformation()
	{
		std::wstring widePath = FileSystem::Utf8ToWideString(directory);
		if (!FileSystem::DirectoryExists(widePath))
			return;

		std::vector<FilePathInfo> newDirectoryInfo;
		newDirectoryInfo.reserve(8);

		for (const auto& file : DirectoryIterator(widePath))
		{
			if (!file.is_regular_file() && !file.is_directory())
				continue;

			newDirectoryInfo.emplace_back();
			FilePathInfo *info = &newDirectoryInfo.back();

			auto path = file.path();
			info->FullPath = path.u8string();
			info->ChildName = path.filename().u8string();
			info->IsDirectory = file.is_directory();
			info->FileSize = file.file_size();
			FormatReadableFileSize(info->ReadableFileSize, info->FileSize);

			if (info->IsDirectory)
			{
				info->ChildName += "/";
			}
			else
			{
				info->FileType = useFileTypeIcons ? GetFileType(info->ChildName) : FileType::Default;
			}
		}

		directoryInfo.clear();
		directoryInfo.reserve(newDirectoryInfo.size());

		for (auto& info : newDirectoryInfo)
			if (info.IsDirectory)
				directoryInfo.push_back(info);

		for (auto& info : newDirectoryInfo)
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
		bool endingSlash = EndsWith(directory, '/') || EndsWith(directory, '\\');
		auto parentDirectory = FileSystem::GetDirectory(endingSlash ? directory.substr(0, directory.length() - 1) : directory);

		SetDirectory(parentDirectory);
	}

	void FileViewer::OpenDirectoryInExplorer()
	{
		FileSystem::OpenInExplorer(Utf8ToUtf16(directory));
	}

	void FileViewer::OpenContextItemProperties()
	{
		FileSystem::OpenExplorerProperties(Utf8ToUtf16(contextMenuFilePathInfo != nullptr ? contextMenuFilePathInfo->FullPath : directory));
	}

	FileType FileViewer::GetFileType(const std::string& fileName)
	{
		for (const auto& definition : fileTypeDictionary)
		{
			for (const auto& extension : definition.Extensions)
			{
				if (EndsWithInsensitive(fileName, extension))
					return definition.Type;
			}
		}

		return FileType::Default;
	}

	const char* FileViewer::GetFileInfoFormatString(const FilePathInfo& info)
	{
		constexpr const char* formatString = "  %s";

		if (info.IsDirectory)
			return info.IsHovered ? FILE_ICON_FORMAT_STRING(ICON_FA_FOLDER_OPEN) : FILE_ICON_FORMAT_STRING(ICON_FA_FOLDER);

		switch (info.FileType)
		{
			CASE_FILE_ICON(FileType::Default, ICON_FA_FILE);
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
	}

	const char* FileViewer::FormatFileType(FileType type)
	{
		switch (type)
		{
		case FileType::Default:
			return "File";
		case FileType::Text:
			return "Text";
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

		int unitIndex = 0;
		while (fileSize > 1024 && unitIndex < 8)
		{
			fileSize /= 1024;
			unitIndex++;
		}

		const char* units = "B  KB MB GB TB PB EB ZB YB";
		const char* unit = &units[unitIndex * 3];

		value.reserve(32);
		snprintf(const_cast<char*>(value.c_str()), value.capacity(), "%llu %c%c", fileSize, *(unit + 0), *(unit + 1));
	}
}