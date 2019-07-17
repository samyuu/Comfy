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

	float GetWidth() const;
	float GetHeight() const;
	const vec2& GetSize() const;

	TextureID_t GetTextureID() const;
	void* GetVoidTexture() const;

	TextureTarget_t GetTextureTarget() const;
	TextureFormat GetTextureFormat() const;

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

