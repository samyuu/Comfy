#include "BinaryWriter.h"
#include <assert.h>

namespace FileSystem
{
	BinaryWriter::BinaryWriter()
	{
		SetPointerMode(PtrMode::Mode32Bit);
	}

	BinaryWriter::BinaryWriter(Stream* stream) : BinaryWriter()
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

	PtrMode BinaryWriter::GetPointerMode() const
	{
		return pointerMode;
	}

	void BinaryWriter::SetPointerMode(PtrMode mode)
	{
		pointerMode = mode;

		switch (pointerMode)
		{
		case PtrMode::Mode32Bit:
			writePtrFunction = Write32BitPtr;
			return;

		case PtrMode::Mode64Bit:
			writePtrFunction = Write64BitPtr;
			return;

		default:
			writePtrFunction = nullptr;
			break;
		}

		assert(false);
	}

	int64_t BinaryWriter::Write(const void* buffer, size_t size)
	{
		return stream->Write(buffer, size);
	}

	void BinaryWriter::WriteStr(const std::string& value)
	{
		Write(reinterpret_cast<const void*>(value.data()), value.size() + 1);
	}

	void BinaryWriter::WriteStrPtr(const std::string* value, int32_t alignment)
	{
		stringPointerPool.push_back({ GetPositionPtr(), value, alignment });
		WritePtr(nullptr);
	}

	void BinaryWriter::WritePtr(const std::function<void(BinaryWriter&)>& func)
	{
		pointerPool.push_back({ GetPositionPtr(), func });
		WritePtr(nullptr);
	}

	void BinaryWriter::WriteDelayedPtr(const std::function<void(BinaryWriter&)>& func)
	{
		delayedWritePool.push_back({ GetPositionPtr(), func });
		WritePtr(nullptr);
	}

	void BinaryWriter::WritePadding(size_t size, uint32_t paddingValue)
	{
		if (size < 0)
			return;

		constexpr size_t maxSize = 32;
		assert(size <= maxSize);
		uint8_t paddingValues[maxSize];

		memset(paddingValues, paddingValue, size);
		Write(paddingValues, size);
	}

	void BinaryWriter::WriteAlignmentPadding(int32_t alignment, uint32_t paddingValue)
	{
		constexpr bool forceExtraPadding = false;
		constexpr int32_t maxAlignment = 32;

		assert(alignment <= maxAlignment);
		uint8_t paddingValues[maxAlignment];

		int32_t value = static_cast<int32_t>(GetPosition());
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
			void* originalStringOffset = GetPositionPtr();
			void* stringOffset = originalStringOffset;

			bool pooledStringFound = false;

			if (poolStrings)
			{
				auto pooledString = writtenStringPool.find(*value.String);
				if (pooledString != writtenStringPool.end())
				{
					stringOffset = pooledString->second;
					pooledStringFound = true;
				}
			}

			SetPosition(value.ReturnAddress);
			WritePtr(stringOffset);

			if (!pooledStringFound)
			{
				SetPosition(stringOffset);
				WriteStr(*value.String);

				if (value.Alignment > 0)
					WriteAlignmentPadding(value.Alignment);
			}
			else
			{
				SetPosition(originalStringOffset);
			}

			if (poolStrings && !pooledStringFound)
				writtenStringPool.insert(std::make_pair(*value.String, stringOffset));
		}

		if (poolStrings)
			writtenStringPool.clear();
		
		stringPointerPool.clear();
	}

	void BinaryWriter::FlushPointerPool()
	{
		for (auto& value : pointerPool)
		{
			void* offset = GetPositionPtr();

			SetPosition(value.ReturnAddress);
			WritePtr(offset);

			SetPosition(offset);
			value.Function(*this);
		}

		pointerPool.clear();
	}

	void BinaryWriter::FlushDelayedWritePool()
	{
		for (auto& value : delayedWritePool)
		{
			void* offset = GetPositionPtr();

			SetPosition(value.ReturnAddress);
			value.Function(*this);
			
			SetPosition(offset);
		}

		delayedWritePool.clear();
	}
}