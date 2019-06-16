#include "AetRenderWindow.h"
#include "../../Graphics/VertexArray.h"
#include "../../Graphics/Buffer.h"
#include "../../Graphics/ComfyVertex.h"
#include "../../Graphics/Shader/Shader.h"

namespace Editor
{
	AetRenderWindow::AetRenderWindow()
	{
	}

	AetRenderWindow::~AetRenderWindow()
	{
	}

	void AetRenderWindow::SetAetObj(AetObj* value)
	{
		aetObj = value;
	}

	void AetRenderWindow::OnUpdateInput()
	{
	}

	void AetRenderWindow::OnUpdate()
	{
	}

	void AetRenderWindow::OnRender()
	{
		// TEMP
		static LineShader shader;
		static VertexBuffer vertexBuffer;
		static VertexArray vertexArray;

		static bool init = true;
		if (init)
		{
			init = false;

			mat4 unit = mat4(1);
			shader.Initialize();
			shader.Use();
			shader.SetUniform(shader.ModelLocation, unit);
			shader.SetUniform(shader.ViewLocation, unit);
			shader.SetUniform(shader.ProjectionLocation, unit);

			LineVertex vertices[] =
			{
				{ vec3(+0.0f, +0.5f, 0.0f), vec4(1, 0, 0, 1) },
				{ vec3(-0.5f, -0.5f, 0.0f), vec4(1, 1, 0, 1) },
				{ vec3(+0.5f, -0.5f, 0.0f), vec4(1, 0, 1, 1) },
			};

			vertexBuffer.Initialize();
			vertexBuffer.Bind();
			vertexBuffer.BufferData(vertices, sizeof(vertices), BufferUsage::StaticDraw);

			BufferLayout layout = 
			{
				{ ShaderDataType::Vec3, "in_position" },
				{ ShaderDataType::Vec4, "in_color" }
			};

			vertexArray.Initialize();
			vertexArray.Bind();
			vertexArray.SetLayout(layout);
		}

		renderTarget.Bind();
		{
			glViewport(0, 0, renderTarget.GetWidth(), renderTarget.GetHeight());

			glClearColor(.4f, .5f, .1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			shader.Use();
			vertexArray.Bind();
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}
		renderTarget.UnBind();
	}

	void AetRenderWindow::OnResize(int width, int height)
	{
		RenderWindowBase::OnResize(width, height);
	}
}
