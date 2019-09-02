#pragma once
#include "Types.h"

namespace Graphics
{
	struct SpriteVertex
	{
		// TODO: consider using 16-bit texture coordinates
		vec2 Position;
		vec2 TextureCoordinates;
		vec2 TextureMaskCoordinates;
		uint32_t Color;
	};

	struct ComfyVertex
	{
		vec3 Position;
		vec2 TextureCoordinates;
		vec4 Color;
	};

	struct LineVertex
	{
		vec3 Position;
		vec4 Color;
	};
}
