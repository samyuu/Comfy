#include "SprSet.h"
#include "IO/Stream/Manipulator/StreamReader.h"
#include "IO/Stream/Manipulator/StreamWriter.h"

using namespace Comfy::IO;

namespace Comfy::Graphics
{
	vec2 Spr::GetSize() const
	{
		return vec2(PixelRegion.z, PixelRegion.w);
	}

	StreamResult SprSet::Read(StreamReader& reader)
	{
		const auto baseHeader = SectionHeader::TryRead(reader, SectionSignature::SPRC);
		SectionHeader::ScanPOFSectionsSetPointerMode(reader);

		if (baseHeader.has_value())
		{
			reader.SetEndianness(baseHeader->Endianness);
			reader.Seek(baseHeader->StartOfSubSectionAddress());

			if (reader.GetPointerMode() == PtrMode::Mode64Bit)
				reader.PushBaseOffset();
		}

		Flags = reader.ReadU32();
		const auto texSetOffset = reader.ReadPtr_32();
		const auto textureCount = reader.ReadSize_32();
		const auto spriteCount = reader.ReadU32();
		const auto spritesOffset = reader.ReadPtr();
		const auto textureNamesOffset = reader.ReadPtr();
		const auto spriteNamesOffset = reader.ReadPtr();
		const auto spriteExtraDataOffset = reader.ReadPtr();

		TexSet = std::make_unique<Graphics::TexSet>();
		if (textureCount > 0)
		{
			auto streamResult = StreamResult::Success;
			if (reader.GetHasSections())
			{
				const auto texSetStartOffset = baseHeader->EndOfSectionAddress();
				reader.ReadAt(texSetStartOffset, [&](StreamReader& reader) { streamResult = TexSet->Read(reader); });
			}
			else
			{
				if (!reader.IsValidPointer(texSetOffset))
					return StreamResult::BadPointer;

				reader.ReadAtOffsetAware(texSetOffset, [&](StreamReader& reader) { streamResult = TexSet->Read(reader); });
			}
			if (streamResult != StreamResult::Success)
				return streamResult;

			if (reader.IsValidPointer(textureNamesOffset) && TexSet->Textures.size() == textureCount)
			{
				reader.ReadAtOffsetAware(textureNamesOffset, [&](StreamReader& reader)
				{
					for (auto& texture : this->TexSet->Textures)
						texture->Name = reader.ReadStrPtrOffsetAware();
				});
			}
		}

		if (spriteCount > 0)
		{
			if (!reader.IsValidPointer(spritesOffset) || !reader.IsValidPointer(spriteExtraDataOffset))
				return StreamResult::BadPointer;

			auto streamResult = StreamResult::Success;
			reader.ReadAtOffsetAware(spritesOffset, [&](StreamReader& reader)
			{
				Sprites.reserve(spriteCount);
				for (size_t i = 0; i < spriteCount; i++)
				{
					auto& sprite = Sprites.emplace_back();
					sprite.TextureIndex = reader.ReadI32();
					sprite.Rotate = reader.ReadI32();
					sprite.TexelRegion = reader.ReadV4();
					sprite.PixelRegion = reader.ReadV4();
				}
			});
			if (streamResult != StreamResult::Success)
				return streamResult;

			if (reader.IsValidPointer(spriteNamesOffset) && Sprites.size() == spriteCount)
			{
				reader.ReadAtOffsetAware(spriteNamesOffset, [&](StreamReader& reader)
				{
					for (auto& sprite : Sprites)
						sprite.Name = reader.ReadStrPtrOffsetAware();
				});
			}

			reader.ReadAtOffsetAware(spriteExtraDataOffset, [&](StreamReader& reader)
			{
				for (auto& sprite : Sprites)
				{
					sprite.Extra.Flags = reader.ReadU32();
					sprite.Extra.ScreenMode = static_cast<ScreenMode>(reader.ReadU32());
				}
			});
		}

		if (baseHeader.has_value() && reader.GetPointerMode() == PtrMode::Mode64Bit)
			reader.PopBaseOffset();

		return StreamResult::Success;
	}

	StreamResult SprSet::Write(StreamWriter& writer)
	{
		writer.WriteU32(Flags);

		const FileAddr texSetPtrAddress = writer.GetPosition();
		writer.WriteU32(0x00000000);
		writer.WriteU32((TexSet != nullptr) ? static_cast<u32>(TexSet->Textures.size()) : 0);

		writer.WriteU32(static_cast<u32>(Sprites.size()));
		writer.WriteFuncPtr([&](StreamWriter& writer)
		{
			for (const auto& sprite : Sprites)
			{
				writer.WriteI32(sprite.TextureIndex);
				writer.WriteI32(sprite.Rotate);
				writer.WriteF32(sprite.TexelRegion.x);
				writer.WriteF32(sprite.TexelRegion.y);
				writer.WriteF32(sprite.TexelRegion.z);
				writer.WriteF32(sprite.TexelRegion.w);
				writer.WriteF32(sprite.PixelRegion.x);
				writer.WriteF32(sprite.PixelRegion.y);
				writer.WriteF32(sprite.PixelRegion.z);
				writer.WriteF32(sprite.PixelRegion.w);
			}
		});

		writer.WriteFuncPtr([&](StreamWriter& writer)
		{
			if (this->TexSet == nullptr)
				return;

			for (const auto& texture : this->TexSet->Textures)
			{
				if (texture->Name.has_value())
					writer.WriteStrPtr(texture->Name.value());
				else
					writer.WritePtr(FileAddr::NullPtr);
			}
		});

		writer.WriteFuncPtr([&](StreamWriter& writer)
		{
			for (const auto& sprite : Sprites)
				writer.WriteStrPtr(sprite.Name);
		});

		writer.WriteFuncPtr([&](StreamWriter& writer)
		{
			for (const auto& sprite : Sprites)
			{
				writer.WriteU32(sprite.Extra.Flags);
				writer.WriteU32(static_cast<u32>(sprite.Extra.ScreenMode));
			}
		});

		writer.FlushPointerPool();
		writer.WriteAlignmentPadding(16);

		writer.FlushStringPointerPool();
		writer.WriteAlignmentPadding(16);

		if (TexSet != nullptr)
		{
			const FileAddr texSetPtr = writer.GetPosition();
			TexSet->Write(writer);

			writer.Seek(texSetPtrAddress);
			writer.WritePtr(texSetPtr);
		}

		return StreamResult::Success;
	}
}
