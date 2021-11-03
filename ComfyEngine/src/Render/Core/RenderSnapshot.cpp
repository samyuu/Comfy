#include "RenderSnapshot.h"
#include "Renderer3D/RenderTarget3D.h"
#include "Render/D3D11/D3D11Texture.h"

namespace Comfy::Render
{
	RenderSnapshot::RenderSnapshot() = default;

	RenderSnapshot::RenderSnapshot(RenderSnapshot&& other)
	{
		texture = std::move(other.texture);
	}

	RenderSnapshot::~RenderSnapshot() = default;

	ivec2 RenderSnapshot::GetSize() const
	{
		return (texture != nullptr) ? texture->GetSize() : ivec2(1, 1);
	}

	ComfyTextureID RenderSnapshot::GetTextureID() const
	{
		return (texture != nullptr) ? (*texture) : ComfyTextureID(nullptr);
	}

	void RenderSnapshot::TakeSnapshot(const RenderTarget3D& renderTarget)
	{
		if (texture == nullptr)
			texture = std::make_unique<D3D11Texture2DAndView>(GlobalD3D11, renderTarget.GetRenderTarget());
		else
			texture->CreateCopyFrom(GlobalD3D11, renderTarget.GetRenderTarget());
	}
}
