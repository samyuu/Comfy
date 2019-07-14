#include "AetRenderWindow.h"
#include "Editor/Theme.h"
#include "FileSystem/FileHelper.h"
#include "Misc/StringHelper.h"
#include "Graphics/Camera.h"
#include "TimeSpan.h"
#include <glfw/glfw3.h>

namespace Editor
{
	AetRenderWindow::AetRenderWindow(SpriteGetter spriteGetter)
	{
		assert(spriteGetter != nullptr);

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
	}

	void AetRenderWindow::PostDrawGui()
	{
		ImGui::Text("Camera.Position: (%.f x %.f)", camera.Position.x, camera.Position.y);
		ImGui::Text("Camera.Zoom: (%.2f)", camera.Zoom);

		vec2 relativeMouse = GetRelativeMouse();
		ImGui::Text("Relative Mouse: %d %d", static_cast<int>(relativeMouse.x), static_cast<int>(relativeMouse.y));

		vec2 mouseWorldSpace = ScreenToWorldSpace(camera.ViewMatrix, GetRelativeMouse());
		ImGui::Text("Mouse World Space: %d %d", static_cast<int>(mouseWorldSpace.x), static_cast<int>(mouseWorldSpace.y));

		vec2 backToScreenSpace = WorldToScreenSpace(camera.ViewMatrix, mouseWorldSpace);
		ImGui::Text("backToScreenSpace: %d %d", static_cast<int>(backToScreenSpace.x), static_cast<int>(backToScreenSpace.y));
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
			UpdateViewControlInput();

		renderTarget.Bind();
		{
			GLCall(glViewport(0, 0, renderTarget.GetWidth(), renderTarget.GetHeight()));

			vec4 backgroundColor = GetColorVec4(EditorColor_DarkClear);
			GLCall(glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, backgroundColor.w));
			GLCall(glClear(GL_COLOR_BUFFER_BIT));

			UpdateViewMatrix();

			renderer.SetUseTextShadow(useTextShadow);
			renderer.Begin(&camera.ViewMatrix);
			{
				vec4 regionColor = vec4(.15f, .15f, .15f, 1.0f);
				vec2 regionSize = (aet == nullptr) ? vec2(1280.0f, 720.0f) : vec2(aet->Width, aet->Height);

				renderer.Draw(vec2(0.0f), regionSize, regionColor);

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

				// Screen - WorldSpace Test
				{
					constexpr float cursorSize = 4.0f;

					vec2 mouseWorldSpace = ScreenToWorldSpace(camera.ViewMatrix, GetRelativeMouse());
					renderer.Draw(mouseWorldSpace - vec2(cursorSize * 0.5f), vec2(cursorSize), vec4(.85f, .14f, .12f, .85f));
				}
			}
			renderer.End();
		}
		renderTarget.UnBind();
	}

	void AetRenderWindow::OnResize(int width, int height)
	{
		RenderWindowBase::OnResize(width, height);

		renderer.Resize(static_cast<float>(width), static_cast<float>(height));
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

		for (const auto& obj : objectCache)
			RenderObjCache(obj);
	}

	void AetRenderWindow::RenderAetObj(AetObj* aetObj)
	{
		if (aetObj->Type != AetObjType::Pic && aetObj->Type != AetObjType::Eff)
			return;

		objectCache.clear();
		AetMgr::GetAddObjects(objectCache, aetObj, currentFrame);

		for (const auto& obj : objectCache)
			RenderObjCache(obj);
	}

	void AetRenderWindow::RenderAetRegion(AetRegion* aetRegion)
	{
		if (aetRegion->Sprites.size() < 1)
		{
			vec4 color = ImGui::ColorConvertU32ToFloat4(aetRegion->Color);
			color.a = 1.0f;

			renderer.Draw(vec2(0.0f), vec2(aetRegion->Width, aetRegion->Height), color);
			return;
		}

		size_t spriteIndex = glm::clamp(static_cast<size_t>(0), static_cast<size_t>(currentFrame), aetRegion->Sprites.size() - 1);
		AetSprite* spriteRegion = &aetRegion->Sprites.at(spriteIndex);

		Texture* texture;
		Sprite* sprite;

		if (!getSprite(spriteRegion, &texture, &sprite))
		{
			renderer.Draw(nullptr, vec4(0, 0, aetRegion->Width, aetRegion->Height), vec2(0.0f), vec2(0.0f), 0.0f, vec2(1.0f), vec4(1.0f), static_cast<AetBlendMode>(currentBlendItem));
		}
		else
		{
			renderer.Draw(texture->Texture2D.get(), sprite->PixelRegion, vec2(0.0f), vec2(0.0f), 0.0f, vec2(1.0f), vec4(1.0f), static_cast<AetBlendMode>(currentBlendItem));
		}
	}

	void AetRenderWindow::UpdateViewMatrix()
	{
		const mat4 identity = mat4(1.0f);
		camera.ViewMatrix = glm::translate(identity, vec3(-camera.Position, 0.0f)) * glm::scale(identity, vec3(camera.Zoom, camera.Zoom, 1.0f));
	}

	void AetRenderWindow::UpdateViewControlInput()
	{
		ImGuiIO& io = ImGui::GetIO();

		constexpr float step = 10.0f;
		if (ImGui::IsKeyPressed(GLFW_KEY_W, true))
			camera.Position.y += step;
		if (ImGui::IsKeyPressed(GLFW_KEY_S, true))
			camera.Position.y -= step;
		if (ImGui::IsKeyPressed(GLFW_KEY_A, true))
			camera.Position.x += step;
		if (ImGui::IsKeyPressed(GLFW_KEY_D, true))
			camera.Position.x -= step;
		if (ImGui::IsKeyPressed(GLFW_KEY_ESCAPE, true))
			camera.Position = vec2(0.0f);
		if (ImGui::IsMouseDragging())
			camera.Position -= vec2(io.MouseDelta.x, io.MouseDelta.y);

		if (io.KeyAlt && io.MouseWheel != 0.0f)
		{
			vec2 relativeMousePos = GetRelativeMouse();
			vec2 preMouseWorldSpace = ScreenToWorldSpace(camera.ViewMatrix, relativeMousePos);

			camera.Zoom *= (io.MouseWheel > 0) ? camera.ZoomStep : (1.0f / camera.ZoomStep);

			UpdateViewMatrix();
			vec2 postMouseWorldSpace = ScreenToWorldSpace(camera.ViewMatrix, relativeMousePos);

			camera.Position -= (postMouseWorldSpace - preMouseWorldSpace) * vec2(camera.Zoom);
		}
	}

	void AetRenderWindow::RenderObjCache(const AetMgr::ObjCache& obj)
	{
		if (obj.Region == nullptr)
			return;

		AetSprite* aetSprite = obj.Region->Sprites.size() < 1 ? nullptr : &obj.Region->Sprites.at(obj.SpriteIndex);

		Texture* texture;
		Sprite* sprite;
		bool validSprite = getSprite(aetSprite, &texture, &sprite);

		renderer.Draw(
			validSprite ? texture->Texture2D.get() : nullptr,
			validSprite ? sprite->PixelRegion : vec4(0, 0, obj.Region->Width, obj.Region->Height),
			obj.Properties.Position,
			obj.Properties.Origin,
			obj.Properties.Rotation,
			obj.Properties.Scale,
			validSprite ? vec4(1.0f, 1.0f, 1.0f, obj.Properties.Opacity) : vec4(0.0f),
			obj.BlendMode);

		if (!validSprite)
		{
			const SpriteVertices& objVertices = renderer.GetLastVertices();
			
			vec4 outlineColor = vec4(.75f, .75f, .75f, obj.Properties.Opacity);

			renderer.DrawRectangle(
				objVertices.TopLeft.Position,
				objVertices.TopRight.Position,
				objVertices.BottomLeft.Position,
				objVertices.BottomRight.Position,
				outlineColor);
		}
	}
}
