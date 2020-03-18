#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Resource/IDTypes.h"
#include <optional>

namespace Comfy::Graphics
{
	struct ObjAnimationData
	{
		struct TexturePattern
		{
			Cached_TxpID ID = TxpID::Invalid;
			Cached_TxpID IDOverride = TxpID::Invalid;
			
			// NOTE: To easily index into and avoid needless searches
			std::optional<std::vector<TxpID>> CachedIDs;
		};

		struct TextureTransform
		{
			TxpID ID = TxpID::Invalid;

			std::optional<bool> RepeatU, RepeatV;
			float Rotation = 0.0f;
			vec2 Translation = vec2(0.0f);
		};

		struct MaterialOverride
		{
			const SubMesh* SubMeshToReplace = nullptr;
			const Material* NewMaterial = nullptr;
		};

		float MorphWeight = 1.0f;
		TxpID ScreenRenderTextureID = TxpID::Invalid;
		
		// TODO: Transparency, automatically add to transparent command list
		// std::optional<vec4> ColorTint;

		std::vector<TexturePattern> TexturePatterns;
		std::vector<TextureTransform> TextureTransforms;

		std::vector<MaterialOverride> MaterialOverrides;
	};
}
