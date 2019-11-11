#pragma once
#include "Types.h"
#include "Graphics/OpenGL/OpenGL.h"
#include "Graphics/RenderCommand.h"

namespace Graphics
{
	typedef GLuint TextureID_t;
	typedef GLenum TextureTarget_t;

	class GL_Texture /*: public IGraphicsObject*/
	{
	public:
		GL_Texture();
		GL_Texture(const GL_Texture&) = delete;
		GL_Texture& operator= (const GL_Texture&) = delete;
		virtual ~GL_Texture();

		void Bind() const /*override*/;
		void Bind(TextureSlot textureSlot) const;
		void UnBind() const /*override*/;
		void UnBind(TextureSlot  textureSlot) const;

		void InitializeID() /*override*/;

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