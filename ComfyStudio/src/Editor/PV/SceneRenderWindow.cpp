#include "SceneRenderWindow.h"
#include "FileSystem/FileHelper.h"
#include "Input/KeyCode.h"
#include "Core/TimeSpan.h"
#include <FontIcons.h>

namespace Editor
{
	using namespace Graphics;

	SceneRenderWindow::SceneRenderWindow(Application* parent, EditorManager* editor) : IEditorComponent(parent, editor)
	{
		renderer = MakeUnique<Renderer3D>();
	}

	SceneRenderWindow::~SceneRenderWindow()
	{
	}

	const char* SceneRenderWindow::GetGuiName() const
	{
		return ICON_FA_TREE "  Scene Window";
	}

	void SceneRenderWindow::Initialize()
	{
		RenderWindowBase::Initialize();

		renderer->Initialize();

		// OBJSET TEST
		{
			testObjSet = MakeUnique<ObjSet>();
			testObjSet->Load("dev_ram/objset/dbg/dbg_obj.bin");
			//testObjSet->Load("dev_ram/objset/stgtst007/stgtst007_obj.bin");
			//testObjSet->Load("dev_ram/objset/cmnitm1001/cmnitm1001_obj.bin");
			//testObjSet->Load("dev_ram/objset/rinitm000/rinitm000_obj.bin");
			//testObjSet->Load("dev_ram/objset/rinitm001/rinitm001_obj.bin");
			//testObjSet->Load("dev_ram/objset/rinitm301/rinitm301_obj.bin");
			//testObjSet->Load("dev_ram/objset/rinitm532/rinitm532_obj.bin");
			//testObjSet->Load("dev_ram/objset/stgns006/stgns006_obj.bin");
			//testObjSet->Load("dev_ram/objset/stgns008/stgns008_obj.bin");
			testObjSet->UploadAll();
		}

		// Load Textures
		// -------------
		{
			std::vector<uint8_t> sprFileBuffer;
			FileSystem::FileReader::ReadEntireFile(std::string("rom/spr/spr_comfy_scene.bin"), &sprFileBuffer);

			sprSet.Parse(sprFileBuffer.data());
			sprSet.TxpSet->UploadAll();

			for (size_t i = 0; i < sprSet.TxpSet->Textures.size(); i++)
				allTextures[i] = sprSet.TxpSet->Textures[i].get();
		}

		// Cube Vertex Data
		// ----------------
		{
			comfyShader.Initialize();

			cubeVertexBuffer.InitializeID();
			cubeVertexBuffer.Bind();
			cubeVertexBuffer.Upload(sizeof(cubeVertices), cubeVertices);

			BufferLayout layout =
			{
				{ ShaderDataType::Vec3, "in_Position" },
				{ ShaderDataType::Vec2, "in_TextureCoords" },
				{ ShaderDataType::Vec4, "in_Color" },
			};

			cubeVao.InitializeID();
			cubeVao.Bind();
			cubeVao.SetLayout(layout);
		}

		// Line Vertex Data
		// ----------------
		{
			lineShader.Initialize();

			lineVertexBuffer.InitializeID();
			lineVertexBuffer.Bind();
			lineVertexBuffer.Upload(sizeof(axisVertices), axisVertices);

			BufferLayout layout =
			{
				{ ShaderDataType::Vec3, "in_Position" },
				{ ShaderDataType::Vec4, "in_Color" },
			};

			lineVao.InitializeID();
			lineVao.Bind();
			lineVao.SetLayout(layout);
		}

		// Screen Vertex Data
		// ------------------
		{
			screenShader.Initialize();
		}

		// Render Targets
		// --------------
		{
			// arbitrary initial size, gets changed by OnReSize() later
			postProcessingRenderTarget.Initialize(RenderTargetDefaultSize);
		}
	}

	void SceneRenderWindow::DrawGui()
	{
		RenderWindowBase::DrawGui();

		// Push / Pop reset the StyleVar for the ComfyDebug window
		OnWindowEnd();
		{
			if (Gui::Begin("Comfy Debug"))
				DrawComfyDebugGui();
			Gui::End();
		}
		OnWindowBegin();
	}

	void SceneRenderWindow::PostDrawGui()
	{
		RefPtr<Obj>& obj = testObjSet->at(testObjectIndex);
		Gui::Text("%s (%d/%d)", obj->Name.c_str(), testObjectIndex, testObjSet->size());
	}

	void SceneRenderWindow::OnWindowBegin()
	{
		RenderWindowBase::PushWindowPadding();
	}

	void SceneRenderWindow::OnWindowEnd()
	{
		RenderWindowBase::PopWindowPadding();
	}

	void SceneRenderWindow::DrawComfyDebugGui()
	{
		if (Gui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
		{
			Gui::Text("Camera");
			Gui::DragFloat("Field Of View", &camera.FieldOfView, 1.0f, 1.0f, 180.0f);
			Gui::DragFloat("Near Plane", &camera.NearPlane, 0.001f, 0.001f, 1.0f);
			Gui::DragFloat("Far Plane", &camera.FarPlane);
			Gui::DragFloat3("Position", glm::value_ptr(camera.Position), 0.01f);
			Gui::DragFloat3("Target", glm::value_ptr(camera.Target), 0.01f);
			Gui::DragFloat("Smoothness", &cameraSmoothness, 1.0f, 0.0f, 250.0f);

			Gui::Text("Camera Rotation");
			Gui::DragFloat("Pitch", &targetCameraPitch, 1.0f);
			Gui::DragFloat("Yaw", &targetCameraYaw, 1.0f);
		}

		if (Gui::CollapsingHeader("Post Processing"))
		{
			Gui::DragFloat("Saturation", &postProcessData.Saturation, 0.015f, 1.0f, 5.0f);
			Gui::DragFloat("Brightness", &postProcessData.Brightness, 0.015f, 0.1f, 5.0f);
		}

		if (Gui::CollapsingHeader("Cubes"))
		{
			char nameBuffer[32];
			for (int i = 0; i < _countof(cubePositions); i++)
			{
				sprintf_s(nameBuffer, sizeof(nameBuffer), "CUBE[%d]", i);
				Gui::DragFloat3(nameBuffer, glm::value_ptr(cubePositions[i]), 0.1f);
			}
		}

		if (Gui::CollapsingHeader("Test Tree"))
		{
			static bool treeNodeCheckboxBool[100];
			static bool treeNodeParticleboxBool[100];
			char buffer[32];
			char hiddenBuffer[sizeof(buffer)];

			for (int i = 0; i < 100; i++)
			{
				sprintf_s(buffer, sizeof(buffer), "eff_pv%03d_F%04d", i, i + 1800);
				sprintf_s(hiddenBuffer, sizeof(hiddenBuffer), "##%s", buffer);

				bool treeNode = Gui::TreeNode(hiddenBuffer);
				Gui::SameLine();
				Gui::Checkbox(buffer, &treeNodeCheckboxBool[i]);

				if (treeNode)
				{
					if (Gui::TreeNode("Emitter"))
					{
						Gui::Checkbox("Particle", &treeNodeParticleboxBool[i]);
						Gui::TreePop();
					}
					Gui::TreePop();
				}
			}
		}

		if (Gui::CollapsingHeader("Image Test"))
		{
			for (auto& txp : sprSet.TxpSet->Textures)
			{
				Gui::Text(txp->Name.c_str());
				Gui::Image(txp->GraphicsTexture->GetVoidTexture(), { 200, 200 }, Gui::UV0_GL, Gui::UV1_GL);
			}
		}
	}

	void SceneRenderWindow::OnUpdateInput()
	{
		return;
	}

	void SceneRenderWindow::OnUpdate()
	{
		UpdateCamera();
	}

	void SceneRenderWindow::UpdateCamera()
	{
		ImGuiIO& io = Gui::GetIO();

		glm::vec3 front;
		front.x = cos(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
		front.y = sin(glm::radians(cameraPitch));
		front.z = sin(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));

		const bool fastCamera = Gui::IsKeyDown(KeyCode_Left_Shift);
		const bool slowCamera = Gui::IsKeyDown(KeyCode_Left_Alt);

		const float cameraSpeed = (slowCamera ? 0.25f : (fastCamera ? 5.5f : 2.25f)) * io.DeltaTime;

		if (Gui::IsWindowFocused())
		{
			if (Gui::IsMouseDown(0))
			{
				targetCameraYaw += io.MouseDelta.x * cameraSensitivity;
				targetCameraPitch -= io.MouseDelta.y * cameraSensitivity;
			}

			if (Gui::IsWindowHovered())
			{
				const float scrollStep = slowCamera ? 0.5f : (fastCamera ? 12.5f : 1.5f);

				if (io.MouseWheel > 0)
					camera.Position += front * scrollStep;
				if (io.MouseWheel < 0)
					camera.Position -= front * scrollStep;
			}
		}

		if (targetCameraPitch > +89.0f) targetCameraPitch = +89.0f;
		if (targetCameraPitch < -89.0f) targetCameraPitch = -89.0f;

		if (Gui::IsWindowFocused())
		{
			if (Gui::IsKeyDown(KeyCode_W) == KeyState_Press)
				camera.Position += front * cameraSpeed;
			if (Gui::IsKeyDown(KeyCode_S) == KeyState_Press)
				camera.Position -= front * cameraSpeed;
			if (Gui::IsKeyDown(KeyCode_A) == KeyState_Press)
				camera.Position -= glm::normalize(glm::cross(front, camera.UpDirection)) * cameraSpeed;
			if (Gui::IsKeyDown(KeyCode_D) == KeyState_Press)
				camera.Position += glm::normalize(glm::cross(front, camera.UpDirection)) * cameraSpeed;

			if (Gui::IsKeyDown(KeyCode_Space))
				camera.Position += camera.UpDirection * cameraSpeed;
			if (Gui::IsKeyDown(KeyCode_Left_Control))
				camera.Position -= camera.UpDirection * cameraSpeed;
		}

		camera.Target = camera.Position + glm::normalize(front);

		if (cameraSmoothness > 0.0f)
		{
			cameraYaw = ImLerp(cameraYaw, targetCameraYaw, io.DeltaTime * cameraSmoothness);
			cameraPitch = ImLerp(cameraPitch, targetCameraPitch, io.DeltaTime * cameraSmoothness);
		}
		else
		{
			cameraYaw = targetCameraYaw;
			cameraPitch = targetCameraPitch;
		}

		camera.UpdateMatrices();
	}

	void SceneRenderWindow::OnRender()
	{
		const ImVec4 baseClearColor = ImColor(GetColor(EditorColor_BaseClear));

		postProcessingRenderTarget.Bind();
		{
			GLCall(glEnable(GL_DEPTH_TEST));
			GLCall(glEnable(GL_BLEND));
			GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

			RenderCommand::SetClearColor(baseClearColor);
			RenderCommand::Clear(ClearTarget_ColorBuffer | ClearTarget_DepthBuffer);

			RenderCommand::SetViewport(renderTarget.GetSize());
			{
				comfyShader.Bind();
				comfyShader.SetUniform(comfyShader.View, camera.GetViewMatrix());
				comfyShader.SetUniform(comfyShader.Projection, camera.GetProjectionMatrix());

				/*
				cubeVao.Bind();

				// Stage:
				{
					mat4 skyModelMatrix = glm::scale(mat4(1.0f), vec3(1000.0f, 1000.0f, 1000.0f));
					mat4 groundModelMatrix = glm::scale(glm::translate(mat4(1.0f), vec3(0, -5.0f, 0)), vec3(999.9f, 1.0f, 999.9));
					mat4 tileModelMatrix = glm::scale(glm::translate(mat4(1.0f), vec3(0, -4.0f, 0)), vec3(39.0f, 1.0f, 39.0f));

					skyTexture->GraphicsTexture->Bind(1);
					skyTexture->GraphicsTexture->Bind(0);
					comfyShader.SetUniform(comfyShader.Model, skyModelMatrix);
					RenderCommand::DrawArrays(PrimitiveType::Triangles, 0, _countof(cubeVertices));

					groundTexture->GraphicsTexture->Bind(1);
					groundTexture->GraphicsTexture->Bind(0);
					comfyShader.SetUniform(comfyShader.Model, groundModelMatrix);
					RenderCommand::DrawArrays(PrimitiveType::Triangles, 0, _countof(cubeVertices));

					tileTexture->GraphicsTexture->Bind(1);
					tileTexture->GraphicsTexture->Bind(0);
					comfyShader.SetUniform(comfyShader.Model, tileModelMatrix);
					RenderCommand::DrawArrays(PrimitiveType::Triangles, 0, _countof(cubeVertices));
				}

				mat4 cubeModelMatrices[_countof(cubePositions)];
				for (size_t i = 0; i < _countof(cubePositions); i++)
					cubeModelMatrices[i] = glm::translate(mat4(1.0f), cubePositions[i]);

				// Lines:
				{
					lineShader.Bind();
					lineShader.SetUniform(lineShader.View, camera.GetViewMatrix());
					lineShader.SetUniform(lineShader.Projection, camera.GetProjectionMatrix());

					lineVao.Bind();
					for (size_t i = 0; i < _countof(cubePositions); i++)
					{
						lineShader.SetUniform(lineShader.Model, cubeModelMatrices[i]);
						RenderCommand::DrawArrays(PrimitiveType::Lines, 0, _countof(axisVertices));
					}
				}

				// Cubes:
				{
					comfyShader.Bind();
					comfyShader.SetUniform(comfyShader.Texture0, 0);
					comfyShader.SetUniform(comfyShader.Texture1, 1);

					goodNiceTexture->GraphicsTexture->Bind(1);
					feelsBadManTexture->GraphicsTexture->Bind(0);

					cubeVao.Bind();
					for (size_t i = 0; i < _countof(cubePositions); i++)
					{
						comfyShader.SetUniform(comfyShader.Model, cubeModelMatrices[i]);
						RenderCommand::DrawArrays(PrimitiveType::Triangles, 0, _countof(cubeVertices));
					}
				}
				*/

				// OBJSET TEST
				{
					if (Gui::IsKeyPressed(KeyCode_Up)) testObjectIndex = glm::clamp(testObjectIndex - 1, 0, static_cast<int>(testObjSet->size() - 1));
					if (Gui::IsKeyPressed(KeyCode_Down)) testObjectIndex = glm::clamp(testObjectIndex + 1, 0, static_cast<int>(testObjSet->size() - 1));

					RefPtr<Obj>& obj = testObjSet->at(testObjectIndex);

					renderer->Begin(camera);
					{
						//const float gridSize = 10.0f;
						//for (float x = 0; x < gridSize; x++)
						//{
						//	for (float z = 0; z < gridSize; z++)
						//		renderer->Draw(obj.get(), vec3(x, 0.0f, z));
						//}

						renderer->Draw(obj.get(), vec3(0.0f, 0.0f, 0.0f));
					}
					renderer->End();
				}
			}
		}
		postProcessingRenderTarget.UnBind();

		renderTarget.Bind();
		{
			GLCall(glDisable(GL_DEPTH_TEST));

			screenShader.Bind();
			screenShader.SetUniform(screenShader.Saturation, postProcessData.Saturation);
			screenShader.SetUniform(screenShader.Brightness, postProcessData.Brightness);

			postProcessingRenderTarget.GetTexture().Bind();
			RenderCommand::DrawArrays(PrimitiveType::Triangles, 0, 6);
		}
		renderTarget.UnBind();
	}

	void SceneRenderWindow::OnResize(ivec2 size)
	{
		RenderWindowBase::OnResize(size);

		vec2 renderRegionSize = GetRenderRegion().GetSize();
		camera.AspectRatio = renderRegionSize.x / renderRegionSize.y;

		postProcessingRenderTarget.Resize(size);
	}
}
