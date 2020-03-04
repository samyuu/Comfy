#pragma once
#include "Graphics/Auth3D/SceneContext.h"

namespace Comfy::Graphics
{
	struct ToneMapData
	{
		GlowParameter Glow;

		std::array<vec2, 512> TextureData;
		UniquePtr<D3D_Texture1D> LookupTexture = nullptr;

	public:
		bool NeedsUpdating(const SceneContext* sceneContext);
		void Update();

	private:
		void GenerateLookupData();
		void UpdateTexture();
	};
}
