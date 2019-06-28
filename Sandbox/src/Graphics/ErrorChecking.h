#pragma once
#include "Logger.h"

#define CHECK_GL_ERROR(description) \
	{ \
		unsigned int __glError__ = glGetError(); \
		if (__glError__ != GL_NO_ERROR) \
			Logger::LogErrorLine("[GL_ERROR] %s(): Line.%d: %s (Error = %d)", __FUNCTION__, __LINE__, description, __glError__); \
	} \
