#pragma once
#include "Stream.h"
#include "Core/CoreTypes.h"

namespace FileSystem
{
	class MemoryStream : public Stream
	{
	public:
		MemoryStream();
		MemoryStream(const String& filePath);
		MemoryStream(const WideString& filePath);
		MemoryStream(Stream* stream);
		MemoryStream(Vector<uint8_t>* source);
		~MemoryStream();

		virtual void Seek(int64_t position) override;
		virtual int64_t GetPosition() const override;
		virtual int64_t GetLength() const override;

		virtual bool IsOpen() const override;
		virtual bool CanRead() const override;
		virtual bool CanWrite() const override;

		virtual int64_t Read(void* buffer, size_t size) override;
		virtual int64_t Write(const void* buffer, size_t size) override;

		void FromStreamSource(Vector<uint8_t>* source);
		void FromFile(const String& filePath);
		void FromFile(const WideString& filePath);
		void FromStream(Stream* stream);
		virtual void Close() override;

	protected:
		bool canRead = false;

		int64_t position = 0L;
		int64_t dataSize = 0L;

		Vector<uint8_t>* dataSource;
		Vector<uint8_t> dataVector;
	};
}