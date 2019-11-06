#include <iostream>
#include "FileSystem/FileHelper.h"
// #include "FileSystem/ComfyArchive/ComfyArchive.h"
#include "Core/Logger.h"

int main(int argc, const char* argv[])
{
	if (argc < 3)
	{
		Logger::LogErrorLine("Insufficient number of arguments");
		return EXIT_FAILURE;
	}

	const std::string inputDirectoryPath = argv[1];
	const std::string outputArchivePath = argv[2];

	if (!FileSystem::DirectoryExists(inputDirectoryPath))
	{
		Logger::LogErrorLine("Invalid directory input path");
		return EXIT_FAILURE;
	}

	// TODO: Pack data source directory into ComfyArchive ComfyData.bin
	FileSystem::WriteAllBytes(outputArchivePath, { 0xCF, 0x5C, 0xAC, 0x90 });

	std::cout << __FUNCTION__"(): Test\n";
	std::cin.get();

	return EXIT_SUCCESS;
}