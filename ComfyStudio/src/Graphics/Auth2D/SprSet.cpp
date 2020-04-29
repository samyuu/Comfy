#include "SprSet.h"
#include "IO/BinaryReader.h"
#include "IO/BinaryWriter.h"

using namespace Comfy::IO;

namespace Comfy::Graphics
{
	vec2 Spr::GetSize() const
	{
		return vec2(PixelRegion.z, PixelRegion.w);
	}

	void SprSet::Write(BinaryWriter& writer)
	{
		writer.WriteU32(Flags);

		const FileAddr texSetPtrAddress = writer.GetPosition();
		writer.WriteU32(0x00000000);
		writer.WriteU32((TexSet != nullptr) ? static_cast<u32>(TexSet->Textures.size()) : 0);

		writer.WriteU32(static_cast<u32>(Sprites.size()));
		writer.WritePtr([&](BinaryWriter& writer)
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

		writer.WritePtr([&](BinaryWriter& writer)
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

		writer.WritePtr([&](BinaryWriter& writer)
		{
			for (const auto& sprite : Sprites)
				writer.WriteStrPtr(sprite.Name);
		});

		writer.WritePtr([&](BinaryWriter& writer)
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

			writer.SetPosition(texSetPtrAddress);
			writer.WritePtr(texSetPtr);
		}
	}

	void SprSet::Parse(const u8* buffer, size_t bufferSize)
	{
		SprSet* sprSet = this;

		sprSet->Flags = *(u32*)(buffer + 0);
		u32 texSetOffset = *(u32*)(buffer + 4);
		u32 textureCount = *(u32*)(buffer + 8);
		u32 spritesCount = *(u32*)(buffer + 12);
		u32 spritesOffset = *(u32*)(buffer + 16);
		u32 textureNamesOffset = *(u32*)(buffer + 20);
		u32 spriteNamesOffset = *(u32*)(buffer + 24);
		u32 spriteExtraDataOffset = *(u32*)(buffer + 28);

		if (texSetOffset != 0)
		{
			sprSet->TexSet = MakeUnique<Graphics::TexSet>();
			sprSet->TexSet->Parse(buffer + texSetOffset, bufferSize - texSetOffset);
		}

		if (spritesOffset != 0)
		{
			const u8* spritesBuffer = buffer + spritesOffset;

			sprSet->Sprites.resize(spritesCount);
			for (u32 i = 0; i < spritesCount; i++)
			{
				Spr* sprite = &sprSet->Sprites[i];

				sprite->TextureIndex = *(i32*)(spritesBuffer + 0);
				sprite->Rotate = *(i32*)(spritesBuffer + 4);
				sprite->TexelRegion = *(vec4*)(spritesBuffer + 8);
				sprite->PixelRegion = *(vec4*)(spritesBuffer + 24);
				spritesBuffer += 40;
			}
		}

		if (textureNamesOffset != 0)
		{
			const u8* textureNamesOffsetBuffer = buffer + textureNamesOffset;

			for (u32 i = 0; i < textureCount; i++)
			{
				u32 nameOffset = ((u32*)textureNamesOffsetBuffer)[i];
				sprSet->TexSet->Textures[i]->Name = (const char*)(buffer + nameOffset);
			}
		}

		if (spriteNamesOffset != 0)
		{
			const u8* spriteNamesOffsetBuffer = buffer + spriteNamesOffset;

			for (u32 i = 0; i < spritesCount; i++)
			{
				u32 nameOffset = ((u32*)spriteNamesOffsetBuffer)[i];
				sprSet->Sprites[i].Name = (const char*)(buffer + nameOffset);
			}
		}

		if (spriteExtraDataOffset != 0)
		{
			const u8* extraDataBuffer = buffer + spriteExtraDataOffset;

			for (auto& sprite : sprSet->Sprites)
			{
				sprite.Extra.Flags = *((u32*)extraDataBuffer + 0);
				sprite.Extra.ScreenMode = *((ScreenMode*)extraDataBuffer + 4);
				extraDataBuffer += 8;
			}
		}
	}
}
