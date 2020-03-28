#pragma once
#include "Graphics/Auth3D/SceneContext.h"
#include "Graphics/Auth3D/ObjSet.h"

namespace Comfy::Graphics::D3D11
{
	struct TextureSamplerCache
	{
	public:
		void CreateIfNeeded(const RenderParameters& renderParameters);
		TextureSampler& GetSampler(MaterialTextureData::TextureSamplerFlags flags);

	private:
		enum AddressMode { Mirror, Repeat, Clamp, AddressMode_Count };

		int32_t lastAnistropicFiltering = -1;
		std::array<std::array<UniquePtr<TextureSampler>, AddressMode_Count>, AddressMode_Count> samplers;
	};
}
