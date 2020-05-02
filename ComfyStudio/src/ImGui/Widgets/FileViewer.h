#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "ImGui/Gui.h"
#include <FontIcons.h>

namespace ImGui
{
	enum class FileType : u16
	{
		Default, Link, Text, Config, Binary, Image, Code, Archive, Video, Audio, Application, Count
	};

	class FileViewer
	{
	public:
		FileViewer(std::string_view directory);
		~FileViewer();

		bool DrawGui();
		void SetDirectory(std::string_view newDirectory);
		std::string_view GetDirectory() const;
		std::string_view GetFileToOpen() const;

	private:
		struct FilePathInfo
		{
			std::string FullPath;
			std::string ChildName;
			bool IsDirectory;
			bool IsHovered;
			FileType FileType;
			u64 FileSize;
			std::string ReadableFileSize;
		};

		const FilePathInfo* contextMenuFilePathInfo = nullptr;
		ExtendedImGuiTextFilter fileFilter;

		bool resizeColumns = true;
		bool appendDirectorySlash = false;
		bool useFileTypeIcons = true;
		char currentDirectoryBuffer[260];

		std::vector<FilePathInfo> directoryInfo;
		std::string currentDirectoryOrArchive, previousDirectory;
		std::string fileToOpen;

		FilePathInfo* DrawFileListGui();

		void UpdateDirectoryInformation();
		void SetDirectoryInternal(const std::string& newDirectory);
		void SetParentDirectory(const std::string& directory);
		void SetResolveFileLinke(const FilePathInfo& info);
		void OpenDirectoryInExplorer();
		void OpenContextItemDefaultProgram();
		void OpenContextItemProperties();

		static FileType GetFileType(const std::string_view fileName);
		static const char* GetFileInfoFormatString(const FilePathInfo& info);
		static const char* FormatFileType(FileType type);
		static void FormatReadableFileSize(std::string& value, u64 fileSize);

		static constexpr std::array<std::pair<FileType, const char*>, static_cast<size_t>(FileType::Count)> fileTypeDictionary =
		{
			std::make_pair(FileType::Default, "."),
			std::make_pair(FileType::Link, ".lnk"),
			std::make_pair(FileType::Text, ".txt"),
			std::make_pair(FileType::Config, ".ini;.xml"),
			std::make_pair(FileType::Binary, ".bin"),
			std::make_pair(FileType::Image, ".png;.jpg;.jpeg;.gif;.dds;.bmp;.psd"),
			std::make_pair(FileType::Code, ".c;.cpp;.cs;.h;.hpp;.glsl;.hlsl"),
			std::make_pair(FileType::Archive, ".farc;.7z;.zip"),
			std::make_pair(FileType::Video, ".mp4;.wmv;.mkv"),
			std::make_pair(FileType::Audio, ".wav;.flac;.ogg;.mp3"),
			std::make_pair(FileType::Application, ".exe;.dll"),
		};
	};
}
