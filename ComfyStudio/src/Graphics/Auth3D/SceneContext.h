#pragma once
#include "FogParameter.h"
#include "GlowParameter.h"
#include "LightParameters.h"
#include "LightDataIBL.h"
#include "Graphics/Camera.h"
#include "Graphics/Direct3D/D3D_RenderTarget.h"

namespace Graphics
{
	struct RenderParameters
	{
		bool Wireframe = false;
		bool AlphaSort = true;
		bool RenderOpaque = true;
		bool RenderTransparent = true;

		bool Clear = true;
		vec4 ClearColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	};

	/*
	struct PostProcessParameters
	{
		float Saturation = 2.2f;
		float Brightness = 0.45455f;
	};
	*/

	class SceneContext
	{
	public:
		RenderParameters RenderParameters;

		FogParameter Fog;
		GlowParameter Glow;
		LightParameter Light;
		LightDataIBL IBL;

		PerspectiveCamera Camera;

		// NOTE: Where the scene gets rendered to
		D3D_DepthRenderTarget RenderTarget = { RenderTargetDefaultSize, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_D32_FLOAT };

		// NOTE: Where the post processed final image gets rendered to
		D3D_RenderTarget* OutputRenderTarget = nullptr;

	public:
		void Resize(ivec2 size);
	};
}
