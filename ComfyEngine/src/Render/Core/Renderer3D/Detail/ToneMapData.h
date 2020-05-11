#pragma once
#include "Render/D3D11/Texture/Texture.h"
#include "Graphics/Auth3D/LightParam/GlowParameter.h"

namespace Comfy::Render::Detail
{
	struct ToneMapData
	{
	public:
		void UpdateIfNeeded(const Graphics::GlowParameter& glow)
		{
			if (NeedsUpdating(glow))
				Update(glow);
		}

		D3D11::Texture1D* GetLookupTexture()
		{
			return lookupTexture.get();
		}

	private:
		bool NeedsUpdating(const Graphics::GlowParameter& glow)
		{
			if (lookupTexture == nullptr)
				return true;

			if (lastSetGlow.Gamma != glow.Gamma)
				return true;

			if (lastSetGlow.SaturatePower != glow.SaturatePower)
				return true;

			if (lastSetGlow.SaturateCoefficient != glow.SaturateCoefficient)
				return true;

			return false;
		}

		void Update(const Graphics::GlowParameter& glow)
		{
			lastSetGlow = glow;
			GenerateLookupData(glow);
			UpdateTexture();
		}

		void GenerateLookupData(const Graphics::GlowParameter& glow)
		{
			const float pixelCount = static_cast<float>(textureData.size());
			const float gammaPower = 1.0f * glow.Gamma * 1.5f;
			const int saturatePowerCount = glow.SaturatePower * 4;

			textureData[0] = vec2(0.0f, 0.0f);
			for (int i = 1; i < static_cast<int>(textureData.size()); i++)
			{
				const float step = (static_cast<float>(i) * 16.0f) / pixelCount;
				const float gamma = glm::pow((1.0f - glm::exp(-step)), gammaPower);

				float saturation = (gamma * 2.0f) - 1.0f;
				for (int j = 0; j < saturatePowerCount; j++)
					saturation *= saturation;

				textureData[i].x = gamma;
				textureData[i].y = ((gamma * glow.SaturateCoefficient) / step) * (1.0f - saturation);
			}
		}

		void UpdateTexture()
		{
			if (lookupTexture == nullptr)
				lookupTexture = std::make_unique<D3D11::Texture1D>(static_cast<i32>(textureData.size()), textureData.data(), DXGI_FORMAT_R32G32_FLOAT);
			else
				lookupTexture->UploadData(sizeof(textureData), textureData.data());
		}

	private:
		Graphics::GlowParameter lastSetGlow;

		std::array<vec2, 512> textureData;
		std::unique_ptr<D3D11::Texture1D> lookupTexture = nullptr;
	};
}
