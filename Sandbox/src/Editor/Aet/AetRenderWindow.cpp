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

	AetRenderWindow::AetRenderWindow(SpriteGetter spriteGetter)
	{
		getSprite = spriteGetter;
	}

	AetRenderWindow::~AetRenderWindow()
	{
	}

	void AetRenderWindow::SetActive(Aet* parent, AetItemTypePtr value)
	{
		aet = parent;
		active = value;
	}

	float AetRenderWindow::SetCurrentFrame(float value)
	{
		return currentFrame = value;
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
			ImGui::InputFloat2("Source Position", &aetSourceRegion.x);
			ImGui::InputFloat2("Source Size", &aetSourceRegion.z);
			ImGui::ColorEdit4("Color", glm::value_ptr(aetColor));
		}

		/*
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
		*/
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

				if (aet == nullptr)
				{
					renderer.Draw(aetPosition, vec2(1920, 1080), regionColor);
				}
				else
				{
					//vec4 regionColor = ImGui::ColorConvertU32ToFloat4(aet->BackgroundColor); color.a = 1.0f;
					renderer.Draw(aetPosition, vec2(aet->Width, aet->Height), regionColor);
				}

				if (active.VoidPointer != nullptr)
				{
					switch (active.Type())
					{
					case AetSelectionType::AetSet:
						RenderAetSet(active.AetSet);
						break;
					case AetSelectionType::Aet:
						RenderAet(active.Aet);
						break;
					case AetSelectionType::AetLayer:
						RenderAetLayer(active.AetLayer);
						break;
					case AetSelectionType::AetObj:
						RenderAetObj(active.AetObj);
						break;
					case AetSelectionType::AetRegion:
						RenderAetRegion(active.AetRegion);
						break;

					case AetSelectionType::None:
					default:
						break;
					}
				}

				/*
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
				*/
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

	void AetRenderWindow::RenderAetSet(AetSet* aetSet)
	{
	}

	void AetRenderWindow::RenderAet(Aet* aet)
	{
	}

	void AetRenderWindow::RenderAetLayer(AetLayer* aetLayer)
	{
		objectCache.clear();
		AetMgr::GetAddObjects(objectCache, aetLayer, currentFrame);

		for (auto& obj : objectCache)
			RenderObjCache(obj);
	}

	void AetRenderWindow::RenderAetObj(AetObj* aetObj)
	{
		if (aetObj->Type != AetObjType::Pic && aetObj->Type != AetObjType::Eff)
			return;

		//Properties properties;
		//AetMgr::Interpolate(aetObj->AnimationData, currentFrame, &properties);

		//AetRegion* region = aetObj->GetRegion();
		//AetSprite* aetSprite = region->Sprites.size() < 1 ? nullptr : &region->Sprites.front();

		//const bool drawPath = true;
		//if (drawPath)
		//{
		//	if (aetObj->AnimationData.Properties != nullptr)
		//	{
		//		const vec4 pathColor = vec4(.5f, .5f, .5f, .75f);
		//		const vec4 directionColor = vec4(.75f, .15f, .15f, .5f);

		//		const KeyFrameCollection& posX = aetObj->AnimationData.Properties->PositionX();
		//		const KeyFrameCollection& posY = aetObj->AnimationData.Properties->PositionY();

		//		const float startFrame = aetObj->LoopStart;
		//		const float endFrame = aetObj->LoopEnd;

		//		vec2 lastPos, pos = vec2(AetMgr::Interpolate(posX, startFrame), AetMgr::Interpolate(posY, startFrame));

		//		for (float f = startFrame; f < endFrame; f += 1.0f)
		//		{
		//			lastPos = pos;
		//			pos = vec2(AetMgr::Interpolate(posX, f), AetMgr::Interpolate(posY, f));

		//			renderer.DrawLine(aetPosition + lastPos, aetPosition + pos, pathColor, 1.5f);
		//		}

		//		renderer.DrawLine(properties.Position + aetPosition, properties.Rotation - 90.0f, properties.Scale.x * region->Width * 1.25f, directionColor, 2.0f);
		//	}
		//}

		objectCache.clear();
		AetMgr::GetAddObjects(objectCache, aetObj, currentFrame);

		for (auto& obj : objectCache)
			RenderObjCache(obj);

		//ImGui::GetForegroundDrawList()->AddText(ImGui::GetWindowPos() + aetPosition + properties.Position - properties.Origin, IM_COL32_WHITE, aetObj->GetName());
	}

	void AetRenderWindow::RenderAetRegion(AetRegion* aetRegion)
	{
		if (aetRegion->Sprites.size() < 1)
		{
			vec4 color = ImGui::ColorConvertU32ToFloat4(aetRegion->Color);
			color.a = 1.0f;

			renderer.Draw(aetPosition, vec2(aetRegion->Width, aetRegion->Height), color);
			return;
		}

		assert(getSprite != nullptr);

		size_t spriteIndex = glm::clamp(static_cast<size_t>(0), static_cast<size_t>(currentFrame), aetRegion->Sprites.size() - 1);
		AetSprite* spriteRegion = &aetRegion->Sprites.at(spriteIndex);

		Texture* texture;
		Sprite* sprite;

		if (!getSprite(spriteRegion, &texture, &sprite))
		{
			renderer.Draw(nullptr, vec4(0, 0, aetRegion->Width, aetRegion->Height), aetPosition, aetOrigin, aetRotation, aetScale, aetColor, (AetBlendMode)currentBlendItem);
		}
		else
		{
			renderer.Draw(texture->Texture2D.get(), sprite->PixelRegion, aetPosition, aetOrigin, aetRotation, aetScale, aetColor, (AetBlendMode)currentBlendItem);
		}
	}

	void AetRenderWindow::RenderObjCache(const AetMgr::ObjCache& obj)
	{
		AetSprite* aetSprite = obj.Region->Sprites.size() < 1 ? nullptr : &obj.Region->Sprites.at(obj.SpriteIndex);

		Texture* texture;
		Sprite* sprite;
		bool validSprite = getSprite(aetSprite, &texture, &sprite);

		renderer.Draw(
			validSprite ? texture->Texture2D.get() : nullptr,
			validSprite ? sprite->PixelRegion : vec4(0, 0, obj.Region->Width, obj.Region->Height),
			obj.Properties.Position + aetPosition,
			obj.Properties.Origin,
			obj.Properties.Rotation,
			obj.Properties.Scale,
			vec4(1.0f, 1.0f, 1.0f, obj.Properties.Opcaity),
			obj.BlendMode);
	}
}
