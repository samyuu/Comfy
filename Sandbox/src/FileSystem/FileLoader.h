#pragma once
#include "Types.h"
#include "FileInterface.h"
#include <vector>
#include <thread>

namespace FileSystem
{
	class ISynchronousFileLoader
	{
	public:
		virtual void LoadSync() = 0;
	};

	class IAsynchronousFileLoader
	{
	public:
		virtual void LoadAsync() = 0;
		virtual void CheckStartLoadAsync() = 0;
	};

	class IFileLoader : public ISynchronousFileLoader, public IAsynchronousFileLoader
	{
	public:
		virtual bool GetIsLoaded() const = 0;
		virtual bool GetFileFound() const = 0;
		virtual bool GetIsLoading() const = 0;

		virtual const std::vector<uint8_t>& GetFileContent() const = 0;
		virtual void Read(IBinaryReadable* readable) const = 0;
		virtual void Parse(IBufferParsable* parsable) const = 0;

		virtual void FreeData() = 0;

	protected:
		virtual void SetIsLoaded(bool value) = 0;
		virtual void SetFileFound(bool value) = 0;
	};

	class FileLoader : public IFileLoader
	{
	public:
		FileLoader();
		FileLoader(const std::string& filePath);
		~FileLoader();

		const std::string& GetFilePath() const;
		void SetFilePath(const std::string& value);

		virtual void LoadSync() override;
		virtual void LoadAsync() override;
		virtual void CheckStartLoadAsync() override;

		virtual bool GetIsLoaded() const override;
		virtual bool GetFileFound() const override;
		virtual bool GetIsLoading() const override;
		
		virtual const std::vector<uint8_t>& GetFileContent() const override;
		virtual void Read(IBinaryReadable* readable) const override;
		virtual void Parse(IBufferParsable* parsable) const override;

		virtual void FreeData() override;

	protected:
		std::string filePath;

		bool isLoaded = false;
		bool fileFound = false;
		std::vector<uint8_t> fileContent;
	
		bool threadStarted = false;
		bool threadRunning = false;
		std::thread loaderThread;

		void CheckFileLocation();

		virtual void SetIsLoaded(bool value) override;
		virtual void SetFileFound(bool value) override;
	};

}