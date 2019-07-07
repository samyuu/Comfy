#include "AetRenderWindow.h"
#include "Editor/Theme.h"
#include "FileSystem/FileHelper.h"
#include "Misc/StringHelper.h"
#include "TimeSpan.h"
#include <glfw/glfw3.h>
#include <stb/stb_image_write.h>

namespace Editor
{
	static void RenderExportImage(RenderTarget& renderTarget, Renderer2D& renderer, Texture* texture, const char* filePath)
	{
		auto texture2D = texture->Texture2D.get();

		auto origSize = renderTarget.GetSize();
		auto newSize = texture2D->GetSize();

		renderer.SetEnableAlphaTest(false);
		{
			renderer.Resize(newSize.x, newSize.y);
			renderTarget.Bind();
			renderTarget.Resize(newSize.x, newSize.y);

			GLCall(glViewport(0, 0, newSize.x, newSize.y));
			GLCall(glClearColor(0, 0, 0, 0));
			GLCall(glClear(GL_COLOR_BUFFER_BIT));

			renderer.Begin();
			renderer.Draw(texture2D, vec4(0.0f, 0.0f, newSize.x, newSize.y), vec2(0, 0), vec2(0, 0), 0, vec2(1, 1), vec4(1, 1, 1, 1));
			renderer.End();

			uint8_t* pixels = new uint8_t[newSize.x * newSize.y * 4];
			{
				renderTarget.GetTexture().Bind();
				GLCall(glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels));

				stbi_flip_vertically_on_write(true);
				stbi_write_png(filePath, newSize.x, newSize.y, 4, pixels, 4 * newSize.x);
			}
			delete[] pixels;

			renderer.Resize(origSize.x, origSize.y);
			renderTarget.Resize(origSize.x, origSize.y);
			renderTarget.UnBind();
		}
		renderer.SetEnableAlphaTest(true);
	}

	AetRenderWindow::AetRenderWindow()
	{
	}

	AetRenderWindow::~AetRenderWindow()
	{
	}

	void AetRenderWindow::SetAetObj(AetLyo* parent, AetObj* value)
	{
		aetLyo = parent;
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
			ImGui::InputFloat2("Scale", &aetScale.x);
			ImGui::InputFloat2("Source Position", &aetSourceRegion.x, 1.0f);
			ImGui::InputFloat2("Source Size", &aetSourceRegion.z, 1.0f);
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

					sprSet.reset();
					sprSet = std::make_unique<SprSet>();
					sprSet->Parse(fileBuffer.data());
					sprSet->Name = GetFileName(sprPath, false);

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
				if (ImGui::CollapsingHeader("Textures"))
				{
					for (int i = 0; i < sprSet->TxpSet->Textures.size(); i++)
					{
						auto tex = sprSet->TxpSet->Textures[i].get();

						ImGui::PushID(&tex);
						ImGui::BeginChild("AetTexturePreviewChildInner", ImVec2(), false, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoBackground);
						if (ImGui::Selectable(tex->Name.c_str(), i == txpIndex))
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

					spriteIndex = -1;
				}
				else if (ImGui::CollapsingHeader("Sprites"))
				{
					for (int i = 0; i < sprSet->Sprites.size(); i++)
					{
						Sprite& sprite = sprSet->Sprites[i];

						ImGui::PushID(&sprite);
						if (ImGui::Selectable(sprite.Name.c_str(), i == spriteIndex))
							spriteIndex = i;

						ImGui::PopID();
					}
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
		if (ImGui::IsWindowFocused())
		{
			constexpr float step = 10.0f;
			if (ImGui::IsKeyPressed(GLFW_KEY_W, true)) aetPosition.y -= step;
			if (ImGui::IsKeyPressed(GLFW_KEY_S, true)) aetPosition.y += step;
			if (ImGui::IsKeyPressed(GLFW_KEY_A, true)) aetPosition.x -= step;
			if (ImGui::IsKeyPressed(GLFW_KEY_D, true)) aetPosition.x += step;
			if (ImGui::IsKeyPressed(GLFW_KEY_ESCAPE, true)) aetPosition = vec2();
			if (ImGui::IsMouseDragging())
				aetPosition += vec2(ImGui::GetIO().MouseDelta.x, ImGui::GetIO().MouseDelta.y);
		}

		renderTarget.Bind();
		{
			GLCall(glViewport(0, 0, renderTarget.GetWidth(), renderTarget.GetHeight()));

			vec4 backgroundColor = GetColorVec4(EditorColor_DarkClear);
			GLCall(glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, backgroundColor.w));
			GLCall(glClear(GL_COLOR_BUFFER_BIT));

			if (newRendererSize.x != 0 && newRendererSize.y != 0)
			{
				renderer.Resize(newRendererSize.x, newRendererSize.y);
				newRendererSize = vec2();
			}

			renderer.SetUseTextShadow(useTextShadow);
			renderer.Begin();
			{
				vec4 regionColor = vec4(.15f, .15f, .15f, 1.0f);
				renderer.Draw(aetPosition, vec2(1920, 1080), regionColor);

				if (sprSet != nullptr)
				{
					if (spriteIndex >= 0 && spriteIndex < sprSet->Sprites.size())
					{
						Sprite* sprite = &sprSet->Sprites[spriteIndex];
						vec4 sourceRegion = vec4(sprite->PixelX, sprite->PixelY, sprite->PixelWidth, sprite->PixelHeight);

						auto texture2D = sprSet->TxpSet->Textures[sprite->TextureIndex]->Texture2D.get();

						renderer.Draw(texture2D, sourceRegion, aetPosition, aetOrigin, aetRotation, aetScale, aetColor, (AetBlendMode)currentBlendItem);
					}
					else if (sprSet->TxpSet->Textures.size() > 0 && (txpIndex >= 0 && txpIndex < sprSet->TxpSet->Textures.size()))
					{
						auto* texture = sprSet->TxpSet->Textures.at(txpIndex).get();
						auto texture2D = texture->Texture2D.get();

						vec4 sourceRegion = (aetSourceRegion.z == 0.0f || aetSourceRegion.w == 0.0f) ? vec4(0.0f, 0.0f, texture2D->GetWidth(), texture2D->GetHeight()) : aetSourceRegion;
						renderer.Draw(texture2D, sourceRegion, aetPosition, aetOrigin, aetRotation, aetScale, aetColor, (AetBlendMode)currentBlendItem);
					}
				}
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
