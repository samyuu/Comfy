#pragma once
#include "LightParam/Parameters.h"
#include "Graphics/Camera.h"
#include "Resource/IDTypes.h"
#include <optional>

namespace Comfy::Graphics
{
	// NOTE: Different user controlable render graphics settings
	struct RenderParameters
	{
		static constexpr ivec2 ShadowMapDefaultResolution = ivec2(2048, 2048);
		static constexpr ivec2 ReflectionDefaultResolution = ivec2(512, 512);

		// DEBUG: Non specific debug flags for quick testing
		uint32_t DebugFlags = 0;
		uint32_t ShaderDebugFlags = 0;
		vec4 ShaderDebugValue = vec4(1.0f, 1.0f, 1.0f, 1.0f);
		bool AllowDebugShaderOverride = true;

		bool DebugVisualizeOcclusionQuery = true;
		bool LastFrameOcclusionQueryOptimization = true;

		// TODO: In the future (to then only enable for certain viewports)
		// bool VisualizeNormals = false;
		// bool VisualizeShadowCasters = false;
		// bool VisualizeShadowReceivers = false;
		// bool VisualizeMorphMeshes = false;
		// bool VisualizeSkinMeshes = false;
		// bool VisualizeMipMapLevels = false; // (Object.CalculateLevelOfDetail(...))
		// bool OverrideSimpleShader = false;

		// NOTE: Used by all newly created texture samplers
		int32_t AnistropicFiltering = D3D11_DEFAULT_MAX_ANISOTROPY;

		bool FrustumCulling = true;
		bool Wireframe = false;
		bool AlphaSort = true;
		bool RenderOpaque = true;
		bool RenderTransparent = true;
		bool RenderBloom = true;
		bool RenderLensFlare = true;
		bool AutoExposure = true;

		bool VertexColoring = true;
		bool DiffuseMapping = true;
		bool AmbientOcclusionMapping = true;
		bool NormalMapping = true;
		bool SpecularMapping = true;
		bool TransparencyMapping = true;
		bool EnvironmentMapping = true;
		bool TranslucencyMapping = true;

		bool RenderPunchThrough = true;
		bool RenderFog = true;
		bool ObjectBillboarding = true;
		bool ObjectMorphing = true;
		bool ObjectSkinning = true;

		// NOTE: Enable to take alpha render target captures with a clear color alpha of zero
		bool ToneMapPreserveAlpha = false;

		ivec2 RenderResolution = D3D11::RenderTargetDefaultSize;

#if COMFY_DEBUG
		uint32_t MultiSampleCount = 1;
#else
		uint32_t MultiSampleCount = 4;
#endif

		bool ShadowMapping = true;
		bool SelfShadowing = true;

		ivec2 ShadowMapResolution = ShadowMapDefaultResolution;
		uint32_t ShadowBlurPasses = 1;

		bool RenderSubsurfaceScattering = true;

		bool RenderReflection = true;
		bool ClearReflection = true;
		ivec2 ReflectionRenderResolution = ReflectionDefaultResolution;

		bool Clear = true;
		vec4 ClearColor = vec4(0.16f, 0.16f, 0.16f, 0.00f);

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
		int32_t currentIndex = 0;
	};

	struct ShadowMappingRenderData
	{
		static constexpr DXGI_FORMAT MainDepthFormat = DXGI_FORMAT_D32_FLOAT;
		static constexpr DXGI_FORMAT PostProcessingFormat = DXGI_FORMAT_R32_FLOAT;

		// NOTE: Main depth render target rendered to using the silhouette shader
		D3D11::DepthOnlyRenderTarget RenderTarget = { RenderParameters::ShadowMapDefaultResolution, MainDepthFormat };

		// NOTE: Full resolution render targets used for initial blurring
		std::array<D3D11::RenderTarget, 2> ExponentialRenderTargets =
		{
			D3D11::RenderTarget { RenderParameters::ShadowMapDefaultResolution, PostProcessingFormat },
			D3D11::RenderTarget { RenderParameters::ShadowMapDefaultResolution, PostProcessingFormat },
		};
		// NOTE: Half resolution render targets used for additional filtering, sampled by stage and character material shaders
		std::array<D3D11::RenderTarget, 2> ExponentialBlurRenderTargets =
		{
			D3D11::RenderTarget { RenderParameters::ShadowMapDefaultResolution / 4, PostProcessingFormat },
			D3D11::RenderTarget { RenderParameters::ShadowMapDefaultResolution / 4, PostProcessingFormat },
		};

		// NOTE: Processed main depth render target with a constant depth value
		D3D11::RenderTarget ThresholdRenderTarget = { RenderParameters::ShadowMapDefaultResolution / 2, PostProcessingFormat };

		// NOTE: Low resolution render targets used for ping pong blurring, sampled by stage material shaders
		std::array<D3D11::RenderTarget, 2> BlurRenderTargets =
		{
			D3D11::RenderTarget { RenderParameters::ShadowMapDefaultResolution / 4, PostProcessingFormat },
			D3D11::RenderTarget { RenderParameters::ShadowMapDefaultResolution / 4, PostProcessingFormat },
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
		D3D11::DepthRenderTarget RenderTarget = { RenderParameters::ReflectionDefaultResolution, D3D11::RenderTargetLDRFormatRGBA, DXGI_FORMAT_D32_FLOAT };
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

	// TODO: Move into Graphics/D3D11/
	struct RenderData
	{
		MainRenderData Main;
		ShadowMappingRenderData Shadow;
		ScreenReflectionRenderData Reflection;
		SubsurfaceScatteringRenderData SubsurfaceScattering;
		SilhouetteRenderData Silhouette;
		BloomRenderData Bloom;
		OutputRenderData Output;
	};

	struct SceneViewport
	{
		PerspectiveCamera Camera;
		RenderParameters Parameters;
		RenderData Data;
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
