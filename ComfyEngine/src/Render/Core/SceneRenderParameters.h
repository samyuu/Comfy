#pragma once
#include "Types.h"
#include "Graphics/D3D11/Texture/RenderTarget.h"

namespace Comfy::Graphics
{
	static constexpr ivec2 ShadowMapDefaultResolution = ivec2(2048, 2048);
	static constexpr ivec2 ReflectionDefaultResolution = ivec2(512, 512);

	// NOTE: Different user controlable render graphics settings
	struct SceneRenderParameters
	{
		// DEBUG: Non specific debug flags for quick testing
		u32 DebugFlags = 0;
		u32 ShaderDebugFlags = 0;
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
		i32 AnistropicFiltering = 16;

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
		u32 MultiSampleCount = 1;
#else
		u32 MultiSampleCount = 4;
#endif

		bool ShadowMapping = true;
		bool SelfShadowing = true;

		ivec2 ShadowMapResolution = ShadowMapDefaultResolution;
		u32 ShadowBlurPasses = 1;

		bool RenderSubsurfaceScattering = true;

		bool RenderReflection = true;
		bool ClearReflection = true;
		ivec2 ReflectionRenderResolution = ReflectionDefaultResolution;

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
}
