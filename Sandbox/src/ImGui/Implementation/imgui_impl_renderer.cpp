#if !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "ImGui/Core/imgui.h"
#include "ImGui/Implementation/Imgui_Impl_Renderer.h"
#include "Graphics/Graphics.h"
#include "Graphics/RenderCommand.h"
#include "Graphics/OpenGL/OpenGL.h"

static struct ImGuiComfyGLRendererContext
{
	// TODO:
	// Graphics::Texture2D
	// Graphics::VertexBuffer
} RendererContext;

// OpenGL Data
static char         g_GlslVersionString[32] = "";
static GLuint       g_FontTexture = 0;
static GLuint       g_ShaderHandle = 0, g_VertHandle = 0, g_FragHandle = 0;
static int          g_AttribLocationTex = 0, g_AttribLocationProjMtx = 0;                                // Uniforms location
static int          g_AttribLocationVtxPos = 0, g_AttribLocationVtxUV = 0, g_AttribLocationVtxColor = 0; // Vertex attributes location
static unsigned int g_VboHandle = 0, g_ElementsHandle = 0;

// Forward Declarations
static void ImGui_ImplOpenGL3_InitPlatformInterface();
static void ImGui_ImplOpenGL3_ShutdownPlatformInterface();

// Functions
bool ImGui_ImplOpenGL3_Init()
{
	// Setup back-end capabilities flags
	ImGuiIO& io = ImGui::GetIO();
	io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;    // We can create multi-viewports on the Renderer side (optional)
	io.BackendRendererName = "ComfyGL3";

	// Store GLSL version string so we can refer to it later in case we recreate shaders. Note: GLSL version is NOT the same as GL version. Leave this to NULL if unsure.
	const char* glsl_version = "#version 420";

	strcpy(g_GlslVersionString, glsl_version);
	strcat(g_GlslVersionString, "\n");

	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		ImGui_ImplOpenGL3_InitPlatformInterface();

	return true;
}

void ImGui_ImplOpenGL3_Shutdown()
{
	ImGui_ImplOpenGL3_ShutdownPlatformInterface();
	ImGui_ImplOpenGL3_DestroyDeviceObjects();
}

void ImGui_ImplOpenGL3_NewFrame()
{
	if (!g_FontTexture)
		ImGui_ImplOpenGL3_CreateDeviceObjects();
}

// OpenGL3 Render function.
// (this used to be set in io.RenderDrawListsFn and called by ImGui::Render(), but you can now call this directly from your main loop)
// Note that this implementation is little overcomplicated because we are saving/setting up/restoring every OpenGL state explicitly, in order to be able to run within any OpenGL engine that doesn't do so.
void ImGui_ImplOpenGL3_RenderDrawData(ImDrawData* draw_data)
{
	// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
	int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
	int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
	if (fb_width <= 0 || fb_height <= 0)
		return;

	const Graphics::RenderCommand::State& renderCommandState = Graphics::RenderCommand::GetState();

	// Backup GL state
	int32_t lastTextureSlot = renderCommandState.LastTextureSlot;
	uint32_t lastProgramID = renderCommandState.LastBoundShaderProgram;
	uint32_t lastTextureID = renderCommandState.GetLastBoundTextureID();
	
	GLint last_sampler; 
	GLCall(glGetIntegerv(GL_SAMPLER_BINDING, &last_sampler));
	
	GLint last_array_buffer; 
	GLCall(glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer));
	
	GLint last_vertex_array_object; 
	GLCall(glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array_object));
	
	GLint last_polygon_mode[2]; 
	GLCall(glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode));
	
	GLint last_viewport[4]; 
	GLCall(glGetIntegerv(GL_VIEWPORT, last_viewport));
	
	GLint last_scissor_box[4]; 
	GLCall(glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box));
	
	GLenum last_blend_src_rgb; 
	GLCall(glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)&last_blend_src_rgb));
	
	GLenum last_blend_dst_rgb; 
	GLCall(glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)&last_blend_dst_rgb));
	
	GLenum last_blend_src_alpha; 
	GLCall(glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&last_blend_src_alpha));
	
	GLenum last_blend_dst_alpha; 
	GLCall(glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&last_blend_dst_alpha));
	
	GLenum last_blend_equation_rgb; 
	GLCall(glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint*)&last_blend_equation_rgb));
	
	GLenum last_blend_equation_alpha; 
	GLCall(glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint*)&last_blend_equation_alpha));
	
	Graphics::RenderCommand::SetTextureSlot(0);

	GLboolean last_enable_blend; 
	GLCall(last_enable_blend = glIsEnabled(GL_BLEND));
	
	GLboolean last_enable_cull_face; 
	GLCall(last_enable_cull_face = glIsEnabled(GL_CULL_FACE));
	
	GLboolean last_enable_depth_test; 
	GLCall(last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST));
	
	GLboolean last_enable_scissor_test; 
	GLCall(last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST));
	
	bool clip_origin_lower_left = true;
	GLenum last_clip_origin = 0; 
	GLCall(glGetIntegerv(GL_CLIP_ORIGIN, (GLint*)&last_clip_origin)); // Support for GL 4.5's glClipControl(GL_UPPER_LEFT)
	
	if (last_clip_origin == GL_UPPER_LEFT)
		clip_origin_lower_left = false;

	// Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, polygon fill
	GLCall(glEnable(GL_BLEND));
	GLCall(glBlendEquation(GL_FUNC_ADD));
	GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	GLCall(glDisable(GL_CULL_FACE));
	GLCall(glDisable(GL_DEPTH_TEST));
	GLCall(glEnable(GL_SCISSOR_TEST));
	GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));

	// Setup viewport, orthographic projection matrix
	// Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayMin is (0,0) for single viewport apps.
	GLCall(glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height));
	float L = draw_data->DisplayPos.x;
	float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
	float T = draw_data->DisplayPos.y;
	float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
	const float ortho_projection[4][4] =
	{
		{ 2.0f / (R - L),   0.0f,         0.0f,   0.0f },
		{ 0.0f,         2.0f / (T - B),   0.0f,   0.0f },
		{ 0.0f,         0.0f,        -1.0f,   0.0f },
		{ (R + L) / (L - R),  (T + B) / (B - T),  0.0f,   1.0f },
	};

	Graphics::RenderCommand::BindShaderProgram(g_ShaderHandle);
	GLCall(glUniform1i(g_AttribLocationTex, 0));
	GLCall(glUniformMatrix4fv(g_AttribLocationProjMtx, 1, GL_FALSE, &ortho_projection[0][0]));
	GLCall(glBindSampler(0, 0)); // We use combined texture/sampler state. Applications using GL 3.3 may set that otherwise.

	// Recreate the VAO every time (this is to easily allow multiple GL contexts to be rendered to. VAO are not shared among GL contexts)
	// The renderer would actually work without any VAO bound, but then our VertexAttrib calls would overwrite the default one currently bound.
	GLuint vertex_array_object = 0;
	GLCall(glGenVertexArrays(1, &vertex_array_object));
	GLCall(glBindVertexArray(vertex_array_object));

	// Bind vertex/index buffers and setup attributes for ImDrawVert
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle));
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ElementsHandle));
	GLCall(glEnableVertexAttribArray(g_AttribLocationVtxPos));
	GLCall(glEnableVertexAttribArray(g_AttribLocationVtxUV));
	GLCall(glEnableVertexAttribArray(g_AttribLocationVtxColor));
	GLCall(glVertexAttribPointer(g_AttribLocationVtxPos, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, pos)));
	GLCall(glVertexAttribPointer(g_AttribLocationVtxUV, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, uv)));
	GLCall(glVertexAttribPointer(g_AttribLocationVtxColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, col)));

	// Will project scissor/clipping rectangles into framebuffer space
	ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
	ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

	// Render command lists
	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		size_t idx_buffer_offset = 0;

		// Upload vertex/index buffers
		GLCall(glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), (const GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW));
		GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx), (const GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW));

		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
		{
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
			if (pcmd->UserCallback)
			{
				// User callback (registered via ImDrawList::AddCallback)
				pcmd->UserCallback(cmd_list, pcmd);
			}
			else
			{
				// Project scissor/clipping rectangles into framebuffer space
				ImVec4 clip_rect;
				clip_rect.x = (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
				clip_rect.y = (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
				clip_rect.z = (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
				clip_rect.w = (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;

				if (clip_rect.x < fb_width && clip_rect.y < fb_height && clip_rect.z >= 0.0f && clip_rect.w >= 0.0f)
				{
					// Apply scissor/clipping rectangle
					if (clip_origin_lower_left)
					{
						GLCall(glScissor((int)clip_rect.x, (int)(fb_height - clip_rect.w), (int)(clip_rect.z - clip_rect.x), (int)(clip_rect.w - clip_rect.y)));
					}
					else
					{
						GLCall(glScissor((int)clip_rect.x, (int)clip_rect.y, (int)clip_rect.z, (int)clip_rect.w)); // Support for GL 4.5 rarely used glClipControl(GL_UPPER_LEFT)
					}

					// Bind texture, Draw
					Graphics::RenderCommand::BindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
					Graphics::RenderCommand::DrawElements(Graphics::PrimitiveType::Triangles, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, (void*)idx_buffer_offset);
				}
			}
			idx_buffer_offset += pcmd->ElemCount * sizeof(ImDrawIdx);
		}
	}

	// Destroy the temporary VAO
	GLCall(glDeleteVertexArrays(1, &vertex_array_object));

	// Restore modified GL state
	Graphics::RenderCommand::BindShaderProgram(lastProgramID);
	
	Graphics::RenderCommand::BindTexture(GL_TEXTURE_2D, lastTextureID);
	Graphics::RenderCommand::SetTextureSlot(lastTextureSlot);
	
	GLCall(glBindSampler(0, last_sampler));
	GLCall(glBindVertexArray(last_vertex_array_object));

	GLCall(glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer));
	GLCall(glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha));
	GLCall(glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb, last_blend_src_alpha, last_blend_dst_alpha));

	if (last_enable_blend) { GLCall(glEnable(GL_BLEND)); } else { GLCall(glDisable(GL_BLEND)); }
	if (last_enable_cull_face) { GLCall(glEnable(GL_CULL_FACE)); } else { GLCall(glDisable(GL_CULL_FACE)); }
	if (last_enable_depth_test) { GLCall(glEnable(GL_DEPTH_TEST)); } else { GLCall(glDisable(GL_DEPTH_TEST)); }
	if (last_enable_scissor_test) { GLCall(glEnable(GL_SCISSOR_TEST)); } else { GLCall(glDisable(GL_SCISSOR_TEST)); }

	GLCall(glPolygonMode(GL_FRONT_AND_BACK, (GLenum)last_polygon_mode[0]));
	GLCall(glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]));
	GLCall(glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]));
}

bool ImGui_ImplOpenGL3_CreateFontsTexture()
{
	// Build texture atlas
	ImGuiIO& io = ImGui::GetIO();
	unsigned char* pixels;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height); // Load as RGBA 32-bits (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.

	// Upload texture to graphics system
	uint32_t lastTextureID = Graphics::RenderCommand::GetState().GetLastBoundTextureID();

	GLCall(glGenTextures(1, &g_FontTexture));
	Graphics::RenderCommand::BindTexture(GL_TEXTURE_2D, g_FontTexture);
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GLCall(glPixelStorei(GL_UNPACK_ROW_LENGTH, 0));
	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels));

	// Store our identifier
	io.Fonts->TexID = (ImTextureID)(intptr_t)g_FontTexture;

	// Restore state
	Graphics::RenderCommand::BindTexture(GL_TEXTURE_2D, lastTextureID);

	return true;
}

void ImGui_ImplOpenGL3_DestroyFontsTexture()
{
	if (g_FontTexture)
	{
		ImGuiIO& io = ImGui::GetIO();
		GLCall(glDeleteTextures(1, &g_FontTexture));
		io.Fonts->TexID = 0;
		g_FontTexture = 0;
	}
}

// If you get an error please report on github. You may try different GL context version or GLSL version. See GL<>GLSL version table at the top of this file.
static bool CheckShader(GLuint handle, const char* desc)
{
	GLint status = 0, log_length = 0;
	GLCall(glGetShaderiv(handle, GL_COMPILE_STATUS, &status));
	GLCall(glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &log_length));
	if ((GLboolean)status == GL_FALSE)
		assert(false);
	if (log_length > 0)
	{
		ImVector<char> buf;
		buf.resize((int)(log_length + 1));
		GLCall(glGetShaderInfoLog(handle, log_length, NULL, (GLchar*)buf.begin()));
		assert(false);
	}
	return (GLboolean)status == GL_TRUE;
}

// If you get an error please report on GitHub. You may try different GL context version or GLSL version.
static bool CheckProgram(GLuint handle, const char* desc)
{
	GLint status = 0, log_length = 0;
	GLCall(glGetProgramiv(handle, GL_LINK_STATUS, &status));
	GLCall(glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &log_length));
	if ((GLboolean)status == GL_FALSE)
		assert(false);
	if (log_length > 0)
	{
		ImVector<char> buf;
		buf.resize((int)(log_length + 1));
		GLCall(glGetProgramInfoLog(handle, log_length, NULL, (GLchar*)buf.begin()));
		assert(false);
	}
	return (GLboolean)status == GL_TRUE;
}

bool ImGui_ImplOpenGL3_CreateDeviceObjects()
{
	// Backup GL state
	uint32_t lastTextureID = Graphics::RenderCommand::GetState().GetLastBoundTextureID();
	
	GLint last_array_buffer;
	GLCall(glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer));
	
	GLint last_vertex_array;
	GLCall(glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array));

	// Parse GLSL version string
	int glsl_version = 130;
	sscanf(g_GlslVersionString, "#version %d", &glsl_version);

	const GLchar* vertex_shader_glsl_410_core =
		"layout (location = 0) in vec2 in_Position;"
		"layout (location = 1) in vec2 in_TextureCoords;"
		"layout (location = 2) in vec4 in_Color;"
		"uniform mat4 u_ProjectionMatrix;"
		"out vec2 VertexTexCoord;"
		"out vec4 VertexColor;"
		"void main() { VertexTexCoord = in_TextureCoords; VertexColor = in_Color; gl_Position = u_ProjectionMatrix * vec4(in_Position.xy, 0.0, 1.0); }";

	const GLchar* fragment_shader_glsl_410_core =
		"in vec2 VertexTexCoord;" 
		"in vec4 VertexColor;" 
		"uniform sampler2D u_Texture;" 
		"layout (location = 0) out vec4 FragColor;" 
		"void main() { FragColor = VertexColor * texture(u_Texture, VertexTexCoord.st); }";

	// Select shaders matching our GLSL versions
	const GLchar* vertex_shader = NULL;
	const GLchar* fragment_shader = NULL;

	if (glsl_version >= 410)
	{
		vertex_shader = vertex_shader_glsl_410_core;
		fragment_shader = fragment_shader_glsl_410_core;
	}
	else
	{
		assert(false);
	}

	// Create shaders
	const GLchar* vertex_shader_with_version[2] = { g_GlslVersionString, vertex_shader };
	GLCall(g_VertHandle = glCreateShader(GL_VERTEX_SHADER));
	GLCall(glShaderSource(g_VertHandle, 2, vertex_shader_with_version, NULL));
	GLCall(glCompileShader(g_VertHandle));
	CheckShader(g_VertHandle, "vertex shader");

	const GLchar* fragment_shader_with_version[2] = { g_GlslVersionString, fragment_shader };
	GLCall(g_FragHandle = glCreateShader(GL_FRAGMENT_SHADER));
	GLCall(glShaderSource(g_FragHandle, 2, fragment_shader_with_version, NULL));
	GLCall(glCompileShader(g_FragHandle));
	CheckShader(g_FragHandle, "fragment shader");

	GLCall(g_ShaderHandle = glCreateProgram());
	GLCall(glAttachShader(g_ShaderHandle, g_VertHandle));
	GLCall(glAttachShader(g_ShaderHandle, g_FragHandle));
	GLCall(glLinkProgram(g_ShaderHandle));
	CheckProgram(g_ShaderHandle, "shader program");

	GLCall(g_AttribLocationTex = glGetUniformLocation(g_ShaderHandle, "u_Texture"));
	GLCall(g_AttribLocationProjMtx = glGetUniformLocation(g_ShaderHandle, "u_ProjectionMatrix"));
	GLCall(g_AttribLocationVtxPos = glGetAttribLocation(g_ShaderHandle, "in_Position"));
	GLCall(g_AttribLocationVtxUV = glGetAttribLocation(g_ShaderHandle, "in_TextureCoords"));
	GLCall(g_AttribLocationVtxColor = glGetAttribLocation(g_ShaderHandle, "in_Color"));

	// Create buffers
	GLCall(glGenBuffers(1, &g_VboHandle));
	GLCall(glGenBuffers(1, &g_ElementsHandle));

	ImGui_ImplOpenGL3_CreateFontsTexture();

	// Restore modified GL state
	Graphics::RenderCommand::BindTexture(GL_TEXTURE_2D, lastTextureID);
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer));
	GLCall(glBindVertexArray(last_vertex_array));

	return true;
}

void ImGui_ImplOpenGL3_DestroyDeviceObjects()
{
	if (g_VboHandle) { GLCall(glDeleteBuffers(1, &g_VboHandle)); }
	if (g_ElementsHandle) { GLCall(glDeleteBuffers(1, &g_ElementsHandle)); }
	g_VboHandle = g_ElementsHandle = 0;
	if (g_ShaderHandle && g_VertHandle) { GLCall(glDetachShader(g_ShaderHandle, g_VertHandle)); }
	if (g_VertHandle) { GLCall(glDeleteShader(g_VertHandle)); }
	g_VertHandle = 0;
	if (g_ShaderHandle && g_FragHandle) { GLCall(glDetachShader(g_ShaderHandle, g_FragHandle)); }
	if (g_FragHandle) { GLCall(glDeleteShader(g_FragHandle)); }
	g_FragHandle = 0;
	if (g_ShaderHandle) { GLCall(glDeleteProgram(g_ShaderHandle)); }
	g_ShaderHandle = 0;

	ImGui_ImplOpenGL3_DestroyFontsTexture();
}

//--------------------------------------------------------------------------------------------------------
// MULTI-VIEWPORT / PLATFORM INTERFACE SUPPORT
// This is an _advanced_ and _optional_ feature, allowing the back-end to create and handle multiple viewports simultaneously.
// If you are new to dear imgui or creating a new binding for dear imgui, it is recommended that you completely ignore this section first..
//--------------------------------------------------------------------------------------------------------

static void ImGui_ImplOpenGL3_RenderWindow(ImGuiViewport* viewport, void*)
{
	if (!(viewport->Flags & ImGuiViewportFlags_NoRendererClear))
	{
		Graphics::RenderCommand::SetClearColor(vec4(0.0f, 0.0f, 0.0f, 1.0f));
		Graphics::RenderCommand::Clear(Graphics::ClearTarget_ColorBuffer);
	}
	ImGui_ImplOpenGL3_RenderDrawData(viewport->DrawData);
}

static void ImGui_ImplOpenGL3_InitPlatformInterface()
{
	ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
	platform_io.Renderer_RenderWindow = ImGui_ImplOpenGL3_RenderWindow;
}

static void ImGui_ImplOpenGL3_ShutdownPlatformInterface()
{
	ImGui::DestroyPlatformWindows();
}
