#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "ImGui/Gui.h"
#include "LoadingTextAnimation.h"
#include <future>

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
		~FileViewer() = default;

	public:
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

		float lastWindowWidth = 0.0f;

		// TODO: Scale with font size (?)
		float fileColumnWidth = 98.0f;
		float sizeColumnWidth = 72.0f;
		float typeColumnWidth = 90.0f;

		// TODO: bool allowArchiveDirectory = true;
		bool resizeColumns = true;
		bool appendDirectoryChildNameSlash = false;
		bool useFileTypeIcons = true;
		char currentDirectoryBuffer[260];

		std::future<void> updateDirectoryInfoFuture;
		BinaryLoadingTextAnimation asyncLoadingAnimation;

		std::vector<FilePathInfo> tempAsyncDirectoryInfo;
		std::vector<FilePathInfo> currentDirectoryInfo;

		bool currentDirectoryIsArchive = false;
		std::string currentDirectoryOrArchive, previousDirectoryOrArchive;

		std::string fileToOpen;

		FilePathInfo* DrawFileListGui();

		void StartAsyncUpdateDirectoryInfo();
		void CheckAsyncUpdateDirectoryInfo();
		bool IsAsyncDirectoryInfoBusy() const;

		void SetParentDirectory(const std::string& directory);
		void SetResolveFileLinke(const FilePathInfo& info);
		void OpenDirectoryInExplorer();
		void OpenContextItemDefaultProgram();
		void OpenContextItemProperties();

		static FileType GetFileType(const std::string_view fileName);
		static const char* GetFileInfoFormatString(const FilePathInfo& info);
		static const char* FormatFileType(FileType type);
		static void FormatReadableFileSize(std::string& value, u64 fileSize);

		static constexpr std::array<std::pair<FileType, std::string_view>, Comfy::EnumCount<FileType>()> fileTypeDictionary =
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
