#pragma once
#include "Render/Core/Renderer3D/RenderTarget3D.h"
#include "TextureSamplerCache.h"
#include "BlendStateCache.h"
#include "ToneMapData.h"
#include "SunOcclusionData.h"

namespace Comfy::Render::Detail
{
	struct MainRenderData
	{
		// NOTE: Main scene HRD render targets
		std::array<D3D11RenderTargetAndView, 2> RenderTargets =
		{
			D3D11RenderTargetAndView { GlobalD3D11, D3D11RenderTargetDefaultSize, D3D11RenderTargetHDRFormatRGBA, DXGI_FORMAT_D32_FLOAT, 1 },
			D3D11RenderTargetAndView { GlobalD3D11, D3D11RenderTargetDefaultSize, D3D11RenderTargetHDRFormatRGBA, DXGI_FORMAT_D32_FLOAT, 1 },
		};

		// NOTE: Optional MSAA resolved copies
		std::array<D3D11RenderTargetAndView, 2> ResolvedRenderTargets =
		{
			D3D11RenderTargetAndView { GlobalD3D11, D3D11RenderTargetDefaultSize, D3D11RenderTargetHDRFormatRGBA },
			D3D11RenderTargetAndView { GlobalD3D11, D3D11RenderTargetDefaultSize, D3D11RenderTargetHDRFormatRGBA },
		};

	public:
		inline bool MSAAEnabled() { return Current().GetMultiSampleCount() > 1; }
		inline bool MSAAEnabledPrevious() { return Previous().GetMultiSampleCount() > 1; }

		inline void AdvanceRenderTarget() { currentIndex = (currentIndex + 1) % RenderTargets.size(); }

		inline D3D11RenderTargetAndView& Current() { return RenderTargets[currentIndex]; }
		inline D3D11RenderTargetAndView& Previous() { return RenderTargets[((currentIndex - 1) + RenderTargets.size()) % RenderTargets.size()]; }

		inline D3D11RenderTargetAndView& CurrentResolved() { return ResolvedRenderTargets[currentIndex]; }
		inline D3D11RenderTargetAndView& PreviousResolved() { return ResolvedRenderTargets[((currentIndex - 1) + ResolvedRenderTargets.size()) % ResolvedRenderTargets.size()]; }

		inline D3D11RenderTargetAndView& CurrentOrResolved() { return MSAAEnabled() ? CurrentResolved() : Current(); }
		inline D3D11RenderTargetAndView& PreviousOrResolved() { return MSAAEnabledPrevious() ? PreviousResolved() : Previous(); }

	private:
		// NOTE: Index of the currently active render target, keep switching to allow for last->current frame post processing effects and screen textures
		i32 currentIndex = 0;
	};

	struct ShadowMappingRenderData
	{
		static constexpr DXGI_FORMAT MainDepthFormat = DXGI_FORMAT_D32_FLOAT;
		static constexpr DXGI_FORMAT PostProcessingFormat = DXGI_FORMAT_R32_FLOAT;

		// NOTE: Main depth render target rendered to using the silhouette shader
		D3D11RenderTargetAndView RenderTarget = { GlobalD3D11, ShadowMapDefaultResolution, DXGI_FORMAT_UNKNOWN, MainDepthFormat };

		// NOTE: Full resolution render targets used for initial blurring
		std::array<D3D11RenderTargetAndView, 2> ExponentialRenderTargets =
		{
			D3D11RenderTargetAndView { GlobalD3D11, ShadowMapDefaultResolution, PostProcessingFormat },
			D3D11RenderTargetAndView { GlobalD3D11, ShadowMapDefaultResolution, PostProcessingFormat },
		};
		// NOTE: Half resolution render targets used for additional filtering, sampled by stage and character material shaders
		std::array<D3D11RenderTargetAndView, 2> ExponentialBlurRenderTargets =
		{
			D3D11RenderTargetAndView { GlobalD3D11, ShadowMapDefaultResolution / 4, PostProcessingFormat },
			D3D11RenderTargetAndView { GlobalD3D11, ShadowMapDefaultResolution / 4, PostProcessingFormat },
		};

		// NOTE: Processed main depth render target with a constant depth value
		D3D11RenderTargetAndView ThresholdRenderTarget = { GlobalD3D11, ShadowMapDefaultResolution / 2, PostProcessingFormat };

		// NOTE: Low resolution render targets used for ping pong blurring, sampled by stage material shaders
		std::array<D3D11RenderTargetAndView, 2> BlurRenderTargets =
		{
			D3D11RenderTargetAndView { GlobalD3D11, ShadowMapDefaultResolution / 4, PostProcessingFormat },
			D3D11RenderTargetAndView { GlobalD3D11, ShadowMapDefaultResolution / 4, PostProcessingFormat },
		};
	};

	struct SubsurfaceScatteringRenderData
	{
		// NOTE: Main render target, same size as the main rendering
		D3D11RenderTargetAndView RenderTarget = { GlobalD3D11, D3D11RenderTargetDefaultSize, D3D11RenderTargetHDRFormatRGBA, DXGI_FORMAT_D32_FLOAT };

		// NOTE: Further reduction and filtering
		std::array<D3D11RenderTargetAndView, 3> FilterRenderTargets =
		{
			D3D11RenderTargetAndView { GlobalD3D11, ivec2(640, 360), D3D11RenderTargetHDRFormatRGBA },
			D3D11RenderTargetAndView { GlobalD3D11, ivec2(320, 180), D3D11RenderTargetHDRFormatRGBA },
			D3D11RenderTargetAndView { GlobalD3D11, ivec2(320, 180), D3D11RenderTargetHDRFormatRGBA },
		};
	};

	struct ScreenReflectionRenderData
	{
		// NOTE: Stage reflection render target primarily used by floor materials
		D3D11RenderTargetAndView RenderTarget = { GlobalD3D11, ReflectionDefaultResolution, D3D11RenderTargetLDRFormatRGBA, DXGI_FORMAT_D32_FLOAT };
	};

	struct SilhouetteRenderData
	{
		// NOTE: Mostly for debugging, render black and white rendering to outline and overlay on the main render target
		D3D11RenderTargetAndView RenderTarget = { GlobalD3D11, D3D11RenderTargetDefaultSize, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_D32_FLOAT };
	};

	struct BloomRenderData
	{
		// NOTE: Base half reduction of the main scene render target
		D3D11RenderTargetAndView BaseRenderTarget = { GlobalD3D11, D3D11RenderTargetDefaultSize, D3D11RenderTargetHDRFormatRGBA };

		// NOTE: Final blurred render target, combination of ReduceRenderTargets / BlurRenderTargets
		D3D11RenderTargetAndView CombinedBlurRenderTarget = { GlobalD3D11, ivec2(256, 144), D3D11RenderTargetHDRFormatRGBA };

		// NOTE: BaseRenderTarget -> 256x144 -> ... -> 32x18
		std::array<D3D11RenderTargetAndView, 4> ReduceRenderTargets =
		{
			D3D11RenderTargetAndView { GlobalD3D11, ivec2(256, 144), D3D11RenderTargetHDRFormatRGBA },
			D3D11RenderTargetAndView { GlobalD3D11, ivec2(128, 72), D3D11RenderTargetHDRFormatRGBA },
			D3D11RenderTargetAndView { GlobalD3D11, ivec2(64, 36), D3D11RenderTargetHDRFormatRGBA },
			D3D11RenderTargetAndView { GlobalD3D11, ivec2(32, 18), D3D11RenderTargetHDRFormatRGBA },
		};

		std::array<D3D11RenderTargetAndView, 4> BlurRenderTargets =
		{
			D3D11RenderTargetAndView { GlobalD3D11, ivec2(256, 144), D3D11RenderTargetHDRFormatRGBA },
			D3D11RenderTargetAndView { GlobalD3D11, ivec2(128, 72), D3D11RenderTargetHDRFormatRGBA },
			D3D11RenderTargetAndView { GlobalD3D11, ivec2(64, 36), D3D11RenderTargetHDRFormatRGBA },
			D3D11RenderTargetAndView { GlobalD3D11, ivec2(32, 18), D3D11RenderTargetHDRFormatRGBA },
		};

		// NOTE: Auto Exposure
		std::array<D3D11RenderTargetAndView, 3> ExposureRenderTargets =
		{
			// NOTE: 32 x 18 ->  8 x  8
			D3D11RenderTargetAndView { GlobalD3D11, ivec2(8, 8), D3D11RenderTargetHDRFormatRGBA },
			// NOTE:  8 x  8 -> 32 x  1
			D3D11RenderTargetAndView { GlobalD3D11, ivec2(32, 1), D3D11RenderTargetHDRFormatRGBA },
			// NOTE: 32 x  1 ->  1 x  1
			D3D11RenderTargetAndView { GlobalD3D11, ivec2(1, 1), D3D11RenderTargetHDRFormatRGBA },
		};
	};

	struct OutputRenderData
	{
		// NOTE: Where the post processed final image gets rendered to
		D3D11RenderTargetAndView RenderTarget = { GlobalD3D11, D3D11RenderTargetDefaultSize, D3D11RenderTargetLDRFormatRGBA };
	};

	class RenderTarget3DImpl : public RenderTarget3D
	{
	public:
		RenderTarget3DImpl() = default;
		~RenderTarget3DImpl() = default;

	public:
		ComfyTextureID GetTextureID() const override { return Output.RenderTarget; }
		const D3D11RenderTargetAndView& GetRenderTarget() const { return Output.RenderTarget; }

	public:
		std::unique_ptr<u8[]> TakeScreenshot() override { return Output.RenderTarget.CopyColorPixelsBackToCPU(GlobalD3D11); }

		SubTargetsArrayView GetSubTargets() override
		{
			size_t addedCount = 0;
			auto add = [&](std::string_view name, D3D11RenderTargetAndView& renderTarget)
			{
				if (renderTarget.GetHasColorBuffer())
				{
					auto& subTarget = tempSubTargets[addedCount++];
					subTarget.Name = name;
					subTarget.Size = renderTarget.GetSize();
					subTarget.TextureID = ComfyTextureID(renderTarget, false);
				}

				if (renderTarget.GetHasDepthBuffer())
				{
					auto& subTarget = tempSubTargets[addedCount++];
					subTarget.Name = name; subTarget.Name += " (Depth)";
					subTarget.Size = renderTarget.GetSize();
					subTarget.TextureID = ComfyTextureID(renderTarget, true);
				}
			};

			add("Main Current", Main.CurrentOrResolved());
			add("Main Previous", Main.PreviousOrResolved());
			add("Shadow Map", Shadow.RenderTarget);
			add("Exponential Shadow Map [0]", Shadow.ExponentialRenderTargets[0]);
			add("Exponential Shadow Map [1]", Shadow.ExponentialRenderTargets[1]);
			add("Exponential Shadow Map Blur [0]", Shadow.ExponentialBlurRenderTargets[0]);
			add("Exponential Shadow Map Blur [1]", Shadow.ExponentialBlurRenderTargets[1]);
			add("Shadow Map Threshold", Shadow.ThresholdRenderTarget);
			add("Shadow Map Blur [0]", Shadow.BlurRenderTargets[0]);
			add("Shadow Map Blur [1]", Shadow.BlurRenderTargets[1]);
			add("Screen Reflection", Reflection.RenderTarget);
			add("SSS Main", SubsurfaceScattering.RenderTarget);
			add("SSS Filter [0]", SubsurfaceScattering.FilterRenderTargets[0]);
			add("SSS Filter [1]", SubsurfaceScattering.FilterRenderTargets[1]);
			add("SSS Filter [2]", SubsurfaceScattering.FilterRenderTargets[2]);
			add("Bloom Base", Bloom.BaseRenderTarget);
			add("Bloom Combined", Bloom.CombinedBlurRenderTarget);
			add("Bloom Reduce->Blur [0]", Bloom.ReduceRenderTargets[0]);
			add("Bloom Reduce->Blur [1]", Bloom.ReduceRenderTargets[1]);
			add("Bloom Reduce->Blur [2]", Bloom.ReduceRenderTargets[2]);
			add("Bloom Reduce->Blur [3]", Bloom.ReduceRenderTargets[3]);
			add("Exposure [0]", Bloom.ExposureRenderTargets[0]);
			add("Exposure [1]", Bloom.ExposureRenderTargets[1]);
			add("Exposure [2]", Bloom.ExposureRenderTargets[2]);
			add("Output", Output.RenderTarget);

			return SubTargetsArrayView { tempSubTargets.data(), addedCount };
		}

	public:
		MainRenderData Main;
		ShadowMappingRenderData Shadow;
		ScreenReflectionRenderData Reflection;
		SubsurfaceScatteringRenderData SubsurfaceScattering;
		SilhouetteRenderData Silhouette;
		BloomRenderData Bloom;
		OutputRenderData Output;

		TextureSamplerCache3D TextureSamplers;
		BlendStateCache BlendStates;
		ToneMapData ToneMap;
		SunOcclusionData Sun;

	private:
		std::array<SubTarget, 64> tempSubTargets = {};
	};
}
