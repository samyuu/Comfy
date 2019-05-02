#pragma once
#include "../pch.h"

typedef GLuint TextureID_t;
typedef GLenum TextureTarget_t;

class Texture
{
public:
	Texture();
	~Texture();
	Texture(const Texture&) = delete;

	void Bind(int textureSlot = 0);
	void UnBind(int textureSlot = 0);

	void Initialize();
	void GenerateEmpty(int width, int height);
	int LoadFromFile(const char* path);

	inline float GetWidth() { return imageWidth; };
	inline float GetHeight() { return imageHeight; };

	inline TextureID_t GetTextureID() { return textureID; }
	inline void* GetVoidTexture() { return reinterpret_cast<void*>(GetTextureID()); }

	inline TextureTarget_t GetTextureTarget() { return textureTarget; };

	static inline GLenum GetTextureSlotEnum(int textureSlot)
	{
		return GL_TEXTURE0 + textureSlot;
	}

protected:
	TextureID_t textureID = NULL;
	TextureTarget_t textureTarget = GL_TEXTURE_2D;

	float imageWidth = 0.0f;
	float imageHeight = 0.0f;
	int imageChannels = 0;

	void Dispose();
};

