#include "Texture.h"
#include "Logger.h"
#include <stb/stb_image.h>
#include <assert.h>

#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT   0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT  0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT  0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT  0x83F3
#define GL_COMPRESSED_RED_RGTC1			  0x8DBB
#define GL_COMPRESSED_RG_RGTC2			  0x8DBD

// static TextureID_t BoundTextures[(GL_TEXTURE31 - GL_TEXTURE0) + 1];

Texture2D::Texture2D()
{
}

Texture2D::~Texture2D()
{
	Dispose();
}

void Texture2D::Bind() const
{
	Bind(0);
}

void Texture2D::Bind(int textureSlot) const
{
	GLCall(glActiveTexture(GetTextureSlotEnum(textureSlot)));
	GLCall(glBindTexture(GetTextureTarget(), textureID));
	
	// TextureID_t& boundTexture = BoundTextures[textureSlot];
	// 
	// if (boundTexture != textureID)
	// {
	// 	glActiveTexture(GetTextureSlotEnum(textureSlot));
	// 	glBindTexture(GetTextureTarget(), textureID);
	// 	boundTexture = textureID;
	// }
}

void Texture2D::UnBind() const
{
	UnBind(0);
}

void Texture2D::UnBind(int textureSlot) const
{
	GLCall(glActiveTexture(GetTextureSlotEnum(textureSlot)));
	GLCall(glBindTexture(GetTextureTarget(), NULL));
}

void Texture2D::InitializeID()
{
	GLCall(glGenTextures(1, &textureID));
}

void Texture2D::UploadEmpty(int width, int height)
{
	imageSize.x = width;
	imageSize.y = height;

	GLCall(glTexImage2D(GetTextureTarget(), 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL));
	
	GLCall(glTexParameteri(GetTextureTarget(), GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GLCall(glTexParameteri(GetTextureTarget(), GL_TEXTURE_MAG_FILTER, GL_LINEAR));
}

bool Texture2D::Upload(const FileSystem::Texture* texture)
{
	size_t mipMapCount = texture->MipMaps.size();
	assert(mipMapCount >= 1);

	const FileSystem::MipMap* baseMipMap = texture->MipMaps.front().get();

	imageSize.x = baseMipMap->Width;
	imageSize.y = baseMipMap->Height;
	textureFormat = baseMipMap->Format;

	InitializeID();
	Bind();

	GLCall(glTexParameteri(GetTextureTarget(), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
	GLCall(glTexParameteri(GetTextureTarget(), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));

	GLCall(glTexParameteri(GetTextureTarget(), GL_TEXTURE_MIN_FILTER, (mipMapCount > 1) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR));
	GLCall(glTexParameteri(GetTextureTarget(), GL_TEXTURE_MAG_FILTER, GL_LINEAR));

	if (mipMapCount > 2)
	{
		GLCall(glTexParameterf(GetTextureTarget(), GL_TEXTURE_BASE_LEVEL, 0));
		GLCall(glTexParameterf(GetTextureTarget(), GL_TEXTURE_MAX_LEVEL, mipMapCount - 1));
		
		GLCall(glTexParameterf(GetTextureTarget(), GL_TEXTURE_MIN_LOD, 0));
		GLCall(glTexParameterf(GetTextureTarget(), GL_TEXTURE_MAX_LOD, mipMapCount - 1));
		GLCall(glTexParameterf(GetTextureTarget(), GL_TEXTURE_LOD_BIAS, -1.0f));
	}

	for (int i = 0; i < mipMapCount; i++)
	{
		FileSystem::MipMap* mipMap = texture->MipMaps[i].get();
		GLenum glFormat = GetGLTextureFormat(mipMap->Format);

		uint8_t* data = mipMap->DataPointer != nullptr ? mipMap->DataPointer : mipMap->Data.data();
		uint32_t dataSize = mipMap->DataPointer != nullptr ? mipMap->DataPointerSize : mipMap->Data.size();

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
	textureFormat = TextureFormat::RGBA8;

	InitializeID();
	Bind();

	GLCall(glTexImage2D(GetTextureTarget(), 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelData));
	GLCall(glGenerateMipmap(GetTextureTarget()));
	
	stbi_image_free(pixelData);

	GLCall(glTexParameteri(GetTextureTarget(), GL_TEXTURE_WRAP_S, GL_REPEAT));
	GLCall(glTexParameteri(GetTextureTarget(), GL_TEXTURE_WRAP_T, GL_REPEAT));
	GLCall(glTexParameteri(GetTextureTarget(), GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GLCall(glTexParameteri(GetTextureTarget(), GL_TEXTURE_MAG_FILTER, GL_LINEAR));

	return true;
}

bool Texture2D::GetIsCompressed(TextureFormat format)
{
	switch (format)
	{
	case TextureFormat::A8:
	case TextureFormat::RGB8:
	case TextureFormat::RGBA8:
	case TextureFormat::RGB5:
	case TextureFormat::RGB5_A1:
	case TextureFormat::RGBA4:
	case TextureFormat::L8:
	case TextureFormat::L8A8:
		return false;

	case TextureFormat::DXT1:
	case TextureFormat::DXT1a:
	case TextureFormat::DXT3:
	case TextureFormat::DXT5:
	case TextureFormat::RGTC1:
	case TextureFormat::RGTC2:
		return true;
	default:
		assert(false);
	}

	return false;
}

GLenum Texture2D::GetGLTextureFormat(TextureFormat format)
{
	switch (format)
	{
	case TextureFormat::A8:
		return GL_ALPHA8;
	case TextureFormat::RGB8:
		return GL_RGB8;
	case TextureFormat::RGBA8:
		return GL_RGBA8;
	case TextureFormat::RGB5:
		return GL_RGB5;
	case TextureFormat::RGB5_A1:
		return GL_RGB5_A1;
	case TextureFormat::RGBA4:
		return GL_RGBA4;
	case TextureFormat::DXT1:
		return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
	case TextureFormat::DXT1a:
		return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
	case TextureFormat::DXT3:
		return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
	case TextureFormat::DXT5:
		return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
	case TextureFormat::RGTC1:
		return GL_COMPRESSED_RED_RGTC1;
	case TextureFormat::RGTC2:
		return GL_COMPRESSED_RG_RGTC2;
	case TextureFormat::L8:
		return GL_LUMINANCE8;
	case TextureFormat::L8A8:
		return GL_LUMINANCE8_ALPHA8;
	default:
		assert(false);
	}

	return GL_INVALID_ENUM;
}

void Texture2D::Dispose()
{
	if (textureID != NULL)
	{
		GLCall(glDeleteTextures(1, &textureID));
		textureID = NULL;
	}
}
