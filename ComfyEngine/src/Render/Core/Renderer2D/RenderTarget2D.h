#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "ImGui/ComfyTextureID.h"

namespace Comfy::Render
{
	class RenderTarget2D : NonCopyable
	{
	public:
		RenderTarget2D() = default;
		virtual ~RenderTarget2D() = default;

	public:
		virtual ComfyTextureID GetTextureID() const = 0;

	public:
		// NOTE: Settings used by the Renderer2D
		struct RenderParam
		{
			ivec2 Resolution = { 1, 1 };
			u32 MultiSampleCount = 1;

			vec4 ClearColor = { 0.84, 0.67, 0.41, 1.0f };
		} Param;
	};
}
