#include "SceneRenderWindow.h"
#include "FileSystem/FileHelper.h"
#include "Input/KeyCode.h"
#include "TimeSpan.h"
#include <FontIcons.h>

namespace Editor
{
	SceneRenderWindow::SceneRenderWindow(Application* parent, EditorManager* editor) : IEditorComponent(parent, editor)
	{
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

		// Load Textures
		// -------------
		{
			std::vector<uint8_t> sprFileBuffer;
			FileSystem::ReadAllBytes("rom/spr/spr_comfy_scene.bin", &sprFileBuffer);

			sprSet.Parse(sprFileBuffer.data());

			for (size_t i = 0; i < sprSet.TxpSet->Textures.size(); i++)
			{
				allTextures[i] = sprSet.TxpSet->Textures[i].get();
				allTextures[i]->UploadTexture2D();
			}
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
			postProcessingRenderTarget.Initialize(RENDER_TARGET_DEFAULT_WIDTH, RENDER_TARGET_DEFAULT_WIDTH);
		}
	}

	void SceneRenderWindow::DrawGui()
	{
		RenderWindowBase::DrawGui();

		// Push / Pop reset the StyleVar for the ComfyDebug window
		OnWindowEnd();
		{
			if (ImGui::Begin("Comfy Debug"))
				DrawComfyDebugGui();
			ImGui::End();
		}
		OnWindowBegin();
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
		if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Text("Camera");
			ImGui::DragFloat("Field Of View", &camera.FieldOfView, 1.0f, 1.0f, 180.0f);
			ImGui::DragFloat("Near Plane", &camera.NearPlane, 0.001f, 0.001f, 1.0f);
			ImGui::DragFloat("Far Plane", &camera.FarPlane);
			ImGui::DragFloat3("Position", glm::value_ptr(camera.Position), 0.01f);
			ImGui::DragFloat3("Target", glm::value_ptr(camera.Target), 0.01f);
			ImGui::DragFloat("Smoothness", &cameraSmoothness, 1.0f, 1.0f, 250.0f);

			ImGui::Text("Camera Rotation");
			ImGui::DragFloat("Pitch", &targetCameraPitch, 1.0f);
			ImGui::DragFloat("Yaw", &targetCameraYaw, 1.0f);
		}

		if (ImGui::CollapsingHeader("Post Processing"))
		{
			ImGui::DragFloat("Saturation", &postProcessData.Saturation, 0.015f, 1.0f, 5.0f);
			ImGui::DragFloat("Brightness", &postProcessData.Brightness, 0.015f, 0.1f, 5.0f);
		}

		if (ImGui::CollapsingHeader("Cubes"))
		{
			char nameBuffer[32];
			for (int i = 0; i < _countof(cubePositions); i++)
			{
				sprintf_s(nameBuffer, sizeof(nameBuffer), "CUBE[%d]", i);
				ImGui::DragFloat3(nameBuffer, glm::value_ptr(cubePositions[i]), 0.1f);
			}
		}

		if (ImGui::CollapsingHeader("Test Tree"))
		{
			static bool treeNodeCheckboxBool[100];
			static bool treeNodeParticleboxBool[100];
			char buffer[32];
			char hiddenBuffer[sizeof(buffer)];

			for (int i = 0; i < 100; i++)
			{
				sprintf_s(buffer, sizeof(buffer), "eff_pv%03d_F%04d", i, i + 1800);
				sprintf_s(hiddenBuffer, sizeof(hiddenBuffer), "##%s", buffer);

				bool treeNode = ImGui::TreeNode(hiddenBuffer);
				ImGui::SameLine();
				ImGui::Checkbox(buffer, &treeNodeCheckboxBool[i]);

				if (treeNode)
				{
					if (ImGui::TreeNode("Emitter"))
					{
						ImGui::Checkbox("Particle", &treeNodeParticleboxBool[i]);
						ImGui::TreePop();
					}
					ImGui::TreePop();
				}
			}
		}

		if (ImGui::CollapsingHeader("Image Test"))
		{
			for (auto& txp : sprSet.TxpSet->Textures)
			{
				ImGui::Text(txp->Name.c_str());
				ImGui::Image(txp->Texture2D->GetVoidTexture(), { 200, 200 }, ImGui::UV0_GL, ImGui::UV1_GL);
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
		ImGuiIO& io = ImGui::GetIO();

		glm::vec3 front;
		front.x = cos(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
		front.y = sin(glm::radians(cameraPitch));
		front.z = sin(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));

		const bool fastCamera = ImGui::IsKeyDown(KeyCode_Left_Shift);
		const bool slowCamera = ImGui::IsKeyDown(KeyCode_Left_Alt);

		const float cameraSpeed = (slowCamera ? 0.25f : (fastCamera ? 5.5f : 2.25f)) * io.DeltaTime;

		if (ImGui::IsWindowFocused())
		{
			if (ImGui::IsMouseDown(0))
			{
				targetCameraYaw += io.MouseDelta.x * cameraSensitivity;
				targetCameraPitch -= io.MouseDelta.y * cameraSensitivity;
			}

			if (ImGui::IsWindowHovered())
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

		if (ImGui::IsWindowFocused())
		{
			if (ImGui::IsKeyDown(KeyCode_W) == KeyState_Press)
				camera.Position += front * cameraSpeed;
			if (ImGui::IsKeyDown(KeyCode_S) == KeyState_Press)
				camera.Position -= front * cameraSpeed;
			if (ImGui::IsKeyDown(KeyCode_A) == KeyState_Press)
				camera.Position -= glm::normalize(glm::cross(front, camera.UpDirection)) * cameraSpeed;
			if (ImGui::IsKeyDown(KeyCode_D) == KeyState_Press)
				camera.Position += glm::normalize(glm::cross(front, camera.UpDirection)) * cameraSpeed;

			if (ImGui::IsKeyDown(KeyCode_Space))
				camera.Position += camera.UpDirection * cameraSpeed;
			if (ImGui::IsKeyDown(KeyCode_Left_Control))
				camera.Position -= camera.UpDirection * cameraSpeed;
		}

		camera.Target = camera.Position + glm::normalize(front);

		cameraYaw = ImLerp(cameraYaw, targetCameraYaw, io.DeltaTime * cameraSmoothness);
		cameraPitch = ImLerp(cameraPitch, targetCameraPitch, io.DeltaTime * cameraSmoothness);

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

				cubeVao.Bind();

				// Stage:
				{
					mat4 skyModelMatrix = glm::scale(mat4(1.0f), vec3(1000.0f, 1000.0f, 1000.0f));
					mat4 groundModelMatrix = glm::scale(glm::translate(mat4(1.0f), vec3(0, -5.0f, 0)), vec3(999.9f, 1.0f, 999.9));
					mat4 tileModelMatrix = glm::scale(glm::translate(mat4(1.0f), vec3(0, -4.0f, 0)), vec3(39.0f, 1.0f, 39.0f));
					
					skyTexture->Texture2D->Bind(1);
					skyTexture->Texture2D->Bind(0);
					comfyShader.SetUniform(comfyShader.Model, skyModelMatrix);
					GLCall(glDrawArrays(GL_TRIANGLES, 0, _countof(cubeVertices)));

					groundTexture->Texture2D->Bind(1);
					groundTexture->Texture2D->Bind(0);
					comfyShader.SetUniform(comfyShader.Model, groundModelMatrix);
					GLCall(glDrawArrays(GL_TRIANGLES, 0, _countof(cubeVertices)));

					tileTexture->Texture2D->Bind(1);
					tileTexture->Texture2D->Bind(0);
					comfyShader.SetUniform(comfyShader.Model, tileModelMatrix);
					GLCall(glDrawArrays(GL_TRIANGLES, 0, _countof(cubeVertices)));
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
						GLCall(glDrawArrays(GL_LINES, 0, _countof(axisVertices)));
					}
				}

				// Cubes:
				{
					comfyShader.Bind();
					comfyShader.SetUniform(comfyShader.Texture0, 0);
					comfyShader.SetUniform(comfyShader.Texture1, 1);
					
					goodNiceTexture->Texture2D->Bind(1);
					feelsBadManTexture->Texture2D->Bind(0);

					cubeVao.Bind();
					for (size_t i = 0; i < _countof(cubePositions); i++)
					{
						comfyShader.SetUniform(comfyShader.Model, cubeModelMatrices[i]);
						GLCall(glDrawArrays(GL_TRIANGLES, 0, _countof(cubeVertices)));
					}
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
			GLCall(glDrawArrays(GL_TRIANGLES, 0, 6));
		}
		renderTarget.UnBind();
	}

	void SceneRenderWindow::OnResize(int width, int height)
	{
		RenderWindowBase::OnResize(width, height);

		ImVec2 renderRegionSize = GetRenderRegion().GetSize();
		camera.AspectRatio = renderRegionSize.x / renderRegionSize.y;

		postProcessingRenderTarget.Resize(width, height);
	}
}
