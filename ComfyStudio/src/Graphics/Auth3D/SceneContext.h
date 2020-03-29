#pragma once
#include "SceneRenderParameters.h"
#include "LightParam/Parameters.h"
#include "Graphics/Camera.h"
#include "Graphics/D3D11/Renderer/ViewportRenderData.h"
#include "Resource/IDTypes.h"
#include <optional>

namespace Comfy::Graphics
{
	struct SceneViewport
	{
		PerspectiveCamera Camera;
		SceneRenderParameters Parameters;
		D3D11::ViewportRenderData Data;
	};

	struct SceneParameters
	{
		struct LensFlareParameters
		{
			// NOTE: Lens flare sun position
			std::optional<vec3> SunPosition;
			const class Obj* SunObj = nullptr;

			struct Textures
			{
				// NOTE: Common sun texture from effcmn
				Cached_TxpID Sun = TxpID::Invalid;
				// NOTE: Stage specific textures
				std::array<Cached_TxpID, 2> Flares = { TxpID::Invalid, TxpID::Invalid };
				Cached_TxpID Ghost = TxpID::Invalid;
			} Textures;
		} LensFlare;

		FogParameter Fog;
		GlowParameter Glow;
		LightParameter Light;
		IBLParameters IBL;
	};
}
