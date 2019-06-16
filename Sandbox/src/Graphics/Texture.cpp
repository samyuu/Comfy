#include "Texture.h"
#include "../Logger.h"

Texture::Texture()
{
}

Texture::~Texture()
{
	Dispose();
}

void Texture::Bind(int textureSlot)
{
	glActiveTexture(GetTextureSlotEnum(textureSlot));
	glBindTexture(GetTextureTarget(), textureID);
}

void Texture::UnBind(int textureSlot)
{
	glActiveTexture(GetTextureSlotEnum(textureSlot));
	glBindTexture(GetTextureTarget(), NULL);
}

void Texture::Initialize()
{
	glGenTextures(1, &textureID);
}

void Texture::GenerateEmpty(int width, int height)
{
	imageWidth = width;
	imageHeight = height;

	glTexImage2D(GetTextureTarget(), 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

int Texture::LoadFromFile(const char* path)
{
	stbi_set_flip_vertically_on_load(true);

	int width, height;
	uint8_t *pixelData = stbi_load(path, &width, &height, &imageChannels, 0);
	
	if (pixelData == nullptr)
		Logger::LogErrorLine("Texture::LoadFromFile(): failed to load texture %s", path);
	
	assert(pixelData != nullptr);

	imageWidth = (float)width;
	imageHeight = (float)height;

	Initialize();
	Bind();
	glTexImage2D(GetTextureTarget(), 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);
	glGenerateMipmap(GetTextureTarget());
	
	stbi_image_free(pixelData);

	glTexParameteri(GetTextureTarget(), GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GetTextureTarget(), GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GetTextureTarget(), GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GetTextureTarget(), GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return 0;
}

void Texture::Dispose()
{
	if (textureID != NULL)
		glDeleteTextures(1, &textureID);
}
