#include "TxpSet.h"
#include "../FileInterface.h"
#include "../BinaryReader.h"

namespace FileSystem
{
	void TxpSet::Read(BinaryReader& reader)
	{
		TxpSet* txpSet = this;
		int64_t baseAddress = reader.GetPosition();

		txpSet->Signature = reader.Read<TxpSig>();
	
		uint32_t count = reader.ReadUInt32();
		uint32_t packedCount = reader.ReadUInt32();

		Textures.reserve(count);
		for (uint32_t i = 0; i < count; i++)
		{
			Textures.emplace_back();
			Texture* texture = &Textures.back();

			void* textureAddress = (void*)((int64_t)reader.ReadPtr() + baseAddress);
			reader.ReadAt(textureAddress, [&texture](BinaryReader& reader)
			{
				int64_t textureBaseAddress = reader.GetPosition();

				texture->Signature = reader.Read<TxpSig>();

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
						mipMap->Width = reader.ReadInt32();
						mipMap->Height = reader.ReadInt32();
						mipMap->Format = reader.Read<TextureFormat>();
						mipMap->Index = reader.ReadUInt32();
						
						uint32_t dataSize = reader.ReadUInt32();
						mipMap->Data = std::make_shared<std::vector<uint8_t>>(dataSize);
						mipMap->Data->resize(dataSize);

						reader.Read(mipMap->Data.get()->data(), dataSize);
					}
				});

			});
		}
	}
}