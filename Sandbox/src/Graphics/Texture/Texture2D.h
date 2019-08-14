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

		void UploadEmpty(int width, int height);
		bool Upload(const FileSystem::Texture* texture);
		bool UploadFromFile(const char* path);

		float GetWidth() const;
		float GetHeight() const;
		const vec2& GetSize() const;

		TextureID_t GetTextureID() const;
		void* GetVoidTexture() const;

	protected:
		vec2 imageSize;
		int imageChannels = 0;

		void Dispose();
	};
}
