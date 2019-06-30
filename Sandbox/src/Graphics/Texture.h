#pragma once
#include "Types.h"
#include "Graphics.h"
#include "GraphicsInterface.h"
#include "FileSystem/Format/TxpSet.h"

typedef GLuint TextureID_t;
typedef GLenum TextureTarget_t;

class Texture2D : public IGraphicsObject
{
public:
	Texture2D();
	~Texture2D();
	Texture2D(const Texture2D&) = delete;

	void Bind() const override;
	void Bind(int textureSlot) const;
	void UnBind() const override;
	void UnBind(int textureSlot) const;

	void InitializeID() override;
	void UploadEmpty(int width, int height);
	bool Upload(const FileSystem::Texture* texture);
	bool UploadFromFile(const char* path);

	inline float GetWidth() const { return imageSize.x; };
	inline float GetHeight() const { return imageSize.y; };
	inline const vec2& GetSize() const { return imageSize; };

	inline TextureID_t GetTextureID() const { return textureID; }
	inline void* GetVoidTexture() const { return reinterpret_cast<void*>(GetTextureID()); }

	inline TextureTarget_t GetTextureTarget() const { return textureTarget; };
	inline TextureFormat GetTextureFormat() const { return textureFormat; };

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
	vec2 imageSize;
	int imageChannels = 0;

	void Dispose();
};

