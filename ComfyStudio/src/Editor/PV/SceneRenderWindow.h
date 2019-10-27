#pragma once
#include "Editor/Core/IEditorComponent.h"
#include "Editor/Core/RenderWindowBase.h"
#include "Graphics/VertexArray.h"
#include "Graphics/Buffer.h"
#include "Graphics/VertexLayouts.h"
#include "Graphics/Shader/Shader.h"
#include "Graphics/Texture/Texture2D.h"
#include "Graphics/Auth3D/Renderer3D.h"
#include "Graphics/Auth3D/ObjSet.h"
#include "Graphics/Camera.h"
#include "Graphics/SprSet.h"

namespace Editor
{
	class SceneRenderWindow : public IEditorComponent, public RenderWindowBase
	{
	public:
		SceneRenderWindow(Application* parent, EditorManager* editor);
		~SceneRenderWindow();

		const char* GetGuiName() const override;
		void Initialize() override;
		void DrawGui() override;
		void PostDrawGui() override;

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
			Graphics::PerspectiveCamera camera;
		};

		struct
		{
			float Saturation = 2.2f;
			float Brightness = 0.45455f;
		} postProcessData;

		int testObjectIndex = 0;
		UniquePtr<Graphics::ObjSet> testObjSet;
		UniquePtr<Graphics::Renderer3D> renderer;

		// Vertex Storage
		// --------------
		struct
		{
			Graphics::VertexArray cubeVao;
			Graphics::VertexArray lineVao;

			Graphics::VertexBuffer cubeVertexBuffer = { Graphics::BufferUsage::StaticDraw };
			Graphics::VertexBuffer lineVertexBuffer = { Graphics::BufferUsage::StaticDraw };
		};

		// Textures
		// --------
		struct
		{
			Graphics::SprSet sprSet;
			union
			{
				Graphics::Txp* allTextures[5];
				struct
				{
					Graphics::Txp* feelsBadManTexture;
					Graphics::Txp* goodNiceTexture;
					Graphics::Txp* groundTexture;
					Graphics::Txp* skyTexture;
					Graphics::Txp* tileTexture;
				};
			};
		};

		// Shaders
		// -------
		struct
		{
			Graphics::ComfyShader comfyShader;
			Graphics::LineShader lineShader;
			Graphics::ScreenShader screenShader;
		};

		Graphics::RenderTarget postProcessingRenderTarget;

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
		Graphics::ComfyVertex cubeVertices[36]
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

		Graphics::LineVertex axisVertices[6] =
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
		// ------------------

		void OnUpdateInput() override;
		void OnUpdate() override;
		void OnRender() override;
		void OnResize(ivec2 size) override;

	private:
		void UpdateCamera();
		void DrawComfyDebugGui();
	};
}