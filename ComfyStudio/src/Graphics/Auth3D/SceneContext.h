#pragma once
#include "Light/FogParameter.h"
#include "Light/GlowParameter.h"
#include "Light/LightParameters.h"
#include "Light/LightDataIBL.h"
#include "Graphics/Camera.h"
#include "Graphics/Direct3D/D3D_RenderTarget.h"

namespace Graphics
{
	// NOTE: Different user controlable render graphics settings
	struct RenderParameters
	{
		// DEBUG: Non specific debug flags for quick testing
		uint32_t DebugFlags = 0;

		// NOTE: Used by all newly created texture samplers
		int32_t AnistropicFiltering = D3D11_DEFAULT_MAX_ANISOTROPY;

		bool FrustumCulling = true;
		bool Wireframe = false;
		bool AlphaSort = true;
		bool RenderOpaque = true;
		bool RenderTransparent = true;
		bool RenderBloom = true;
		bool RenderFog = true;

		ivec2 RenderResolution = RenderTargetDefaultSize;
		uint32_t MultiSampleCount = 4;

		static constexpr ivec2 ReflectionDefaultResolution = ivec2(512, 512);

		bool RenderReflection = true;
		bool ClearReflection = true;
		ivec2 ReflectionRenderResolution = ReflectionDefaultResolution;

		bool Clear = true;
		vec4 ClearColor = vec4(0.16f, 0.16f, 0.16f, 0.0f);

		/*
		struct PostProcessParameters
		{
			float Saturation = 2.2f;
			float Brightness = 0.45455f;
		} PostProcess;
		*/
	};

	struct MainRenderData
	{
		// NOTE: Main scene HRD render targets
		std::array<D3D_DepthRenderTarget, 2> RenderTargets =
		{
			D3D_DepthRenderTarget { RenderTargetDefaultSize, RenderTargetHDRFormatRGBA, DXGI_FORMAT_D32_FLOAT, 1 },
			D3D_DepthRenderTarget { RenderTargetDefaultSize, RenderTargetHDRFormatRGBA, DXGI_FORMAT_D32_FLOAT, 1 },
		};

	private:
		// NOTE: Index of the currently active render target, keep switching to allow for last->current frame post processing effects and screen textures
		int32_t currentIndex = 0;

	public:
		inline void AdvanceRenderTarget() { currentIndex = (currentIndex + 1) % RenderTargets.size(); }
		inline D3D_DepthRenderTarget& CurrentRenderTarget() { return RenderTargets[currentIndex]; }
		inline D3D_DepthRenderTarget& PreviousRenderTarget() { return RenderTargets[((currentIndex - 1) + RenderTargets.size()) % RenderTargets.size()]; }
	};

	struct ScreenReflectionRenderData
	{
		// NOTE: Stage reflection render target primarily used by floor materials
		D3D_DepthRenderTarget RenderTarget = { RenderParameters::ReflectionDefaultResolution, RenderTargetLDRFormatRGBA, DXGI_FORMAT_D32_FLOAT };
	};

	struct SilhouetteRenderData
	{
		// NOTE: Mostly for debugging, render black and white rendering to outline and overlay on the main render target
		D3D_DepthRenderTarget RenderTarget = { RenderTargetDefaultSize, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_D32_FLOAT };
	};

	struct BloomRenderData
	{
		// NOTE: Base half reduction of the main scene render target
		D3D_RenderTarget BaseRenderTarget = { RenderTargetDefaultSize, RenderTargetHDRFormatRGBA };

		// NOTE: Final blurred render target, combination of ReduceRenderTargets / BlurRenderTargets
		D3D_RenderTarget CombinedBlurRenderTarget = { ivec2(256, 144), RenderTargetHDRFormatRGBA };

		// NOTE: BaseRenderTarget -> 256x144 -> ... -> 32x18
		std::array<D3D_RenderTarget, 4> ReduceRenderTargets =
		{
			D3D_RenderTarget { ivec2(256, 144), RenderTargetHDRFormatRGBA },
			D3D_RenderTarget { ivec2(128,  72), RenderTargetHDRFormatRGBA },
			D3D_RenderTarget { ivec2( 64,  36), RenderTargetHDRFormatRGBA },
			D3D_RenderTarget { ivec2( 32,  18), RenderTargetHDRFormatRGBA },
		};

		std::array<D3D_RenderTarget, 4> BlurRenderTargets =
		{
			D3D_RenderTarget { ivec2(256, 144), RenderTargetHDRFormatRGBA },
			D3D_RenderTarget { ivec2(128,  72), RenderTargetHDRFormatRGBA },
			D3D_RenderTarget { ivec2( 64,  36), RenderTargetHDRFormatRGBA },
			D3D_RenderTarget { ivec2( 32,  18), RenderTargetHDRFormatRGBA },
		};
	};

	struct OutputRenderData
	{
		// NOTE: Where the post processed final image gets rendered to
		D3D_RenderTarget* RenderTarget = nullptr;
	};

	struct RenderData
	{
		// TODO: Non multisample copies of MainRenderTargets for both screen texture rendering and post processing (?)

		MainRenderData Main;
		ScreenReflectionRenderData Reflection;
		SilhouetteRenderData Silhouette;
		BloomRenderData Bloom;
		OutputRenderData Output;
	};

	// TODO: Separate Viewport specific data from scene state
	class SceneContext
	{
	public:
		RenderParameters RenderParameters;

		FogParameter Fog;
		GlowParameter Glow;
		LightParameter Light;
		LightDataIBL IBL;

		PerspectiveCamera Camera;

		RenderData RenderData;
	};
}
