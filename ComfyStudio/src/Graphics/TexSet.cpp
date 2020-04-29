#include "TexSet.h"
#include "Auth2D/SprSet.h"
#include "Auth3D/ObjSet.h"
#include "IO/FileInterface.h"
#include "IO/Stream/Manipulator/StreamReader.h"
#include "IO/Stream/Manipulator/StreamWriter.h"
#include "IO/FileReader.h"

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

	void TexSet::Write(StreamWriter& writer)
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
	}

	void TexSet::Parse(const u8* buffer, size_t bufferSize)
	{
		TexSet& texSet = *this;

		TxpSig signature = *(TxpSig*)(buffer + 0);
		u32 textureCount = *(u32*)(buffer + 4);
		u32 packedCount = *(u32*)(buffer + 8);
		u32* offsets = (u32*)(buffer + 12);

		assert(signature == TxpSig::TexSet);

		Textures.reserve(textureCount);
		for (u32 i = 0; i < textureCount; i++)
		{
			Textures.push_back(MakeRef<Tex>());
			ParseTex(buffer + offsets[i], *Textures[i]);
		}
	}

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

	void TexSet::SetTextureIDs(const ObjSet& objSet)
	{
		const auto& textureIDs = objSet.TextureIDs;
		assert(textureIDs.size() <= Textures.size());

		for (size_t i = 0; i < textureIDs.size(); i++)
			Textures[i]->ID = textureIDs[i];
	}

	UniquePtr<TexSet> TexSet::MakeUniqueReadParseUpload(std::string_view filePath, const ObjSet* objSet)
	{
		std::vector<u8> fileContent;
		FileReader::ReadEntireFile(filePath, &fileContent);

		if (fileContent.empty())
			return nullptr;

		auto texSet = MakeUnique<TexSet>();
		{
			texSet->Parse(fileContent.data(), fileContent.size());
			texSet->UploadAll(nullptr);

			if (objSet != nullptr)
				texSet->SetTextureIDs(*objSet);
		}
		return texSet;
	}

	void TexSet::ParseTex(const u8* buffer, Tex& tex)
	{
		TxpSig signature = *(TxpSig*)(buffer + 0);
		u32 mipMapCount = *(u32*)(buffer + 4);
		u8 mipLevels = *(u8*)(buffer + 8);
		u8 arraySize = *(u8*)(buffer + 9);

		u32* offsets = (u32*)(buffer + 12);
		const u8* mipMapBuffer = buffer + *offsets;
		++offsets;

		assert(signature == TxpSig::Texture2D || signature == TxpSig::CubeMap || signature == TxpSig::Rectangle);
		// assert(mipMapCount == tex.MipLevels * tex.ArraySize);

		tex.MipMapsArray.resize(arraySize);

		for (auto& mipMaps : tex.MipMapsArray)
		{
			mipMaps.resize(mipLevels);

			for (auto& mipMap : mipMaps)
			{
				TxpSig signature = *(TxpSig*)(mipMapBuffer + 0);
				mipMap.Size = *(ivec2*)(mipMapBuffer + 4);
				mipMap.Format = *(TextureFormat*)(mipMapBuffer + 12);

				u8 mipIndex = *(u8*)(mipMapBuffer + 16);
				u8 arrayIndex = *(u8*)(mipMapBuffer + 17);

				mipMap.DataSize = *(u32*)(mipMapBuffer + 20);
				mipMap.Data = MakeUnique<u8[]>(mipMap.DataSize);
				std::memcpy(mipMap.Data.get(), mipMapBuffer + 24, mipMap.DataSize);

				mipMapBuffer = buffer + *offsets;
				++offsets;
			}
		}
	}
}
