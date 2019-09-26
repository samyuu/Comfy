#pragma once
#include "OpenGLBaseDefines.h"

#pragma region OpenGL Function Type Defines
typedef void glTexParameterf_t(GLenum target, GLenum pname, GLfloat param);
typedef void glTexParameterfv_t(GLenum target, GLenum pname, const GLfloat* params);
typedef void glTexParameteri_t(GLenum target, GLenum pname, GLint param);
typedef void glTexParameteriv_t(GLenum target, GLenum pname, const GLint* params);
typedef void glTexImage1D_t(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void* pixels);
typedef void glTexImage2D_t(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels);
typedef void glDrawBuffer_t(GLenum buf);
typedef void glClear_t(GLbitfield mask);
typedef void glClearColor_t(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void glDrawArrays_t(GLenum mode, GLint first, GLsizei count);
typedef void glDrawElements_t(GLenum mode, GLsizei count, GLenum type, const void* indices);
typedef void glGetPointerv_t(GLenum pname, void** params);
typedef void glPolygonOffset_t(GLfloat factor, GLfloat units);
typedef void glCopyTexImage1D_t(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
typedef void glCopyTexImage2D_t(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
typedef void glCopyTexSubImage1D_t(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
typedef void glCopyTexSubImage2D_t(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void glTexSubImage1D_t(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void* pixels);
typedef void glTexSubImage2D_t(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
typedef void glBindTexture_t(GLenum target, GLuint texture);
typedef void glDeleteTextures_t(GLsizei n, const GLuint* textures);
typedef void glGenTextures_t(GLsizei n, GLuint* textures);
typedef void glLineWidth_t(GLfloat width);
typedef void glPointSize_t(GLfloat size);
typedef void glPolygonMode_t(GLenum face, GLenum mode);
typedef void glScissor_t(GLint x, GLint y, GLsizei width, GLsizei height);
typedef void glDisable_t(GLenum cap);
typedef void glEnable_t(GLenum cap);
typedef void glBlendFunc_t(GLenum sfactor, GLenum dfactor);
typedef void glStencilFunc_t(GLenum func, GLint ref, GLuint mask);
typedef void glDepthFunc_t(GLenum func);
typedef void glPixelStoref_t(GLenum pname, GLfloat param);
typedef void glPixelStorei_t(GLenum pname, GLint param);
typedef void glReadBuffer_t(GLenum src);
typedef void glReadPixels_t(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels);
typedef void glGetBooleanv_t(GLenum pname, GLboolean* data);
typedef void glGetDoublev_t(GLenum pname, GLdouble* data);
typedef GLenum glGetError_t(void);
typedef void glGetFloatv_t(GLenum pname, GLfloat* data);
typedef void glGetIntegerv_t(GLenum pname, GLint* data);
typedef const GLubyte* glGetString_t(GLenum name);
typedef void glGetTexImage_t(GLenum target, GLint level, GLenum format, GLenum type, void* pixels);
typedef void glGetTexParameterfv_t(GLenum target, GLenum pname, GLfloat* params);
typedef void glGetTexParameteriv_t(GLenum target, GLenum pname, GLint* params);
typedef void glGetTexLevelParameterfv_t(GLenum target, GLint level, GLenum pname, GLfloat* params);
typedef void glGetTexLevelParameteriv_t(GLenum target, GLint level, GLenum pname, GLint* params);
typedef GLboolean glIsEnabled_t(GLenum cap);
typedef void glDepthRange_t(GLdouble n, GLdouble f);
typedef void glViewport_t(GLint x, GLint y, GLsizei width, GLsizei height);
typedef void glVertexAttribPointer_t(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);
typedef GLint glGetAttribLocation_t(GLuint program, const GLchar* name);
typedef void glGetProgramiv_t(GLuint program, GLenum pname, GLint* params);
typedef void glGetProgramInfoLog_t(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef void glGetShaderiv_t(GLuint shader, GLenum pname, GLint *params);
typedef void glGetShaderInfoLog_t(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef void glGetShaderSource_t(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* source);
typedef GLint glGetUniformLocation_t(GLuint program, const GLchar* name);
typedef void glGetUniformfv_t(GLuint program, GLint location, GLfloat* params);
typedef void glGetUniformiv_t(GLuint program, GLint location, GLint* params);
typedef void glGetVertexAttribdv_t(GLuint index, GLenum pname, GLdouble* params);
typedef void glGetVertexAttribfv_t(GLuint index, GLenum pname, GLfloat* params);
typedef void glGetVertexAttribiv_t(GLuint index, GLenum pname, GLint* params);
typedef void glGetVertexAttribPointerv_t(GLuint index, GLenum pname, void** pointer);
typedef void glLinkProgram_t(GLuint program);
typedef void glShaderSource_t(GLuint shader, GLsizei count, const GLchar *const*string, const GLint* length);
typedef void glUseProgram_t(GLuint program);
typedef void glUniform1f_t(GLint location, GLfloat v0);
typedef void glUniform2f_t(GLint location, GLfloat v0, GLfloat v1);
typedef void glUniform3f_t(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void glUniform4f_t(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void glUniform1i_t(GLint location, GLint v0);
typedef void glUniform2i_t(GLint location, GLint v0, GLint v1);
typedef void glUniform3i_t(GLint location, GLint v0, GLint v1, GLint v2);
typedef void glUniform4i_t(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
typedef void glUniform1fv_t(GLint location, GLsizei count, const GLfloat* value);
typedef void glUniform2fv_t(GLint location, GLsizei count, const GLfloat* value);
typedef void glUniform3fv_t(GLint location, GLsizei count, const GLfloat* value);
typedef void glUniform4fv_t(GLint location, GLsizei count, const GLfloat* value);
typedef void glUniform1iv_t(GLint location, GLsizei count, const GLint* value);
typedef void glUniform2iv_t(GLint location, GLsizei count, const GLint* value);
typedef void glUniform3iv_t(GLint location, GLsizei count, const GLint* value);
typedef void glUniform4iv_t(GLint location, GLsizei count, const GLint* value);
typedef void glUniformMatrix2fv_t(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void glUniformMatrix3fv_t(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void glUniformMatrix4fv_t(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void glActiveTexture_t(GLenum texture);
typedef void glCompressedTexImage3D_t(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void* data);
typedef void glCompressedTexImage2D_t(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* data);
typedef void glCompressedTexImage1D_t(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void* data);
typedef void glCompressedTexSubImage3D_t(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* data);
typedef void glCompressedTexSubImage2D_t(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data);
typedef void glCompressedTexSubImage1D_t(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void* data);
typedef void glBlendFuncSeparate_t(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
typedef void glMultiDrawArrays_t(GLenum mode, const GLint* first, const GLsizei* count, GLsizei drawcount);
typedef void glMultiDrawElements_t(GLenum mode, const GLsizei* count, GLenum type, const void* const* indices, GLsizei drawcount);
typedef void glBindRenderbuffer_t(GLenum target, GLuint renderbuffer);
typedef void glDeleteRenderbuffers_t(GLsizei n, const GLuint* renderbuffers);
typedef void glGenRenderbuffers_t(GLsizei n, GLuint* renderbuffers);
typedef void glRenderbufferStorage_t(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void glGetRenderbufferParameteriv_t(GLenum target, GLenum pname, GLint *params);
typedef void glBindFramebuffer_t(GLenum target, GLuint framebuffer);
typedef void glDeleteFramebuffers_t(GLsizei n, const GLuint* framebuffers);
typedef void glGenFramebuffers_t(GLsizei n, GLuint* framebuffers);
typedef GLenum glCheckFramebufferStatus_t(GLenum target);
typedef void glFramebufferTexture1D_t(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void glFramebufferTexture2D_t(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void glFramebufferTexture3D_t(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
typedef void glFramebufferRenderbuffer_t(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef void glGetFramebufferAttachmentParameteriv_t(GLenum target, GLenum attachment, GLenum pname, GLint* params);
typedef void glGenerateMipmap_t(GLenum target);
typedef void glBlendEquationSeparate_t(GLenum modeRGB, GLenum modeAlpha);
typedef void glAttachShader_t(GLuint program, GLuint shader);
typedef void glCompileShader_t(GLuint shader);
typedef GLuint glCreateProgram_t(void);
typedef GLuint glCreateShader_t(GLenum type);
typedef void glDeleteProgram_t(GLuint program);
typedef void glDeleteShader_t(GLuint shader);
typedef void glDetachShader_t(GLuint program, GLuint shader);
typedef void glDisableVertexAttribArray_t(GLuint index);
typedef void glEnableVertexAttribArray_t(GLuint index);
typedef void glGetActiveAttrib_t(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
typedef void glBlendColor_t(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void glBlendEquation_t(GLenum mode);
typedef void glBindBuffer_t(GLenum target, GLuint buffer);
typedef void glDeleteBuffers_t(GLsizei n, const GLuint *buffers);
typedef void glGenBuffers_t(GLsizei n, GLuint *buffers);
typedef void glBufferData_t(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
typedef void glBufferSubData_t(GLenum target, GLintptr offset, GLsizeiptr size, const void* data);
typedef void glGetBufferSubData_t(GLenum target, GLintptr offset, GLsizeiptr size, void* data);
typedef void* glMapBuffer_t(GLenum target, GLenum access);
typedef GLboolean glUnmapBuffer_t(GLenum target);
typedef void glBindVertexArray_t(GLuint array);
typedef void glDeleteVertexArrays_t(GLsizei n, const GLuint* arrays);
typedef void glGenVertexArrays_t(GLsizei n, GLuint* arrays);
typedef void glGetBufferParameteriv_t(GLenum target, GLenum pname, GLint* params);
typedef void glGetBufferPointerv_t(GLenum target, GLenum pname, void** params);
typedef void glGenSamplers_t(GLsizei count, GLuint* samplers);
typedef void glDeleteSamplers_t(GLsizei count, const GLuint* samplers);
typedef void glBindSampler_t(GLuint unit, GLuint sampler);
typedef void glSamplerParameteri_t(GLuint sampler, GLenum pname, GLint param);
typedef void glSamplerParameteriv_t(GLuint sampler, GLenum pname, const GLint* param);
typedef void glSamplerParameterf_t(GLuint sampler, GLenum pname, GLfloat param);
typedef void glSamplerParameterfv_t(GLuint sampler, GLenum pname, const GLfloat* param);
typedef void glSamplerParameterIiv_t(GLuint sampler, GLenum pname, const GLint* param);
typedef void glSamplerParameterIuiv_t(GLuint sampler, GLenum pname, const GLuint* param);
typedef void glGetSamplerParameteriv_t(GLuint sampler, GLenum pname, GLint* params);
typedef void glGetSamplerParameterIiv_t(GLuint sampler, GLenum pname, GLint* params);
typedef void glGetSamplerParameterfv_t(GLuint sampler, GLenum pname, GLfloat* params);
typedef void glGetSamplerParameterIuiv_t(GLuint sampler, GLenum pname, GLuint* params);
typedef void glObjectLabel_t(GLenum identifier, GLuint name, GLsizei length, const char* label);
#pragma endregion

#pragma region OpenGL Function Defines
#define glTexParameterf							Graphics::OpenGL::GLTexParameterf
#define glTexParameterfv						Graphics::OpenGL::GLTexParameterfv
#define glTexParameteri							Graphics::OpenGL::GLTexParameteri
#define glTexParameteriv						Graphics::OpenGL::GLTexParameteriv
#define glTexImage1D							Graphics::OpenGL::GLTexImage1D
#define glTexImage2D							Graphics::OpenGL::GLTexImage2D
#define glDrawBuffer							Graphics::OpenGL::GLDrawBuffer
#define glClear									Graphics::OpenGL::GLClear
#define glClearColor							Graphics::OpenGL::GLClearColor
#define glDrawArrays							Graphics::OpenGL::GLDrawArrays
#define glDrawElements							Graphics::OpenGL::GLDrawElements
#define glGetPointerv							Graphics::OpenGL::GLGetPointerv
#define glPolygonOffset 						Graphics::OpenGL::GLPolygonOffset
#define glCopyTexImage1D 						Graphics::OpenGL::GLCopyTexImage1D
#define glCopyTexImage2D 						Graphics::OpenGL::GLCopyTexImage2D
#define glCopyTexSubImage1D 					Graphics::OpenGL::GLCopyTexSubImage1D
#define glCopyTexSubImage2D 					Graphics::OpenGL::GLCopyTexSubImage2D
#define glTexSubImage1D 						Graphics::OpenGL::GLTexSubImage1D
#define glTexSubImage2D 						Graphics::OpenGL::GLTexSubImage2D
#define glBindTexture 							Graphics::OpenGL::GLBindTexture
#define glDeleteTextures 						Graphics::OpenGL::GLDeleteTextures
#define glGenTextures 							Graphics::OpenGL::GLGenTextures
#define glLineWidth 							Graphics::OpenGL::GLLineWidth
#define glPointSize 							Graphics::OpenGL::GLPointSize
#define glPolygonMode 							Graphics::OpenGL::GLPolygonMode
#define glScissor 								Graphics::OpenGL::GLScissor
#define glDisable 								Graphics::OpenGL::GLDisable
#define glEnable 								Graphics::OpenGL::GLEnable
#define glBlendFunc 							Graphics::OpenGL::GLBlendFunc
#define glStencilFunc 							Graphics::OpenGL::GLStencilFunc
#define glDepthFunc 							Graphics::OpenGL::GLDepthFunc
#define glPixelStoref 							Graphics::OpenGL::GLPixelStoref
#define glPixelStorei 							Graphics::OpenGL::GLPixelStorei
#define glReadBuffer 							Graphics::OpenGL::GLReadBuffer
#define glReadPixels 							Graphics::OpenGL::GLReadPixels
#define glGetBooleanv 							Graphics::OpenGL::GLGetBooleanv
#define glGetDoublev 							Graphics::OpenGL::GLGetDoublev
#define glGetError 								Graphics::OpenGL::GLGetError
#define glGetFloatv 							Graphics::OpenGL::GLGetFloatv
#define glGetIntegerv 							Graphics::OpenGL::GLGetIntegerv
#define glGetString 							Graphics::OpenGL::GLGetString
#define glGetTexImage 							Graphics::OpenGL::GLGetTexImage
#define glGetTexParameterfv 					Graphics::OpenGL::GLGetTexParameterfv
#define glGetTexParameteriv 					Graphics::OpenGL::GLGetTexParameteriv
#define glGetTexLevelParameterfv 				Graphics::OpenGL::GLGetTexLevelParameterfv
#define glGetTexLevelParameteriv 				Graphics::OpenGL::GLGetTexLevelParameteriv
#define glIsEnabled 							Graphics::OpenGL::GLIsEnabled
#define glDepthRange 							Graphics::OpenGL::GLDepthRange
#define glViewport 								Graphics::OpenGL::GLViewport
#define glVertexAttribPointer 					Graphics::OpenGL::GLVertexAttribPointer
#define glGetAttribLocation 					Graphics::OpenGL::GLGetAttribLocation
#define glGetProgramiv 							Graphics::OpenGL::GLGetProgramiv
#define glGetProgramInfoLog 					Graphics::OpenGL::GLGetProgramInfoLog
#define glGetShaderiv 							Graphics::OpenGL::GLGetShaderiv
#define glGetShaderInfoLog 						Graphics::OpenGL::GLGetShaderInfoLog
#define glGetShaderSource 						Graphics::OpenGL::GLGetShaderSource
#define glGetUniformLocation 					Graphics::OpenGL::GLGetUniformLocation
#define glGetUniformfv 							Graphics::OpenGL::GLGetUniformfv
#define glGetUniformiv 							Graphics::OpenGL::GLGetUniformiv
#define glGetVertexAttribdv 					Graphics::OpenGL::GLGetVertexAttribdv
#define glGetVertexAttribfv 					Graphics::OpenGL::GLGetVertexAttribfv
#define glGetVertexAttribiv 					Graphics::OpenGL::GLGetVertexAttribiv
#define glGetVertexAttribPointerv 				Graphics::OpenGL::GLGetVertexAttribPointerv
#define glLinkProgram 							Graphics::OpenGL::GLLinkProgram
#define glShaderSource 							Graphics::OpenGL::GLShaderSource
#define glUseProgram 							Graphics::OpenGL::GLUseProgram
#define glUniform1f 							Graphics::OpenGL::GLUniform1f
#define glUniform2f 							Graphics::OpenGL::GLUniform2f
#define glUniform3f 							Graphics::OpenGL::GLUniform3f
#define glUniform4f 							Graphics::OpenGL::GLUniform4f
#define glUniform1i 							Graphics::OpenGL::GLUniform1i
#define glUniform2i 							Graphics::OpenGL::GLUniform2i
#define glUniform3i 							Graphics::OpenGL::GLUniform3i
#define glUniform4i 							Graphics::OpenGL::GLUniform4i
#define glUniform1fv 							Graphics::OpenGL::GLUniform1fv
#define glUniform2fv 							Graphics::OpenGL::GLUniform2fv
#define glUniform3fv 							Graphics::OpenGL::GLUniform3fv
#define glUniform4fv 							Graphics::OpenGL::GLUniform4fv
#define glUniform1iv 							Graphics::OpenGL::GLUniform1iv
#define glUniform2iv 							Graphics::OpenGL::GLUniform2iv
#define glUniform3iv 							Graphics::OpenGL::GLUniform3iv
#define glUniform4iv 							Graphics::OpenGL::GLUniform4iv
#define glUniformMatrix2fv 						Graphics::OpenGL::GLUniformMatrix2fv
#define glUniformMatrix3fv 						Graphics::OpenGL::GLUniformMatrix3fv
#define glUniformMatrix4fv 						Graphics::OpenGL::GLUniformMatrix4fv
#define glActiveTexture 						Graphics::OpenGL::GLActiveTexture
#define glCompressedTexImage3D 					Graphics::OpenGL::GLCompressedTexImage3D
#define glCompressedTexImage2D 					Graphics::OpenGL::GLCompressedTexImage2D
#define glCompressedTexImage1D 					Graphics::OpenGL::GLCompressedTexImage1D
#define glCompressedTexSubImage3D 				Graphics::OpenGL::GLCompressedTexSubImage3D
#define glCompressedTexSubImage2D 				Graphics::OpenGL::GLCompressedTexSubImage2D
#define glCompressedTexSubImage1D 				Graphics::OpenGL::GLCompressedTexSubImage1D
#define glBlendFuncSeparate 					Graphics::OpenGL::GLBlendFuncSeparate
#define glMultiDrawArrays 						Graphics::OpenGL::GLMultiDrawArrays
#define glMultiDrawElements 					Graphics::OpenGL::GLMultiDrawElements
#define glBindRenderbuffer 						Graphics::OpenGL::GLBindRenderbuffer
#define glDeleteRenderbuffers 					Graphics::OpenGL::GLDeleteRenderbuffers
#define glGenRenderbuffers 						Graphics::OpenGL::GLGenRenderbuffers
#define glRenderbufferStorage 					Graphics::OpenGL::GLRenderbufferStorage
#define glGetRenderbufferParameteriv 			Graphics::OpenGL::GLGetRenderbufferParameteriv
#define glBindFramebuffer 						Graphics::OpenGL::GLBindFramebuffer
#define glDeleteFramebuffers 					Graphics::OpenGL::GLDeleteFramebuffers
#define glGenFramebuffers 						Graphics::OpenGL::GLGenFramebuffers
#define glCheckFramebufferStatus 				Graphics::OpenGL::GLCheckFramebufferStatus
#define glFramebufferTexture1D 					Graphics::OpenGL::GLFramebufferTexture1D
#define glFramebufferTexture2D 					Graphics::OpenGL::GLFramebufferTexture2D
#define glFramebufferTexture3D 					Graphics::OpenGL::GLFramebufferTexture3D
#define glFramebufferRenderbuffer 				Graphics::OpenGL::GLFramebufferRenderbuffer
#define glGetFramebufferAttachmentParameteriv 	Graphics::OpenGL::GLGetFramebufferAttachmentParameteriv
#define glGenerateMipmap 						Graphics::OpenGL::GLGenerateMipmap
#define glBlendEquationSeparate 				Graphics::OpenGL::GLBlendEquationSeparate
#define glAttachShader 							Graphics::OpenGL::GLAttachShader
#define glCompileShader 						Graphics::OpenGL::GLCompileShader
#define glCreateProgram 						Graphics::OpenGL::GLCreateProgram
#define glCreateShader 							Graphics::OpenGL::GLCreateShader
#define glDeleteProgram 						Graphics::OpenGL::GLDeleteProgram
#define glDeleteShader 							Graphics::OpenGL::GLDeleteShader
#define glDetachShader 							Graphics::OpenGL::GLDetachShader
#define glDisableVertexAttribArray 				Graphics::OpenGL::GLDisableVertexAttribArray
#define glEnableVertexAttribArray 				Graphics::OpenGL::GLEnableVertexAttribArray
#define glGetActiveAttrib 						Graphics::OpenGL::GLGetActiveAttrib
#define glBlendColor 							Graphics::OpenGL::GLBlendColor
#define glBlendEquation 						Graphics::OpenGL::GLBlendEquation
#define glBindBuffer 							Graphics::OpenGL::GLBindBuffer
#define glDeleteBuffers 						Graphics::OpenGL::GLDeleteBuffers
#define glGenBuffers 							Graphics::OpenGL::GLGenBuffers
#define glBufferData 							Graphics::OpenGL::GLBufferData
#define glBufferSubData 						Graphics::OpenGL::GLBufferSubData
#define glGetBufferSubData 						Graphics::OpenGL::GLGetBufferSubData
#define glMapBuffer 							Graphics::OpenGL::GLMapBuffer
#define glUnmapBuffer 							Graphics::OpenGL::GLUnmapBuffer
#define glBindVertexArray 						Graphics::OpenGL::GLBindVertexArray
#define glDeleteVertexArrays 					Graphics::OpenGL::GLDeleteVertexArrays
#define glGenVertexArrays 						Graphics::OpenGL::GLGenVertexArrays
#define glGetBufferParameteriv 					Graphics::OpenGL::GLGetBufferParameteriv
#define glGetBufferPointerv 					Graphics::OpenGL::GLGetBufferPointerv
#define glGenSamplers 							Graphics::OpenGL::GLGenSamplers
#define glDeleteSamplers 						Graphics::OpenGL::GLDeleteSamplers
#define glBindSampler 							Graphics::OpenGL::GLBindSampler
#define glSamplerParameteri 					Graphics::OpenGL::GLSamplerParameteri
#define glSamplerParameteriv 					Graphics::OpenGL::GLSamplerParameteriv
#define glSamplerParameterf						Graphics::OpenGL::GLSamplerParameterf
#define glSamplerParameterfv					Graphics::OpenGL::GLSamplerParameterfv
#define glSamplerParameterIiv					Graphics::OpenGL::GLSamplerParameterIiv
#define glSamplerParameterIuiv					Graphics::OpenGL::GLSamplerParameterIuiv
#define glGetSamplerParameteriv					Graphics::OpenGL::GLGetSamplerParameteriv
#define glGetSamplerParameterIiv				Graphics::OpenGL::GLGetSamplerParameterIiv
#define glGetSamplerParameterfv					Graphics::OpenGL::GLGetSamplerParameterfv
#define glGetSamplerParameterIuiv				Graphics::OpenGL::GLGetSamplerParameterIuiv
#define glObjectLabel							Graphics::OpenGL::GLObjectLabel
#pragma endregion

namespace Graphics
{
	class OpenGL
	{
	public:
		static glTexParameterf_t*							GLTexParameterf;
		static glTexParameterfv_t*							GLTexParameterfv;
		static glTexParameteri_t*							GLTexParameteri;
		static glTexParameteriv_t*							GLTexParameteriv;
		static glTexImage1D_t*								GLTexImage1D;
		static glTexImage2D_t*								GLTexImage2D;
		static glDrawBuffer_t*								GLDrawBuffer;
		static glClear_t*									GLClear;
		static glClearColor_t*								GLClearColor;
		static glDrawArrays_t*								GLDrawArrays;
		static glDrawElements_t*							GLDrawElements;
		static glGetPointerv_t*								GLGetPointerv;
		static glPolygonOffset_t*							GLPolygonOffset;
		static glCopyTexImage1D_t*							GLCopyTexImage1D;
		static glCopyTexImage2D_t*							GLCopyTexImage2D;
		static glCopyTexSubImage1D_t*						GLCopyTexSubImage1D;
		static glCopyTexSubImage2D_t*						GLCopyTexSubImage2D;
		static glTexSubImage1D_t*							GLTexSubImage1D;
		static glTexSubImage2D_t*							GLTexSubImage2D;
		static glBindTexture_t*								GLBindTexture;
		static glDeleteTextures_t*							GLDeleteTextures;
		static glGenTextures_t*								GLGenTextures;
		static glLineWidth_t*								GLLineWidth;
		static glPointSize_t*								GLPointSize;
		static glPolygonMode_t*								GLPolygonMode;
		static glScissor_t*									GLScissor;
		static glDisable_t*									GLDisable;
		static glEnable_t*									GLEnable;
		static glBlendFunc_t*								GLBlendFunc;
		static glStencilFunc_t*								GLStencilFunc;
		static glDepthFunc_t*								GLDepthFunc;
		static glPixelStoref_t*								GLPixelStoref;
		static glPixelStorei_t*								GLPixelStorei;
		static glReadBuffer_t*								GLReadBuffer;
		static glReadPixels_t*								GLReadPixels;
		static glGetBooleanv_t*								GLGetBooleanv;
		static glGetDoublev_t*								GLGetDoublev;
		static glGetError_t*								GLGetError;
		static glGetFloatv_t*								GLGetFloatv;
		static glGetIntegerv_t*								GLGetIntegerv;
		static glGetString_t*								GLGetString;
		static glGetTexImage_t*								GLGetTexImage;
		static glGetTexParameterfv_t*						GLGetTexParameterfv;
		static glGetTexParameteriv_t*						GLGetTexParameteriv;
		static glGetTexLevelParameterfv_t*					GLGetTexLevelParameterfv;
		static glGetTexLevelParameteriv_t*					GLGetTexLevelParameteriv;
		static glIsEnabled_t*								GLIsEnabled;
		static glDepthRange_t*								GLDepthRange;
		static glViewport_t*								GLViewport;
		static glVertexAttribPointer_t*						GLVertexAttribPointer;
		static glGetAttribLocation_t*						GLGetAttribLocation;
		static glGetProgramiv_t*							GLGetProgramiv;
		static glGetProgramInfoLog_t*						GLGetProgramInfoLog;
		static glGetShaderiv_t*								GLGetShaderiv;
		static glGetShaderInfoLog_t*						GLGetShaderInfoLog;
		static glGetShaderSource_t*							GLGetShaderSource;
		static glGetUniformLocation_t*						GLGetUniformLocation;
		static glGetUniformfv_t*							GLGetUniformfv;
		static glGetUniformiv_t*							GLGetUniformiv;
		static glGetVertexAttribdv_t*						GLGetVertexAttribdv;
		static glGetVertexAttribfv_t*						GLGetVertexAttribfv;
		static glGetVertexAttribiv_t*						GLGetVertexAttribiv;
		static glGetVertexAttribPointerv_t*					GLGetVertexAttribPointerv;
		static glLinkProgram_t*								GLLinkProgram;
		static glShaderSource_t*							GLShaderSource;
		static glUseProgram_t*								GLUseProgram;
		static glUniform1f_t*								GLUniform1f;
		static glUniform2f_t*								GLUniform2f;
		static glUniform3f_t*								GLUniform3f;
		static glUniform4f_t*								GLUniform4f;
		static glUniform1i_t*								GLUniform1i;
		static glUniform2i_t*								GLUniform2i;
		static glUniform3i_t*								GLUniform3i;
		static glUniform4i_t*								GLUniform4i;
		static glUniform1fv_t*								GLUniform1fv;
		static glUniform2fv_t*								GLUniform2fv;
		static glUniform3fv_t*								GLUniform3fv;
		static glUniform4fv_t*								GLUniform4fv;
		static glUniform1iv_t*								GLUniform1iv;
		static glUniform2iv_t*								GLUniform2iv;
		static glUniform3iv_t*								GLUniform3iv;
		static glUniform4iv_t*								GLUniform4iv;
		static glUniformMatrix2fv_t*						GLUniformMatrix2fv;
		static glUniformMatrix3fv_t*						GLUniformMatrix3fv;
		static glUniformMatrix4fv_t*						GLUniformMatrix4fv;
		static glActiveTexture_t*							GLActiveTexture;
		static glCompressedTexImage3D_t*					GLCompressedTexImage3D;
		static glCompressedTexImage2D_t*					GLCompressedTexImage2D;
		static glCompressedTexImage1D_t*					GLCompressedTexImage1D;
		static glCompressedTexSubImage3D_t*					GLCompressedTexSubImage3D;
		static glCompressedTexSubImage2D_t*					GLCompressedTexSubImage2D;
		static glCompressedTexSubImage1D_t*					GLCompressedTexSubImage1D;
		static glBlendFuncSeparate_t*						GLBlendFuncSeparate;
		static glMultiDrawArrays_t*							GLMultiDrawArrays;
		static glMultiDrawElements_t*						GLMultiDrawElements;
		static glBindRenderbuffer_t*						GLBindRenderbuffer;
		static glDeleteRenderbuffers_t*						GLDeleteRenderbuffers;
		static glGenRenderbuffers_t*						GLGenRenderbuffers;
		static glRenderbufferStorage_t*						GLRenderbufferStorage;
		static glGetRenderbufferParameteriv_t*				GLGetRenderbufferParameteriv;
		static glBindFramebuffer_t*							GLBindFramebuffer;
		static glDeleteFramebuffers_t*						GLDeleteFramebuffers;
		static glGenFramebuffers_t*							GLGenFramebuffers;
		static glCheckFramebufferStatus_t*					GLCheckFramebufferStatus;
		static glFramebufferTexture1D_t*					GLFramebufferTexture1D;
		static glFramebufferTexture2D_t*					GLFramebufferTexture2D;
		static glFramebufferTexture3D_t*					GLFramebufferTexture3D;
		static glFramebufferRenderbuffer_t*					GLFramebufferRenderbuffer;
		static glGetFramebufferAttachmentParameteriv_t*		GLGetFramebufferAttachmentParameteriv;
		static glGenerateMipmap_t*							GLGenerateMipmap;
		static glBlendEquationSeparate_t*					GLBlendEquationSeparate;
		static glAttachShader_t*							GLAttachShader;
		static glCompileShader_t*							GLCompileShader;
		static glCreateProgram_t*							GLCreateProgram;
		static glCreateShader_t*							GLCreateShader;
		static glDeleteProgram_t*							GLDeleteProgram;
		static glDeleteShader_t*							GLDeleteShader;
		static glDetachShader_t*							GLDetachShader;
		static glDisableVertexAttribArray_t*				GLDisableVertexAttribArray;
		static glEnableVertexAttribArray_t*					GLEnableVertexAttribArray;
		static glGetActiveAttrib_t*							GLGetActiveAttrib;
		static glBlendColor_t*								GLBlendColor;
		static glBlendEquation_t*							GLBlendEquation;
		static glBindBuffer_t*								GLBindBuffer;
		static glDeleteBuffers_t*							GLDeleteBuffers;
		static glGenBuffers_t*								GLGenBuffers;
		static glBufferData_t*								GLBufferData;
		static glBufferSubData_t*							GLBufferSubData;
		static glGetBufferSubData_t*						GLGetBufferSubData;
		static glMapBuffer_t*								GLMapBuffer;
		static glUnmapBuffer_t*								GLUnmapBuffer;
		static glBindVertexArray_t*							GLBindVertexArray;
		static glDeleteVertexArrays_t*						GLDeleteVertexArrays;
		static glGenVertexArrays_t*							GLGenVertexArrays;
		static glGetBufferParameteriv_t*					GLGetBufferParameteriv;
		static glGetBufferPointerv_t*						GLGetBufferPointerv;
		static glGenSamplers_t*								GLGenSamplers;
		static glDeleteSamplers_t*							GLDeleteSamplers;
		static glBindSampler_t*								GLBindSampler;
		static glSamplerParameteri_t*						GLSamplerParameteri;
		static glSamplerParameteriv_t*						GLSamplerParameteriv;
		static glSamplerParameterf_t*						GLSamplerParameterf;
		static glSamplerParameterfv_t*						GLSamplerParameterfv;
		static glSamplerParameterIiv_t*						GLSamplerParameterIiv;
		static glSamplerParameterIuiv_t*					GLSamplerParameterIuiv;
		static glGetSamplerParameteriv_t*					GLGetSamplerParameteriv;
		static glGetSamplerParameterIiv_t*					GLGetSamplerParameterIiv;
		static glGetSamplerParameterfv_t*					GLGetSamplerParameterfv;
		static glGetSamplerParameterIuiv_t*					GLGetSamplerParameterIuiv;
		static glObjectLabel_t*								GLObjectLabel;
	};
}
