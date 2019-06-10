#pragma once
#include "Stream.h"

class MemoryStream : public Stream
{
public:
	MemoryStream();
	MemoryStream(const wchar_t* filePath);
	MemoryStream(Stream* stream);
	~MemoryStream();

	virtual void Seek(int64_t position) override;
	virtual int64_t GetPosition() const override;
	virtual int64_t GetLength() const override;

	virtual bool IsOpen() const override;
	virtual bool CanRead() const override;
	virtual bool CanWrite() const override;

	virtual int64_t Read(void* buffer, size_t size) override;
	virtual int64_t Write(void* buffer, size_t size) override;

	void FromFile(const wchar_t* filePath);
	void FromStream(Stream* stream);
	virtual void Close() override;

protected:
	bool canRead = false;
	
	int64_t position = 0L;
	int64_t dataSize = 0L;
	
	uint8_t* data = nullptr;
};
