#pragma once
#include "Types.h"
#include "../Texture/RenderTarget.h"
#include "../Texture/DepthBuffer.h"
#include "Detail/BlendStateCache.h"
#include "Detail/SunOcclusionData.h"
#include "Detail/TextureSamplerCache.h"
#include "Detail/ToneMapData.h"
#include "Graphics/Auth3D/SceneRenderParameters.h"

namespace Comfy::Render::D3D11
{
	struct MainRenderData
	{
		// NOTE: Main scene HRD render targets
		std::array<D3D11::DepthRenderTarget, 2> RenderTargets =
		{
			D3D11::DepthRenderTarget { RenderTargetDefaultSize, RenderTargetHDRFormatRGBA, DXGI_FORMAT_D32_FLOAT, 1 },
			D3D11::DepthRenderTarget { RenderTargetDefaultSize, RenderTargetHDRFormatRGBA, DXGI_FORMAT_D32_FLOAT, 1 },
		};

		// NOTE: Optional MSAA resolved copies
		std::array<D3D11::RenderTarget, 2> ResolvedRenderTargets =
		{
			D3D11::RenderTarget { RenderTargetDefaultSize, RenderTargetHDRFormatRGBA },
			D3D11::RenderTarget { RenderTargetDefaultSize, RenderTargetHDRFormatRGBA },
		};

	public:
		inline bool MSAAEnabled() { return Current().GetMultiSampleCount() > 1; }
		inline bool MSAAEnabledPrevious() { return Previous().GetMultiSampleCount() > 1; }

		inline void AdvanceRenderTarget() { currentIndex = (currentIndex + 1) % RenderTargets.size(); }

		inline D3D11::DepthRenderTarget& Current() { return RenderTargets[currentIndex]; }
		inline D3D11::DepthRenderTarget& Previous() { return RenderTargets[((currentIndex - 1) + RenderTargets.size()) % RenderTargets.size()]; }

		inline D3D11::RenderTarget& CurrentResolved() { return ResolvedRenderTargets[currentIndex]; }
		inline D3D11::RenderTarget& PreviousResolved() { return ResolvedRenderTargets[((currentIndex - 1) + ResolvedRenderTargets.size()) % ResolvedRenderTargets.size()]; }

		inline D3D11::RenderTarget& CurrentOrResolved() { return MSAAEnabled() ? CurrentResolved() : Current(); }
		inline D3D11::RenderTarget& PreviousOrResolved() { return MSAAEnabledPrevious() ? PreviousResolved() : Previous(); }

	private:
		// NOTE: Index of the currently active render target, keep switching to allow for last->current frame post processing effects and screen textures
		i32 currentIndex = 0;
	};

	struct ShadowMappingRenderData
	{
		static constexpr DXGI_FORMAT MainDepthFormat = DXGI_FORMAT_D32_FLOAT;
		static constexpr DXGI_FORMAT PostProcessingFormat = DXGI_FORMAT_R32_FLOAT;

		// NOTE: Main depth render target rendered to using the silhouette shader
		D3D11::DepthOnlyRenderTarget RenderTarget = { ShadowMapDefaultResolution, MainDepthFormat };

		// NOTE: Full resolution render targets used for initial blurring
		std::array<D3D11::RenderTarget, 2> ExponentialRenderTargets =
		{
			D3D11::RenderTarget { ShadowMapDefaultResolution, PostProcessingFormat },
			D3D11::RenderTarget { ShadowMapDefaultResolution, PostProcessingFormat },
		};
		// NOTE: Half resolution render targets used for additional filtering, sampled by stage and character material shaders
		std::array<D3D11::RenderTarget, 2> ExponentialBlurRenderTargets =
		{
			D3D11::RenderTarget { ShadowMapDefaultResolution / 4, PostProcessingFormat },
			D3D11::RenderTarget { ShadowMapDefaultResolution / 4, PostProcessingFormat },
		};

		// NOTE: Processed main depth render target with a constant depth value
		D3D11::RenderTarget ThresholdRenderTarget = { ShadowMapDefaultResolution / 2, PostProcessingFormat };

		// NOTE: Low resolution render targets used for ping pong blurring, sampled by stage material shaders
		std::array<D3D11::RenderTarget, 2> BlurRenderTargets =
		{
			D3D11::RenderTarget { ShadowMapDefaultResolution / 4, PostProcessingFormat },
			D3D11::RenderTarget { ShadowMapDefaultResolution / 4, PostProcessingFormat },
		};
	};

	struct SubsurfaceScatteringRenderData
	{
		// NOTE: Main render target, same size as the main rendering
		D3D11::DepthRenderTarget RenderTarget = { RenderTargetDefaultSize, RenderTargetHDRFormatRGBA, DXGI_FORMAT_D32_FLOAT };

		// NOTE: Further reduction and filtering
		std::array<D3D11::RenderTarget, 3> FilterRenderTargets =
		{
			D3D11::RenderTarget { ivec2(640, 360), RenderTargetHDRFormatRGBA },
			D3D11::RenderTarget { ivec2(320, 180), RenderTargetHDRFormatRGBA },
			D3D11::RenderTarget { ivec2(320, 180), RenderTargetHDRFormatRGBA },
		};
	};

	struct ScreenReflectionRenderData
	{
		// NOTE: Stage reflection render target primarily used by floor materials
		D3D11::DepthRenderTarget RenderTarget = { ReflectionDefaultResolution, RenderTargetLDRFormatRGBA, DXGI_FORMAT_D32_FLOAT };
	};

	struct SilhouetteRenderData
	{
		// NOTE: Mostly for debugging, render black and white rendering to outline and overlay on the main render target
		D3D11::DepthRenderTarget RenderTarget = { RenderTargetDefaultSize, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_D32_FLOAT };
	};

	struct BloomRenderData
	{
		// NOTE: Base half reduction of the main scene render target
		D3D11::RenderTarget BaseRenderTarget = { RenderTargetDefaultSize, RenderTargetHDRFormatRGBA };

		// NOTE: Final blurred render target, combination of ReduceRenderTargets / BlurRenderTargets
		D3D11::RenderTarget CombinedBlurRenderTarget = { ivec2(256, 144), RenderTargetHDRFormatRGBA };

		// NOTE: BaseRenderTarget -> 256x144 -> ... -> 32x18
		std::array<D3D11::RenderTarget, 4> ReduceRenderTargets =
		{
			D3D11::RenderTarget { ivec2(256, 144), RenderTargetHDRFormatRGBA },
			D3D11::RenderTarget { ivec2(128, 72), RenderTargetHDRFormatRGBA },
			D3D11::RenderTarget { ivec2(64, 36), RenderTargetHDRFormatRGBA },
			D3D11::RenderTarget { ivec2(32, 18), RenderTargetHDRFormatRGBA },
		};

		std::array<D3D11::RenderTarget, 4> BlurRenderTargets =
		{
			D3D11::RenderTarget { ivec2(256, 144), RenderTargetHDRFormatRGBA },
			D3D11::RenderTarget { ivec2(128, 72), RenderTargetHDRFormatRGBA },
			D3D11::RenderTarget { ivec2(64, 36), RenderTargetHDRFormatRGBA },
			D3D11::RenderTarget { ivec2(32, 18), RenderTargetHDRFormatRGBA },
		};

		// NOTE: Auto Exposure
		std::array<D3D11::RenderTarget, 3> ExposureRenderTargets =
		{
			// NOTE: 32 x 18 ->  8 x  8
			D3D11::RenderTarget { ivec2(8, 8), RenderTargetHDRFormatRGBA },
			// NOTE:  8 x  8 -> 32 x  1
			D3D11::RenderTarget { ivec2(32, 1), RenderTargetHDRFormatRGBA },
			// NOTE: 32 x  1 ->  1 x  1
			D3D11::RenderTarget { ivec2(1, 1), RenderTargetHDRFormatRGBA },
		};
	};

	struct OutputRenderData
	{
		// NOTE: Where the post processed final image gets rendered to
		D3D11::RenderTarget RenderTarget = { RenderTargetDefaultSize, RenderTargetLDRFormatRGBA };
	};

	struct ViewportRenderData
	{
		MainRenderData Main;
		ShadowMappingRenderData Shadow;
		ScreenReflectionRenderData Reflection;
		SubsurfaceScatteringRenderData SubsurfaceScattering;
		SilhouetteRenderData Silhouette;
		BloomRenderData Bloom;
		OutputRenderData Output;

		TextureSamplerCache TextureSamplers;
		BlendStateCache BlendStates;
		ToneMapData ToneMap;
		SunOcclusionData Sun;
	};
}
