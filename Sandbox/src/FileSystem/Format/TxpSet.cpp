#include "TxpSet.h"
#include "FileSystem/FileInterface.h"
#include "FileSystem/BinaryReader.h"
#include "Graphics/Texture/Texture2D.h"
#include <assert.h>

namespace FileSystem
{
	static TxpSig ReadTxpSig(BinaryReader& reader)
	{
		return { reader.ReadChar(), reader.ReadChar(), reader.ReadChar(), static_cast<TxpType>(reader.ReadChar()) };
	}

	void TxpSet::Read(BinaryReader& reader)
	{
		TxpSet* txpSet = this;
		int64_t baseAddress = reader.GetPosition();

		txpSet->Signature = ReadTxpSig(reader);
		assert(txpSet->Signature.Type == TxpType_TxpSet);

		uint32_t textureCount = reader.ReadUInt32();
		uint32_t packedCount = reader.ReadUInt32();

		Textures.reserve(textureCount);
		for (uint32_t i = 0; i < textureCount; i++)
		{
			Textures.push_back(MakeUnique<Texture>());
			Texture* texture = Textures.back().get();

			void* textureAddress = (void*)((int64_t)reader.ReadPtr() + baseAddress);
			reader.ReadAt(textureAddress, [&texture](BinaryReader& reader)
			{
				int64_t textureBaseAddress = reader.GetPosition();

				texture->Signature = ReadTxpSig(reader);
				assert(texture->Signature.Type == TxpType_Texture || texture->Signature.Type == TxpType_TextureAlt);

				uint32_t mipMapCount = reader.ReadUInt32();
				uint32_t packedInfo = reader.ReadUInt32();

				void* mipMapsAddress = (void*)((int64_t)reader.ReadPtr() + textureBaseAddress);
				reader.ReadAt(mipMapsAddress, [mipMapCount, &texture](BinaryReader& reader)
				{
					texture->MipMaps.reserve(mipMapCount);
					for (uint32_t i = 0; i < mipMapCount; i++)
					{
						texture->MipMaps.push_back(MakeRefPtr<MipMap>());
						MipMap* mipMap = texture->MipMaps.back().get();

						mipMap->Signature = ReadTxpSig(reader);
						assert(mipMap->Signature.Type == TxpType_MipMap);

						mipMap->Width = reader.ReadInt32();
						mipMap->Height = reader.ReadInt32();
						mipMap->Format = static_cast<TextureFormat>(reader.ReadInt32());
						mipMap->Index = reader.ReadUInt32();

						uint32_t dataSize = reader.ReadUInt32();
						mipMap->Data.resize(dataSize);

						// uint8_t* allocationTestBuffer = new uint8_t[dataSize];
						// reader.Read(allocationTestBuffer, dataSize);
						// //reader.SetPosition(reader.GetPosition() + dataSize);

						reader.Read(mipMap->Data.data(), dataSize);
					}
				});

			});
		}
	}

	static void ParseTexture(const uint8_t* buffer, Texture* texture)
	{
		texture->Signature = *(TxpSig*)(buffer + 0);
		uint32_t mipMapCount = *(uint32_t*)(buffer + 4);
		uint32_t packedInfo = *(uint32_t*)(buffer + 8);
		uint32_t offset = *(uint32_t*)(buffer + 12);
		const uint8_t* mipMapBuffer = buffer + offset;

		texture->MipMaps.reserve(mipMapCount);
		for (uint32_t i = 0; i < mipMapCount; i++)
		{
			texture->MipMaps.push_back(MakeRefPtr<MipMap>());
			MipMap* mipMap = texture->MipMaps.back().get();

			mipMap->Signature = *(TxpSig*)(mipMapBuffer + 0);
			mipMap->Width = *(uint32_t*)(mipMapBuffer + 4);
			mipMap->Height = *(uint32_t*)(mipMapBuffer + 8);
			mipMap->Format = *(TextureFormat*)(mipMapBuffer + 12);
			uint32_t index = *(uint32_t*)(mipMapBuffer + 16);

			mipMap->DataPointerSize = *(uint32_t*)(mipMapBuffer + 20);
			mipMap->DataPointer = (mipMapBuffer + 24);

			mipMapBuffer += 24 + mipMap->DataPointerSize;
		}
	}

	void TxpSet::Parse(const uint8_t* buffer)
	{
		TxpSet* txpSet = this;

		txpSet->Signature = *(TxpSig*)(buffer + 0);
		uint32_t textureCount = *(uint32_t*)(buffer + 4);
		uint32_t packedCount = *(uint32_t*)(buffer + 8);
		uint32_t* offsets = (uint32_t*)(buffer + 12);

		Textures.reserve(textureCount);
		for (uint32_t i = 0; i < textureCount; i++)
		{
			Textures.push_back(MakeUnique<Texture>());
			Texture* texture = Textures.back().get();

			uint32_t offset = offsets[i];
			ParseTexture(buffer + offset, texture);
		}
	}

	void TxpSet::UploadAll()
	{
		for (int i = 0; i < Textures.size(); i++)
		{
			Texture* texture = Textures[i].get();
			texture->GraphicsTexture = MakeRefPtr<Graphics::Texture2D>();
			texture->GraphicsTexture->Upload(texture);
		}
	}

	void Texture::UploadTexture2D()
	{
		assert(GraphicsTexture == nullptr);

		GraphicsTexture = MakeRefPtr<Graphics::Texture2D>();
		GraphicsTexture->Upload(this);
	}
}