#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "IO/Stream/BinaryMode.h"
#include "IO/Stream/FileSection.h"
#include "IO/Stream/IStream.h"
#include "Misc/EndianHelper.h"
#include <stack>

namespace Comfy::IO
{
	class StreamManipulator
	{
	public:
		static constexpr u32 PaddingValue = 0xCCCCCCCC;

	protected:
		StreamManipulator(IStream& stream) : underlyingStream(&stream)
		{
		}

		~StreamManipulator() = default;

	public:
		inline void PushBaseOffset() { baseOffsetStack.push(baseOffset = GetPosition()); }
		inline void PopBaseOffset() { baseOffsetStack.pop(); baseOffset = (baseOffsetStack.empty() ? FileAddr::NullPtr : baseOffsetStack.top()); }

	public:
		inline void Seek(FileAddr position) { return underlyingStream->Seek(position); }
		inline void SeekOffsetAware(FileAddr position) { return underlyingStream->Seek(position + baseOffset); }
		inline void Skip(FileAddr increment) { return underlyingStream->Seek(underlyingStream->GetPosition() + increment); }

		inline FileAddr GetPosition() const { return underlyingStream->GetPosition(); }
		inline FileAddr GetLength() const { return underlyingStream->GetLength(); }
		inline FileAddr GetRemaining() const { return underlyingStream->GetLength() - underlyingStream->GetPosition(); }
		
		inline bool EndOfFile() const { return underlyingStream->GetPosition() >= underlyingStream->GetLength(); }

		inline PtrMode GetPointerMode() const { return pointerMode; }
		inline void SetPointerMode(PtrMode value) { pointerMode = value; OnPointerModeChanged(); }

		inline Endianness GetEndianness() const { return endianness; }
		inline void SetEndianness(Endianness value) { endianness = value; OnEndiannessChanged(); }

		inline bool GetHasSections() const { return hasSections; }
		inline void SetHasSections(bool value) { hasSections = value; }

	protected:
		virtual void OnPointerModeChanged() = 0;
		virtual void OnEndiannessChanged() = 0;

	protected:
		PtrMode pointerMode = PtrMode::Mode32Bit;
		Endianness endianness = Endianness::Little;
		bool hasSections = false;

		IStream* underlyingStream = nullptr;

		FileAddr baseOffset = FileAddr::NullPtr;
		std::stack<FileAddr> baseOffsetStack;
	};
}
