#include "Texture.h"
#include "Graphics/RenderCommand.h"

#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT   0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT  0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT  0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT  0x83F3
#define GL_COMPRESSED_RED_RGTC1			  0x8DBB
#define GL_COMPRESSED_RG_RGTC2			  0x8DBD

namespace Graphics
{
	Texture::Texture()
	{
	}
	
	Texture::~Texture()
	{
	}

	void Texture::Bind() const
	{
		Bind(0);
	}

	void Texture::Bind(int32_t textureSlot) const
	{
		RenderCommand::SetTextureSlot(textureSlot);
		RenderCommand::BindTexture(GetTextureTarget(), textureID);
	}

	void Texture::UnBind() const
	{
		UnBind(0);
	}

	void Texture::UnBind(int32_t textureSlot) const
	{
		RenderCommand::SetTextureSlot(textureSlot);
		RenderCommand::BindTexture(GetTextureTarget(), NULL);
	}

	void Texture::InitializeID()
	{
		GLCall(glGenTextures(1, &textureID));
	}

	TextureTarget_t Texture::GetTextureTarget() const
	{
		return textureTarget;
	};

	TextureFormat Texture::GetTextureFormat() const
	{
		return textureFormat;
	};

	bool Texture::GetIsCompressed(TextureFormat format)
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

	GLenum Texture::GetGLTextureFormat(TextureFormat format)
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
}