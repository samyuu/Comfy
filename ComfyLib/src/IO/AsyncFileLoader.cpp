#include "AsyncFileLoader.h"
#include "Stream/Manipulator/StreamReader.h"
#include "Stream/MemoryStream.h"
#include "File.h"
#include "Misc/StringHelper.h"
#include "Core/Logger.h"
#include <assert.h>

namespace Comfy::IO
{
	AsyncFileLoader::AsyncFileLoader()
	{
	}

	AsyncFileLoader::AsyncFileLoader(std::string_view filePath)
	{
		SetFilePath(filePath);
	}

	AsyncFileLoader::~AsyncFileLoader()
	{
		if (loaderThread != nullptr && loaderThread->joinable())
			loaderThread->join();
	}

	const std::string& AsyncFileLoader::GetFilePath() const
	{
		return filePath;
	}

	void AsyncFileLoader::SetFilePath(std::string_view value)
	{
		assert(!isLoaded && !threadRunning && loaderThread == nullptr);
		filePath = value;
	}

	void AsyncFileLoader::LoadSync()
	{
		assert(!isLoaded && !filePath.empty());

		CheckFileLocation();
		if (!fileFound)
			return;

		if (File::ReadAllBytes(filePath, fileContent))
			SetIsLoaded(true);
	}

	void AsyncFileLoader::LoadAsync()
	{
		assert(!isLoaded && !threadRunning && !filePath.empty());

		CheckFileLocation();
		if (!fileFound)
			return;

		loaderThread = MakeUnique<std::thread>([this]()
		{
			threadRunning = true;

			if (File::ReadAllBytes(filePath, fileContent))
				SetIsLoaded(true);

			threadRunning = false;
		});
	}

	void AsyncFileLoader::CheckStartLoadAsync()
	{
		if (!isLoaded && loaderThread == nullptr && !threadRunning)
			LoadAsync();
	}

	bool AsyncFileLoader::GetIsLoaded() const
	{
		return isLoaded;
	}

	bool AsyncFileLoader::GetFileFound() const
	{
		return fileFound;
	}

	bool AsyncFileLoader::GetIsLoading() const
	{
		return fileFound && threadRunning && !isLoaded;
	}

	std::pair<const u8*, size_t> AsyncFileLoader::GetFileContents() const
	{
		assert(fileFound && isLoaded);
		return std::make_pair(fileContent.data(), fileContent.size());
	}

	void AsyncFileLoader::Read(IStreamReadable& readable) const
	{
		assert(fileFound && isLoaded);
		auto memoryStream = MemoryStream();
		memoryStream.FromStreamSource(const_cast<AsyncFileLoader*>(this)->fileContent);

		auto reader = StreamReader(memoryStream);
		readable.Read(reader);
	}

	void AsyncFileLoader::Parse(IBufferParsable& parsable) const
	{
		assert(fileFound && isLoaded);
		parsable.Parse(fileContent.data(), fileContent.size());
	}

	void AsyncFileLoader::FreeData()
	{
		assert(fileFound && isLoaded);
		fileContent.clear();
		fileContent.shrink_to_fit();
	}

	void AsyncFileLoader::CheckFileLocation()
	{
		SetFileFound(File::Exists(filePath));

		if (!fileFound)
			Logger::LogErrorLine(__FUNCTION__"(): %s not found", filePath.c_str());
	}

	void AsyncFileLoader::SetIsLoaded(bool value)
	{
		isLoaded = true;
	}

	void AsyncFileLoader::SetFileFound(bool value)
	{
		fileFound = value;
	}
}
