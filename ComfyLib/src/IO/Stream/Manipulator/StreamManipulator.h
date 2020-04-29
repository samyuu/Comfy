#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "IO/BinaryMode.h"
#include "IO/Stream/IStream.h"

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
		inline FileAddr GetPosition() const { return underlyingStream->GetPosition(); }
		inline void SetPosition(FileAddr position) { return underlyingStream->Seek(position); }

		inline FileAddr GetLength() const { return underlyingStream->GetLength(); }
		inline bool EndOfFile() const { return underlyingStream->GetPosition() >= underlyingStream->GetLength(); }

		inline void SkipPosition(FileAddr increment) { return underlyingStream->Seek(GetPosition() + increment); }

		inline PtrMode GetPointerMode() const { return pointerMode; }
		inline void SetPointerMode(PtrMode value) { pointerMode = value; OnPointerModeChanged(); }

		inline Endianness GetEndianness() const { return endianness; }
		inline void SetEndianness(Endianness value) { endianness = value; OnEndiannessChanged(); }

	protected:
		virtual void OnPointerModeChanged() = 0;
		virtual void OnEndiannessChanged() = 0;

	protected:
		PtrMode pointerMode = PtrMode::Mode32Bit;
		Endianness endianness = Endianness::Little;

		IStream* underlyingStream = nullptr;
	};
}
