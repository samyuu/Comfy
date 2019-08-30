#include "RenderCommand.h"
#include "Graphics.h"

namespace Graphics
{
	static inline GLenum GetGLPrimitiveEnum(PrimitiveType primitiveType)
	{
		switch (primitiveType)
		{
		case PrimitiveType::Points:
			return GL_POINTS;
		case PrimitiveType::Lines:
			return GL_LINES;
		case PrimitiveType::Triangles:
			return GL_TRIANGLES;
		case PrimitiveType::TriangleStrip:
			return GL_TRIANGLE_STRIP;
		default:
			assert(false);
			return GL_INVALID_ENUM;
		}
	}

	RenderCommand::State::State()
	{
		memset(this, 0, sizeof(RenderCommand::state));
	}

	uint32_t& RenderCommand::State::GetLastBoundTextureID()
	{
		return LastBoundTexture[LastTextureSlot];
	}

	const uint32_t& RenderCommand::State::GetLastBoundTextureID() const
	{
		return LastBoundTexture[LastTextureSlot];
	}

	RenderCommand::State RenderCommand::state;

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

	void RenderCommand::SetViewport(int32_t width, int32_t height)
	{
		GLCall(glViewport(0, 0, static_cast<GLint>(width), static_cast<GLint>(height)));
	}

	void RenderCommand::SetViewport(const vec2& size)
	{
		RenderCommand::SetViewport(static_cast<GLint>(size.x), static_cast<GLint>(size.y));
	}

	void RenderCommand::BindShaderProgram(uint32_t programID)
	{
		if (!OptimizeRedundantCommands || programID != state.LastBoundShaderProgram)
		{
			GLCall(glUseProgram(programID));
		}

		state.LastBoundShaderProgram = programID;
	}

	void RenderCommand::SetTextureSlot(int32_t textureSlot)
	{
		if (!OptimizeRedundantCommands || textureSlot != state.LastTextureSlot)
		{
			GLCall(glActiveTexture(GL_TEXTURE0 + textureSlot));
		}

		state.LastTextureSlot = textureSlot;
	}

	void RenderCommand::BindTexture(uint32_t textureTargetEnum, uint32_t textureID)
	{
		uint32_t& lastBoundTextureID = state.GetLastBoundTextureID();

		if (!OptimizeRedundantCommands || textureID != lastBoundTextureID)
		{
			GLCall(glBindTexture(textureTargetEnum, textureID));
		}

		lastBoundTextureID = textureID;
	}

	void RenderCommand::DrawArrays(PrimitiveType primitiveType, int32_t first, int32_t count)
	{
		// TODO: count vertices
		GLCall(glDrawArrays(GetGLPrimitiveEnum(primitiveType), first, count));
	}

	void RenderCommand::DrawElements(PrimitiveType primitiveType, int32_t count, uint32_t typeEnum, const void* indices)
	{
		// TODO: count vertices
		GLCall(glDrawElements(GetGLPrimitiveEnum(primitiveType), count, typeEnum, indices));
	}
}
