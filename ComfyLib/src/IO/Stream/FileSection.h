#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "IO/Stream/BinaryMode.h"
#include "IO/Stream/IStream.h"
#include "Misc/EndianHelper.h"
#include <optional>

namespace Comfy::IO
{
	class StreamReader;

	enum class SectionEndianness : u32
	{
		Little = 0x10000000,
		Big = 0x18000000,
	};

	// NOTE: Defines in little endian byte order as all section header data should be
	enum class SectionSignature : u32
	{
		EOFC = 0x43464F45, // NOTE: End of File

		AEDB = 0x42444541, // NOTE: AetDB
		SPDB = 0x42445053, // NOTE: SprDB
		MOSI = 0x49534F4D, // NOTE: ObjDB
		MTXI = 0x4958544D, // NOTE: TexDB

		AETC = 0x43544541, // NOTE: AetSet

		SPRC = 0x43525053, // NOTE: SprSet
		TXPC = 0x43505854, // NOTE: SprSet.TexSet

		MTXD = 0x4458544D, // NOTE: TexSet

		MOSD = 0x44534F4D, // NOTE: ObjSet
		OMDL = 0x4C444D4F, // NOTE: ObjSet.Obj
		OIDX = 0x5844494F, // NOTE: ObjSet.IndexBuffer
		OVTX = 0x5854564F, // NOTE: ObjSet.VertexBuffer
		OSKN = 0x4E4B534F, // NOTE: ObjSet.Skin

		ENRS = 0x53524E45, // NOTE: Endianness Reverse Table
		POF0 = 0x30464F50, // NOTE: Relocation Table 32-bit
		POF1 = 0x31464F50, // NOTE: Relocation Table 64-bit
	};

	// NOTE: Represents the data extracted from the header not the in-file header layout
	struct SectionHeader
	{
	public:
		// NOTE: Absolute address of the header itself within the file
		FileAddr HeaderAddress;
		// NOTE: Relative offset to the header address to the start of the section data
		FileAddr DataOffset;

		// NOTE: Number of bytes until the end of all sub sections including POF / ENRS
		size_t SectionSize;
		// NOTE: Number of bytes until the next sub section or EOFC
		size_t DataSize;

		// NOTE: Little endian signature
		SectionSignature Signature;

		// NOTE: Depth level or 0 for top level sections
		u32 Depth;

		// NOTE: Endianness of the contained section data
		Endianness Endianness;

	public:
		inline FileAddr StartOfSubSectionAddress() const { return HeaderAddress + DataOffset; }

		inline FileAddr EndOfSectionAddress() const { return HeaderAddress + DataOffset + static_cast<FileAddr>(SectionSize); }
		inline FileAddr EndOfSubSectionAddress() const { return HeaderAddress + DataOffset + static_cast<FileAddr>(DataSize); }

	public:
		static SectionHeader Read(StreamReader& reader);
		static std::optional<SectionHeader> TryRead(StreamReader& reader, SectionSignature expectedSignature);

		static void ScanPOFSectionsSetPointerMode(StreamReader& reader);
	};
}
