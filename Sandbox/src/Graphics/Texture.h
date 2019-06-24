#pragma once
#include "../pch.h"
#include "../FileSystem/Format/TxpSet.h"

typedef GLuint TextureID_t;
typedef GLenum TextureTarget_t;

typedef FileSystem::TextureFormat TextureFormat;

class Texture2D
{
public:
	Texture2D();
	~Texture2D();
	Texture2D(const Texture2D&) = delete;

	void Bind(int textureSlot = 0);
	void UnBind(int textureSlot = 0);

	void Initialize();
	void GenerateEmpty(int width, int height);

	bool Load(const FileSystem::Texture* texture);
	bool LoadFromFile(const char* path);

	inline float GetWidth() { return imageWidth; };
	inline float GetHeight() { return imageHeight; };

	inline void SetSize(float width, float height) { imageWidth = width; imageHeight = height; };

	inline TextureID_t GetTextureID() { return textureID; }
	inline void* GetVoidTexture() { return reinterpret_cast<void*>(GetTextureID()); }

	inline TextureTarget_t GetTextureTarget() { return textureTarget; };
	inline TextureFormat GetTextureFormat() { return textureFormat; };


	static constexpr GLenum GetTextureSlotEnum(int textureSlot)
	{
		return GL_TEXTURE0 + textureSlot;
	}

	static bool GetIsCompressed(TextureFormat format);
	static GLenum GetGLTextureFormat(TextureFormat format);

protected:
	TextureID_t textureID = NULL;
	TextureTarget_t textureTarget = GL_TEXTURE_2D;

	TextureFormat textureFormat;
	float imageWidth = 0.0f;
	float imageHeight = 0.0f;
	int imageChannels = 0;

	void Dispose();
};

