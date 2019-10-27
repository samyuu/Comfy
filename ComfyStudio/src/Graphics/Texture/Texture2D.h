#pragma once
#include "Texture.h"
#include "Graphics/TxpSet.h"

namespace Graphics
{
	class Texture2D : public Texture
	{
	public:
		Texture2D();
		Texture2D(Texture2D&) = delete;
		~Texture2D();

		void UploadEmpty(ivec2 size);
		bool Create(const Txp* txp);
		bool CreateFromFile(const char* path);
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
