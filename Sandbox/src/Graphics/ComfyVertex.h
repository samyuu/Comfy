#pragma once
#include "Types.h"

struct SpriteVertex
{
	vec2 Position;
	vec2 TextureCoordinates;
	vec4 Color;
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