#include "FileViewer.h"
#include "ImGui/imgui_extensions.h"
#include "FileSystem/FileHelper.h"
#include <filesystem>

using FileSystemPath = std::filesystem::path;
using DirectoryIterator = std::filesystem::directory_iterator;

namespace ImGui
{
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
		std::string* clickedFilePath = nullptr;

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
					SetDirectoryInternal(pathBuffer);
				PopItemWidth();
			}
			PopStyleVar(1);
		}
		EndChild();

		ImGui::Separator();

		BeginChild("FileListChild##FileViewer");
		{
			OpenPopupOnItemClick("ContextMenu##FileViewer", 1);

			if (BeginPopupContextWindow("ContextMenu##FileViewer"))
			{
				if (MenuItem("Open in Explorer..."))
					OpenDirectoryInExplorer();
				EndPopup();
			}

			bool anyItemClicked = false;
			for (auto& info : directoryInfo)
			{
				ImGuiTreeNodeFlags flags = info.IsDirectory ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_Bullet;
				if (WideTreeNodeEx(anyItemClicked ? "" : info.ChildName.c_str(), flags))
					TreePop();

				if (!anyItemClicked && IsItemClicked())
				{
					if (info.IsDirectory)
						SetDirectory(FileSystem::Combine(directory, info.ChildName));
					else
						clickedFilePath = &info.FullPath;

					anyItemClicked = true;
				}
			}
		}
		EndChild();

		PopStyleVar(1);
		PopID();

		if (clickedFilePath != nullptr)
			fileToOpen = *clickedFilePath;

		return clickedFilePath != nullptr;
	}

	void FileViewer::SetDirectory(const std::string& directory)
	{
		strcpy_s(pathBuffer, directory.c_str());
		SetDirectoryInternal(directory);
	}

	std::string FileViewer::GetFileToOpen() const
	{
		return fileToOpen;
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
				info->ChildName += "/";
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

	void FileViewer::SetDirectoryInternal(const std::string& directory)
	{
		this->directory = directory;
		UpdateDirectoryInformation();
	}

	void FileViewer::SetParentDirectory(const std::string& directory)
	{
		bool endingSlash = (directory.size() > 0 && directory.back() == '/');
		auto parentDirectory = FileSystem::GetDirectory(endingSlash ? directory.substr(0, directory.length() - 1) : directory);

		SetDirectory(parentDirectory);
	}

	void FileViewer::OpenDirectoryInExplorer()
	{
		FileSystem::OpenInExplorer(directory);
	}
}