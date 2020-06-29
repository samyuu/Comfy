#pragma once
#include "Render/Core/Renderer3D/RenderTarget3D.h"
#include "TextureSamplerCache.h"
#include "BlendStateCache.h"
#include "ToneMapData.h"
#include "SunOcclusionData.h"
#include "Render/D3D11/Texture/RenderTarget.h"

namespace Comfy::Render::Detail
{
	struct MainRenderData
	{
		// NOTE: Main scene HRD render targets
		std::array<D3D11::DepthRenderTarget, 2> RenderTargets =
		{
			D3D11::DepthRenderTarget { D3D11::RenderTargetDefaultSize, D3D11::RenderTargetHDRFormatRGBA, DXGI_FORMAT_D32_FLOAT, 1 },
			D3D11::DepthRenderTarget { D3D11::RenderTargetDefaultSize, D3D11::RenderTargetHDRFormatRGBA, DXGI_FORMAT_D32_FLOAT, 1 },
		};

		// NOTE: Optional MSAA resolved copies
		std::array<D3D11::RenderTarget, 2> ResolvedRenderTargets =
		{
			D3D11::RenderTarget { D3D11::RenderTargetDefaultSize, D3D11::RenderTargetHDRFormatRGBA },
			D3D11::RenderTarget { D3D11::RenderTargetDefaultSize, D3D11::RenderTargetHDRFormatRGBA },
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
		D3D11::DepthRenderTarget RenderTarget = { D3D11::RenderTargetDefaultSize, D3D11::RenderTargetHDRFormatRGBA, DXGI_FORMAT_D32_FLOAT };

		// NOTE: Further reduction and filtering
		std::array<D3D11::RenderTarget, 3> FilterRenderTargets =
		{
			D3D11::RenderTarget { ivec2(640, 360), D3D11::RenderTargetHDRFormatRGBA },
			D3D11::RenderTarget { ivec2(320, 180), D3D11::RenderTargetHDRFormatRGBA },
			D3D11::RenderTarget { ivec2(320, 180), D3D11::RenderTargetHDRFormatRGBA },
		};
	};

	struct ScreenReflectionRenderData
	{
		// NOTE: Stage reflection render target primarily used by floor materials
		D3D11::DepthRenderTarget RenderTarget = { ReflectionDefaultResolution, D3D11::RenderTargetLDRFormatRGBA, DXGI_FORMAT_D32_FLOAT };
	};

	struct SilhouetteRenderData
	{
		// NOTE: Mostly for debugging, render black and white rendering to outline and overlay on the main render target
		D3D11::DepthRenderTarget RenderTarget = { D3D11::RenderTargetDefaultSize, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_D32_FLOAT };
	};

	struct BloomRenderData
	{
		// NOTE: Base half reduction of the main scene render target
		D3D11::RenderTarget BaseRenderTarget = { D3D11::RenderTargetDefaultSize, D3D11::RenderTargetHDRFormatRGBA };

		// NOTE: Final blurred render target, combination of ReduceRenderTargets / BlurRenderTargets
		D3D11::RenderTarget CombinedBlurRenderTarget = { ivec2(256, 144), D3D11::RenderTargetHDRFormatRGBA };

		// NOTE: BaseRenderTarget -> 256x144 -> ... -> 32x18
		std::array<D3D11::RenderTarget, 4> ReduceRenderTargets =
		{
			D3D11::RenderTarget { ivec2(256, 144), D3D11::RenderTargetHDRFormatRGBA },
			D3D11::RenderTarget { ivec2(128, 72), D3D11::RenderTargetHDRFormatRGBA },
			D3D11::RenderTarget { ivec2(64, 36), D3D11::RenderTargetHDRFormatRGBA },
			D3D11::RenderTarget { ivec2(32, 18), D3D11::RenderTargetHDRFormatRGBA },
		};

		std::array<D3D11::RenderTarget, 4> BlurRenderTargets =
		{
			D3D11::RenderTarget { ivec2(256, 144), D3D11::RenderTargetHDRFormatRGBA },
			D3D11::RenderTarget { ivec2(128, 72), D3D11::RenderTargetHDRFormatRGBA },
			D3D11::RenderTarget { ivec2(64, 36), D3D11::RenderTargetHDRFormatRGBA },
			D3D11::RenderTarget { ivec2(32, 18), D3D11::RenderTargetHDRFormatRGBA },
		};

		// NOTE: Auto Exposure
		std::array<D3D11::RenderTarget, 3> ExposureRenderTargets =
		{
			// NOTE: 32 x 18 ->  8 x  8
			D3D11::RenderTarget { ivec2(8, 8), D3D11::RenderTargetHDRFormatRGBA },
			// NOTE:  8 x  8 -> 32 x  1
			D3D11::RenderTarget { ivec2(32, 1), D3D11::RenderTargetHDRFormatRGBA },
			// NOTE: 32 x  1 ->  1 x  1
			D3D11::RenderTarget { ivec2(1, 1), D3D11::RenderTargetHDRFormatRGBA },
		};
	};

	struct OutputRenderData
	{
		// NOTE: Where the post processed final image gets rendered to
		D3D11::RenderTarget RenderTarget = { D3D11::RenderTargetDefaultSize, D3D11::RenderTargetLDRFormatRGBA };
	};

	class RenderTarget3DImpl : public RenderTarget3D
	{
	public:
		RenderTarget3DImpl() = default;
		~RenderTarget3DImpl() = default;

	public:
		ComfyTextureID GetTextureID() const override { return Output.RenderTarget; }
		const D3D11::RenderTarget& GetRenderTarget() const { return Output.RenderTarget; }

	public:
		std::unique_ptr<u8[]> TakeScreenshot() override { return Output.RenderTarget.StageAndCopyBackBuffer(); }

		SubTargetView GetSubTargets() override 
		{
			auto create = [](const char* name, auto& renderTarget) -> SubTarget { return SubTarget { name, renderTarget.GetSize(), ComfyTextureID(renderTarget) }; };

			tempSubTargets = 
			{
				create("Main Current", Main.CurrentOrResolved()),
				create("Main Previous", Main.PreviousOrResolved()),
				create("Shadow Map", Shadow.RenderTarget),
				create("Exponential Shadow Map [0]", Shadow.ExponentialRenderTargets[0]),
				create("Exponential Shadow Map [1]", Shadow.ExponentialRenderTargets[1]),
				create("Exponential Shadow Map Blur [0]", Shadow.ExponentialBlurRenderTargets[0]),
				create("Exponential Shadow Map Blur [1]", Shadow.ExponentialBlurRenderTargets[1]),
				create("Shadow Map Threshold", Shadow.ThresholdRenderTarget),
				create("Shadow Map Blur [0]", Shadow.BlurRenderTargets[0]),
				create("Shadow Map Blur [1]", Shadow.BlurRenderTargets[1]),
				create("Screen Reflection", Reflection.RenderTarget),
				create("SSS Main", SubsurfaceScattering.RenderTarget),
				create("SSS Filter [0]", SubsurfaceScattering.FilterRenderTargets[0]),
				create("SSS Filter [1]", SubsurfaceScattering.FilterRenderTargets[1]),
				create("SSS Filter [2]", SubsurfaceScattering.FilterRenderTargets[2]),
				create("Bloom Base", Bloom.BaseRenderTarget),
				create("Bloom Combined", Bloom.CombinedBlurRenderTarget),
				create("Bloom Reduce->Blur [0]", Bloom.ReduceRenderTargets[0]),
				create("Bloom Reduce->Blur [1]", Bloom.ReduceRenderTargets[1]),
				create("Bloom Reduce->Blur [2]", Bloom.ReduceRenderTargets[2]),
				create("Bloom Reduce->Blur [3]", Bloom.ReduceRenderTargets[3]),
				create("Exposure [0]", Bloom.ExposureRenderTargets[0]),
				create("Exposure [1]", Bloom.ExposureRenderTargets[1]),
				create("Exposure [2]", Bloom.ExposureRenderTargets[2]),
				create("Output", Output.RenderTarget),
			};
			
			return SubTargetView { tempSubTargets.data(), tempSubTargets.size() };
		}

	public:
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

	private:
		std::array<SubTarget, 25> tempSubTargets = {};
	};
}
