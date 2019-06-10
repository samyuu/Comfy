#pragma once
#include "Stream.h"
#include <array>
#include <functional>

#define DefineReadType(typeName, methodName) inline typeName Read##methodName() { return Read<typeName>(); };

enum PtrMode : uint8_t
{
	PtrMode_32Bit, 
	PtrMode_64Bit,
};

class BinaryReader
{
public:
	BinaryReader();
	BinaryReader(Stream* stream);
	~BinaryReader();

	void OpenStream(Stream* stream);
	void Close();

	bool IsOpen();
	bool GetLeaveOpen();

	int64_t GetPosition() const;
	void* GetPositionPtr() const;
	void SetPosition(int64_t position);
	void SetPosition(void* position);

	int64_t GetLength() const;
	bool EndOfFile() const;

	PtrMode GetPointerMode() const;
	void SetPointerMode(PtrMode mode);

	int64_t Read(void* buffer, size_t size);

	template <typename T> T Read();
	template <typename T, size_t size> std::array<T, size> Read();

	void ReadAt(void* position, const std::function<void(BinaryReader&)>& func);
	template <typename T> T ReadAt(void* position, const std::function<T(BinaryReader&)>& func);

	void* ReadPtr();
	std::string ReadStr();
	std::string ReadStrPtr();

	DefineReadType(bool, Boolean);
	DefineReadType(char, Char);
	DefineReadType(int8_t, SByte);
	DefineReadType(int8_t, Byte);
	DefineReadType(int16_t, Int16);
	DefineReadType(uint16_t, UInt16);
	DefineReadType(int32_t, Int32);
	DefineReadType(uint32_t, UInt32);
	DefineReadType(int64_t, Int64);
	DefineReadType(uint64_t, UInt64);
	DefineReadType(float, Float);
	DefineReadType(float, Single);
	DefineReadType(double, Double);

protected:
	PtrMode pointerMode = PtrMode_32Bit;
	bool leaveOpen = false;
	Stream* stream = nullptr;
};

template<typename T> 
T BinaryReader::Read()
{
	T value;
	Read(&value, sizeof(value));
	return value;
}

template <typename T, size_t size>
std::array<T, size> BinaryReader::Read()
{
	std::array<T, size> value;
	Read(value.data(), sizeof(T) * size);
	return value;
}

template <typename T> 
inline T BinaryReader::ReadAt(void* position, const std::function<T(BinaryReader&)>& func)
{
	int64_t prePos = GetPosition();
	SetPosition(position);
	T value = func(*this);
	SetPosition(prePos);
	
	return value;
}
