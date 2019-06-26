#pragma once
#include "Editor/IEditorComponent.h"
#include "Editor/RenderWindowBase.h"
#include "Graphics/VertexArray.h"
#include "Graphics/Buffer.h"
#include "Graphics/ComfyVertex.h"
#include "Graphics/Shader/Shader.h"
#include "Graphics/Texture.h"
#include "Graphics/Camera.h"
#include "FileSystem/Format/SprSet.h"

namespace Editor
{
	using namespace FileSystem;

	class SceneRenderWindow : public IEditorComponent, public RenderWindowBase
	{
	public:
		SceneRenderWindow(Application* parent, PvEditor* editor);
		~SceneRenderWindow();

		const char* GetGuiName() const override;
		void Initialize() override;
		void DrawGui() override;

		void OnWindowBegin() override;
		void OnWindowEnd() override;

	protected:
		// Scene Camera
		// ------------
		struct
		{
			float cameraSmoothness = 50.0f;
			float cameraPitch, cameraYaw = -90.0f, cameraRoll;
			float targetCameraPitch, targetCameraYaw = -90.0f;
			float cameraSensitivity = 0.25f;
			Camera camera;
		};

		// Vertex Storage
		// --------------
		struct
		{
			VertexArray cubeVao;
			VertexArray lineVao;
			VertexArray screenVao;

			VertexBuffer cubeVertexBuffer;
			VertexBuffer lineVertexBuffer;
			VertexBuffer screenVertexBuffer;
		};

		// Textures
		// --------
		struct
		{
			SprSet sprSet;
			union
			{
				Texture* allTextures[5];
				struct
				{
					Texture* feelsBadManTexture;
					Texture* goodNiceTexture;
					Texture* groundTexture;
					Texture* skyTexture;
					Texture* tileTexture;
				};
			};
		};

		// Shaders
		// -------
		struct
		{
			ComfyShader comfyShader;
			LineShader lineShader;
			ScreenShader screenShader;
		};

		RenderTarget postProcessingRenderTarget;

		// Dynamic Scene Data
		// ------------------
		vec3 cubePositions[10] =
		{
			vec3(+0.0f, +0.0f, -10.0f),
			vec3(+2.0f, +5.0f, -15.0f),
			vec3(-1.5f, -2.2f, -02.5f),
			vec3(-3.8f, -2.0f, -12.3f),
			vec3(+2.4f, -0.4f, -03.5f),
			vec3(-1.7f, +3.0f, -07.5f),
			vec3(+1.3f, -2.0f, -02.5f),
			vec3(+1.5f, +2.0f, -02.5f),
			vec3(+1.5f, +0.2f, -01.5f),
			vec3(-1.3f, +1.0f, -01.5f),
		};
		// ------------------

		// Static Vertex Data
		// ------------------
		const vec4 cubeColor = vec4(0.9f, 0.8f, 0.01f, 1.00f);
		ComfyVertex cubeVertices[36]
		{
			{ vec3(-0.5f, -0.5f, -0.5f), vec2(0.0f, 0.0f), cubeColor },
			{ vec3(+0.5f, -0.5f, -0.5f), vec2(1.0f, 0.0f), cubeColor },
			{ vec3(+0.5f, +0.5f, -0.5f), vec2(1.0f, 1.0f), cubeColor },
			{ vec3(+0.5f, +0.5f, -0.5f), vec2(1.0f, 1.0f), cubeColor },
			{ vec3(-0.5f, +0.5f, -0.5f), vec2(0.0f, 1.0f), cubeColor },
			{ vec3(-0.5f, -0.5f, -0.5f), vec2(0.0f, 0.0f), cubeColor },

			{ vec3(-0.5f, -0.5f, +0.5f), vec2(0.0f, 0.0f), cubeColor },
			{ vec3(+0.5f, -0.5f, +0.5f), vec2(1.0f, 0.0f), cubeColor },
			{ vec3(+0.5f, +0.5f, +0.5f), vec2(1.0f, 1.0f), cubeColor },
			{ vec3(+0.5f, +0.5f, +0.5f), vec2(1.0f, 1.0f), cubeColor },
			{ vec3(-0.5f, +0.5f, +0.5f), vec2(0.0f, 1.0f), cubeColor },
			{ vec3(-0.5f, -0.5f, +0.5f), vec2(0.0f, 0.0f), cubeColor },

			{ vec3(-0.5f, +0.5f, +0.5f), vec2(1.0f, 0.0f), cubeColor },
			{ vec3(-0.5f, +0.5f, -0.5f), vec2(1.0f, 1.0f), cubeColor },
			{ vec3(-0.5f, -0.5f, -0.5f), vec2(0.0f, 1.0f), cubeColor },
			{ vec3(-0.5f, -0.5f, -0.5f), vec2(0.0f, 1.0f), cubeColor },
			{ vec3(-0.5f, -0.5f, +0.5f), vec2(0.0f, 0.0f), cubeColor },
			{ vec3(-0.5f, +0.5f, +0.5f), vec2(1.0f, 0.0f), cubeColor },

			{ vec3(+0.5f, +0.5f, +0.5f), vec2(1.0f, 0.0f), cubeColor },
			{ vec3(+0.5f, +0.5f, -0.5f), vec2(1.0f, 1.0f), cubeColor },
			{ vec3(+0.5f, -0.5f, -0.5f), vec2(0.0f, 1.0f), cubeColor },
			{ vec3(+0.5f, -0.5f, -0.5f), vec2(0.0f, 1.0f), cubeColor },
			{ vec3(+0.5f, -0.5f, +0.5f), vec2(0.0f, 0.0f), cubeColor },
			{ vec3(+0.5f, +0.5f, +0.5f), vec2(1.0f, 0.0f), cubeColor },

			{ vec3(-0.5f, -0.5f, -0.5f), vec2(0.0f, 1.0f), cubeColor },
			{ vec3(+0.5f, -0.5f, -0.5f), vec2(1.0f, 1.0f), cubeColor },
			{ vec3(+0.5f, -0.5f, +0.5f), vec2(1.0f, 0.0f), cubeColor },
			{ vec3(+0.5f, -0.5f, +0.5f), vec2(1.0f, 0.0f), cubeColor },
			{ vec3(-0.5f, -0.5f, +0.5f), vec2(0.0f, 0.0f), cubeColor },
			{ vec3(-0.5f, -0.5f, -0.5f), vec2(0.0f, 1.0f), cubeColor },

			{ vec3(-0.5f, +0.5f, -0.5f), vec2(0.0f, 1.0f), cubeColor },
			{ vec3(+0.5f, +0.5f, -0.5f), vec2(1.0f, 1.0f), cubeColor },
			{ vec3(+0.5f, +0.5f, +0.5f), vec2(1.0f, 0.0f), cubeColor },
			{ vec3(+0.5f, +0.5f, +0.5f), vec2(1.0f, 0.0f), cubeColor },
			{ vec3(-0.5f, +0.5f, +0.5f), vec2(0.0f, 0.0f), cubeColor },
			{ vec3(-0.5f, +0.5f, -0.5f), vec2(0.0f, 1.0f), cubeColor },
		};

		LineVertex axisVertices[6] =
		{
			// X-Axis
			{ vec3(0.0f, 0.0f, 0.0f), vec4(1.0f, 0.1f, 0.3f, 1.0f) },
			{ vec3(1.0f, 0.0f, 0.0f), vec4(1.0f, 0.1f, 0.3f, 1.0f) },

			// Y-Axis										 
			{ vec3(0.0f, 0.0f, 0.0f), vec4(0.2f, 1.0f, 0.1f, 1.0f) },
			{ vec3(0.0f, 1.0f, 0.0f), vec4(0.2f, 1.0f, 0.1f, 1.0f) },

			// Z-Axis										 
			{ vec3(0.0f, 0.0f, 0.0f), vec4(0.1f, 0.7f, 1.0f, 1.0f) },
			{ vec3(0.0f, 0.0f, 1.0f), vec4(0.1f, 0.7f, 1.0f, 1.0f) },
		};

		ScreenVertex screenVertices[6]
		{
			{ vec2(-1.0f, +1.0f),  vec2(0.0f, 1.0f) }, // top left
			{ vec2(+1.0f, +1.0f),  vec2(1.0f, 1.0f) }, // top right
			{ vec2(+1.0f, -1.0f),  vec2(1.0f, 0.0f) }, // bottom rights

			{ vec2(-1.0f, +1.0f),  vec2(0.0f, 1.0f) }, // top left
			{ vec2(+1.0f, -1.0f),  vec2(1.0f, 0.0f) }, // bottom right
			{ vec2(-1.0f, -1.0f),  vec2(0.0f, 0.0f) }, // bottom left
		};
		// ------------------

		void OnUpdateInput() override;
		void OnUpdate() override;
		void OnRender() override;
		void OnResize(int width, int height) override;

	private:
		void UpdateCamera();
		void DrawComfyDebugGui();
	};
}