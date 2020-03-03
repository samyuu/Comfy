#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Core/IDTypes.h"
#include <optional>

namespace Graphics
{
	struct ObjAnimationData
	{
		struct TexturePattern
		{
			TxpID ID = TxpID::Invalid;
			TxpID IDOverride = TxpID::Invalid;
			
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

		float MorphWeight = 1.0f;
		TxpID ScreenRenderTextureID = TxpID::Invalid;
		
		// TODO: Transparency, automatically add to transparent command list
		// std::optional<vec4> ColorTint;

		std::vector<TexturePattern> TexturePatterns;
		std::vector<TextureTransform> TextureTransforms;
	};
}
