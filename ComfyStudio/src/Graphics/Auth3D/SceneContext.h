#pragma once
#include "LightParam/Parameters.h"
#include "Graphics/Camera.h"
#include "Graphics/D3D11/Renderer/ViewportRenderData.h"
#include "Resource/IDTypes.h"
#include <optional>

namespace Comfy::Graphics
{
	// NOTE: Different user controlable render graphics settings
	struct RenderParameters
	{
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

		ivec2 ShadowMapResolution = D3D11::ShadowMapDefaultResolution;
		uint32_t ShadowBlurPasses = 1;

		bool RenderSubsurfaceScattering = true;

		bool RenderReflection = true;
		bool ClearReflection = true;
		ivec2 ReflectionRenderResolution = D3D11::ReflectionDefaultResolution;

		bool Clear = true;
		vec4 ClearColor = vec4(0.16f, 0.16f, 0.16f, 0.00f);

		/*
		struct OutputPostProcessParameters
		{
			float Saturation = 2.2f;
			float Brightness = 0.45455f;
		} OutputPostProcess;
		*/
	};

	struct SceneViewport
	{
		PerspectiveCamera Camera;
		RenderParameters Parameters;
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
