#pragma once
#include "Types.h"

namespace Comfy::Graphics
{
	enum ShaderFlags : uint32_t
	{
		ShaderFlags_VertexColor = 1 << 0,

		ShaderFlags_DiffuseTexture = 1 << 1,
		ShaderFlags_AmbientTexture = 1 << 2,
		ShaderFlags_NormalTexture = 1 << 3,
		ShaderFlags_SpecularTexture = 1 << 4,
		ShaderFlags_TransparencyTexture = 1 << 5,
		ShaderFlags_EnvironmentTexture = 1 << 6,
		ShaderFlags_TranslucencyTexture = 1 << 7,
		ShaderFlags_ScreenTexture = 1 << 8,

		ShaderFlags_PunchThrough = 1 << 9,
		ShaderFlags_ClipPlane = 1 << 10,
		ShaderFlags_LinearFog = 1 << 11,

		ShaderFlags_Morph = 1 << 12,
		ShaderFlags_MorphColor = 1 << 13,
		ShaderFlags_Skinning = 1 << 14,

		ShaderFlags_Shadow = 1 << 15,
		ShaderFlags_ShadowSecondary = 1 << 16,
		ShaderFlags_SelfShadow = 1 << 17,
	};
}
