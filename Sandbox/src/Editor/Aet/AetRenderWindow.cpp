#include "AetRenderWindow.h"
#include "AetIcons.h"
#include "Editor/Theme.h"
#include "FileSystem/FileHelper.h"
#include "Misc/StringHelper.h"
#include "Graphics/Camera.h"
#include "TimeSpan.h"
#include "Input/KeyCode.h"

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
		static Properties properties;

		if (active.Type() == AetSelectionType::AetObj && active.AetObj != nullptr && active.AetObj->AnimationData.Properties != nullptr)
		{
			AetMgr::Interpolate(active.AetObj->AnimationData, &properties, currentFrame);
		}
		else
		{
			properties = {};
		}

		constexpr float itemWidth = 68.0f;

		// ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetStyleColorVec4(ImGuiCol_TabUnfocused));
		// ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImGui::GetStyleColorVec4(ImGuiCol_TabHovered));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.0f, 3.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 1.0f));
		ImGui::PushItemWidth(itemWidth);
		{
			constexpr float percentFactor = 100.0f;

			const ImVec2 spacing(4.0f, 0.0f);
			float buffer;

			ImGui::DragFloat("##PositionXDragFloat::AetRenderWindow", &properties.Position.x, 1.0f, 0.0f, 0.0f, "X: %.f");
			ImGui::SameLine();

			ImGui::DragFloat("##PositionYDragFloat::AetRenderWindow", &properties.Position.y, 1.0f, 0.0f, 0.0f, "Y: %.f");
			ImGui::SameLine();

			ImGui::ItemSize(spacing);
			ImGui::SameLine();

			ImGui::DragFloat("##OriginXDragFloat::AetRenderWindow", &properties.Origin.x, 1.0f, 0.0f, 0.0f, "X: %.f");
			ImGui::SameLine();

			ImGui::DragFloat("##OriginYDragFloat::AetRenderWindow", &properties.Origin.y, 1.0f, 0.0f, 0.0f, "Y: %.f");
			ImGui::SameLine();

			ImGui::ItemSize(spacing);
			ImGui::SameLine();

			ImGui::DragFloat("##RotationDragFloat::AetRenderWindow", &properties.Rotation, 1.0f, 0.0f, 0.0f, "R: %.2f");
			ImGui::SameLine();

			ImGui::ItemSize(spacing);
			ImGui::SameLine();

			buffer = properties.Scale.x * percentFactor;
			if (ImGui::DragFloat("##ScaleXDragFloat::AetRenderWindow", &buffer, 1.0f, 0.0f, 0.0f, "W: %.2f %%"))
				properties.Scale.x = buffer * (1.0f / percentFactor);
			ImGui::SameLine();

			buffer = properties.Scale.y * percentFactor;
			if (ImGui::DragFloat("##ScaleYDragFloat::AetRenderWindow", &buffer, 1.0f, 0.0f, 0.0f, "H: %.2f %%"))
				properties.Scale.y = buffer * (1.0f / percentFactor);
			ImGui::SameLine();

			ImGui::ItemSize(spacing);
			ImGui::SameLine();

			buffer = properties.Opacity * percentFactor;
			if (ImGui::DragFloat("##OpacityDragFloat::AetRenderWindow", &buffer, 1.0f, 0.00000001f, 100.0f, "O: %.2f %%"))
				properties.Opacity = glm::max(0.0f, buffer * (1.0f / percentFactor));
			ImGui::SameLine();

			ImGui::SetCursorPosX(ImGui::GetWindowWidth() - itemWidth - 2);
			
			float zoomPercentage = camera.Zoom * percentFactor;
			if (ImGui::DragFloat("##ZoomDragFloat::AetRenderWindow", &zoomPercentage, 0.15f, camera.ZoomMin * percentFactor, camera.ZoomMax * percentFactor, "%.2f %%"))
				this->SetUpdateCameraZoom(zoomPercentage * (1.0f / percentFactor), ImGui::GetWindowSize() / 2.0f);
		}
		ImGui::PopItemWidth();
		ImGui::PopStyleVar(2);
		// ImGui::PopStyleColor(2);

		if (active.Type() == AetSelectionType::AetObj && active.AetObj != nullptr && active.AetObj->AnimationData.Properties != nullptr)
		{
			KeyFrameProperties& keyFrameProperties = *active.AetObj->AnimationData.Properties;

			keyFrameProperties.OriginX().front().Value = properties.Origin.x;
			keyFrameProperties.OriginY().front().Value = properties.Origin.y;
			keyFrameProperties.PositionX().front().Value = properties.Position.x;
			keyFrameProperties.PositionY().front().Value = properties.Position.y;
			keyFrameProperties.Rotation().front().Value = properties.Rotation;
			keyFrameProperties.ScaleX().front().Value = properties.Scale.x;
			keyFrameProperties.ScaleY().front().Value = properties.Scale.y;
			keyFrameProperties.Opacity().front().Value = properties.Opacity;
		}
	}

	static std::vector<const SpriteVertices*> verticesPointers;
	static const AetMgr::ObjCache* selectedAetObj = nullptr;

	void AetRenderWindow::PostDrawGui()
	{
		if (false)
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

		//if (ImGui::IsMouseClicked())
		//{
		//	selectedObjIndex = -1;

		//	for (const auto& obj : objectCache)
		//	{
		//		obj.Properties
		//	}
		//}

		ImDrawList* drawList = ImGui::GetWindowDrawList();
		auto renderRegion = GetRenderRegion();

		for (const auto& vertices : verticesPointers)
		{
			ImU32 outlineColor = ImColor(vec4(1.0f));

			vec2 tl = WorldToScreenSpace(camera.ViewMatrix, vertices->TopLeft.Position) + renderRegion.GetTL();
			vec2 tr = WorldToScreenSpace(camera.ViewMatrix, vertices->TopRight.Position) + renderRegion.GetTL();
			vec2 bl = WorldToScreenSpace(camera.ViewMatrix, vertices->BottomLeft.Position) + renderRegion.GetTL();
			vec2 br = WorldToScreenSpace(camera.ViewMatrix, vertices->BottomRight.Position) + renderRegion.GetTL();

			drawList->AddLine(tl, tr, outlineColor);
			drawList->AddLine(tr, br, outlineColor);
			drawList->AddLine(br, bl, outlineColor);
			drawList->AddLine(bl, tl, outlineColor);

			// drawList->AddLine(tl, br, outlineColor);
			// drawList->AddLine(tr, bl , outlineColor);

			drawList->AddCircleFilled(tl, 3.5f, outlineColor);
			drawList->AddCircleFilled(tr, 3.5f, outlineColor);
			drawList->AddCircleFilled(bl, 3.5f, outlineColor);
			drawList->AddCircleFilled(br, 3.5f, outlineColor);
		}

		verticesPointers.clear();

	}

	void AetRenderWindow::OnUpdateInput()
	{
		//if (ImGui::IsMouseClicked(0))
		selectedAetObj = nullptr;
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
				RenderGrid();

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

	void AetRenderWindow::RenderGrid()
	{
		vec2 regionSize = (aet == nullptr) ? vec2(1280.0f, 720.0f) : vec2(aet->Width, aet->Height);

		vec4 gridColors[2] =
		{
			vec4(.15f, .15f, .15f, 1.0f),
			vec4(0.32f, 0.32f, 0.32f, 1.0f)
		};

		renderer.Draw(
			vec2(0.0f),
			regionSize,
			gridColors[0]);

		renderer.DrawCheckerboardRectangle(
			vec2(0.0f),
			vec2(1.0f),
			vec2(0.0f),
			0.0f,
			regionSize,
			gridColors[1],
			camera.Zoom);
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
		if (ImGui::IsKeyPressed(KeyCode_W, true))
			camera.Position.y += step;
		if (ImGui::IsKeyPressed(KeyCode_S, true))
			camera.Position.y -= step;
		if (ImGui::IsKeyPressed(KeyCode_A, true))
			camera.Position.x += step;
		if (ImGui::IsKeyPressed(KeyCode_D, true))
			camera.Position.x -= step;
		if (ImGui::IsKeyPressed(KeyCode_Escape, true))
		{ 
			camera.Position = vec2(0.0f); 
			camera.Zoom = 1.0f; 
		}
		if (ImGui::IsMouseDragging())
			camera.Position -= vec2(io.MouseDelta.x, io.MouseDelta.y);

		if (io.KeyAlt && io.MouseWheel != 0.0f)
		{
			float newZoom = camera.Zoom * ((io.MouseWheel > 0) ? camera.ZoomStep : (1.0f / camera.ZoomStep));
			SetUpdateCameraZoom(newZoom, GetRelativeMouse());
		}
	}

	void AetRenderWindow::SetUpdateCameraZoom(float newZoom, vec2 origin)
	{
		vec2 worldSpace = ScreenToWorldSpace(camera.ViewMatrix, origin);
		camera.Zoom = glm::clamp(newZoom, camera.ZoomMin, camera.ZoomMax);
		UpdateViewMatrix();
		vec2 postWorldSpace = ScreenToWorldSpace(camera.ViewMatrix, origin);

		camera.Position -= (postWorldSpace - worldSpace) * vec2(camera.Zoom);
	}

	static bool Contains(const vec2& tl, const vec2& tr, const vec2& bl, const vec2& br, const vec2& point)
	{
		vec2 e = vec2(tr.x - tl.x, tr.y - tl.y);
		vec2 f = vec2(bl.x - tl.x, bl.y - tl.y);

		return !(
			((point.x - tl.x) * e.x + (point.y - tl.y) * e.y < 0.0) ||
			((point.x - tr.x) * e.x + (point.y - tr.y) * e.y > 0.0) ||
			((point.x - tl.x) * f.x + (point.y - tl.y) * f.y < 0.0) ||
			((point.x - bl.x) * f.x + (point.y - bl.y) * f.y > 0.0));
	}

	void AetRenderWindow::RenderObjCache(const AetMgr::ObjCache& obj)
	{
		if (obj.Region == nullptr)
			return;

		AetSprite* aetSprite = obj.Region->Sprites.size() < 1 ? nullptr : &obj.Region->Sprites.at(obj.SpriteIndex);

		Texture* texture;
		Sprite* sprite;
		bool validSprite = getSprite(aetSprite, &texture, &sprite);

		if (validSprite)
		{
			renderer.Draw(
				texture->Texture2D.get(),
				sprite->PixelRegion,
				obj.Properties.Position,
				obj.Properties.Origin,
				obj.Properties.Rotation,
				obj.Properties.Scale,
				vec4(1.0f, 1.0f, 1.0f, obj.Properties.Opacity),
				obj.BlendMode);
		}
		else
		{
			vec4 dummyColor(0.79f, 0.90f, 0.57f, 0.50f);
			dummyColor.a *= obj.Properties.Opacity;

			renderer.Draw(
				nullptr,
				vec4(0, 0, obj.Region->Width, obj.Region->Height),
				obj.Properties.Position,
				obj.Properties.Origin,
				obj.Properties.Rotation,
				obj.Properties.Scale,
				dummyColor,
				obj.BlendMode);
		}

		//if (ImGui::IsMouseClicked(0))
		{
			const SpriteVertices& objVertices = renderer.GetLastVertices();

			vec2 value = ScreenToWorldSpace(camera.ViewMatrix, GetRelativeMouse());

			bool contains = Contains(
				objVertices.TopLeft.Position,
				objVertices.TopRight.Position,
				objVertices.BottomLeft.Position,
				objVertices.BottomRight.Position,
				value);

			if (contains)
			{
				selectedAetObj = &obj;

				const SpriteVertices& objVertices = renderer.GetLastVertices();
				verticesPointers.push_back(&objVertices);
			}
		}

		if (!validSprite)
		{
			//const SpriteVertices& vertices = renderer.GetLastVertices();
			//renderer.DrawRectangle(
			//	vertices.TopLeft.Position, 
			//	vertices.TopRight.Position, 
			//	vertices.BottomLeft.Position, 
			//	vertices.BottomRight.Position, 
			//	vec4(2.0f));
		}
	}
}
