#include "ToneMapData.h"

namespace Comfy::Graphics::D3D11
{
	void ToneMapData::UpdateIfNeeded(const GlowParameter& glow)
	{
		if (NeedsUpdating(glow))
			Update(glow);
	}

	Texture1D* ToneMapData::GetLookupTexture()
	{
		return lookupTexture.get();
	}

	bool ToneMapData::NeedsUpdating(const GlowParameter& glow)
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

	void ToneMapData::Update(const GlowParameter& glow)
	{
		lastSetGlow = glow;
		GenerateLookupData(glow);
		UpdateTexture();
	}

	void ToneMapData::GenerateLookupData(const GlowParameter& glow)
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

	void ToneMapData::UpdateTexture()
	{
		if (lookupTexture == nullptr)
		{
			lookupTexture = MakeUnique<Texture1D>(static_cast<int32_t>(textureData.size()), textureData.data(), DXGI_FORMAT_R32G32_FLOAT);
		}
		else
		{
			lookupTexture->UploadData(sizeof(textureData), textureData.data());
		}
	}
}
