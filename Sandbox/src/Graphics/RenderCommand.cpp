#include "RenderCommand.h"
#include "Graphics.h"

namespace Graphics
{
	void RenderCommand::SetClearColor(const vec4& color)
	{
		GLCall(glClearColor(color.x, color.y, color.z, color.w));
	}

	void RenderCommand::Clear(ClearTarget target)
	{
		assert(target != ClearTarget_None);

		GLbitfield mask = 0;
		if (target & ClearTarget_ColorBuffer)
			mask |= GL_COLOR_BUFFER_BIT;
		if (target & ClearTarget_DepthBuffer)
			mask |= GL_DEPTH_BUFFER_BIT;
		if (target & ClearTarget_AccumBuffer)
			mask |= GL_ACCUM_BUFFER_BIT;
		if (target & ClearTarget_StencilBuffer)
			mask |= GL_STENCIL_BUFFER_BIT;
		GLCall(glClear(mask));
	}

	void RenderCommand::SetViewport(const vec2& size)
	{
		GLCall(glViewport(0, 0, static_cast<GLint>(size.x), static_cast<GLint>(size.y)));
	}
}
