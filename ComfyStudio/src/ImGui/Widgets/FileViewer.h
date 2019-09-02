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
		Vector<String> Extensions;
		FileTypeDefinition(FileType type, Vector<String> extensions) : Type(type), Extensions(extensions) {};
	};

	class FileViewer
	{
	public:
		FileViewer(const String& directory);
		~FileViewer();

		bool DrawGui();
		void SetDirectory(String directory);
		const String& GetDirectory() const;
		const String& GetFileToOpen() const;

	private:
		struct FilePathInfo 
		{ 
			String FullPath;
			String ChildName;
			bool IsDirectory;
			bool IsHovered;
			FileType FileType;
			uint64_t FileSize;
			String ReadableFileSize;
		};

		const FilePathInfo* contextMenuFilePathInfo = nullptr;
		ExtendedImGuiTextFilter fileFilter;

		bool resizeColumns = true;
		bool appendDirectorySlash = false;
		bool useFileTypeIcons = true;
		char currentDirectoryBuffer[_MAX_PATH];

		Vector<FilePathInfo> directoryInfo;
		String directory, previousDirectory;
		String fileToOpen;

		FilePathInfo* DrawFileListGui();

		void UpdateDirectoryInformation();
		void SetDirectoryInternal(const String& newDirectory);
		void SetParentDirectory(const String& directory);
		void OpenDirectoryInExplorer();
		void OpenContextItemDefaultProgram();
		void OpenContextItemProperties();

		static FileType GetFileType(const String& fileName);
		static const char* GetFileInfoFormatString(const FilePathInfo& info);
		static const char* FormatFileType(FileType type);
		static void FormatReadableFileSize(String& value, uint64_t fileSize);
		
		static Array<FileTypeDefinition, static_cast<size_t>(FileType::Count)> fileTypeDictionary;
	};
}