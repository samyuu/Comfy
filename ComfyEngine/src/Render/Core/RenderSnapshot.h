#pragma once
#include "Types.h"
#include "ImGui/ComfyTextureID.h"

namespace Comfy::Render
{
	struct D3D11Texture2DAndView;

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
		std::unique_ptr<D3D11Texture2DAndView> texture;
	};
}
