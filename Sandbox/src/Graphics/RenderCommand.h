#pragma once
#include "Types.h"

namespace Graphics
{
	typedef int32_t ClearTarget;
	enum ClearTarget_Enum : ClearTarget
	{
		ClearTarget_None = 0,
		ClearTarget_ColorBuffer = 1 << 0,
		ClearTarget_DepthBuffer = 1 << 1,
		ClearTarget_AccumBuffer = 1 << 2,
		ClearTarget_StencilBuffer = 1 << 3,
	};

	class RenderCommand
	{
	private:
		RenderCommand() = delete;
		~RenderCommand() = delete;

	public:
		static void SetClearColor(const vec4& color);
		static void Clear(ClearTarget target);
		static void SetViewport(const vec2& size);
	};
}
