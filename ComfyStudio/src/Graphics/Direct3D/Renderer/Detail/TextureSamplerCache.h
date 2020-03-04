#pragma once
#include "Graphics/Auth3D/SceneContext.h"
#include "Graphics/Auth3D/ObjSet.h"

namespace Comfy::Graphics
{
	struct TextureSamplerCache
	{
	public:
		void CreateIfNeeded(const RenderParameters& renderParameters);
		D3D_TextureSampler& GetSampler(MaterialTextureFlags flags);

	private:
		enum AddressMode { Mirror, Repeat, Clamp, AddressMode_Count };

		int32_t lastAnistropicFiltering = -1;
		std::array<std::array<UniquePtr<D3D_TextureSampler>, AddressMode_Count>, AddressMode_Count> samplers;
	};
}
