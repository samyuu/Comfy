#pragma once
#include "Types.h"
#include "Graphics/Auth3D/ObjSet.h"

namespace Comfy::Render
{
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
			bool IgnoreShadowCastObjFlags = true;
			
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
}
