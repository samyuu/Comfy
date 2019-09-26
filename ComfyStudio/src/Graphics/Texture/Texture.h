#pragma once
#include "Types.h"
#include "Graphics/Graphics.h"
#include "Graphics/GraphicsInterface.h"
#include "Graphics/RenderCommand.h"
#include "FileSystem/Format/TxpSet.h"

namespace Graphics
{
	typedef GLuint TextureID_t;
	typedef GLenum TextureTarget_t;

	class Texture : public IGraphicsObject
	{
	public:
		Texture();
		Texture(Texture&) = delete;
		virtual ~Texture();

		void Bind() const override;
		void Bind(TextureSlot textureSlot) const;
		void UnBind() const override;
		void UnBind(TextureSlot  textureSlot) const;

		void InitializeID() override;

		TextureTarget_t GetTextureTarget() const;
		TextureFormat GetTextureFormat() const;

		static bool GetIsCompressed(TextureFormat format);
		static GLenum GetGLTextureFormat(TextureFormat format);

	protected:
		TextureID_t textureID = NULL;
		TextureTarget_t textureTarget = GL_INVALID_ENUM;;

		TextureFormat textureFormat;
	};
}