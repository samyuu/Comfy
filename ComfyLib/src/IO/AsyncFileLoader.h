#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Stream/FileInterfaces.h"
#include <atomic>
#include <thread>

namespace Comfy::IO
{
	class AsyncFileLoader final : NonCopyable
	{
	public:
		AsyncFileLoader();
		AsyncFileLoader(std::string_view filePath);
		~AsyncFileLoader();

		const std::string& GetFilePath() const;
		void SetFilePath(std::string_view value);

		void LoadSync();
		void LoadAsync();
		void CheckStartLoadAsync();

		bool GetIsLoaded() const;
		bool GetFileFound() const;
		bool GetIsLoading() const;

		std::pair<const u8*, size_t> GetFileContents() const;

		bool Read(IStreamReadable& readable) const;
		bool Parse(IBufferParsable& parsable) const;

		void FreeData();

	protected:
		std::string filePath;

		bool isLoaded = false;
		bool fileFound = false;

		// TODO:
		// std::unique_ptr<u8[]> fileContent = nullptr;
		// size_t fileSize = {};
		std::vector<u8> fileContent;

		// TODO: Implement using std::async / std::future
		std::atomic_bool threadRunning = false;
		std::unique_ptr<std::thread> loaderThread = nullptr;

		void CheckFileLocation();

		void SetIsLoaded(bool value);
		void SetFileFound(bool value);
	};
}
