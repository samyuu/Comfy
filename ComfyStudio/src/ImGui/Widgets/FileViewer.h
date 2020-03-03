#pragma once
#include "CoreTypes.h"
#include "ImGui/Gui.h"
#include <FontIcons.h>

namespace ImGui
{
	enum class FileType : uint16_t
	{
		Default, Link, Text, Config, Binary, Image, Code, Archive, Video, Audio, Application, Count
	};

	class FileViewer
	{
	public:
		FileViewer(const std::string_view directory);
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
		void SetResolveFileLinke(const FilePathInfo& info);
		void OpenDirectoryInExplorer();
		void OpenContextItemDefaultProgram();
		void OpenContextItemProperties();

		static FileType GetFileType(const std::string_view fileName);
		static const char* GetFileInfoFormatString(const FilePathInfo& info);
		static const char* FormatFileType(FileType type);
		static void FormatReadableFileSize(std::string& value, uint64_t fileSize);

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
