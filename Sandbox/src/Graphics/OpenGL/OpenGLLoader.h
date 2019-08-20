#pragma once
#include "OpenGL.h"

typedef void* OpenGLFunctionLoader(const char* functionName);

namespace Graphics
{
	class OpenGLLoader
	{
	public:
		static void LoadFunctions(OpenGLFunctionLoader* functionLoader);

	private:
		static inline void SetLoadFunction(OpenGLFunctionLoader* functionLoader, void** function, const char* functionName);
	};
}