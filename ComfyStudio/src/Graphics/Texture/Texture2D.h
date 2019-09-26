#pragma once
#include "Texture.h"

namespace Graphics
{
	class Texture2D : public Texture
	{
	public:
		Texture2D();
		Texture2D(Texture2D&) = delete;
		~Texture2D();

		void UploadEmpty(ivec2 size);
		bool Create(const FileSystem::Texture* texture);
		bool CreateFromFile(const char* path);
		bool CreateFromRgbaBuffer(ivec2 size, const Vector<uint32_t>& pixels);

		float GetWidth() const;
		float GetHeight() const;
		const vec2& GetSize() const;

		TextureID_t GetTextureID() const;
		void* GetVoidTexture() const;

		void SetObjectLabel(const char* label) override;

	protected:
		vec2 imageSize;
		int imageChannels = 0;

		void Dispose();
	};
}
