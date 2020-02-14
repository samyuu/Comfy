#pragma once
#include "Stream.h"
#include "CoreTypes.h"

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

		void Seek(int64_t position) override;
		int64_t GetPosition() const override;
		int64_t GetLength() const override;

		bool IsOpen() const override;
		bool CanRead() const override;
		bool CanWrite() const override;

		int64_t Read(void* buffer, size_t size) override;
		int64_t Write(const void* buffer, size_t size) override;

		void FromStreamSource(std::vector<uint8_t>* source);
		void FromFile(const std::string& filePath);
		void FromFile(const std::wstring& filePath);
		void FromStream(Stream* stream);
		void Close() override;

	protected:
		bool canRead = false;

		int64_t position = 0L;
		int64_t dataSize = 0L;

		std::vector<uint8_t>* dataSource;
		std::vector<uint8_t> dataVector;
	};
}