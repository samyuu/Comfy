#include "AetRenderWindow.h"
#include "Editor/Theme.h"
#include "FileSystem/FileHelper.h"
#include "Misc/StringHelper.h"
#include "TimeSpan.h"
#include <glfw/glfw3.h>

namespace Editor
{
	AetRenderWindow::AetRenderWindow()
	{
	}

	AetRenderWindow::~AetRenderWindow()
	{
	}

	void AetRenderWindow::SetAetLyo(AetLyo* value)
	{
		aetLyo = value;
	}

	void AetRenderWindow::SetAetObj(AetObj* value)
	{
		aetObj = value;
	}

	void AetRenderWindow::OnDrawGui()
	{
		if (ImGui::CollapsingHeader("View Settings"))
		{
			const char* blendModeNames = "None\0None\0None\0Alpha\0None\0Additive\0DstColorZero\0SrcAlphaOneMinusSrcColor\0Transparent";
			ImGui::Combo("Blend Mode", &currentBlendItem, blendModeNames);
			ImGui::SameLine();
			ImGui::Checkbox("Use Text Shadow", &useTextShadow);

			//ImGui::ShowDemoWindow
			ImGui::InputFloat2("Position", glm::value_ptr(aetPosition));
			ImGui::InputFloat2("Origin", glm::value_ptr(aetOrigin));
			ImGui::InputFloat("Rotation", &aetRotation, 1.0f, 10.0f);
			ImGui::InputFloat2("Scale", &aetScale.x, 1.0f, 10.0f);
			ImGui::InputFloat2("Source Position", &aetSourceRegion.x, 1.0f, 10.0f);
			ImGui::InputFloat2("Source Size", &aetSourceRegion.z, 1.0f, 10.0f);
			//ImGui::ColorButton("Color", (const ImVec4&)aetColor);
			ImGui::ColorEdit4("Color", glm::value_ptr(aetColor));
		}

		ImGui::Begin("SprSet Loader", nullptr, ImGuiWindowFlags_None);
		{
			ImGui::BeginChild("SprSetLoaderChild##AetRenderWindow");
			if (fileViewer.DrawGui())
			{
				std::string sprPath = fileViewer.GetFileToOpen();
				if (StartsWithInsensitive(GetFileName(sprPath), "spr_") && EndsWithInsensitive(sprPath, ".bin"))
				{
					std::vector<uint8_t> fileBuffer;
					ReadAllBytes(sprPath, &fileBuffer);

					sprSet = std::make_unique<SprSet>();
					sprSet->Parse(fileBuffer.data());

					for (int i = 0; i < sprSet->TxpSet->Textures.size(); i++)
					{
						sprSet->TxpSet->Textures[i]->Texture2D = std::make_shared<Texture2D>();
						sprSet->TxpSet->Textures[i]->Texture2D->Upload(sprSet->TxpSet->Textures[i].get());
					}
				}
			}
			ImGui::EndChild();
		}
		ImGui::End();

		ImGui::Begin("Txp Preview", nullptr, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoBackground);
		{
			if (sprSet != nullptr)
			{
				for (int i = 0; i < sprSet->TxpSet->Textures.size(); i++)
				{
					auto tex = sprSet->TxpSet->Textures[i].get();

					ImGui::PushID(&tex);
					ImGui::BeginChild("AetTexturePreviewChildInner", ImVec2(), false, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoBackground);
					if (ImGui::Selectable(sprSet->TxpSet->Textures[i]->Name.c_str(), i == txpIndex))
						txpIndex = i;

					if (ImGui::IsItemHovered())
					{
						auto size = ImVec2(tex->Texture2D->GetWidth(), tex->Texture2D->GetHeight());

						float ratio = size.y / size.x;
						size.x = __min(size.x, 320);
						size.y = size.x * ratio;

						ImGui::BeginTooltip();
						ImGui::Image(tex->Texture2D->GetVoidTexture(), size, ImGui::UV0_GL, ImGui::UV1_GL);
						ImGui::EndTooltip();
					}

					ImGui::EndChild();
					ImGui::PopID();
				}
			}
		}
		ImGui::End();
	}

	void AetRenderWindow::OnUpdateInput()
	{
	}

	void AetRenderWindow::OnUpdate()
	{
	}

	void AetRenderWindow::OnRender()
	{
		constexpr float step = 10.0f;
		if (ImGui::IsKeyPressed(GLFW_KEY_W, true)) aetPosition.y -= step;
		if (ImGui::IsKeyPressed(GLFW_KEY_S, true)) aetPosition.y += step;
		if (ImGui::IsKeyPressed(GLFW_KEY_A, true)) aetPosition.x -= step;
		if (ImGui::IsKeyPressed(GLFW_KEY_D, true)) aetPosition.x += step;
		if (ImGui::IsKeyPressed(GLFW_KEY_ESCAPE, true)) aetPosition = vec2();
		if (ImGui::IsWindowFocused() && ImGui::IsMouseDragging())
			aetPosition += vec2(ImGui::GetIO().MouseDelta.x, ImGui::GetIO().MouseDelta.y);

		renderTarget.Bind();
		{
			GLCall(glViewport(0, 0, renderTarget.GetWidth(), renderTarget.GetHeight()));

			GLCall(glEnable(GL_BLEND));
			GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

			vec4 backgroundColor = GetColorVec4(EditorColor_DarkClear);
			GLCall(glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, backgroundColor.w));
			GLCall(glClear(GL_COLOR_BUFFER_BIT));

			if (newRendererSize.x != 0 && newRendererSize.y != 0)
			{
				renderer.Resize(newRendererSize.x, newRendererSize.y);
				newRendererSize = vec2();
			}

			renderer.Begin();
			{
				vec4 regionColor = vec4(.15f, .15f, .15f, 1.0f);
				renderer.Draw(aetPosition, vec2(1920, 1080), regionColor);
				//renderer.Draw(nullptr, aetSourceRegion, aetPosition, aetOrigin, aetRotation, aetScale, regionColor, (AetBlendMode)currentBlendItem);

				if ((sprSet != nullptr && sprSet->TxpSet->Textures.size() > 0) && (txpIndex >= 0 && txpIndex < sprSet->TxpSet->Textures.size()))
				{
					auto* texture = sprSet->TxpSet->Textures.at(txpIndex).get();
					auto texture2D = texture->Texture2D.get();

					vec4 sourceRegion = (aetSourceRegion.z == 0.0f || aetSourceRegion.w == 0.0f) ? vec4(0.0f, 0.0f, texture2D->GetWidth(), texture2D->GetHeight()) : aetSourceRegion;
					renderer.Draw(texture2D, sourceRegion, aetPosition, aetOrigin, aetRotation, aetScale, aetColor, (AetBlendMode)currentBlendItem);
				}

				renderer.GetShader()->Bind();
				renderer.GetShader()->SetUniform(renderer.GetShader()->UseTextShadowLocation, useTextShadow);
			}
			renderer.End();
		}
		renderTarget.UnBind();
	}

	void AetRenderWindow::OnResize(int width, int height)
	{
		RenderWindowBase::OnResize(width, height);

		newRendererSize = vec2(width, height);
	}

	void AetRenderWindow::OnInitialize()
	{
		renderer.Initialize();
	}
}
