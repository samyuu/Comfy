#pragma once
#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"
#include <string>
#include <vector>
#include <array>
#include <FontIcons.h>

namespace ImGui
{
	enum class FileType : uint16_t
	{
		Default, Text, Config, Binary, Image, Code, Archive, Video, Audio, Count
	};

	struct FileTypeDefinition
	{
		FileType Type;
		std::vector<std::string> Extensions;
		FileTypeDefinition(FileType type, std::vector<std::string> extensions) : Type(type), Extensions(extensions) {};
	};

	class FileViewer
	{
	public:
		FileViewer(const std::string& directory);
		~FileViewer();

		bool DrawGui();
		void SetDirectory(std::string directory);
		std::string GetFileToOpen() const;

	private:
		struct FilePathInfo 
		{ 
			std::string FullPath;
			std::string ChildName;
			bool IsDirectory;
			bool IsHovered;
			FileType FileType;
		};

		bool useFileTypeIcons = true;
		char pathBuffer[_MAX_PATH];

		std::vector<FilePathInfo> directoryInfo;
		std::string directory, previousDirectory;
		std::string fileToOpen;

		FileType GetFileType(const std::string& fileName) const;
		const char* GetFileInfoFormatString(const FilePathInfo& info) const;
		void UpdateDirectoryInformation();
		void SetDirectoryInternal(const std::string& newDirectory);
		void SetParentDirectory(const std::string& directory);
		void OpenDirectoryInExplorer();

		static std::array<FileTypeDefinition, static_cast<size_t>(FileType::Count)> fileTypeDictionary;
	};
}