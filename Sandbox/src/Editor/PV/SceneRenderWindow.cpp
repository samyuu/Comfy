#include "SceneRenderWindow.h"
#include "TimeSpan.h"
#include "FileSystem/FileHelper.h"
#include <glfw/glfw3.h>

namespace Editor
{
	SceneRenderWindow::SceneRenderWindow(Application* parent, PvEditor* editor) : IEditorComponent(parent, editor)
	{
	}

	SceneRenderWindow::~SceneRenderWindow()
	{
	}

	const char* SceneRenderWindow::GetGuiName() const
	{
		return "Scene Window";
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
				{ ShaderDataType::Vec3, "in_position" },
				{ ShaderDataType::Vec2, "in_texture_coords" },
				{ ShaderDataType::Vec4, "in_color" },
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
				{ ShaderDataType::Vec3, "in_position" },
				{ ShaderDataType::Vec4, "in_color" },
			};

			lineVao.InitializeID();
			lineVao.Bind();
			lineVao.SetLayout(layout);
		}

		// Screen Vertex Data
		// ------------------
		{
			screenShader.Initialize();

			screenVertexBuffer.InitializeID();
			screenVertexBuffer.Bind();
			screenVertexBuffer.Upload(sizeof(screenVertices), screenVertices);

			BufferLayout layout =
			{
				{ ShaderDataType::Vec2, "in_position" },
				{ ShaderDataType::Vec2, "in_texture_coords" },
			};

			screenVao.InitializeID();
			screenVao.Bind();
			screenVao.SetLayout(layout);
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

		if (ImGui::CollapsingHeader("Cubes"))
		{
			char nameBuffer[32];
			for (size_t i = 0; i < _countof(cubePositions); i++)
			{
				sprintf_s(nameBuffer, sizeof(nameBuffer), "CUBE[%zd]", i);
				ImGui::DragFloat3(nameBuffer, glm::value_ptr(cubePositions[i]), 0.1f);
			}
		}

		if (ImGui::CollapsingHeader("Test Tree"))
		{
			static bool treeNodeCheckboxBool[100];
			static bool treeNodeParticleboxBool[100];
			char buffer[32];
			char hiddenBuffer[sizeof(buffer)];

			for (size_t i = 0; i < 100; i++)
			{
				sprintf_s(buffer, sizeof(buffer), "eff_pv%03d_F%04zd", i, i + 1800);
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
			for (auto txp : sprSet.TxpSet->Textures)
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

		const bool fastCamera = ImGui::IsKeyDown(GLFW_KEY_LEFT_SHIFT);
		const bool slowCamera = ImGui::IsKeyDown(GLFW_KEY_LEFT_ALT);

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
			if (ImGui::IsKeyDown(GLFW_KEY_W) == GLFW_PRESS)
				camera.Position += front * cameraSpeed;
			if (ImGui::IsKeyDown(GLFW_KEY_S) == GLFW_PRESS)
				camera.Position -= front * cameraSpeed;
			if (ImGui::IsKeyDown(GLFW_KEY_A) == GLFW_PRESS)
				camera.Position -= glm::normalize(glm::cross(front, camera.UpDirection)) * cameraSpeed;
			if (ImGui::IsKeyDown(GLFW_KEY_D) == GLFW_PRESS)
				camera.Position += glm::normalize(glm::cross(front, camera.UpDirection)) * cameraSpeed;

			if (ImGui::IsKeyDown(GLFW_KEY_SPACE))
				camera.Position += camera.UpDirection * cameraSpeed;
			if (ImGui::IsKeyDown(GLFW_KEY_LEFT_CONTROL))
				camera.Position -= camera.UpDirection * cameraSpeed;
		}

		camera.Target = camera.Position + glm::normalize(front);

		cameraYaw = ImLerp(cameraYaw, targetCameraYaw, io.DeltaTime * cameraSmoothness);
		cameraPitch = ImLerp(cameraPitch, targetCameraPitch, io.DeltaTime * cameraSmoothness);

		camera.Update();
	}

	void SceneRenderWindow::OnRender()
	{
		const ImVec4 baseClearColor = ImColor(GetColor(EditorColor_BaseClear));

		postProcessingRenderTarget.Bind();
		{
			glEnable(GL_DEPTH_TEST);
			//glDisable(GL_BLEND);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glClearColor(baseClearColor.x, baseClearColor.y, baseClearColor.z, baseClearColor.w);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glViewport(0, 0, renderTarget.GetWidth(), renderTarget.GetHeight());
			{
				mat4 cubeModelMatrices[_countof(cubePositions)];
				for (size_t i = 0; i < _countof(cubePositions); i++)
					cubeModelMatrices[i] = glm::translate(mat4(1.0f), cubePositions[i]);

				feelsBadManTexture->Texture2D->Bind(0);
				goodNiceTexture->Texture2D->Bind(1);
				comfyShader.Bind();
				comfyShader.SetUniform(comfyShader.Texture0Location, 0);
				comfyShader.SetUniform(comfyShader.Texture1Location, 1);
				comfyShader.SetUniform(comfyShader.ViewLocation, camera.GetViewMatrix());
				comfyShader.SetUniform(comfyShader.ProjectionLocation, camera.GetProjectionMatrix());

				cubeVao.Bind();
				for (size_t i = 0; i < _countof(cubePositions); i++)
				{
					comfyShader.SetUniform(comfyShader.ModelLocation, cubeModelMatrices[i]);
					glDrawArrays(GL_TRIANGLES, 0, _countof(cubeVertices));
				}

				feelsBadManTexture->Texture2D->Bind(0);
				tileTexture->Texture2D->Bind(1);
				tileTexture->Texture2D->Bind(0);
				mat4 tileModelMatrix = glm::scale(glm::translate(mat4(1.0f), vec3(0, -4.0f, 0)), vec3(39.0f, 1.0f, 39.0f));
				comfyShader.SetUniform(comfyShader.ModelLocation, tileModelMatrix);
				glDrawArrays(GL_TRIANGLES, 0, _countof(cubeVertices));

				feelsBadManTexture->Texture2D->Bind(0);
				skyTexture->Texture2D->Bind(1);
				skyTexture->Texture2D->Bind(0);
				mat4 skyModelMatrix = glm::scale(mat4(1.0f), vec3(1000.0f, 1000.0f, 1000.0f));
				comfyShader.SetUniform(comfyShader.ModelLocation, skyModelMatrix);
				glDrawArrays(GL_TRIANGLES, 0, _countof(cubeVertices));

				feelsBadManTexture->Texture2D->Bind(0);
				groundTexture->Texture2D->Bind(1);
				groundTexture->Texture2D->Bind(0);
				mat4 groundModelMatrix = glm::scale(glm::translate(mat4(1.0f), vec3(0, -5.0f, 0)), vec3(999.9f, 1.0f, 999.9));
				comfyShader.SetUniform(comfyShader.ModelLocation, groundModelMatrix);
				glDrawArrays(GL_TRIANGLES, 0, _countof(cubeVertices));

				{
					lineShader.Bind();
					lineShader.SetUniform(lineShader.ViewLocation, camera.GetViewMatrix());
					lineShader.SetUniform(lineShader.ProjectionLocation, camera.GetProjectionMatrix());

					lineVao.Bind();
					for (size_t i = 0; i < _countof(cubePositions); i++)
					{
						lineShader.SetUniform(lineShader.ModelLocation, cubeModelMatrices[i]);
						glDrawArrays(GL_LINES, 0, _countof(axisVertices));
					}
				}
			}
		}
		postProcessingRenderTarget.UnBind();

		renderTarget.Bind();
		{
			glDisable(GL_DEPTH_TEST);

			screenVao.Bind();
			screenShader.Bind();
			postProcessingRenderTarget.GetTexture().Bind();
			glDrawArrays(GL_TRIANGLES, 0, _countof(screenVertices));
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
