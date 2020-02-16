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
		static constexpr ivec2 ShadowMapDefaultResolution = ivec2(2048, 2048);
		static constexpr ivec2 ReflectionDefaultResolution = ivec2(512, 512);

		// DEBUG: Non specific debug flags for quick testing
		uint32_t DebugFlags = 0;
		uint32_t ShaderDebugFlags = 0;

		// NOTE: Used by all newly created texture samplers
		int32_t AnistropicFiltering = D3D11_DEFAULT_MAX_ANISOTROPY;

		bool FrustumCulling = true;
		bool Wireframe = false;
		bool AlphaSort = true;
		bool RenderOpaque = true;
		bool RenderTransparent = true;
		bool RenderBloom = true;
		bool VertexColoring = true;
		bool DiffuseMapping = true;
		bool AmbientOcclusionMapping = true;
		bool NormalMapping = true;
		bool SpecularMapping = true;
		bool AlphaTesting = true;
		bool CubeReflection = true;
		bool RenderFog = true;

		// NOTE: Enable to take alpha render target captures with a clear color alpha of zero
		bool ToneMapPreserveAlpha = true;

		ivec2 RenderResolution = RenderTargetDefaultSize;

#if COMFY_DEBUG
		uint32_t MultiSampleCount = 1;
#else
		uint32_t MultiSampleCount = 4;
#endif

		bool RenderShadowMap = true;
		ivec2 ShadowMapResolution = ShadowMapDefaultResolution;

		uint32_t ShadowBlurPasses = 1;

		bool RenderSubsurfaceScattering = true;

		bool RenderReflection = true;
		bool ClearReflection = true;
		ivec2 ReflectionRenderResolution = ReflectionDefaultResolution;

		bool Clear = true;
		vec4 ClearColor = vec4(0.16f, 0.16f, 0.16f, 1.00f);

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

	struct ShadowMappingRenderData
	{
		static constexpr DXGI_FORMAT MainDepthFormat = DXGI_FORMAT_D32_FLOAT;
		static constexpr DXGI_FORMAT PostProcessingFormat = DXGI_FORMAT_R32_FLOAT;

		// NOTE: Main depth render target rendered to using the silhouette shader
		D3D_DepthOnlyRenderTarget RenderTarget = { RenderParameters::ShadowMapDefaultResolution, MainDepthFormat };

		// NOTE: Full resolution render targets used for initial blurring
		std::array<D3D_RenderTarget, 2> ExponentialRenderTargets =
		{
			D3D_RenderTarget { RenderParameters::ShadowMapDefaultResolution, PostProcessingFormat },
			D3D_RenderTarget { RenderParameters::ShadowMapDefaultResolution, PostProcessingFormat },
		};
		// NOTE: Half resolution render targets used for additional filtering, sampled by stage and character material shaders
		std::array<D3D_RenderTarget, 2> ExponentialBlurRenderTargets =
		{
			D3D_RenderTarget { RenderParameters::ShadowMapDefaultResolution / 4, PostProcessingFormat },
			D3D_RenderTarget { RenderParameters::ShadowMapDefaultResolution / 4, PostProcessingFormat },
		};

		// NOTE: Processed main depth render target with a constant depth value
		D3D_RenderTarget ThresholdRenderTarget = { RenderParameters::ShadowMapDefaultResolution / 2, PostProcessingFormat };
		
		// NOTE: Low resolution render targets used for ping pong blurring, sampled by stage material shaders
		std::array<D3D_RenderTarget, 2> BlurRenderTargets =
		{ 
			D3D_RenderTarget { RenderParameters::ShadowMapDefaultResolution / 4, PostProcessingFormat },
			D3D_RenderTarget { RenderParameters::ShadowMapDefaultResolution / 4, PostProcessingFormat },
		};
	};

	struct SubsurfaceScatteringRenderData
	{
		// NOTE: Main render target, same size as the main rendering
		D3D_DepthRenderTarget RenderTarget = { RenderTargetDefaultSize, RenderTargetHDRFormatRGBA, DXGI_FORMAT_D32_FLOAT };

		// NOTE: Further reduction and filtering
		std::array<D3D_RenderTarget, 3> FilterRenderTargets = 
		{
			D3D_RenderTarget { ivec2(640, 360), RenderTargetHDRFormatRGBA },
			D3D_RenderTarget { ivec2(320, 180), RenderTargetHDRFormatRGBA },
			D3D_RenderTarget { ivec2(320, 180), RenderTargetHDRFormatRGBA },
		};
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
		D3D_RenderTarget RenderTarget = { RenderTargetDefaultSize, RenderTargetLDRFormatRGBA };
	};

	struct RenderData
	{
		// TODO: Non multisample copies of MainRenderTargets for both screen texture rendering and post processing (?)

		MainRenderData Main;
		ShadowMappingRenderData Shadow;
		ScreenReflectionRenderData Reflection;
		SubsurfaceScatteringRenderData SubsurfaceScattering;
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
