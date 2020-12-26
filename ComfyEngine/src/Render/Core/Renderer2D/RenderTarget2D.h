#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "ImGui/ComfyTextureID.h"

namespace Comfy::Render
{
	// TODO: Should this be renamed to Viewport2D to clearly separate from the D3D11::RenderTargets (?)
	class RenderTarget2D : NonCopyable
	{
	public:
		RenderTarget2D() = default;
		virtual ~RenderTarget2D() = default;

	public:
		virtual ComfyTextureID GetTextureID() const = 0;

	public:
		virtual std::unique_ptr<u8[]> TakeScreenshot() = 0;

	public:
		// NOTE: Settings used by the Renderer2D
		struct RenderParam
		{
			ivec2 Resolution = { 1, 1 };
			u32 MultiSampleCount = 1;

			bool Clear = true;
			vec4 ClearColor = { 0.84, 0.67, 0.41, 1.0f };

			bool PostProcessingEnabled = false;
			struct PostProcessingData
			{
				// NOTE: Usually in range 2.0f to 3.0f
				f32 Gamma = 2.8f;

				// NOTE: Usually in range 0.35f to 0.80f
				f32 Contrast = 0.455f;

				// NOTE: Color levels matrix, usually in range 0.75f to 1.25f
				std::array<vec3, 3> ColorCoefficientsRGB =
				{
					vec3(1.0f, 0.0f, 0.0f),
					vec3(0.0f, 1.0f, 0.0f),
					vec3(0.0f, 0.0f, 1.0f),
				};
			} PostProcessing;

		} Param;
	};
}
