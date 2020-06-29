#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "ImGui/ComfyTextureID.h"

namespace Comfy::Render
{
	namespace D3D11
	{
		class Texture2D;
	}

	class RenderTarget2D;
	class RenderTarget3D;

	class RenderSnapshot : NonCopyable
	{
	public:
		RenderSnapshot();
		RenderSnapshot(RenderSnapshot&& other);
		~RenderSnapshot();

	public:
		ivec2 GetSize() const;
		ComfyTextureID GetTextureID() const;

		void TakeSnapshot(const RenderTarget3D& renderTarget);

	private:
		std::unique_ptr<D3D11::Texture2D> texture;
	};
}
