#pragma once
#include "Logger.h"
#include <glad/glad.h>

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

#ifdef COMFY_DEBUG  
#define GLCall(expression) \
	{ \
		expression; \
		__CHECK_GL_ERROR(#expression); \
	}
#else
#define GLCall(expression) \
	{ \
		expression; \
	}
#endif
