#pragma once
#include "Types.h"
#include "Graphics/Auth3D/SceneRenderParameters.h"
#include "Graphics/Auth3D/ObjSet.h"

namespace Comfy::Graphics::D3D11
{
	struct TextureSamplerCache
	{
	public:
		void CreateIfNeeded(const SceneRenderParameters& renderParameters);
		TextureSampler& GetSampler(MaterialTextureData::TextureSamplerFlags flags);

	private:
		enum AddressMode { Mirror, Repeat, Clamp, AddressMode_Count };

		i32 lastAnistropicFiltering = -1;
		std::array<std::array<std::unique_ptr<TextureSampler>, AddressMode_Count>, AddressMode_Count> samplers;
	};
}
