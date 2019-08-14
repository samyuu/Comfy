#pragma once
#include "Core/Logger.h"
#include <glad/glad.h>

namespace Graphics
{
#if defined(COMFY_DEBUG)

	static void __OnGlError()
	{
		int __breakpoint__ = true;
	}

#define __CHECK_GL_ERROR(description) \
	{ \
		unsigned int __glError__ = glGetError(); \
		if (__glError__ != GL_NO_ERROR) \
		{ \
			Logger::LogErrorLine("[GL_ERROR] %s(): Line.%d: %s (Error = %d)", __FUNCTION__, __LINE__, description, __glError__); \
			__OnGlError(); \
		} \
	}

#define GLCall(expression) \
	{ \
		expression; \
		__CHECK_GL_ERROR(#expression); \
	}

#elif defined(COMFY_RELEASE)

#define GLCall(expression) \
	{ \
		expression; \
	}

#endif /* COMFY_DEBUG / COMFY_RELEASE */
}
