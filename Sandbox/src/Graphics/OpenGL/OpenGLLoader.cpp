#include "OpenGLLoader.h"
#include "OpenGL.h"
#include "Core/DebugStopwatch.h"

#define LoadGLFunction(function) SetLoadFunction(functionLoader, reinterpret_cast<void**>(&function), #function)

namespace Graphics
{
	void OpenGLLoader::SetLoadFunction(OpenGLFunctionLoader* functionLoader, void** function, const char* functionName)
	{
		*function = functionLoader(functionName);
		assert(*function != nullptr);
	}

	void OpenGLLoader::LoadFunctions(OpenGLFunctionLoader* functionLoader)
	{
		LoadGLFunction(glTexParameterf);
		LoadGLFunction(glTexParameterfv);
		LoadGLFunction(glTexParameteri);
		LoadGLFunction(glTexParameteriv);
		LoadGLFunction(glTexImage1D);
		LoadGLFunction(glTexImage2D);
		LoadGLFunction(glDrawBuffer);
		LoadGLFunction(glClear);
		LoadGLFunction(glClearColor);
		LoadGLFunction(glDrawArrays);
		LoadGLFunction(glDrawElements);
		LoadGLFunction(glGetPointerv);
		LoadGLFunction(glPolygonOffset);
		LoadGLFunction(glCopyTexImage1D);
		LoadGLFunction(glCopyTexImage2D);
		LoadGLFunction(glCopyTexSubImage1D);
		LoadGLFunction(glCopyTexSubImage2D);
		LoadGLFunction(glTexSubImage1D);
		LoadGLFunction(glTexSubImage2D);
		LoadGLFunction(glBindTexture);
		LoadGLFunction(glDeleteTextures);
		LoadGLFunction(glGenTextures);
		LoadGLFunction(glLineWidth);
		LoadGLFunction(glPointSize);
		LoadGLFunction(glPolygonMode);
		LoadGLFunction(glScissor);
		LoadGLFunction(glDisable);
		LoadGLFunction(glEnable);
		LoadGLFunction(glBlendFunc);
		LoadGLFunction(glStencilFunc);
		LoadGLFunction(glDepthFunc);
		LoadGLFunction(glPixelStoref);
		LoadGLFunction(glPixelStorei);
		LoadGLFunction(glReadBuffer);
		LoadGLFunction(glReadPixels);
		LoadGLFunction(glGetBooleanv);
		LoadGLFunction(glGetDoublev);
		LoadGLFunction(glGetError);
		LoadGLFunction(glGetFloatv);
		LoadGLFunction(glGetIntegerv);
		LoadGLFunction(glGetString);
		LoadGLFunction(glGetTexImage);
		LoadGLFunction(glGetTexParameterfv);
		LoadGLFunction(glGetTexParameteriv);
		LoadGLFunction(glGetTexLevelParameterfv);
		LoadGLFunction(glGetTexLevelParameteriv);
		LoadGLFunction(glIsEnabled);
		LoadGLFunction(glDepthRange);
		LoadGLFunction(glViewport);
		LoadGLFunction(glVertexAttribPointer);
		LoadGLFunction(glGetAttribLocation);
		LoadGLFunction(glGetProgramiv);
		LoadGLFunction(glGetProgramInfoLog);
		LoadGLFunction(glGetShaderiv);
		LoadGLFunction(glGetShaderInfoLog);
		LoadGLFunction(glGetShaderSource);
		LoadGLFunction(glGetUniformLocation);
		LoadGLFunction(glGetUniformfv);
		LoadGLFunction(glGetUniformiv);
		LoadGLFunction(glGetVertexAttribdv);
		LoadGLFunction(glGetVertexAttribfv);
		LoadGLFunction(glGetVertexAttribiv);
		LoadGLFunction(glGetVertexAttribPointerv);
		LoadGLFunction(glLinkProgram);
		LoadGLFunction(glShaderSource);
		LoadGLFunction(glUseProgram);
		LoadGLFunction(glUniform1f);
		LoadGLFunction(glUniform2f);
		LoadGLFunction(glUniform3f);
		LoadGLFunction(glUniform4f);
		LoadGLFunction(glUniform1i);
		LoadGLFunction(glUniform2i);
		LoadGLFunction(glUniform3i);
		LoadGLFunction(glUniform4i);
		LoadGLFunction(glUniform1fv);
		LoadGLFunction(glUniform2fv);
		LoadGLFunction(glUniform3fv);
		LoadGLFunction(glUniform4fv);
		LoadGLFunction(glUniform1iv);
		LoadGLFunction(glUniform2iv);
		LoadGLFunction(glUniform3iv);
		LoadGLFunction(glUniform4iv);
		LoadGLFunction(glUniformMatrix2fv);
		LoadGLFunction(glUniformMatrix3fv);
		LoadGLFunction(glUniformMatrix4fv);
		LoadGLFunction(glActiveTexture);
		LoadGLFunction(glCompressedTexImage3D);
		LoadGLFunction(glCompressedTexImage2D);
		LoadGLFunction(glCompressedTexImage1D);
		LoadGLFunction(glCompressedTexSubImage3D);
		LoadGLFunction(glCompressedTexSubImage2D);
		LoadGLFunction(glCompressedTexSubImage1D);
		LoadGLFunction(glBlendFuncSeparate);
		LoadGLFunction(glMultiDrawArrays);
		LoadGLFunction(glMultiDrawElements);
		LoadGLFunction(glBindRenderbuffer);
		LoadGLFunction(glDeleteRenderbuffers);
		LoadGLFunction(glGenRenderbuffers);
		LoadGLFunction(glRenderbufferStorage);
		LoadGLFunction(glGetRenderbufferParameteriv);
		LoadGLFunction(glIsFramebuffer);
		LoadGLFunction(glBindFramebuffer);
		LoadGLFunction(glDeleteFramebuffers);
		LoadGLFunction(glGenFramebuffers);
		LoadGLFunction(glCheckFramebufferStatus);
		LoadGLFunction(glFramebufferTexture1D);
		LoadGLFunction(glFramebufferTexture2D);
		LoadGLFunction(glFramebufferTexture3D);
		LoadGLFunction(glFramebufferRenderbuffer);
		LoadGLFunction(glGetFramebufferAttachmentParameteriv);
		LoadGLFunction(glGenerateMipmap);
		LoadGLFunction(glBlendEquationSeparate);
		LoadGLFunction(glAttachShader);
		LoadGLFunction(glCompileShader);
		LoadGLFunction(glCreateProgram);
		LoadGLFunction(glCreateShader);
		LoadGLFunction(glDeleteProgram);
		LoadGLFunction(glDeleteShader);
		LoadGLFunction(glDetachShader);
		LoadGLFunction(glDisableVertexAttribArray);
		LoadGLFunction(glEnableVertexAttribArray);
		LoadGLFunction(glGetActiveAttrib);
		LoadGLFunction(glBlendColor);
		LoadGLFunction(glBlendEquation);
		LoadGLFunction(glBindBuffer);
		LoadGLFunction(glDeleteBuffers);
		LoadGLFunction(glGenBuffers);
		LoadGLFunction(glIsBuffer);
		LoadGLFunction(glBufferData);
		LoadGLFunction(glBufferSubData);
		LoadGLFunction(glGetBufferSubData);
		LoadGLFunction(glMapBuffer);
		LoadGLFunction(glUnmapBuffer);
		LoadGLFunction(glBindVertexArray);
		LoadGLFunction(glDeleteVertexArrays);
		LoadGLFunction(glGenVertexArrays);
		LoadGLFunction(glGetBufferParameteriv);
		LoadGLFunction(glGetBufferPointerv);
		LoadGLFunction(glGenSamplers);
		LoadGLFunction(glDeleteSamplers);
		LoadGLFunction(glIsSampler);
		LoadGLFunction(glBindSampler);
		LoadGLFunction(glSamplerParameteri);
		LoadGLFunction(glSamplerParameteriv);
		LoadGLFunction(glSamplerParameterf);
		LoadGLFunction(glSamplerParameterfv);
		LoadGLFunction(glSamplerParameterIiv);
		LoadGLFunction(glSamplerParameterIuiv);
		LoadGLFunction(glGetSamplerParameteriv);
		LoadGLFunction(glGetSamplerParameterIiv);
		LoadGLFunction(glGetSamplerParameterfv);
		LoadGLFunction(glGetSamplerParameterIuiv);
	}
}