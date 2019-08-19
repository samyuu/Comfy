#include "Texture2D.h"
#include "Core/Logger.h"
#include <stb/stb_image.h>
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

	void Texture2D::UploadEmpty(int width, int height)
	{
		imageSize.x = static_cast<float>(width);
		imageSize.y = static_cast<float>(height);
		textureFormat = TextureFormat::RGBA8;

		GLCall(glTexImage2D(GetTextureTarget(), 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));

		GLCall(glTexParameteri(GetTextureTarget(), GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GLCall(glTexParameteri(GetTextureTarget(), GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	}

	bool Texture2D::Upload(const FileSystem::Texture* texture)
	{
		GLint mipMapCount = static_cast<GLint>(texture->MipMaps.size());
		assert(mipMapCount >= 1);

		const FileSystem::MipMap* baseMipMap = texture->MipMaps.front().get();

		imageSize.x = static_cast<float>(baseMipMap->Width);
		imageSize.y = static_cast<float>(baseMipMap->Height);
		textureFormat = baseMipMap->Format;

		InitializeID();
		Bind();

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
			FileSystem::MipMap* mipMap = texture->MipMaps[i].get();
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

			// else textureLod(...) won't work for RTC2 textures
			if (i == 0 && mipMapCount <= 2)
			{
				GLCall(glGenerateMipmap(GetTextureTarget()));
			}
		}

		return true;
	}

	bool Texture2D::UploadFromFile(const char* path)
	{
		stbi_set_flip_vertically_on_load(true);

		int width, height;
		uint8_t *pixelData = stbi_load(path, &width, &height, &imageChannels, 0);

		if (pixelData == nullptr)
			Logger::LogErrorLine(__FUNCTION__"(): failed to load texture %s", path);

		assert(pixelData != nullptr);

		imageSize.x = (float)width;
		imageSize.y = (float)height;
		textureFormat = imageChannels == 3 ? TextureFormat::RGB8 : TextureFormat::RGBA8;

		InitializeID();
		Bind();

		GLCall(glTexImage2D(GetTextureTarget(), 0, GetGLTextureFormat(textureFormat), width, height, 0, imageChannels == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, pixelData));
		GLCall(glGenerateMipmap(GetTextureTarget()));

		stbi_image_free(pixelData);

		GLCall(glTexParameteri(GetTextureTarget(), GL_TEXTURE_WRAP_S, GL_REPEAT));
		GLCall(glTexParameteri(GetTextureTarget(), GL_TEXTURE_WRAP_T, GL_REPEAT));
		GLCall(glTexParameteri(GetTextureTarget(), GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GLCall(glTexParameteri(GetTextureTarget(), GL_TEXTURE_MAG_FILTER, GL_LINEAR));

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

	void Texture2D::Dispose()
	{
		if (textureID != NULL)
		{
			GLCall(glDeleteTextures(1, &textureID));
			textureID = NULL;
		}
	}
}