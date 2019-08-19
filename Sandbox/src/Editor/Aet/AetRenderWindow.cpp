#include "AetRenderWindow.h"
#include "AetIcons.h"
#include "Editor/Theme.h"
#include "FileSystem/FileHelper.h"
#include "Misc/StringHelper.h"
#include "Input/KeyCode.h"
#include "Core/TimeSpan.h"

namespace Editor
{
	class AetObjMousePicker
	{
		// TODO:
	};

	AetRenderWindow::AetRenderWindow(SpriteGetterFunction* spriteGetter)
	{
		assert(spriteGetter != nullptr);

		renderer = MakeUnique<Renderer2D>();
		aetRenderer = MakeUnique<AetRenderer>(renderer.get());
		aetRenderer->SetSpriteGetterFunction(spriteGetter);
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

	static AetKeyFrame* GetKeyFrame(AetObj* aetObj, int propertyIndex, float inputFrame)
	{
		KeyFrameCollection& keyFrames = aetObj->AnimationData->Properties[propertyIndex];
		bool firstFrame = inputFrame == aetObj->LoopStart;

		return AetMgr::GetKeyFrameAt(keyFrames, (firstFrame && keyFrames.size() == 1 ? keyFrames.front().Frame : inputFrame));
	}

	ImGuiWindowFlags AetRenderWindow::GetChildWinodwFlags() const
	{
		return ImGuiWindowFlags_None;
		return ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_AlwaysVerticalScrollbar;
	}

	void AetRenderWindow::OnDrawGui()
	{
		constexpr bool showRuler = false;
		constexpr float percentFactor = 100.0f;
		constexpr float itemWidth = 74.0f;
		constexpr float rulerSize = 18.0f;

		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.0f, 3.0f));
		Gui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 1.0f));
		Gui::PushItemWidth(itemWidth);
		{
			if (active.Type() == AetSelectionType::AetObj && active.VoidPointer != nullptr && active.AetObj->AnimationData != nullptr)
			{
				static Properties properties;

				AetMgr::Interpolate(active.AetObj->AnimationData.get(), &properties, currentFrame);

				float roundedCurrentFrame = glm::round(currentFrame);
				AetKeyFrame* currentKeyFrames[PropertyType_Count];

				// TEMP TEST:
				/*
				if (!isPlayback && Gui::IsKeyPressed(KeyCode_F1, false))
				{
					for (int i = 0; i < PropertyType_Count; i++)
					{
						AetKeyFrame* existingKeyFrame = GetKeyFrame(active.AetObj, i, roundedCurrentFrame);
						if (existingKeyFrame == nullptr)
						{
							auto& keyFrames = active.AetObj->AnimationData->Properties.KeyFrames[i];
							keyFrames.push_back(AetKeyFrame(roundedCurrentFrame, ((float*)&properties)[i], 0.0f));

							struct ComparisonStruct { inline bool operator() (const AetKeyFrame& keyFrameA, const AetKeyFrame& keyFrameB) { return (keyFrameA.Frame < keyFrameB.Frame); } };
							std::sort(keyFrames.begin(), keyFrames.end(), ComparisonStruct());
						}
					}
				}
				*/

				for (int i = 0; i < PropertyType_Count; i++)
					currentKeyFrames[i] = isPlayback ? nullptr : GetKeyFrame(active.AetObj, i, roundedCurrentFrame);

				if (Gui::ComfyInputFloat("##PositionXDragFloat::AetRenderWindow", &properties.Position.x, 1.0f, 0.0f, 0.0f, "X: %.f", !currentKeyFrames[PropertyType_PositionX]))
				{
					currentKeyFrames[PropertyType_PositionX]->Value = properties.Position.x;
				}
				Gui::SameLine();

				if (Gui::ComfyInputFloat("##PositionYDragFloat::AetRenderWindow", &properties.Position.y, 1.0f, 0.0f, 0.0f, "Y: %.f", !currentKeyFrames[PropertyType_PositionY]))
				{
					currentKeyFrames[PropertyType_PositionY]->Value = properties.Position.y;
				}
				Gui::SameLine();
				Gui::ExtendedVerticalSeparator();

				if (Gui::ComfyInputFloat("##OriginXDragFloat::AetRenderWindow", &properties.Origin.x, 1.0f, 0.0f, 0.0f, "X: %.f", !currentKeyFrames[PropertyType_OriginX]))
				{
					currentKeyFrames[PropertyType_OriginX]->Value = properties.Origin.x;
				}
				Gui::SameLine();

				if (Gui::ComfyInputFloat("##OriginYDragFloat::AetRenderWindow", &properties.Origin.y, 1.0f, 0.0f, 0.0f, "Y: %.f", !currentKeyFrames[PropertyType_OriginY]))
				{
					currentKeyFrames[PropertyType_OriginY]->Value = properties.Origin.y;
				}
				Gui::SameLine();
				Gui::ExtendedVerticalSeparator();

				if (Gui::ComfyInputFloat("##RotationDragFloat::AetRenderWindow", &properties.Rotation, 1.0f, 0.0f, 0.0f, "R: %.2f", !currentKeyFrames[PropertyType_Rotation]))
				{
					currentKeyFrames[PropertyType_Rotation]->Value = properties.Rotation;
				}
				Gui::SameLine();
				Gui::ExtendedVerticalSeparator();

				float scaleXBuffer = properties.Scale.x * percentFactor;
				if (Gui::ComfyInputFloat("##ScaleXDragFloat::AetRenderWindow", &scaleXBuffer, 1.0f, 0.0f, 0.0f, "W: %.2f %%", !currentKeyFrames[PropertyType_ScaleX]))
				{
					properties.Scale.x = scaleXBuffer * (1.0f / percentFactor);
					currentKeyFrames[PropertyType_ScaleX]->Value = properties.Scale.x;
				}
				Gui::SameLine();

				float scaleYBuffer = properties.Scale.y * percentFactor;
				if (Gui::ComfyInputFloat("##ScaleYDragFloat::AetRenderWindow", &scaleYBuffer, 1.0f, 0.0f, 0.0f, "H: %.2f %%", !currentKeyFrames[PropertyType_ScaleY]))
				{
					properties.Scale.y = scaleYBuffer * (1.0f / percentFactor);
					currentKeyFrames[PropertyType_ScaleY]->Value = properties.Scale.y;
				}
				Gui::SameLine();
				Gui::ExtendedVerticalSeparator();

				float opacityBuffer = properties.Opacity * percentFactor;
				if (Gui::ComfyInputFloat("##OpacityDragFloat::AetRenderWindow", &opacityBuffer, 1.0f, 0.00000001f, 100.0f, "O: %.2f %%", !currentKeyFrames[PropertyType_Opacity]))
				{
					properties.Opacity = glm::max(0.0f, opacityBuffer * (1.0f / percentFactor));
					currentKeyFrames[PropertyType_Opacity]->Value = properties.Opacity;
				}
			}
			else
			{
				Gui::Text("	<none>");
			}
			Gui::SameLine();

			Gui::SetCursorPosX(Gui::GetWindowWidth() - itemWidth - 2);

			float zoomPercentage = camera.Zoom * percentFactor;
			if (Gui::ComfyInputFloat("##ZoomDragFloat::AetRenderWindow", &zoomPercentage, 0.15f, cameraController.ZoomMin * percentFactor, cameraController.ZoomMax * percentFactor, "%.2f %%"))
				cameraController.SetUpdateCameraZoom(camera, zoomPercentage * (1.0f / percentFactor), camera.GetProjectionCenter());
		}
		Gui::PopItemWidth();
		Gui::PopStyleVar(2);

		if (showRuler)
		{
			ImU32 rulerColor = Gui::GetColorU32(ImGuiCol_ScrollbarBg);
			ImU32 rulerSeparatorColor = ImU32(0xFF212121);

			Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(0.0f));
			vec2 rulerTopLeft = Gui::GetCursorScreenPos();
			vec2 rulerTopRight = rulerTopLeft + vec2(Gui::GetContentRegionAvail().x, 0.0);
			vec2 rulerBotLeft = rulerTopLeft + vec2(0.0f, Gui::GetContentRegionAvail().y);

			auto* drawList = Gui::GetWindowDrawList();

			vec2 size(Gui::GetContentRegionAvail().x, rulerSize);
			drawList->AddRectFilled(rulerTopLeft, rulerTopLeft + size, rulerColor);
			Gui::ItemSize(size);

			size = vec2(rulerSize, Gui::GetContentRegionAvail().y);
			drawList->AddRectFilled(Gui::GetCursorScreenPos(), Gui::GetCursorScreenPos() + size, rulerColor);
			Gui::ItemSize(size);

			Gui::SameLine();
			Gui::PopStyleVar();

			drawList->AddLine(rulerTopLeft, rulerTopRight, rulerSeparatorColor);
			drawList->AddLine(rulerTopLeft, rulerBotLeft, rulerSeparatorColor);
		}
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
		static BoxTransformControl testTransformControl;
		static Properties testProperties{};

		auto worldToScreen = [this](vec2& value) { value = camera.WorldToScreenSpace(value) + GetRenderRegion().GetTL(); };
		auto screenToWorld = [this](vec2& value) { value = camera.ScreenToWorldSpace(value) - GetRenderRegion().GetTL(); };

		//testProperties.Rotation = 45.0f;
		testProperties.Scale = vec2(2.0f);
		//testTransformControl.Draw(&testProperties, vec2(200.0f, 200.0f), worldToScreen, screenToWorld, camera.Zoom);

		ImDrawList* drawList = Gui::GetWindowDrawList();
		auto renderRegion = GetRenderRegion();

		ImU32 outlineColor = ImColor(vec4(1.0f));
		ImU32 originColor = ImColor(vec4(1.0f, 0.0f, 0.0f, 1.0f));
		for (const auto& vertices : verticesPointers)
		{
			vec2 tl = camera.WorldToScreenSpace(vertices.Vertices->TopLeft.Position) + renderRegion.GetTL();
			vec2 tr = camera.WorldToScreenSpace(vertices.Vertices->TopRight.Position) + renderRegion.GetTL();
			vec2 bl = camera.WorldToScreenSpace(vertices.Vertices->BottomLeft.Position) + renderRegion.GetTL();
			vec2 br = camera.WorldToScreenSpace(vertices.Vertices->BottomRight.Position) + renderRegion.GetTL();

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
		//if (Gui::IsMouseClicked(0))
		selectedAetObj = nullptr;
	}

	void AetRenderWindow::OnUpdate()
	{
	}

	void AetRenderWindow::OnRender()
	{
		cameraController.Update(camera, GetRelativeMouse());

		renderTarget.Bind();
		{
			Graphics::RenderCommand::SetViewport(renderTarget.GetSize());
			Graphics::RenderCommand::SetClearColor(GetColorVec4(EditorColor_DarkClear));
			Graphics::RenderCommand::Clear(Graphics::ClearTarget_ColorBuffer);

			camera.UpdateMatrices();

			renderer->SetUseTextShadow(useTextShadow);
			renderer->Begin(camera);
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
				if (Gui::IsWindowFocused())
				{
					constexpr float cursorSize = 4.0f;

					vec2 mouseWorldSpace = camera.ScreenToWorldSpace(GetRelativeMouse());
					renderer->Draw(mouseWorldSpace - vec2(cursorSize * 0.5f), vec2(cursorSize), GetColorVec4(EditorColor_CursorInner));
				}
			}
			renderer->End();
		}
		renderTarget.UnBind();

		// TODO:
		//Gui::ItemSize(GetRenderRegion().GetSize() - vec2(Gui::GetStyle) + camera.Position);
	}

	void AetRenderWindow::OnResize(int width, int height)
	{
		RenderWindowBase::OnResize(width, height);
		camera.ProjectionSize = vec2(width, height);
	}

	void AetRenderWindow::OnInitialize()
	{
		renderer->Initialize();
	}

	void AetRenderWindow::RenderGrid()
	{
		checkerboardGrid.Size = (aet == nullptr) ? vec2(1280.0f, 720.0f) : vec2(aet->Width, aet->Height);
		checkerboardGrid.Render(renderer.get());
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

		RenderObjCache(objectCache);
	}

	void AetRenderWindow::RenderAetObj(AetObj* aetObj)
	{
		if (aetObj->Type != AetObjType::Pic && aetObj->Type != AetObjType::Eff)
			return;

		objectCache.clear();
		AetMgr::GetAddObjects(objectCache, aetObj, currentFrame);

		RenderObjCache(objectCache);
	}

	void AetRenderWindow::RenderAetRegion(AetRegion* aetRegion)
	{
		int32_t spriteIndex = glm::clamp(0, static_cast<int32_t>(currentFrame), aetRegion->SpriteSize() - 1);
		AetSprite* spriteRegion = aetRegion->GetSprite(spriteIndex);

		const Texture* texture;
		const Sprite* sprite;

		if (aetRegion->SpriteSize() < 1 || !(*aetRenderer->GetSpriteGetterFunction())(spriteRegion, &texture, &sprite))
		{
			renderer->Draw(nullptr, vec4(0, 0, aetRegion->Width, aetRegion->Height), vec2(0.0f), vec2(0.0f), 0.0f, vec2(1.0f), dummyColor, static_cast<AetBlendMode>(currentBlendItem));
		}
		else
		{
			renderer->Draw(texture->GraphicsTexture.get(), sprite->PixelRegion, vec2(0.0f), vec2(0.0f), 0.0f, vec2(1.0f), vec4(1.0f), static_cast<AetBlendMode>(currentBlendItem));
		}
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
		if (obj.Region == nullptr || !obj.Visible)
			return;

		const Texture* texture;
		const Sprite* sprite;
		bool validSprite = (*aetRenderer->GetSpriteGetterFunction())(obj.Region->GetSprite(obj.SpriteIndex), &texture, &sprite);

		if (validSprite)
		{
			renderer->Draw(
				texture->GraphicsTexture.get(),
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
			renderer->Draw(
				nullptr,
				vec4(0, 0, obj.Region->Width, obj.Region->Height),
				obj.Properties.Position,
				obj.Properties.Origin,
				obj.Properties.Rotation,
				obj.Properties.Scale,
				vec4(dummyColor.r, dummyColor.g, dummyColor.b, dummyColor.a * obj.Properties.Opacity),
				obj.BlendMode);
		}

		if (Gui::IsWindowFocused() && Gui::IsWindowHovered())
		{
			const SpriteVertices& objVertices = renderer->GetLastVertices();

			vec2 value = camera.ScreenToWorldSpace(GetRelativeMouse());

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
	}

	void AetRenderWindow::RenderObjCache(const AetMgr::ObjCache& maskObj, const AetMgr::ObjCache& obj)
	{
		if (maskObj.Region == nullptr || obj.Region == nullptr)
			return;

		const Texture* maskTexture;
		const Sprite* maskSprite;
		bool validMaskSprite = (*aetRenderer->GetSpriteGetterFunction())(maskObj.Region->GetSprite(maskObj.SpriteIndex), &maskTexture, &maskSprite);

		const Texture* texture;
		const Sprite* sprite;
		bool validSprite = (*aetRenderer->GetSpriteGetterFunction())(obj.Region->GetSprite(obj.SpriteIndex), &texture, &sprite);

		if (validMaskSprite && validSprite)
		{
			renderer->Draw(
				maskTexture->GraphicsTexture.get(),
				maskSprite->PixelRegion,
				maskObj.Properties.Position,
				maskObj.Properties.Origin,
				maskObj.Properties.Rotation,
				maskObj.Properties.Scale,
				texture->GraphicsTexture.get(),
				sprite->PixelRegion,
				obj.Properties.Position,
				obj.Properties.Origin,
				obj.Properties.Rotation,
				obj.Properties.Scale,
				vec4(1.0f, 1.0f, 1.0f, obj.Properties.Opacity),
				obj.BlendMode);
		}
	}

	void AetRenderWindow::RenderObjCache(const std::vector<AetMgr::ObjCache>& objectCache)
	{
		bool singleObject = objectCache.size() == 1;

		for (size_t i = 0; i < objectCache.size(); i++)
		{
			auto& obj = objectCache[i];

			if (obj.UseTextureMask && !singleObject && (i + 1 < objectCache.size()))
			{
				RenderObjCache(objectCache[i + 1], obj);
				i++;
			}
			else
			{
				RenderObjCache(obj);
			}
		}
	}
}
