#include "BinaryWriter.h"
#include <assert.h>

namespace FileSystem
{
	BinaryWriter::BinaryWriter(Stream* stream)
	{
		OpenStream(stream);
	}

	BinaryWriter::~BinaryWriter()
	{
		Close();
	}

	void BinaryWriter::OpenStream(Stream* stream)
	{
		assert(!IsOpen());
		assert(stream->CanWrite());

		this->stream = stream;
	}

	void BinaryWriter::Close()
	{
		if (IsOpen() && !GetLeaveOpen())
			stream->Close();
	}

	bool BinaryWriter::IsOpen()
	{
		return stream != nullptr;
	}

	bool BinaryWriter::GetLeaveOpen()
	{
		return leaveOpen;
	}

	int64_t BinaryWriter::Write(const void* buffer, size_t size)
	{
		return stream->Write(buffer, size);
	}

	void BinaryWriter::WriteStr(const std::string& value)
	{
		Write(reinterpret_cast<const void*>(value.data()), value.size() + 1);
	}

	void BinaryWriter::WriteStrPtr(const std::string* value)
	{
		stringPointerPool.push_back({ GetPositionPtr(), value });
		WriteUInt32(PaddingValue);
	}

	void BinaryWriter::WritePtr(const std::function<void(BinaryWriter&)>& func)
	{
		pointerPool.push_back({ GetPositionPtr(), func });
		WriteUInt32(PaddingValue);
	}

	void BinaryWriter::WriteDelayedPtr(const std::function<void(BinaryWriter&)>& func)
	{
		delayedWritePool.push_back({ GetPositionPtr(), func });
		WriteUInt32(PaddingValue);
	}

	void BinaryWriter::WriteAlignmentPadding(int32_t alignment, uint32_t paddingValue)
	{
		constexpr bool forceExtraPadding = false;
		constexpr int32_t maxAlignment = 32;

		assert(alignment <= maxAlignment);
		uint8_t paddingValues[maxAlignment];

		int32_t value = GetPosition();
		int32_t paddingSize = ((value + (alignment - 1)) & ~(alignment - 1)) - value;

		if (forceExtraPadding && paddingSize <= 0)
			paddingSize = alignment;

		if (paddingSize > 0)
		{
			memset(paddingValues, paddingValue, paddingSize);
			Write(paddingValues, paddingSize);
		}
	}

	void BinaryWriter::FlushStringPointerPool()
	{
		for (auto& value : stringPointerPool)
		{
			uint32_t stringOffset = static_cast<uint32_t>(GetPosition());

			SetPosition(value.ReturnAddress);
			WriteUInt32(stringOffset);

			SetPosition(stringOffset);
			WriteStr(*value.String);
		}

		stringPointerPool.clear();
	}

	void BinaryWriter::FlushPointerPool()
	{
		for (auto& value : pointerPool)
		{
			uint32_t offset = static_cast<uint32_t>(GetPosition());

			SetPosition(value.ReturnAddress);
			WriteUInt32(offset);

			SetPosition(offset);
			value.Function(*this);
		}

		pointerPool.clear();
	}

	void BinaryWriter::FlushDelayedWritePool()
	{
		for (auto& value : delayedWritePool)
		{
			uint32_t offset = static_cast<uint32_t>(GetPosition());

			SetPosition(value.ReturnAddress);
			value.Function(*this);
		}

		delayedWritePool.clear();
	}
}