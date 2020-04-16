#include "TexSet.h"
#include "Auth2D/SprSet.h"
#include "Auth3D/ObjSet.h"
#include "FileSystem/FileInterface.h"
#include "FileSystem/BinaryReader.h"
#include "FileSystem/BinaryWriter.h"
#include "FileSystem/FileReader.h"

using namespace Comfy::FileSystem;

namespace Comfy::Graphics
{
	const std::vector<TexMipMap>& Tex::GetMipMaps(uint32_t arrayIndex) const
	{
		return MipMapsArray[arrayIndex];
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

	void TexSet::Write(BinaryWriter& writer)
	{
		const uint32_t textureCount = static_cast<uint32_t>(Textures.size());
		constexpr uint32_t packedMask = 0x01010100;

		const FileAddr setBaseAddress = writer.GetPosition();
		writer.WriteU32(static_cast<uint32_t>(Signature));
		writer.WriteU32(textureCount);
		writer.WriteU32(textureCount | packedMask);

		for (const auto& texture : Textures)
		{
			writer.WritePtr([&](BinaryWriter& writer)
			{
				const FileAddr texBaseAddress = writer.GetPosition();
				const uint8_t arraySize = static_cast<uint8_t>(texture->MipMapsArray.size());
				const uint8_t mipLevels = (arraySize > 0) ? static_cast<uint8_t>(texture->MipMapsArray.front().size()) : 0;

				writer.WriteU32(static_cast<uint32_t>(texture->Signature));
				writer.WriteU32(arraySize * mipLevels);
				writer.WriteU8(mipLevels);
				writer.WriteU8(arraySize);
				writer.WriteU8(0x01);
				writer.WriteU8(0x01);

				for (uint8_t arrayIndex = 0; arrayIndex < arraySize; arrayIndex++)
				{
					for (uint8_t mipIndex = 0; mipIndex < mipLevels; mipIndex++)
					{
						writer.WritePtr([arrayIndex, mipIndex, &texture](BinaryWriter& writer)
						{
							const auto& mipMap = texture->MipMapsArray[arrayIndex][mipIndex];
							writer.WriteU32(static_cast<uint32_t>(mipMap.Signature));
							writer.WriteI32(mipMap.Size.x);
							writer.WriteI32(mipMap.Size.y);
							writer.WriteU32(static_cast<uint32_t>(mipMap.Format));
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

	void TexSet::Parse(const uint8_t* buffer, size_t bufferSize)
	{
		TexSet& texSet = *this;

		texSet.Signature = *(TxpSig*)(buffer + 0);
		uint32_t textureCount = *(uint32_t*)(buffer + 4);
		uint32_t packedCount = *(uint32_t*)(buffer + 8);
		uint32_t* offsets = (uint32_t*)(buffer + 12);

		assert(texSet.Signature == TxpSig::TexSet);

		Textures.reserve(textureCount);
		for (uint32_t i = 0; i < textureCount; i++)
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

#if COMFY_D3D11_DEBUG_NAMES
			char debugNameBuffer[128];
			sprintf_s(debugNameBuffer, "%s %s: %s",
				(tex->Signature == TxpSig::Texture2D) ? "Texture2D" : "CubeMap",
				(parentSprSet != nullptr) ? parentSprSet->Name.c_str() : "TexSet",
				tex->GetName().data());

			debugName = debugNameBuffer;
#endif

			if (tex->Signature == TxpSig::Texture2D)
				tex->GPU_Texture2D = GPU::MakeTexture2D(*tex, debugName);
			else if (tex->Signature == TxpSig::CubeMap)
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
		std::vector<uint8_t> fileContent;
		FileReader::ReadEntireFile(filePath, &fileContent);

		if (fileContent.empty())
			return nullptr;

		auto texSet = MakeUnique<TexSet>();;
		{
			texSet->Parse(fileContent.data(), fileContent.size());
			texSet->UploadAll(nullptr);

			if (objSet != nullptr)
				texSet->SetTextureIDs(*objSet);
		}
		return texSet;
	}

	void TexSet::ParseTex(const uint8_t* buffer, Tex& tex)
	{
		tex.Signature = *(TxpSig*)(buffer + 0);
		uint32_t mipMapCount = *(uint32_t*)(buffer + 4);
		uint8_t mipLevels = *(uint8_t*)(buffer + 8);
		uint8_t arraySize = *(uint8_t*)(buffer + 9);

		uint32_t* offsets = (uint32_t*)(buffer + 12);
		const uint8_t* mipMapBuffer = buffer + *offsets;
		++offsets;

		assert(tex.Signature == TxpSig::Texture2D || tex.Signature == TxpSig::CubeMap || tex.Signature == TxpSig::Rectangle);
		// assert(mipMapCount == tex.MipLevels * tex.ArraySize);

		tex.MipMapsArray.resize(arraySize);

		for (auto& mipMaps : tex.MipMapsArray)
		{
			mipMaps.resize(mipLevels);

			for (auto& mipMap : mipMaps)
			{
				mipMap.Signature = *(TxpSig*)(mipMapBuffer + 0);
				mipMap.Size = *(ivec2*)(mipMapBuffer + 4);
				mipMap.Format = *(TextureFormat*)(mipMapBuffer + 12);

				mipMap.MipIndex = *(uint8_t*)(mipMapBuffer + 16);
				mipMap.ArrayIndex = *(uint8_t*)(mipMapBuffer + 17);

				mipMap.DataSize = *(uint32_t*)(mipMapBuffer + 20);
				mipMap.Data = MakeUnique<uint8_t[]>(mipMap.DataSize);
				std::memcpy(mipMap.Data.get(), mipMapBuffer + 24, mipMap.DataSize);

				mipMapBuffer = buffer + *offsets;
				++offsets;
			}
		}
	}
}
