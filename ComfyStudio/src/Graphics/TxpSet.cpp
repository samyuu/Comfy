#include "TxpSet.h"
#include "SprSet.h"
#include "FileSystem/FileInterface.h"
#include "FileSystem/BinaryReader.h"
#include "Graphics/Direct3D/D3D_Texture2D.h"

using namespace FileSystem;

namespace Graphics
{
	const std::vector<MipMap>& Txp::GetMipMaps(uint32_t arrayIndex) const
	{
		return MipMapsArray[arrayIndex];
	}

	ivec2 Txp::GetSize() const
	{
		if (MipMapsArray.size() < 1 || MipMapsArray.front().size() < 1)
			return ivec2(0, 0);

		return MipMapsArray.front().front().Size;
	}

	TextureFormat Txp::GetFormat() const
	{
		if (MipMapsArray.size() < 1 || MipMapsArray.front().size() < 1)
			return TextureFormat::Unknown;

		return MipMapsArray.front().front().Format;
	}

	void TxpSet::Parse(const uint8_t* buffer)
	{
		TxpSet* txpSet = this;

		txpSet->Signature = *(TxpSig*)(buffer + 0);
		uint32_t textureCount = *(uint32_t*)(buffer + 4);
		uint32_t packedCount = *(uint32_t*)(buffer + 8);
		uint32_t* offsets = (uint32_t*)(buffer + 12);

		assert(txpSet->Signature.Type == TxpSig::TxpSet);

		Txps.resize(textureCount);
		for (uint32_t i = 0; i < textureCount; i++)
			ParseTxp(buffer + offsets[i], &Txps[i]);
	}

	void TxpSet::UploadAll(SprSet* parentSprSet)
	{
		for (auto& txp : Txps)
		{
			if (txp.Signature.Type != TxpSig::Texture2D)
				continue;

			txp.Texture2D = MakeUnique<D3D_ImmutableTexture2D>(txp);
			D3D_SetObjectDebugName(txp.Texture2D->GetTexture(), "%s: %s", (parentSprSet != nullptr) ? parentSprSet->Name.c_str() : "TxpSet", txp.Name.empty() ? "???" : txp.Name.c_str());
		}
	}

	void TxpSet::ParseTxp(const uint8_t* buffer, Txp* txp)
	{
		txp->Signature = *(TxpSig*)(buffer + 0);
		uint32_t mipMapCount = *(uint32_t*)(buffer + 4);
		txp->MipLevels = *(uint8_t*)(buffer + 8);
		txp->ArraySize = *(uint8_t*)(buffer + 9);

		uint32_t* offsets = (uint32_t*)(buffer + 12);
		const uint8_t* mipMapBuffer = buffer + *offsets;
		++offsets;

		assert(txp->Signature.Type == TxpSig::Texture2D || txp->Signature.Type == TxpSig::CubeMap || txp->Signature.Type == TxpSig::Rectangle);
		assert(mipMapCount == txp->MipLevels * txp->ArraySize);

		txp->MipMapsArray.resize(txp->ArraySize);

		for (auto& mipMaps : txp->MipMapsArray)
		{
			mipMaps.resize(txp->MipLevels);

			for (auto& mipMap : mipMaps)
			{
				mipMap.Signature = *(TxpSig*)(mipMapBuffer + 0);
				mipMap.Size = *(ivec2*)(mipMapBuffer + 4);
				mipMap.Format = *(TextureFormat*)(mipMapBuffer + 12);

				mipMap.MipIndex = *(uint8_t*)(mipMapBuffer + 16);
				mipMap.ArrayIndex = *(uint8_t*)(mipMapBuffer + 17);

				mipMap.DataPointerSize = *(uint32_t*)(mipMapBuffer + 20);
				mipMap.DataPointer = (mipMapBuffer + 24);

				mipMapBuffer = buffer + *offsets;
				++offsets;
			}
		}
	}
}
