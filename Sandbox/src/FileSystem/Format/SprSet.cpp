#include "SprSet.h"
#include "FileSystem/BinaryReader.h"

namespace FileSystem
{
	void SprSet::Read(BinaryReader& reader)
	{
		SprSet* sprSet = this;

		sprSet->Signature = reader.ReadUInt32();

		void* txpSetOffset = reader.ReadPtr();
		uint32_t textureCount = reader.ReadUInt32();

		if (txpSetOffset != nullptr)
		{
			reader.ReadAt(txpSetOffset, [&sprSet](BinaryReader& reader)
			{
				sprSet->TxpSet = std::make_unique<FileSystem::TxpSet>();
				sprSet->TxpSet->Read(reader);
			});
		}

		uint32_t spritesCount = reader.ReadUInt32();
		void* spritesOffset = reader.ReadPtr();

		if (spritesOffset != nullptr)
		{
			reader.ReadAt(spritesOffset, [spritesCount, &sprSet](BinaryReader& reader)
			{
				sprSet->Sprites.reserve(spritesCount);
				for (uint32_t i = 0; i < spritesCount; i++)
				{
					sprSet->Sprites.emplace_back();
					Sprite* sprite = &sprSet->Sprites.back();

					sprite->TextureIndex = reader.ReadInt32();
					sprite->Unknown = reader.ReadFloat();
					sprite->TexelRegion = reader.Read<vec4>();
					sprite->PixelRegion = reader.Read<vec4>();
				}
			});
		}

		void* textureNamesOffset = reader.ReadPtr();
		if (textureNamesOffset != nullptr)
		{
			reader.ReadAt(textureNamesOffset, [&sprSet](BinaryReader& reader)
			{
				for (auto &texture : sprSet->TxpSet->Textures)
					texture->Name = reader.ReadStrPtr();
			});
		}

		void* spriteNamesOffset = reader.ReadPtr();
		if (spriteNamesOffset != nullptr)
		{
			reader.ReadAt(spriteNamesOffset, [&sprSet](BinaryReader& reader)
			{
				for (Sprite &sprite : sprSet->Sprites)
					sprite.Name = reader.ReadStrPtr();
			});
		}

		void* spriteExtraDataOffset = reader.ReadPtr();
		if (spriteExtraDataOffset != nullptr)
		{
			reader.ReadAt(spriteExtraDataOffset, [&sprSet](BinaryReader& reader)
			{
				for (Sprite &sprite : sprSet->Sprites)
				{
					sprite.ExtraData.Zero = reader.ReadUInt32();
					sprite.ExtraData.Low = reader.ReadUInt32();
				}
			});
		}
	}

	void SprSet::Parse(uint8_t* buffer)
	{
		SprSet* sprSet = this;

		sprSet->Signature = *(uint32_t*)(buffer + 0);
		uint32_t txpSetOffset = *(uint32_t*)(buffer + 4);
		uint32_t textureCount = *(uint32_t*)(buffer + 8);
		uint32_t spritesCount = *(uint32_t*)(buffer + 12);
		uint32_t spritesOffset = *(uint32_t*)(buffer + 16);
		uint32_t textureNamesOffset = *(uint32_t*)(buffer + 20);
		uint32_t spriteNamesOffset = *(uint32_t*)(buffer + 24);
		uint32_t spriteExtraDataOffset = *(uint32_t*)(buffer + 28);

		if (txpSetOffset != 0)
		{
			sprSet->TxpSet = std::make_unique<FileSystem::TxpSet>();
			sprSet->TxpSet->Parse(buffer + txpSetOffset);
		}

		if (spritesOffset != 0)
		{
			uint8_t* spritesBuffer = buffer + spritesOffset;

			sprSet->Sprites.resize(spritesCount);
			for (uint32_t i = 0; i < spritesCount; i++)
			{
				Sprite* sprite = &sprSet->Sprites[i];

				sprite->TextureIndex = *(uint32_t*)(spritesBuffer + 0);
				sprite->Unknown = *(float*)(spritesBuffer + 4);
				sprite->TexelRegion = *(vec4*)(spritesBuffer + 8);
				sprite->PixelRegion = *(vec4*)(spritesBuffer + 24);
				spritesBuffer += 40;
			}
		}

		if (textureNamesOffset != 0)
		{
			uint8_t* textureNamesOffsetBuffer = buffer + textureNamesOffset;

			for (uint32_t i = 0; i < textureCount; i++)
			{
				uint32_t nameOffset = ((uint32_t*)textureNamesOffsetBuffer)[i];
				char* name = (char*)(buffer + nameOffset);
				sprSet->TxpSet->Textures[i]->Name = std::string(name);
			}
		}

		if (spriteNamesOffset != 0)
		{
			uint8_t* spriteNamesOffsetBuffer = buffer + spriteNamesOffset;

			for (uint32_t i = 0; i < spritesCount; i++)
			{
				uint32_t nameOffset = ((uint32_t*)spriteNamesOffsetBuffer)[i];
				char* name = (char*)(buffer + nameOffset);
				sprSet->Sprites[i].Name = std::string(name);
			}
		}

		if (spriteExtraDataOffset != 0)
		{
			uint8_t* extraDataBuffer = buffer + spriteExtraDataOffset;

			for (Sprite &sprite : sprSet->Sprites)
			{
				sprite.ExtraData.Zero = *((uint32_t*)extraDataBuffer + 0);
				sprite.ExtraData.Low = *((uint32_t*)extraDataBuffer + 4);
				extraDataBuffer += 8;
			}
		}
	}
}