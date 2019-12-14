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
		// DEBUG: Non specific debug flags for quick testing
		uint32_t DebugFlags = 0;

		// NOTE: Used by all newly created texture samplers
		int32_t AnistropicFiltering = D3D11_DEFAULT_MAX_ANISOTROPY;

		bool Wireframe = false;
		bool WireframeOverlay = false;
		bool AlphaSort = true;
		bool RenderOpaque = true;
		bool RenderTransparent = true;

		static constexpr ivec2 ReflectionDefaultResolution = ivec2(512, 512);

		bool RenderReflection = true;
		ivec2 ReflectionResolution = ReflectionDefaultResolution;

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

		// NOTE: Main scene HRD render target
		D3D_DepthRenderTarget RenderTarget = { RenderTargetDefaultSize, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_D32_FLOAT };
		// NOTE: Stage reflection render target primarily used by floor materials
		D3D_DepthRenderTarget ReflectionRenderTarget = { RenderParameters::ReflectionDefaultResolution, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_D32_FLOAT };

		// NOTE: Where the post processed final image gets rendered to
		D3D_RenderTarget* OutputRenderTarget = nullptr;

	public:
		void Resize(ivec2 size);
	};
}
