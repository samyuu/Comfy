#include "AetRenderWindow.h"
#include "AetIcons.h"
#include "Editor/Theme.h"
#include "FileSystem/FileHelper.h"
#include "Misc/StringHelper.h"
#include "Graphics/Camera.h"
#include "TimeSpan.h"
#include "Input/KeyCode.h"
#include "App/TestTasks.h"

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

	void AetRenderWindow::SetIsPlayback(bool value)
	{
		isPlayback = value;
	}

	float AetRenderWindow::SetCurrentFrame(float value)
	{
		return currentFrame = value;
	}

	static KeyFrame* GetKeyFrame(AetObj* aetObj, int propertyIndex, float inputFrame)
	{
		KeyFrameCollection& keyFrames = aetObj->AnimationData->Properties[propertyIndex];
		bool firstFrame = inputFrame == aetObj->LoopStart;

		return AetMgr::GetKeyFrameAt(keyFrames, (firstFrame && keyFrames.size() == 1 ? keyFrames.front().Frame : inputFrame));
	}

	void AetRenderWindow::OnDrawGui()
	{
		if (testTask != nullptr)
		{
			testTask->PreDrawGui();
			return;
		}

		constexpr float percentFactor = 100.0f;
		constexpr float itemWidth = 74.0f;

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.0f, 3.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 1.0f));
		ImGui::PushItemWidth(itemWidth);
		{
			if (active.Type() == AetSelectionType::AetObj && active.AetObj->AnimationData != nullptr)
			{
				static Properties properties;

				AetMgr::Interpolate(active.AetObj->AnimationData.get(), &properties, currentFrame);

				float roundedCurrentFrame = glm::round(currentFrame);
				KeyFrame* currentKeyFrames[PropertyType_Count];

				for (int i = 0; i < PropertyType_Count; i++)
					currentKeyFrames[i] = isPlayback ? nullptr : GetKeyFrame(active.AetObj, i, roundedCurrentFrame);

				if (ImGui::ExtendedInputFloat("##PositionXDragFloat::AetRenderWindow", &properties.Position.x, 1.0f, 0.0f, 0.0f, "X: %.f", !currentKeyFrames[PropertyType_PositionX]))
				{
					currentKeyFrames[PropertyType_PositionX]->Value = properties.Position.x;
				}
				ImGui::SameLine();

				if (ImGui::ExtendedInputFloat("##PositionYDragFloat::AetRenderWindow", &properties.Position.y, 1.0f, 0.0f, 0.0f, "Y: %.f", !currentKeyFrames[PropertyType_PositionY]))
				{
					currentKeyFrames[PropertyType_PositionY]->Value = properties.Position.y;
				}
				ImGui::SameLine();
				ImGui::ExtendedVerticalSeparator();

				if (ImGui::ExtendedInputFloat("##OriginXDragFloat::AetRenderWindow", &properties.Origin.x, 1.0f, 0.0f, 0.0f, "X: %.f", !currentKeyFrames[PropertyType_OriginX]))
				{
					currentKeyFrames[PropertyType_OriginX]->Value = properties.Origin.x;
				}
				ImGui::SameLine();

				if (ImGui::ExtendedInputFloat("##OriginYDragFloat::AetRenderWindow", &properties.Origin.y, 1.0f, 0.0f, 0.0f, "Y: %.f", !currentKeyFrames[PropertyType_OriginY]))
				{
					currentKeyFrames[PropertyType_OriginY]->Value = properties.Origin.y;
				}
				ImGui::SameLine();
				ImGui::ExtendedVerticalSeparator();

				if (ImGui::ExtendedInputFloat("##RotationDragFloat::AetRenderWindow", &properties.Rotation, 1.0f, 0.0f, 0.0f, "R: %.2f", !currentKeyFrames[PropertyType_Rotation]))
				{
					currentKeyFrames[PropertyType_Rotation]->Value = properties.Rotation;
				}
				ImGui::SameLine();
				ImGui::ExtendedVerticalSeparator();

				float scaleXBuffer = properties.Scale.x * percentFactor;
				if (ImGui::ExtendedInputFloat("##ScaleXDragFloat::AetRenderWindow", &scaleXBuffer, 1.0f, 0.0f, 0.0f, "W: %.2f %%", !currentKeyFrames[PropertyType_ScaleX]))
				{
					properties.Scale.x = scaleXBuffer * (1.0f / percentFactor);
					currentKeyFrames[PropertyType_ScaleX]->Value = properties.Scale.x;
				}
				ImGui::SameLine();

				float scaleYBuffer = properties.Scale.y * percentFactor;
				if (ImGui::ExtendedInputFloat("##ScaleYDragFloat::AetRenderWindow", &scaleYBuffer, 1.0f, 0.0f, 0.0f, "H: %.2f %%", !currentKeyFrames[PropertyType_ScaleY]))
				{
					properties.Scale.y = scaleYBuffer * (1.0f / percentFactor);
					currentKeyFrames[PropertyType_ScaleY]->Value = properties.Scale.y;
				}
				ImGui::SameLine();
				ImGui::ExtendedVerticalSeparator();

				float opacityBuffer = properties.Opacity * percentFactor;
				if (ImGui::ExtendedInputFloat("##OpacityDragFloat::AetRenderWindow", &opacityBuffer, 1.0f, 0.00000001f, 100.0f, "O: %.2f %%", !currentKeyFrames[PropertyType_Opacity]))
				{
					properties.Opacity = glm::max(0.0f, opacityBuffer * (1.0f / percentFactor));
					currentKeyFrames[PropertyType_Opacity]->Value = properties.Opacity;
				}
			}
			else
			{
				ImGui::Text("	<none>");
			}
			ImGui::SameLine();

			ImGui::SetCursorPosX(ImGui::GetWindowWidth() - itemWidth - 2);

			float zoomPercentage = camera.Zoom * percentFactor;
			if (ImGui::ExtendedInputFloat("##ZoomDragFloat::AetRenderWindow", &zoomPercentage, 0.15f, camera.ZoomMin * percentFactor, camera.ZoomMax * percentFactor, "%.2f %%"))
				this->SetUpdateCameraZoom(zoomPercentage * (1.0f / percentFactor), ImGui::GetWindowSize() / 2.0f);

		}
		ImGui::PopItemWidth();
		ImGui::PopStyleVar(2);
	}

	struct TempVertexStruct
	{
		const SpriteVertices* Vertices;
		const AetMgr::ObjCache* ObjCache;
	};

	static std::vector<TempVertexStruct> verticesPointers(0);
	static const AetMgr::ObjCache* selectedAetObj = nullptr;

	void AetRenderWindow::PostDrawGui()
	{
		if (testTask != nullptr)
		{
			testTask->PostDrawGui();
			return;
		}

		static BoxTransformControl testTransformControl;
		static Properties testProperties{};

		auto worldToScreen = [this](vec2& value) { value = WorldToScreenSpace(camera.ViewMatrix, value) + GetRenderRegion().GetTL(); };
		auto screenToWorld = [this](vec2& value) { value = ScreenToWorldSpace(camera.ViewMatrix, value) - GetRenderRegion().GetTL(); };

		//testProperties.Rotation = 45.0f;
		testProperties.Scale = vec2(2.0f);
		testTransformControl.Draw(&testProperties, vec2(200.0f, 200.0f), worldToScreen, screenToWorld, camera.Zoom);

		ImDrawList* drawList = ImGui::GetWindowDrawList();
		auto renderRegion = GetRenderRegion();

		ImU32 outlineColor = ImColor(vec4(1.0f));
		ImU32 originColor = ImColor(vec4(1.0f, 0.0f, 0.0f, 1.0f));
		for (const auto& vertices : verticesPointers)
		{
			vec2 tl = WorldToScreenSpace(camera.ViewMatrix, vertices.Vertices->TopLeft.Position) + renderRegion.GetTL();
			vec2 tr = WorldToScreenSpace(camera.ViewMatrix, vertices.Vertices->TopRight.Position) + renderRegion.GetTL();
			vec2 bl = WorldToScreenSpace(camera.ViewMatrix, vertices.Vertices->BottomLeft.Position) + renderRegion.GetTL();
			vec2 br = WorldToScreenSpace(camera.ViewMatrix, vertices.Vertices->BottomRight.Position) + renderRegion.GetTL();

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


			//vec2 origin = vertices.ObjCache->Properties.Origin + vertices.ObjCache->Properties.Position;
			//vec2 originScreenSpace = WorldToScreenSpace(camera.ViewMatrix, origin) + renderRegion.GetTL();
			//drawList->AddCircle(originScreenSpace, 3.5f, originColor);
		}

		verticesPointers.clear();

	}

	void AetRenderWindow::OnUpdateInput()
	{
		//if (ImGui::IsMouseClicked(0))
		selectedAetObj = nullptr;

		if (ImGui::IsKeyPressed(KeyCode_F10, false))
		{
			if (testTask == nullptr)
			{
				testTask = std::make_unique<App::TaskPs4Menu>();
				testTask->Initialize();
			}
			else
			{
				testTask.reset();
			}
		}
	}

	void AetRenderWindow::OnUpdate()
	{
	}

	void AetRenderWindow::OnRender()
	{
		for (int i = 0; i < 5; i++)
		{
			if (ImGui::IsMouseClicked(i))
				windowHoveredOnClick[i] = ImGui::IsWindowHovered();
		}

		if (ImGui::IsWindowFocused())
			UpdateViewControlInput();

		renderTarget.Bind();
		{
			GLCall(glViewport(0, 0, static_cast<GLint>(renderTarget.GetWidth()), static_cast<GLint>(renderTarget.GetHeight())));

			vec4 backgroundColor = GetColorVec4(EditorColor_DarkClear);
			GLCall(glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, backgroundColor.w));
			GLCall(glClear(GL_COLOR_BUFFER_BIT));

			UpdateViewMatrix();

			if (testTask != nullptr)
			{
				renderer.Begin(&camera.ViewMatrix);
				testTask->Update();
				testTask->Render(renderer);
				renderer.End();
			}
			else
			{
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
					if (ImGui::IsWindowFocused())
					{
						constexpr float cursorSize = 4.0f;

						vec2 mouseWorldSpace = ScreenToWorldSpace(camera.ViewMatrix, GetRelativeMouse());
						renderer.Draw(mouseWorldSpace - vec2(cursorSize * 0.5f), vec2(cursorSize), GetColorVec4(EditorColor_CursorInner));
					}
				}
				renderer.End();
			}
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

		renderer.Draw(
			vec2(0.0f),
			regionSize,
			gridConfig.Color);

		renderer.DrawCheckerboardRectangle(
			vec2(0.0f),
			vec2(1.0f),
			vec2(0.0f),
			0.0f,
			regionSize,
			gridConfig.ColorAlt,
			camera.Zoom * gridConfig.GridSize);
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
		size_t spriteIndex = glm::clamp(static_cast<size_t>(0), static_cast<size_t>(currentFrame), aetRegion->Sprites.size() - 1);
		AetSprite* spriteRegion = &aetRegion->Sprites.at(spriteIndex);

		Texture* texture;
		Sprite* sprite;

		if (aetRegion->Sprites.size() < 1 || !getSprite(spriteRegion, &texture, &sprite))
		{
			renderer.Draw(nullptr, vec4(0, 0, aetRegion->Width, aetRegion->Height), vec2(0.0f), vec2(0.0f), 0.0f, vec2(1.0f), dummyColor, static_cast<AetBlendMode>(currentBlendItem));
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
			camera.Position.y -= step;
		if (ImGui::IsKeyPressed(KeyCode_S, true))
			camera.Position.y += step;
		if (ImGui::IsKeyPressed(KeyCode_A, true))
			camera.Position.x -= step;
		if (ImGui::IsKeyPressed(KeyCode_D, true))
			camera.Position.x += step;
		if (ImGui::IsKeyPressed(KeyCode_Escape, true))
		{
			camera.Position = vec2(0.0f);
			camera.Zoom = 1.0f;
		}
		if (windowHoveredOnClick[1] && ImGui::IsMouseDown(1))
		{
			camera.Position -= vec2(io.MouseDelta.x, io.MouseDelta.y);
			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
		}

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
			renderer.Draw(
				nullptr,
				vec4(0, 0, obj.Region->Width, obj.Region->Height),
				obj.Properties.Position,
				obj.Properties.Origin,
				obj.Properties.Rotation,
				obj.Properties.Scale,
				vec4(dummyColor.r, dummyColor.g, dummyColor.b, dummyColor.a * obj.Properties.Opacity),
				obj.BlendMode);
		}

		if (ImGui::IsWindowFocused() && ImGui::IsWindowHovered())
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
				verticesPointers.push_back(TempVertexStruct{ &objVertices, &obj });
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
