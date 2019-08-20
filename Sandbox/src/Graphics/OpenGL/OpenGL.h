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
#pragma endregion

#pragma region OpenGL Function Defines
#define glTexParameterf							Graphics::OpenGL::Comfy_glTexParameterf
#define glTexParameterfv						Graphics::OpenGL::Comfy_glTexParameterfv
#define glTexParameteri							Graphics::OpenGL::Comfy_glTexParameteri
#define glTexParameteriv						Graphics::OpenGL::Comfy_glTexParameteriv
#define glTexImage1D							Graphics::OpenGL::Comfy_glTexImage1D
#define glTexImage2D							Graphics::OpenGL::Comfy_glTexImage2D
#define glDrawBuffer							Graphics::OpenGL::Comfy_glDrawBuffer
#define glClear									Graphics::OpenGL::Comfy_glClear
#define glClearColor							Graphics::OpenGL::Comfy_glClearColor
#define glDrawArrays							Graphics::OpenGL::Comfy_glDrawArrays
#define glDrawElements							Graphics::OpenGL::Comfy_glDrawElements
#define glGetPointerv							Graphics::OpenGL::Comfy_glGetPointerv
#define glPolygonOffset 						Graphics::OpenGL::Comfy_glPolygonOffset
#define glCopyTexImage1D 						Graphics::OpenGL::Comfy_glCopyTexImage1D
#define glCopyTexImage2D 						Graphics::OpenGL::Comfy_glCopyTexImage2D
#define glCopyTexSubImage1D 					Graphics::OpenGL::Comfy_glCopyTexSubImage1D
#define glCopyTexSubImage2D 					Graphics::OpenGL::Comfy_glCopyTexSubImage2D
#define glTexSubImage1D 						Graphics::OpenGL::Comfy_glTexSubImage1D
#define glTexSubImage2D 						Graphics::OpenGL::Comfy_glTexSubImage2D
#define glBindTexture 							Graphics::OpenGL::Comfy_glBindTexture
#define glDeleteTextures 						Graphics::OpenGL::Comfy_glDeleteTextures
#define glGenTextures 							Graphics::OpenGL::Comfy_glGenTextures
#define glLineWidth 							Graphics::OpenGL::Comfy_glLineWidth
#define glPointSize 							Graphics::OpenGL::Comfy_glPointSize
#define glPolygonMode 							Graphics::OpenGL::Comfy_glPolygonMode
#define glScissor 								Graphics::OpenGL::Comfy_glScissor
#define glDisable 								Graphics::OpenGL::Comfy_glDisable
#define glEnable 								Graphics::OpenGL::Comfy_glEnable
#define glBlendFunc 							Graphics::OpenGL::Comfy_glBlendFunc
#define glStencilFunc 							Graphics::OpenGL::Comfy_glStencilFunc
#define glDepthFunc 							Graphics::OpenGL::Comfy_glDepthFunc
#define glPixelStoref 							Graphics::OpenGL::Comfy_glPixelStoref
#define glPixelStorei 							Graphics::OpenGL::Comfy_glPixelStorei
#define glReadBuffer 							Graphics::OpenGL::Comfy_glReadBuffer
#define glReadPixels 							Graphics::OpenGL::Comfy_glReadPixels
#define glGetBooleanv 							Graphics::OpenGL::Comfy_glGetBooleanv
#define glGetDoublev 							Graphics::OpenGL::Comfy_glGetDoublev
#define glGetError 								Graphics::OpenGL::Comfy_glGetError
#define glGetFloatv 							Graphics::OpenGL::Comfy_glGetFloatv
#define glGetIntegerv 							Graphics::OpenGL::Comfy_glGetIntegerv
#define glGetString 							Graphics::OpenGL::Comfy_glGetString
#define glGetTexImage 							Graphics::OpenGL::Comfy_glGetTexImage
#define glGetTexParameterfv 					Graphics::OpenGL::Comfy_glGetTexParameterfv
#define glGetTexParameteriv 					Graphics::OpenGL::Comfy_glGetTexParameteriv
#define glGetTexLevelParameterfv 				Graphics::OpenGL::Comfy_glGetTexLevelParameterfv
#define glGetTexLevelParameteriv 				Graphics::OpenGL::Comfy_glGetTexLevelParameteriv
#define glIsEnabled 							Graphics::OpenGL::Comfy_glIsEnabled
#define glDepthRange 							Graphics::OpenGL::Comfy_glDepthRange
#define glViewport 								Graphics::OpenGL::Comfy_glViewport
#define glVertexAttribPointer 					Graphics::OpenGL::Comfy_glVertexAttribPointer
#define glGetAttribLocation 					Graphics::OpenGL::Comfy_glGetAttribLocation
#define glGetProgramiv 							Graphics::OpenGL::Comfy_glGetProgramiv
#define glGetProgramInfoLog 					Graphics::OpenGL::Comfy_glGetProgramInfoLog
#define glGetShaderiv 							Graphics::OpenGL::Comfy_glGetShaderiv
#define glGetShaderInfoLog 						Graphics::OpenGL::Comfy_glGetShaderInfoLog
#define glGetShaderSource 						Graphics::OpenGL::Comfy_glGetShaderSource
#define glGetUniformLocation 					Graphics::OpenGL::Comfy_glGetUniformLocation
#define glGetUniformfv 							Graphics::OpenGL::Comfy_glGetUniformfv
#define glGetUniformiv 							Graphics::OpenGL::Comfy_glGetUniformiv
#define glGetVertexAttribdv 					Graphics::OpenGL::Comfy_glGetVertexAttribdv
#define glGetVertexAttribfv 					Graphics::OpenGL::Comfy_glGetVertexAttribfv
#define glGetVertexAttribiv 					Graphics::OpenGL::Comfy_glGetVertexAttribiv
#define glGetVertexAttribPointerv 				Graphics::OpenGL::Comfy_glGetVertexAttribPointerv
#define glLinkProgram 							Graphics::OpenGL::Comfy_glLinkProgram
#define glShaderSource 							Graphics::OpenGL::Comfy_glShaderSource
#define glUseProgram 							Graphics::OpenGL::Comfy_glUseProgram
#define glUniform1f 							Graphics::OpenGL::Comfy_glUniform1f
#define glUniform2f 							Graphics::OpenGL::Comfy_glUniform2f
#define glUniform3f 							Graphics::OpenGL::Comfy_glUniform3f
#define glUniform4f 							Graphics::OpenGL::Comfy_glUniform4f
#define glUniform1i 							Graphics::OpenGL::Comfy_glUniform1i
#define glUniform2i 							Graphics::OpenGL::Comfy_glUniform2i
#define glUniform3i 							Graphics::OpenGL::Comfy_glUniform3i
#define glUniform4i 							Graphics::OpenGL::Comfy_glUniform4i
#define glUniform1fv 							Graphics::OpenGL::Comfy_glUniform1fv
#define glUniform2fv 							Graphics::OpenGL::Comfy_glUniform2fv
#define glUniform3fv 							Graphics::OpenGL::Comfy_glUniform3fv
#define glUniform4fv 							Graphics::OpenGL::Comfy_glUniform4fv
#define glUniform1iv 							Graphics::OpenGL::Comfy_glUniform1iv
#define glUniform2iv 							Graphics::OpenGL::Comfy_glUniform2iv
#define glUniform3iv 							Graphics::OpenGL::Comfy_glUniform3iv
#define glUniform4iv 							Graphics::OpenGL::Comfy_glUniform4iv
#define glUniformMatrix2fv 						Graphics::OpenGL::Comfy_glUniformMatrix2fv
#define glUniformMatrix3fv 						Graphics::OpenGL::Comfy_glUniformMatrix3fv
#define glUniformMatrix4fv 						Graphics::OpenGL::Comfy_glUniformMatrix4fv
#define glActiveTexture 						Graphics::OpenGL::Comfy_glActiveTexture
#define glCompressedTexImage3D 					Graphics::OpenGL::Comfy_glCompressedTexImage3D
#define glCompressedTexImage2D 					Graphics::OpenGL::Comfy_glCompressedTexImage2D
#define glCompressedTexImage1D 					Graphics::OpenGL::Comfy_glCompressedTexImage1D
#define glCompressedTexSubImage3D 				Graphics::OpenGL::Comfy_glCompressedTexSubImage3D
#define glCompressedTexSubImage2D 				Graphics::OpenGL::Comfy_glCompressedTexSubImage2D
#define glCompressedTexSubImage1D 				Graphics::OpenGL::Comfy_glCompressedTexSubImage1D
#define glBlendFuncSeparate 					Graphics::OpenGL::Comfy_glBlendFuncSeparate
#define glMultiDrawArrays 						Graphics::OpenGL::Comfy_glMultiDrawArrays
#define glMultiDrawElements 					Graphics::OpenGL::Comfy_glMultiDrawElements
#define glBindRenderbuffer 						Graphics::OpenGL::Comfy_glBindRenderbuffer
#define glDeleteRenderbuffers 					Graphics::OpenGL::Comfy_glDeleteRenderbuffers
#define glGenRenderbuffers 						Graphics::OpenGL::Comfy_glGenRenderbuffers
#define glRenderbufferStorage 					Graphics::OpenGL::Comfy_glRenderbufferStorage
#define glGetRenderbufferParameteriv 			Graphics::OpenGL::Comfy_glGetRenderbufferParameteriv
#define glBindFramebuffer 						Graphics::OpenGL::Comfy_glBindFramebuffer
#define glDeleteFramebuffers 					Graphics::OpenGL::Comfy_glDeleteFramebuffers
#define glGenFramebuffers 						Graphics::OpenGL::Comfy_glGenFramebuffers
#define glCheckFramebufferStatus 				Graphics::OpenGL::Comfy_glCheckFramebufferStatus
#define glFramebufferTexture1D 					Graphics::OpenGL::Comfy_glFramebufferTexture1D
#define glFramebufferTexture2D 					Graphics::OpenGL::Comfy_glFramebufferTexture2D
#define glFramebufferTexture3D 					Graphics::OpenGL::Comfy_glFramebufferTexture3D
#define glFramebufferRenderbuffer 				Graphics::OpenGL::Comfy_glFramebufferRenderbuffer
#define glGetFramebufferAttachmentParameteriv 	Graphics::OpenGL::Comfy_glGetFramebufferAttachmentParameteriv
#define glGenerateMipmap 						Graphics::OpenGL::Comfy_glGenerateMipmap
#define glBlendEquationSeparate 				Graphics::OpenGL::Comfy_glBlendEquationSeparate
#define glAttachShader 							Graphics::OpenGL::Comfy_glAttachShader
#define glCompileShader 						Graphics::OpenGL::Comfy_glCompileShader
#define glCreateProgram 						Graphics::OpenGL::Comfy_glCreateProgram
#define glCreateShader 							Graphics::OpenGL::Comfy_glCreateShader
#define glDeleteProgram 						Graphics::OpenGL::Comfy_glDeleteProgram
#define glDeleteShader 							Graphics::OpenGL::Comfy_glDeleteShader
#define glDetachShader 							Graphics::OpenGL::Comfy_glDetachShader
#define glDisableVertexAttribArray 				Graphics::OpenGL::Comfy_glDisableVertexAttribArray
#define glEnableVertexAttribArray 				Graphics::OpenGL::Comfy_glEnableVertexAttribArray
#define glGetActiveAttrib 						Graphics::OpenGL::Comfy_glGetActiveAttrib
#define glBlendColor 							Graphics::OpenGL::Comfy_glBlendColor
#define glBlendEquation 						Graphics::OpenGL::Comfy_glBlendEquation
#define glBindBuffer 							Graphics::OpenGL::Comfy_glBindBuffer
#define glDeleteBuffers 						Graphics::OpenGL::Comfy_glDeleteBuffers
#define glGenBuffers 							Graphics::OpenGL::Comfy_glGenBuffers
#define glBufferData 							Graphics::OpenGL::Comfy_glBufferData
#define glBufferSubData 						Graphics::OpenGL::Comfy_glBufferSubData
#define glGetBufferSubData 						Graphics::OpenGL::Comfy_glGetBufferSubData
#define glMapBuffer 							Graphics::OpenGL::Comfy_glMapBuffer
#define glUnmapBuffer 							Graphics::OpenGL::Comfy_glUnmapBuffer
#define glBindVertexArray 						Graphics::OpenGL::Comfy_glBindVertexArray
#define glDeleteVertexArrays 					Graphics::OpenGL::Comfy_glDeleteVertexArrays
#define glGenVertexArrays 						Graphics::OpenGL::Comfy_glGenVertexArrays
#define glGetBufferParameteriv 					Graphics::OpenGL::Comfy_glGetBufferParameteriv
#define glGetBufferPointerv 					Graphics::OpenGL::Comfy_glGetBufferPointerv
#define glGenSamplers 							Graphics::OpenGL::Comfy_glGenSamplers
#define glDeleteSamplers 						Graphics::OpenGL::Comfy_glDeleteSamplers
#define glBindSampler 							Graphics::OpenGL::Comfy_glBindSampler
#define glSamplerParameteri 					Graphics::OpenGL::Comfy_glSamplerParameteri
#define glSamplerParameteriv 					Graphics::OpenGL::Comfy_glSamplerParameteriv
#define glSamplerParameterf						Graphics::OpenGL::Comfy_glSamplerParameterf
#define glSamplerParameterfv					Graphics::OpenGL::Comfy_glSamplerParameterfv
#define glSamplerParameterIiv					Graphics::OpenGL::Comfy_glSamplerParameterIiv
#define glSamplerParameterIuiv					Graphics::OpenGL::Comfy_glSamplerParameterIuiv
#define glGetSamplerParameteriv					Graphics::OpenGL::Comfy_glGetSamplerParameteriv
#define glGetSamplerParameterIiv				Graphics::OpenGL::Comfy_glGetSamplerParameterIiv
#define glGetSamplerParameterfv					Graphics::OpenGL::Comfy_glGetSamplerParameterfv
#define glGetSamplerParameterIuiv				Graphics::OpenGL::Comfy_glGetSamplerParameterIuiv
#pragma endregion

namespace Graphics
{
	class OpenGL
	{
	public:
		static glTexParameterf_t*							Comfy_glTexParameterf;
		static glTexParameterfv_t*							Comfy_glTexParameterfv;
		static glTexParameteri_t*							Comfy_glTexParameteri;
		static glTexParameteriv_t*							Comfy_glTexParameteriv;
		static glTexImage1D_t*								Comfy_glTexImage1D;
		static glTexImage2D_t*								Comfy_glTexImage2D;
		static glDrawBuffer_t*								Comfy_glDrawBuffer;
		static glClear_t*									Comfy_glClear;
		static glClearColor_t*								Comfy_glClearColor;
		static glDrawArrays_t*								Comfy_glDrawArrays;
		static glDrawElements_t*							Comfy_glDrawElements;
		static glGetPointerv_t*								Comfy_glGetPointerv;
		static glPolygonOffset_t*							Comfy_glPolygonOffset;
		static glCopyTexImage1D_t*							Comfy_glCopyTexImage1D;
		static glCopyTexImage2D_t*							Comfy_glCopyTexImage2D;
		static glCopyTexSubImage1D_t*						Comfy_glCopyTexSubImage1D;
		static glCopyTexSubImage2D_t*						Comfy_glCopyTexSubImage2D;
		static glTexSubImage1D_t*							Comfy_glTexSubImage1D;
		static glTexSubImage2D_t*							Comfy_glTexSubImage2D;
		static glBindTexture_t*								Comfy_glBindTexture;
		static glDeleteTextures_t*							Comfy_glDeleteTextures;
		static glGenTextures_t*								Comfy_glGenTextures;
		static glLineWidth_t*								Comfy_glLineWidth;
		static glPointSize_t*								Comfy_glPointSize;
		static glPolygonMode_t*								Comfy_glPolygonMode;
		static glScissor_t*									Comfy_glScissor;
		static glDisable_t*									Comfy_glDisable;
		static glEnable_t*									Comfy_glEnable;
		static glBlendFunc_t*								Comfy_glBlendFunc;
		static glStencilFunc_t*								Comfy_glStencilFunc;
		static glDepthFunc_t*								Comfy_glDepthFunc;
		static glPixelStoref_t*								Comfy_glPixelStoref;
		static glPixelStorei_t*								Comfy_glPixelStorei;
		static glReadBuffer_t*								Comfy_glReadBuffer;
		static glReadPixels_t*								Comfy_glReadPixels;
		static glGetBooleanv_t*								Comfy_glGetBooleanv;
		static glGetDoublev_t*								Comfy_glGetDoublev;
		static glGetError_t*								Comfy_glGetError;
		static glGetFloatv_t*								Comfy_glGetFloatv;
		static glGetIntegerv_t*								Comfy_glGetIntegerv;
		static glGetString_t*								Comfy_glGetString;
		static glGetTexImage_t*								Comfy_glGetTexImage;
		static glGetTexParameterfv_t*						Comfy_glGetTexParameterfv;
		static glGetTexParameteriv_t*						Comfy_glGetTexParameteriv;
		static glGetTexLevelParameterfv_t*					Comfy_glGetTexLevelParameterfv;
		static glGetTexLevelParameteriv_t*					Comfy_glGetTexLevelParameteriv;
		static glIsEnabled_t*								Comfy_glIsEnabled;
		static glDepthRange_t*								Comfy_glDepthRange;
		static glViewport_t*								Comfy_glViewport;
		static glVertexAttribPointer_t*						Comfy_glVertexAttribPointer;
		static glGetAttribLocation_t*						Comfy_glGetAttribLocation;
		static glGetProgramiv_t*							Comfy_glGetProgramiv;
		static glGetProgramInfoLog_t*						Comfy_glGetProgramInfoLog;
		static glGetShaderiv_t*								Comfy_glGetShaderiv;
		static glGetShaderInfoLog_t*						Comfy_glGetShaderInfoLog;
		static glGetShaderSource_t*							Comfy_glGetShaderSource;
		static glGetUniformLocation_t*						Comfy_glGetUniformLocation;
		static glGetUniformfv_t*							Comfy_glGetUniformfv;
		static glGetUniformiv_t*							Comfy_glGetUniformiv;
		static glGetVertexAttribdv_t*						Comfy_glGetVertexAttribdv;
		static glGetVertexAttribfv_t*						Comfy_glGetVertexAttribfv;
		static glGetVertexAttribiv_t*						Comfy_glGetVertexAttribiv;
		static glGetVertexAttribPointerv_t*					Comfy_glGetVertexAttribPointerv;
		static glLinkProgram_t*								Comfy_glLinkProgram;
		static glShaderSource_t*							Comfy_glShaderSource;
		static glUseProgram_t*								Comfy_glUseProgram;
		static glUniform1f_t*								Comfy_glUniform1f;
		static glUniform2f_t*								Comfy_glUniform2f;
		static glUniform3f_t*								Comfy_glUniform3f;
		static glUniform4f_t*								Comfy_glUniform4f;
		static glUniform1i_t*								Comfy_glUniform1i;
		static glUniform2i_t*								Comfy_glUniform2i;
		static glUniform3i_t*								Comfy_glUniform3i;
		static glUniform4i_t*								Comfy_glUniform4i;
		static glUniform1fv_t*								Comfy_glUniform1fv;
		static glUniform2fv_t*								Comfy_glUniform2fv;
		static glUniform3fv_t*								Comfy_glUniform3fv;
		static glUniform4fv_t*								Comfy_glUniform4fv;
		static glUniform1iv_t*								Comfy_glUniform1iv;
		static glUniform2iv_t*								Comfy_glUniform2iv;
		static glUniform3iv_t*								Comfy_glUniform3iv;
		static glUniform4iv_t*								Comfy_glUniform4iv;
		static glUniformMatrix2fv_t*						Comfy_glUniformMatrix2fv;
		static glUniformMatrix3fv_t*						Comfy_glUniformMatrix3fv;
		static glUniformMatrix4fv_t*						Comfy_glUniformMatrix4fv;
		static glActiveTexture_t*							Comfy_glActiveTexture;
		static glCompressedTexImage3D_t*					Comfy_glCompressedTexImage3D;
		static glCompressedTexImage2D_t*					Comfy_glCompressedTexImage2D;
		static glCompressedTexImage1D_t*					Comfy_glCompressedTexImage1D;
		static glCompressedTexSubImage3D_t*					Comfy_glCompressedTexSubImage3D;
		static glCompressedTexSubImage2D_t*					Comfy_glCompressedTexSubImage2D;
		static glCompressedTexSubImage1D_t*					Comfy_glCompressedTexSubImage1D;
		static glBlendFuncSeparate_t*						Comfy_glBlendFuncSeparate;
		static glMultiDrawArrays_t*							Comfy_glMultiDrawArrays;
		static glMultiDrawElements_t*						Comfy_glMultiDrawElements;
		static glBindRenderbuffer_t*						Comfy_glBindRenderbuffer;
		static glDeleteRenderbuffers_t*						Comfy_glDeleteRenderbuffers;
		static glGenRenderbuffers_t*						Comfy_glGenRenderbuffers;
		static glRenderbufferStorage_t*						Comfy_glRenderbufferStorage;
		static glGetRenderbufferParameteriv_t*				Comfy_glGetRenderbufferParameteriv;
		static glBindFramebuffer_t*							Comfy_glBindFramebuffer;
		static glDeleteFramebuffers_t*						Comfy_glDeleteFramebuffers;
		static glGenFramebuffers_t*							Comfy_glGenFramebuffers;
		static glCheckFramebufferStatus_t*					Comfy_glCheckFramebufferStatus;
		static glFramebufferTexture1D_t*					Comfy_glFramebufferTexture1D;
		static glFramebufferTexture2D_t*					Comfy_glFramebufferTexture2D;
		static glFramebufferTexture3D_t*					Comfy_glFramebufferTexture3D;
		static glFramebufferRenderbuffer_t*					Comfy_glFramebufferRenderbuffer;
		static glGetFramebufferAttachmentParameteriv_t*		Comfy_glGetFramebufferAttachmentParameteriv;
		static glGenerateMipmap_t*							Comfy_glGenerateMipmap;
		static glBlendEquationSeparate_t*					Comfy_glBlendEquationSeparate;
		static glAttachShader_t*							Comfy_glAttachShader;
		static glCompileShader_t*							Comfy_glCompileShader;
		static glCreateProgram_t*							Comfy_glCreateProgram;
		static glCreateShader_t*							Comfy_glCreateShader;
		static glDeleteProgram_t*							Comfy_glDeleteProgram;
		static glDeleteShader_t*							Comfy_glDeleteShader;
		static glDetachShader_t*							Comfy_glDetachShader;
		static glDisableVertexAttribArray_t*				Comfy_glDisableVertexAttribArray;
		static glEnableVertexAttribArray_t*					Comfy_glEnableVertexAttribArray;
		static glGetActiveAttrib_t*							Comfy_glGetActiveAttrib;
		static glBlendColor_t*								Comfy_glBlendColor;
		static glBlendEquation_t*							Comfy_glBlendEquation;
		static glBindBuffer_t*								Comfy_glBindBuffer;
		static glDeleteBuffers_t*							Comfy_glDeleteBuffers;
		static glGenBuffers_t*								Comfy_glGenBuffers;
		static glBufferData_t*								Comfy_glBufferData;
		static glBufferSubData_t*							Comfy_glBufferSubData;
		static glGetBufferSubData_t*						Comfy_glGetBufferSubData;
		static glMapBuffer_t*								Comfy_glMapBuffer;
		static glUnmapBuffer_t*								Comfy_glUnmapBuffer;
		static glBindVertexArray_t*							Comfy_glBindVertexArray;
		static glDeleteVertexArrays_t*						Comfy_glDeleteVertexArrays;
		static glGenVertexArrays_t*							Comfy_glGenVertexArrays;
		static glGetBufferParameteriv_t*					Comfy_glGetBufferParameteriv;
		static glGetBufferPointerv_t*						Comfy_glGetBufferPointerv;
		static glGenSamplers_t*								Comfy_glGenSamplers;
		static glDeleteSamplers_t*							Comfy_glDeleteSamplers;
		static glBindSampler_t*								Comfy_glBindSampler;
		static glSamplerParameteri_t*						Comfy_glSamplerParameteri;
		static glSamplerParameteriv_t*						Comfy_glSamplerParameteriv;
		static glSamplerParameterf_t*						Comfy_glSamplerParameterf;
		static glSamplerParameterfv_t*						Comfy_glSamplerParameterfv;
		static glSamplerParameterIiv_t*						Comfy_glSamplerParameterIiv;
		static glSamplerParameterIuiv_t*					Comfy_glSamplerParameterIuiv;
		static glGetSamplerParameteriv_t*					Comfy_glGetSamplerParameteriv;
		static glGetSamplerParameterIiv_t*					Comfy_glGetSamplerParameterIiv;
		static glGetSamplerParameterfv_t*					Comfy_glGetSamplerParameterfv;
		static glGetSamplerParameterIuiv_t*					Comfy_glGetSamplerParameterIuiv;
	};
}
