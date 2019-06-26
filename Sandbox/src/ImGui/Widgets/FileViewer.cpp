#include "FileViewer.h"
#include "ImGui/imgui_extensions.h"
#include "FileSystem/FileHelper.h"
#include "Misc/StringHelper.h"
#include <filesystem>

#define FILE_ICON_FORMAT_STRING(iconName) (iconName "  %s")
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
		bool parentFocused = IsRootWindowOrAnyChildFocused() && IsRootWindowOrAnyChildHovered();
		ImVec2 windowSize(ImGui::GetWindowWidth(), 0);

		PushID(this);
		PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.5f));

		BeginChild("FileDirectoryChild##FileViewer", ImVec2(windowSize.x, 24));
		{
			PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
			{
				bool backClicked = parentFocused && IsMouseReleased(3);
				if (ArrowButton("##PreviousDirectoryButton", ImGuiDir_Up) || backClicked)
					SetParentDirectory(directory);

				SameLine();

				PushItemWidth(windowSize.x);
				if (InputText("##dir/", pathBuffer, sizeof(pathBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
					SetDirectory(pathBuffer);
				PopItemWidth();
			}
			PopStyleVar(1);
		}
		EndChild();
		ImGui::Separator();

		bool filedClicked = false;
		BeginChild("FileListChild##FileViewer");
		{
			OpenPopupOnItemClick("ContextMenu##FileViewer", 1);

			if (BeginPopupContextWindow("ContextMenu##FileViewer"))
			{
				if (MenuItem("Open in Explorer..."))
					OpenDirectoryInExplorer();
				EndPopup();
			}

			FilePathInfo* clickedInfo = nullptr;
			char pathBuffer[_MAX_PATH];

			for (auto& info : directoryInfo)
			{
				ImGuiTreeNodeFlags flags = info.IsDirectory ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None;

				sprintf_s(pathBuffer, GetFileInfoFormatString(info), info.ChildName.c_str());
				if (WideTreeNodeNoArrow(pathBuffer, flags))
					TreePop();

				info.IsHovered = IsItemHovered();
				if (IsItemClicked())
					clickedInfo = &info;
			}

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

		strcpy_s(pathBuffer, directory.c_str());
		if (!endingSlash)
		{
			char* destination = (char*)(pathBuffer + directory.size());
			size_t size = (size_t)sizeof(pathBuffer - directory.size());
			strcpy_s(destination, size, "/");
		}

		SetDirectoryInternal(adjustedDirectory);
	}

	std::string FileViewer::GetFileToOpen() const
	{
		return fileToOpen;
	}

	FileType FileViewer::GetFileType(const std::string& fileName) const
	{
		if (useFileTypeIcons)
		{
			for (const auto& definition : fileTypeDictionary)
			{
				for (const auto& extension : definition.Extensions)
				{
					if (EndsWithInsensitive(fileName, extension))
						return definition.Type;
				}
			}
		}

		return FileType::Default;
	}

	const char* FileViewer::GetFileInfoFormatString(const FilePathInfo& info) const
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
		default:
			return FILE_ICON_FORMAT_STRING(ICON_FA_FILE);
		}
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
			newDirectoryInfo.emplace_back();
			FilePathInfo *info = &newDirectoryInfo.back();

			auto path = file.path();
			info->FullPath = path.u8string();
			info->ChildName = path.filename().u8string();
			info->IsDirectory = file.is_directory();

			if (info->IsDirectory)
			{
				info->ChildName += "/";
			}
			else
			{
				info->FileType = GetFileType(info->ChildName);
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
		FileSystem::OpenInExplorer(directory);
	}
}