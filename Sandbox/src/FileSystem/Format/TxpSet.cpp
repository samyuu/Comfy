#include "TxpSet.h"
#include "FileSystem/FileInterface.h"
#include "FileSystem/BinaryReader.h"
#include <assert.h>

namespace FileSystem
{
	void TxpSet::Read(BinaryReader& reader)
	{
		TxpSet* txpSet = this;
		int64_t baseAddress = reader.GetPosition();

		txpSet->Signature = reader.Read<TxpSig>();
		assert(txpSet->Signature.Type == TxpType_TxpSet);

		uint32_t count = reader.ReadUInt32();
		uint32_t packedCount = reader.ReadUInt32();

		Textures.reserve(count);
		for (uint32_t i = 0; i < count; i++)
		{
			Textures.push_back(std::make_unique<Texture>());
			Texture* texture = Textures.back().get();

			void* textureAddress = (void*)((int64_t)reader.ReadPtr() + baseAddress);
			reader.ReadAt(textureAddress, [&texture](BinaryReader& reader)
			{
				int64_t textureBaseAddress = reader.GetPosition();

				texture->Signature = reader.Read<TxpSig>();
				assert(texture->Signature.Type == TxpType_Texture);

				uint32_t mipMapCount = reader.ReadUInt32();
				uint32_t packedInfo = reader.ReadUInt32();
				
				void* mipMapsAddress = (void*)((int64_t)reader.ReadPtr() + textureBaseAddress);
				reader.ReadAt(mipMapsAddress, [mipMapCount, &texture](BinaryReader& reader)
				{
					texture->MipMaps.reserve(mipMapCount);
					for (uint32_t i = 0; i < mipMapCount; i++)
					{
						texture->MipMaps.push_back(std::make_shared<MipMap>());
						MipMap* mipMap = texture->MipMaps.back().get();

						mipMap->Signature = reader.Read<TxpSig>();
						assert(mipMap->Signature.Type == TxpType_MipMap);

						mipMap->Width = reader.ReadInt32();
						mipMap->Height = reader.ReadInt32();
						mipMap->Format = reader.Read<TextureFormat>();
						mipMap->Index = reader.ReadUInt32();
						
						uint32_t dataSize = reader.ReadUInt32();
						mipMap->Data.resize(dataSize);

						reader.Read(mipMap->Data.data(), dataSize);
					}
				});

			});
		}
	}
}