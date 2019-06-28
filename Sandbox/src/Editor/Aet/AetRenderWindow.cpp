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
		const char* blendModeNames = "None\0None\0None\0Alpha\0None\0Additive\0DstColorZero\0SrcAlphaOneMinusSrcColor\0Transparent";
		ImGui::Combo("Blend Mode", &currentBlendItem, blendModeNames);

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
		renderTarget.Bind();
		{
			glViewport(0, 0, renderTarget.GetWidth(), renderTarget.GetHeight());

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			//ImVec4 backgroundColor = (aetLyo == nullptr) ? ImVec4(.4f, .5f, .1f, 1.0f) : ImGui::ColorConvertU32ToFloat4(aetLyo->BackgroundColor);
			//ImVec4 backgroundColor = ImVec4(0.65f, 0.50f, 0.00f, 1.0f);
			ImVec4 backgroundColor = ImColor(GetColor(EditorColor_BaseClear)).Value;
			glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, backgroundColor.w);
			glClear(GL_COLOR_BUFFER_BIT);

			if ((sprSet != nullptr && sprSet->TxpSet->Textures.size() > 0) && (txpIndex >= 0 && txpIndex < sprSet->TxpSet->Textures.size()))
			{
				// swapBuffers(0) / FNT 24 
				// glDrawArrays(): [DEBUG_STOPWATCH] Renderer2D Begin() to End() : 0.15 MS
				// DEBUG_STOPWATCH("Renderer2D Begin() to End()");

				renderer.Begin();
				{
					constexpr float step = 10.0f;
					static vec2 txpPos = vec2(0, 0);
					if (ImGui::IsKeyPressed(GLFW_KEY_W, true)) txpPos.y -= step;
					if (ImGui::IsKeyPressed(GLFW_KEY_S, true)) txpPos.y += step;
					if (ImGui::IsKeyPressed(GLFW_KEY_A, true)) txpPos.x -= step;
					if (ImGui::IsKeyPressed(GLFW_KEY_D, true)) txpPos.x += step;
					if (ImGui::IsKeyPressed(GLFW_KEY_ESCAPE, true)) txpPos = vec2();
					if (ImGui::IsWindowFocused() && ImGui::IsMouseDragging()) 
						txpPos += vec2(ImGui::GetIO().MouseDelta.x, ImGui::GetIO().MouseDelta.y);

					auto* texture = sprSet->TxpSet->Textures.at(txpIndex).get();
					auto relativeMouse = ImGui::GetMousePos() - ImGui::GetWindowPos();
					auto mousePos = vec2(relativeMouse.x, relativeMouse.y);

					renderer.Draw(vec2(), vec2(ImGui::GetWindowWidth(), ImGui::GetWindowHeight()), vec4(.1f, .1f, .1f, 1.0f));
					renderer.Draw(txpPos, vec2(1920, 1080), vec4(.15f, .15f, .15f, 1.0f));

					renderer.Draw(texture->Texture2D.get(), txpPos, vec4(1.0f, 1.0f, 1.0f, 1.0f), (AetBlendMode)currentBlendItem);
				}
				renderer.End();
			}
		}
		renderTarget.UnBind();
	}

	void AetRenderWindow::OnResize(int width, int height)
	{
		RenderWindowBase::OnResize(width, height);

		renderer.Resize(width, height);
	}

	void AetRenderWindow::OnInitialize()
	{
		renderer.Initialize();
	}
}
