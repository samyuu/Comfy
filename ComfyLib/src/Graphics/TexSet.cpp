#include "TexSet.h"
#include "Auth2D/SprSet.h"
#include "Auth3D/ObjSet.h"
#include "IO/Stream/Manipulator/StreamReader.h"
#include "IO/Stream/Manipulator/StreamWriter.h"
#include "IO/File.h"

using namespace Comfy::IO;

namespace Comfy::Graphics
{
	const std::vector<TexMipMap>& Tex::GetMipMaps(u32 arrayIndex) const
	{
		return MipMapsArray[arrayIndex];
	}

	TxpSig Tex::GetSignature() const
	{
		return (MipMapsArray.size() == 6) ? TxpSig::CubeMap : TxpSig::Texture2D;
	}

	ivec2 Tex::GetSize() const
	{
		if (MipMapsArray.size() < 1 || MipMapsArray.front().size() < 1)
			return ivec2(0, 0);

		return MipMapsArray.front().front().Size;
	}

	TextureFormat Tex::GetFormat() const
	{
		if (MipMapsArray.size() < 1 || MipMapsArray.front().size() < 1)
			return TextureFormat::Unknown;

		return MipMapsArray.front().front().Format;
	}

	std::string_view Tex::GetName() const
	{
		return (Name.has_value()) ? Name.value() : UnknownName;
	}

	StreamResult Tex::Read(StreamReader& reader)
	{
		reader.PushBaseOffset();
		const auto texSignature = static_cast<TxpSig>(reader.ReadU32());
		const auto mipMapCount = reader.ReadU32();

		const auto mipLevels = reader.ReadU8();
		const auto arraySize = reader.ReadU8();
		const auto depth = reader.ReadU8();
		const auto dimensions = reader.ReadU8();

		if (texSignature != TxpSig::Texture2D && texSignature != TxpSig::CubeMap)
			return StreamResult::BadFormat;

		const auto adjustedMipLevels = (texSignature == TxpSig::CubeMap) ? (mipMapCount / arraySize) : mipMapCount;

		MipMapsArray.reserve(arraySize);
		for (size_t i = 0; i < arraySize; i++)
		{
			auto& mipMaps = MipMapsArray.emplace_back();
			mipMaps.reserve(adjustedMipLevels);

			for (size_t j = 0; j < adjustedMipLevels; j++)
			{
				const auto mipMapOffset = reader.ReadPtr_32();
				if (!reader.IsValidPointer(mipMapOffset))
					return StreamResult::BadPointer;

				auto streamResult = StreamResult::Success;
				reader.ReadAtOffsetAware(mipMapOffset, [&](StreamReader& reader)
				{
					const auto mipSignature = static_cast<TxpSig>(reader.ReadU32());
					if (mipSignature != TxpSig::MipMap)
					{
						streamResult = StreamResult::BadFormat;
						return;
					}

					auto& mipMap = mipMaps.emplace_back();
					mipMap.Size.x = reader.ReadI32();
					mipMap.Size.y = reader.ReadI32();
					mipMap.Format = static_cast<TextureFormat>(reader.ReadU32());

					const auto mipIndex = reader.ReadU8();
					const auto arrayIndex = reader.ReadU8();
					const auto padding = reader.ReadU16();

					mipMap.DataSize = reader.ReadU32();
					if (mipMap.DataSize > static_cast<size_t>(reader.GetRemaining()))
					{
						streamResult = StreamResult::BadCount;
						return;
					}

					mipMap.Data = std::make_unique<u8[]>(mipMap.DataSize);
					reader.ReadBuffer(mipMap.Data.get(), mipMap.DataSize);
				});
				if (streamResult != StreamResult::Success)
					return streamResult;
			}
		}

		reader.PopBaseOffset();
		return StreamResult::Success;
	}

	StreamResult TexSet::Read(StreamReader& reader)
	{
		auto baseHeader = SectionHeader::TryRead(reader, SectionSignature::MTXD);
		if (!baseHeader.has_value())
			baseHeader = SectionHeader::TryRead(reader, SectionSignature::TXPC);

		SectionHeader::ScanPOFSectionsSetPointerMode(reader);

		if (baseHeader.has_value())
		{
			reader.SetEndianness(baseHeader->Endianness);
			reader.Seek(baseHeader->StartOfSubSectionAddress());

			if (reader.GetPointerMode() == PtrMode::Mode64Bit)
				reader.PushBaseOffset();
		}

		reader.PushBaseOffset();

		const auto setSignature = static_cast<TxpSig>(reader.ReadU32());
		const auto textureCount = reader.ReadU32();
		const auto packedInfo = reader.ReadU32();

		if (setSignature != TxpSig::TexSet)
			return StreamResult::BadFormat;

		Textures.reserve(textureCount);
		for (size_t i = 0; i < textureCount; i++)
		{
			const auto textureOffset = reader.ReadPtr_32();
			if (!reader.IsValidPointer(textureOffset))
				return StreamResult::BadPointer;

			auto streamResult = StreamResult::Success;
			reader.ReadAtOffsetAware(textureOffset, [&](StreamReader& reader)
			{
				streamResult = Textures.emplace_back(std::make_shared<Tex>())->Read(reader);
			});

			if (streamResult != StreamResult::Success)
				return streamResult;
		}

		reader.PopBaseOffset();

		if (baseHeader.has_value() && reader.GetPointerMode() == PtrMode::Mode64Bit)
			reader.PopBaseOffset();

		return StreamResult::Success;
	}

	StreamResult TexSet::Write(StreamWriter& writer)
	{
		const u32 textureCount = static_cast<u32>(Textures.size());
		constexpr u32 packedMask = 0x01010100;

		const FileAddr setBaseAddress = writer.GetPosition();
		writer.WriteU32(static_cast<u32>(TxpSig::TexSet));
		writer.WriteU32(textureCount);
		writer.WriteU32(textureCount | packedMask);

		for (const auto& texture : Textures)
		{
			writer.WriteFuncPtr([&](StreamWriter& writer)
			{
				const FileAddr texBaseAddress = writer.GetPosition();
				const u8 arraySize = static_cast<u8>(texture->MipMapsArray.size());
				const u8 mipLevels = (arraySize > 0) ? static_cast<u8>(texture->MipMapsArray.front().size()) : 0;

				writer.WriteU32(static_cast<u32>(texture->GetSignature()));
				writer.WriteU32(arraySize * mipLevels);
				writer.WriteU8(mipLevels);
				writer.WriteU8(arraySize);
				writer.WriteU8(0x01);
				writer.WriteU8(0x01);

				for (u8 arrayIndex = 0; arrayIndex < arraySize; arrayIndex++)
				{
					for (u8 mipIndex = 0; mipIndex < mipLevels; mipIndex++)
					{
						writer.WriteFuncPtr([arrayIndex, mipIndex, &texture](StreamWriter& writer)
						{
							const auto& mipMap = texture->MipMapsArray[arrayIndex][mipIndex];
							writer.WriteU32(static_cast<u32>(TxpSig::MipMap));
							writer.WriteI32(mipMap.Size.x);
							writer.WriteI32(mipMap.Size.y);
							writer.WriteU32(static_cast<u32>(mipMap.Format));
							writer.WriteU8(mipIndex);
							writer.WriteU8(arrayIndex);
							writer.WriteU8(0x00);
							writer.WriteU8(0x00);
							writer.WriteU32(mipMap.DataSize);
							writer.WriteBuffer(mipMap.Data.get(), mipMap.DataSize);
						}, texBaseAddress);
					}
				}
			}, setBaseAddress);
		}

		writer.FlushPointerPool();
		writer.WriteAlignmentPadding(16);

		return StreamResult::Success;
	}

	void TexSet::SetTextureIDs(const ObjSet& objSet)
	{
		const auto& textureIDs = objSet.TextureIDs;
		assert(textureIDs.size() <= Textures.size());

		const auto textureCount = std::min(Textures.size(), textureIDs.size());
		for (size_t i = 0; i < textureCount; i++)
			Textures[i]->ID = textureIDs[i];
	}

	/* // TODO: Move upload responsibility to Comfy::Render
	void TexSet::UploadAll(SprSet* parentSprSet)
	{
		for (auto& tex : Textures)
		{
			const char* debugName = nullptr;
			const auto signature = tex->GetSignature();

#if COMFY_D3D11_DEBUG_NAMES
			char debugNameBuffer[128];
			sprintf_s(debugNameBuffer, "%s %s: %s",
				(signature == TxpSig::Texture2D) ? "Texture2D" : "CubeMap",
				(parentSprSet != nullptr) ? parentSprSet->Name.c_str() : "TexSet",
				tex->GetName().data());

			debugName = debugNameBuffer;
#endif

			if (signature == TxpSig::Texture2D)
				tex->GPU_Texture2D = GPU::MakeTexture2D(*tex, debugName);
			else if (signature == TxpSig::CubeMap)
				tex->GPU_CubeMap = GPU::MakeCubeMap(*tex, debugName);
		}
	}
	*/

	std::unique_ptr<TexSet> TexSet::LoadSetTextureIDs(std::string_view filePath, const ObjSet* objSet)
	{
		auto texSet = IO::File::Load<TexSet>(filePath);
		if (texSet != nullptr)
		{
			if (objSet != nullptr)
				texSet->SetTextureIDs(*objSet);
		}
		return texSet;
	}
}
