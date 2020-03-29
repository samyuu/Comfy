#pragma once
#include "Graphics/D3D11/Texture/Texture.h"
#include "Graphics/Auth3D/LightParam/GlowParameter.h"

namespace Comfy::Graphics::D3D11
{
	struct ToneMapData
	{
	public:
		void UpdateIfNeeded(const GlowParameter& glow);
		Texture1D* GetLookupTexture();

	private:
		bool NeedsUpdating(const GlowParameter& glow);
		void Update(const GlowParameter& glow);

		void GenerateLookupData(const GlowParameter& glow);
		void UpdateTexture();

	private:
		GlowParameter lastSetGlow;

		std::array<vec2, 512> textureData;
		UniquePtr<Texture1D> lookupTexture = nullptr;
	};
}
