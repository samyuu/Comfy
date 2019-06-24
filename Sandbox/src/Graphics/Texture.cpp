#include "Texture.h"
#include "../Logger.h"

#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT   0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT  0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT  0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT  0x83F3
#define GL_COMPRESSED_RED_RGTC1			  0x8DBB
#define GL_COMPRESSED_RG_RGTC2			  0x8DBD

Texture2D::Texture2D()
{
}

Texture2D::~Texture2D()
{
	Dispose();
}

void Texture2D::Bind(int textureSlot)
{
	glActiveTexture(GetTextureSlotEnum(textureSlot));
	glBindTexture(GetTextureTarget(), textureID);
}

void Texture2D::UnBind(int textureSlot)
{
	glActiveTexture(GetTextureSlotEnum(textureSlot));
	glBindTexture(GetTextureTarget(), NULL);
}

void Texture2D::Initialize()
{
	glGenTextures(1, &textureID);
}

void Texture2D::GenerateEmpty(int width, int height)
{
	imageWidth = width;
	imageHeight = height;

	glTexImage2D(GetTextureTarget(), 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

bool Texture2D::Load(const FileSystem::Texture* texture)
{
	size_t mipMapCount = texture->MipMaps.size();
	assert(mipMapCount >= 1);

	const FileSystem::MipMap* baseMipMap = texture->MipMaps.front().get();

	imageWidth = baseMipMap->Width;
	imageHeight = baseMipMap->Height;
	textureFormat = baseMipMap->Format;

	Initialize();
	Bind();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipMapCount + 1);

	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, 0);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, txp->MipMaps.size());
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 100000.0f);

	//glTexStorage2D(GL_TEXTURE_2D, txp->MipMaps.size(), format, mipMap->Width, mipMap->Height);

	// TODO:

	for (int i = 0; i < mipMapCount; i++)
	{
		FileSystem::MipMap* mipMap = texture->MipMaps[i].get();
		GLenum glFormat = GetGLTextureFormat(mipMap->Format);

		if (GetIsCompressed(mipMap->Format))
		{
			glTexImage2D(GL_TEXTURE_2D, i, glFormat, mipMap->Width, mipMap->Height, 0, glFormat, GL_UNSIGNED_BYTE, mipMap->Data.data());
		}
		else
		{
			glCompressedTexImage2D(GL_TEXTURE_2D, i, glFormat, mipMap->Width, mipMap->Height, 0, mipMap->Data.size(), mipMap->Data.data());
		}
	}


	int error = glGetError();
	if (error != GL_NO_ERROR)
		printf("error: %d\n", error);

	return true;
}

bool Texture2D::LoadFromFile(const char* path)
{
	stbi_set_flip_vertically_on_load(true);

	int width, height;
	uint8_t *pixelData = stbi_load(path, &width, &height, &imageChannels, 0);

	if (pixelData == nullptr)
		Logger::LogErrorLine("Texture2D::LoadFromFile(): failed to load texture %s", path);

	assert(pixelData != nullptr);

	imageWidth = (float)width;
	imageHeight = (float)height;
	textureFormat = FileSystem::TextureFormat_RGBA;

	Initialize();
	Bind();
	glTexImage2D(GetTextureTarget(), 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);
	glGenerateMipmap(GetTextureTarget());

	stbi_image_free(pixelData);

	glTexParameteri(GetTextureTarget(), GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GetTextureTarget(), GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GetTextureTarget(), GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GetTextureTarget(), GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return true;
}

bool Texture2D::GetIsCompressed(TextureFormat format)
{
	switch (format)
	{
	case FileSystem::TextureFormat_RGB:
	case FileSystem::TextureFormat_RGBA:
	case FileSystem::TextureFormat_RGBA4:
		return true;
	case FileSystem::TextureFormat_DXT1:
	case FileSystem::TextureFormat_DXT3:
	case FileSystem::TextureFormat_DXT5:
	case FileSystem::TextureFormat_ATI1:
	case FileSystem::TextureFormat_ATI2:
	default:
		return false;
	}
}

GLenum Texture2D::GetGLTextureFormat(TextureFormat format)
{
	switch (format)
	{
	case FileSystem::TextureFormat_RGB: return GL_RGB;
	case FileSystem::TextureFormat_RGBA: return GL_RGBA;
	case FileSystem::TextureFormat_RGBA4: return GL_RGBA4;
	case FileSystem::TextureFormat_DXT1: return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
	case FileSystem::TextureFormat_DXT3: return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
	case FileSystem::TextureFormat_DXT5: return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
	case FileSystem::TextureFormat_ATI1: return GL_COMPRESSED_RED_RGTC1;
	case FileSystem::TextureFormat_ATI2: return GL_COMPRESSED_RG_RGTC2;

	default:
		assert(false);
	}

	return GL_INVALID_ENUM;
}

void Texture2D::Dispose()
{
	if (textureID != NULL)
		glDeleteTextures(1, &textureID);
}
