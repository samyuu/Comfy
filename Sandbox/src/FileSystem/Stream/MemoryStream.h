#pragma once
#include "Stream.h"
#include <vector>
#include <string>

namespace FileSystem
{
	class MemoryStream : public Stream
	{
	public:
		MemoryStream();
		MemoryStream(const std::string& filePath);
		MemoryStream(const std::wstring& filePath);
		MemoryStream(Stream* stream);
		MemoryStream(std::vector<uint8_t>* source);
		~MemoryStream();

		virtual void Seek(int64_t position) override;
		virtual int64_t GetPosition() const override;
		virtual int64_t GetLength() const override;

		virtual bool IsOpen() const override;
		virtual bool CanRead() const override;
		virtual bool CanWrite() const override;

		virtual int64_t Read(void* buffer, size_t size) override;
		virtual int64_t Write(const void* buffer, size_t size) override;

		void FromStreamSource(std::vector<uint8_t>* source);
		void FromFile(const std::string& filePath);
		void FromFile(const std::wstring& filePath);
		void FromStream(Stream* stream);
		virtual void Close() override;

	protected:
		bool canRead = false;

		int64_t position = 0L;
		int64_t dataSize = 0L;

		std::vector<uint8_t>* dataSource;
		std::vector<uint8_t> dataVector;
	};
}