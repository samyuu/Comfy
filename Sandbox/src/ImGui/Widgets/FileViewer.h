#pragma once
#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"
#include <string>
#include <vector>

namespace ImGui
{
	class FileViewer
	{
	public:
		FileViewer(const std::string& directory);
		~FileViewer();

		bool DrawGui();
		void SetDirectory(const std::string& directory);
		std::string GetFileToOpen() const;

	private:
		struct FilePathInfo 
		{ 
			std::string FullPath;
			std::string ChildName;
			bool IsDirectory; 
		};

		char pathBuffer[_MAX_PATH];
		std::vector<FilePathInfo> directoryInfo;
		std::string directory;
		std::string fileToOpen;

		void UpdateDirectoryInformation();
		void SetDirectoryInternal(const std::string& directory);
		void SetParentDirectory(const std::string& directory);
		void OpenDirectoryInExplorer();
	};
}