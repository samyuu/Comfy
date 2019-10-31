#include "Texture2D.h"
#include "Core/Logger.h"
#include <assert.h>

namespace Graphics
{
	Texture2D::Texture2D()
	{
		textureTarget = GL_TEXTURE_2D;
	}

	Texture2D::~Texture2D()
	{
		Dispose();
	}

	void Texture2D::UploadEmpty(ivec2 size)
	{
		constexpr ivec2 minimumSize = ivec2(1.0, 1.0);
		imageSize = glm::max(minimumSize, size);
		textureFormat = TextureFormat::RGBA8;

		GLCall(glTexImage2D(GetTextureTarget(), 0, GL_RGBA8, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));

		GLCall(glTexParameteri(GetTextureTarget(), GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GLCall(glTexParameteri(GetTextureTarget(), GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	}

	bool Texture2D::Create(const Txp* txp)
	{
		GLint mipMapCount = static_cast<GLint>(txp->MipMaps.size());
		assert(mipMapCount >= 1);

		const MipMap* baseMipMap = txp->MipMaps.front().get();

		imageSize.x = static_cast<float>(baseMipMap->Width);
		imageSize.y = static_cast<float>(baseMipMap->Height);
		textureFormat = baseMipMap->Format;

		InitializeID();
		Bind();

		SetObjectLabel(txp->Name.c_str());

		GLCall(glTexParameteri(GetTextureTarget(), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
		GLCall(glTexParameteri(GetTextureTarget(), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));

		vec4 borderColor = { 0.0, 0.0, 0.0, 0.0f };
		GLCall(glTexParameterfv(GetTextureTarget(), GL_TEXTURE_BORDER_COLOR, glm::value_ptr(borderColor)));

		GLCall(glTexParameteri(GetTextureTarget(), GL_TEXTURE_MIN_FILTER, (mipMapCount > 1) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR));
		GLCall(glTexParameteri(GetTextureTarget(), GL_TEXTURE_MAG_FILTER, GL_LINEAR));

		if (mipMapCount > 2)
		{
			GLCall(glTexParameteri(GetTextureTarget(), GL_TEXTURE_BASE_LEVEL, 0));
			GLCall(glTexParameteri(GetTextureTarget(), GL_TEXTURE_MAX_LEVEL, mipMapCount - 1));

			GLCall(glTexParameteri(GetTextureTarget(), GL_TEXTURE_MIN_LOD, 0));
			GLCall(glTexParameteri(GetTextureTarget(), GL_TEXTURE_MAX_LOD, mipMapCount - 1));
			GLCall(glTexParameterf(GetTextureTarget(), GL_TEXTURE_LOD_BIAS, -1.0f));
		}

		for (int i = 0; i < mipMapCount; i++)
		{
			const MipMap* mipMap = txp->MipMaps[i].get();
			GLenum glFormat = GetGLTextureFormat(mipMap->Format);

			const uint8_t* data = mipMap->DataPointer != nullptr ? mipMap->DataPointer : mipMap->Data.data();
			uint32_t dataSize = mipMap->DataPointer != nullptr ? mipMap->DataPointerSize : static_cast<uint32_t>(mipMap->Data.size());

			if (GetIsCompressed(mipMap->Format))
			{
				GLCall(glCompressedTexImage2D(GetTextureTarget(), i, glFormat, mipMap->Width, mipMap->Height, 0, dataSize, data));
			}
			else
			{
				GLCall(glTexImage2D(GetTextureTarget(), i, glFormat, mipMap->Width, mipMap->Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data));
			}

			// NOTE: Else textureLod(...) won't work for RTC2 textures
			if (i == 0 && mipMapCount <= 2)
			{
				GLCall(glGenerateMipmap(GetTextureTarget()));
			}
		}

		return true;
	}

	bool Texture2D::CreateFromRgbaBuffer(ivec2 size, const uint32_t* pixels)
	{
		assert(size.x > 0 && size.y > 0);

		imageSize = size;
		textureFormat = TextureFormat::RGBA8;

		InitializeID();
		Bind();

		GLCall(glTexParameteri(GetTextureTarget(), GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GLCall(glTexParameteri(GetTextureTarget(), GL_TEXTURE_MAG_FILTER, GL_LINEAR));

		GLCall(glTexImage2D(GetTextureTarget(), 0, GL_RGBA8, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels));

		return true;
	}

	float Texture2D::GetWidth() const
	{
		return imageSize.x;
	};

	float Texture2D::GetHeight() const
	{
		return imageSize.y;
	};

	const vec2& Texture2D::GetSize() const
	{
		return imageSize;
	};

	TextureID_t Texture2D::GetTextureID() const
	{
		return textureID;
	}

#pragma warning(push)
#pragma warning(disable : 4312) // 'operation' : conversion from 'type1' to 'type2' of greater size
	void* Texture2D::GetVoidTexture() const
	{
		return reinterpret_cast<void*>(GetTextureID());
	}
#pragma warning(pop)

	void Texture2D::SetObjectLabel(const char* label)
	{
		GLCall(glObjectLabel(GL_TEXTURE, textureID, -1, label));
	}

	void Texture2D::Dispose()
	{
		if (textureID != NULL)
		{
			GLCall(glDeleteTextures(1, &textureID));
			textureID = NULL;
		}
	}
}