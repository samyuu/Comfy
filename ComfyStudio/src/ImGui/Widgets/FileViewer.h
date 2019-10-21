#pragma once
#include "ImGui/Gui.h"
#include "Core/CoreTypes.h"
#include <FontIcons.h>

namespace ImGui
{
	enum class FileType : uint16_t
	{
		Default, Text, Config, Binary, Image, Code, Archive, Video, Audio, Application, Count
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
		const std::string& GetDirectory() const;
		const std::string& GetFileToOpen() const;

	private:
		struct FilePathInfo 
		{ 
			std::string FullPath;
			std::string ChildName;
			bool IsDirectory;
			bool IsHovered;
			FileType FileType;
			uint64_t FileSize;
			std::string ReadableFileSize;
		};

		const FilePathInfo* contextMenuFilePathInfo = nullptr;
		ExtendedImGuiTextFilter fileFilter;

		bool resizeColumns = true;
		bool appendDirectorySlash = false;
		bool useFileTypeIcons = true;
		char currentDirectoryBuffer[_MAX_PATH];

		std::vector<FilePathInfo> directoryInfo;
		std::string directory, previousDirectory;
		std::string fileToOpen;

		FilePathInfo* DrawFileListGui();

		void UpdateDirectoryInformation();
		void SetDirectoryInternal(const std::string& newDirectory);
		void SetParentDirectory(const std::string& directory);
		void OpenDirectoryInExplorer();
		void OpenContextItemDefaultProgram();
		void OpenContextItemProperties();

		static FileType GetFileType(const std::string& fileName);
		static const char* GetFileInfoFormatString(const FilePathInfo& info);
		static const char* FormatFileType(FileType type);
		static void FormatReadableFileSize(std::string& value, uint64_t fileSize);
		
		static const std::array<FileTypeDefinition, static_cast<size_t>(FileType::Count)> fileTypeDictionary;
	};
}