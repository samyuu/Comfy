#pragma once
#include "GL_Texture.h"
#include "Graphics/TxpSet.h"

namespace Graphics
{
	class GL_Texture2D : public GL_Texture
	{
	public:
		GL_Texture2D();
		GL_Texture2D(const GL_Texture2D&) = delete;
		GL_Texture2D& operator= (const GL_Texture2D&) = delete;
		~GL_Texture2D();

		void UploadEmpty(ivec2 size);
		bool Create(const Txp* txp);
		bool CreateFromRgbaBuffer(ivec2 size, const uint32_t* pixels);

		float GetWidth() const;
		float GetHeight() const;
		const vec2& GetSize() const;

		TextureID_t GetTextureID() const;
		void* GetVoidTexture() const;

		void SetObjectLabel(const char* label) override;

	protected:
		vec2 imageSize = vec2(0.0, 0.0f);
		int imageChannels = 0;

		void Dispose();
	};
}
