#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Render/Core/Camera.h"
#include "Resource/IDTypes.h"
#include "Graphics/Auth3D/ObjSet.h"
#include "Graphics/Auth3D/LightParam/Parameters.h"
#include <optional>
#include <functional>

namespace Comfy::Render
{
	using TexGetter = std::function<const Graphics::Tex*(const Cached_TexID* texID)>;

	static constexpr ivec2 ShadowMapDefaultResolution = ivec2(2048, 2048);
	static constexpr ivec2 ReflectionDefaultResolution = ivec2(512, 512);

	/*
	struct SceneViewport
	{
		PerspectiveCamera Camera;
		SceneRenderParameters Parameters;
		D3D11::ViewportRenderData Data;
	};
	*/

	// NOTE: Storage for all of the render targets, managed by the renederer
	struct ViewportData3D;

	struct ViewportParam3D
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

		ivec2 RenderResolution = { 1, 1 };

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

	struct SceneParam3D
	{
		struct LensFlareParam
		{
			// NOTE: Lens flare sun position
			std::optional<vec3> SunPosition;
			const Graphics::Obj* SunObj = nullptr;

			struct Textures
			{
				// NOTE: Common sun texture from effcmn
				Cached_TexID Sun = TexID::Invalid;
				// NOTE: Stage specific textures
				std::array<Cached_TexID, 2> Flares = { TexID::Invalid, TexID::Invalid };
				Cached_TexID Ghost = TexID::Invalid;
			} Textures;
		} LensFlare;

		Graphics::FogParameter Fog;
		Graphics::GlowParameter Glow;
		Graphics::LightParameter Light;
		Graphics::IBLParameters IBL;
	};

	struct RenderCommand3D
	{
	public:
		RenderCommand3D() = default;
		RenderCommand3D(const Graphics::Obj& obj) : SourceObj(&obj) {}
		RenderCommand3D(const Graphics::Obj& obj, const vec3& position) : SourceObj(&obj), Transform(position) {}

	public:
		struct FlagsData
		{
			bool IsReflection = false;
			bool SilhouetteOutline = false;

			bool CastsShadow = false;
			bool ReceivesShadow = true;

			// TODO:
			// int ShadowMapIndex = -1;
			// bool SubsurfaceScattering = false;
			// bool SubsurfaceScatteringFocusPoint = false;
			// bool Skeleton = false;

			// NOTE: Optionally render the specified subset of the SourceObj instead
			int MeshIndex = -1;
			int SubMeshIndex = -1;
		};

		struct DynamicData
		{
			struct TexturePattern
			{
				Cached_TexID SourceID = TexID::Invalid;
				Cached_TexID OverrideID = TexID::Invalid;

				// NOTE: To easily index into and avoid needless searches
				std::optional<std::vector<TexID>> CachedIDs;
			};

			struct TextureTransform
			{
				TexID SourceID = TexID::Invalid;

				std::optional<bool> RepeatU, RepeatV;
				float Rotation = 0.0f;
				vec2 Translation = vec2(0.0f);
			};

			struct MaterialOverride
			{
				const Graphics::SubMesh* SubMeshToReplace = nullptr;
				const Graphics::Material* NewMaterial = nullptr;
			};

			const Graphics::Obj* MorphObj = nullptr;
			float MorphWeight = 1.0f;

			TexID ScreenRenderTextureID = TexID::Invalid;

			// TODO: Transparency, automatically add to transparent command list
			// std::optional<vec4> ColorTint;

			std::vector<TexturePattern> TexturePatterns;
			std::vector<TextureTransform> TextureTransforms;

			std::vector<MaterialOverride> MaterialOverrides;
		};

	public:
		const Graphics::Obj* SourceObj = nullptr;
		Graphics::Transform Transform = Graphics::Transform(vec3(0.0f));

		FlagsData Flags;

		// NOTE: Additional optional data typically animated by A3Ds, stores as a pointer to avoid expensive copies
		const DynamicData* Dynamic = nullptr;
	};

	class Renderer3D : NonCopyable
	{
	public:
		Renderer3D(TexGetter texGetter);
		~Renderer3D();

	public:
		void Begin(PerspectiveCamera& camera, ViewportData3D& viewportData, const ViewportParam3D& viewportParam, const SceneParam3D& sceneParam);
		void Draw(const RenderCommand3D& command);

		// TODO: DrawRect(), DrawCircle(), DrawCylinder(), DrawLine(), DrawSpriteBillboard(), etc.

		void End();

		const Graphics::Tex* GetTexFromTextureID(const Cached_TexID* textureID) const;

	private:
		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}
