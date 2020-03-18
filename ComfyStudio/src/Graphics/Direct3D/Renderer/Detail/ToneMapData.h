#pragma once
#include "Graphics/Auth3D/SceneContext.h"

namespace Comfy::Graphics
{
	struct ToneMapData
	{
	public:
		bool NeedsUpdating(const GlowParameter& glow);
		void Update(const GlowParameter& glow);

		D3D_Texture1D* GetLookupTexture();

	private:
		void GenerateLookupData(const GlowParameter& glow);
		void UpdateTexture();

	private:
		GlowParameter lastSetGlow;

		std::array<vec2, 512> textureData;
		UniquePtr<D3D_Texture1D> lookupTexture = nullptr;
	};
}
