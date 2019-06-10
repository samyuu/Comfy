#include "SprSet.h"
#include "../BinaryReader.h"

namespace File
{
	void SprSet::Read(BinaryReader& reader)
	{
		SprSet* sprSet = this;

		sprSet->Signature = reader.ReadUInt32();

		void* txpSetOffset = reader.ReadPtr();
		uint32_t txpCount = reader.ReadUInt32();

		reader.ReadAt(txpSetOffset, [&sprSet](BinaryReader& reader) 
		{
			sprSet->TxpSet.Read(reader);
		});

		uint32_t spritesCount = reader.ReadUInt32();
		void* spritesOffset = reader.ReadPtr();

		reader.ReadAt(spritesOffset, [spritesCount, &sprSet](BinaryReader& reader)
		{
			sprSet->Sprites.reserve(spritesCount);
			for (uint32_t i = 0; i < spritesCount; i++)
			{
				sprSet->Sprites.emplace_back();
				Sprite* sprite = &sprSet->Sprites.back();

				sprite->TextureIndex = reader.ReadInt32();
				sprite->Unknown = reader.ReadFloat();
				sprite->TexelX = reader.ReadFloat();
				sprite->TexelY = reader.ReadFloat();
				sprite->TexelWidth = reader.ReadFloat();
				sprite->TexelHeight = reader.ReadFloat();
				sprite->PixelX = reader.ReadFloat();
				sprite->PixelY = reader.ReadFloat();
				sprite->PixelWidth = reader.ReadFloat();
				sprite->PixelHeight = reader.ReadFloat();
			}
		});

		reader.ReadAt(reader.ReadPtr(), [&sprSet](BinaryReader& reader)
		{
			for (Texture &texture : sprSet->TxpSet.Textures)
				texture.Name = reader.ReadStrPtr();
		});

		reader.ReadAt(reader.ReadPtr(), [&sprSet](BinaryReader& reader)
		{
			for (Sprite &sprite : sprSet->Sprites)
				sprite.Name = reader.ReadStrPtr();
		});

		reader.ReadAt(reader.ReadPtr(), [&sprSet](BinaryReader& reader)
		{
			for (Sprite &sprite : sprSet->Sprites)
			{
				sprite.ExtraData.Zero = reader.ReadUInt32();
				sprite.ExtraData.Low = reader.ReadUInt32();
			}
		});
	}
}