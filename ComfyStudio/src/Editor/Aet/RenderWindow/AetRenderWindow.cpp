#include "AetRenderWindow.h"
#include "Tools/HandTool.h"
#include "Tools/MoveTool.h"
#include "Tools/RotateTool.h"
#include "Tools/ScaleTool.h"
#include "Tools/TransformTool.h"
#include "Editor/Aet/AetIcons.h"
#include "Editor/Core/Theme.h"
#include "FileSystem/FileHelper.h"
#include "Misc/StringHelper.h"
#include "Input/KeyCode.h"
#include "Core/TimeSpan.h"

namespace Editor
{
	AetRenderWindow::AetRenderWindow(SpriteGetterFunction* spriteGetter)
	{
		assert(spriteGetter != nullptr);

		checkerboardBaseGrid.Color = checkerboardGrid.Color * 0.5f;
		checkerboardBaseGrid.ColorAlt = checkerboardGrid.ColorAlt * 0.5f;

		renderer = MakeUnique<Renderer2D>();
		aetRenderer = MakeUnique<AetRenderer>(renderer.get());
		aetRenderer->SetSpriteGetterFunction(spriteGetter);

		tools[AetToolType_Hand] = MakeUnique<HandTool>();
		tools[AetToolType_Move] = MakeUnique<MoveTool>();
		tools[AetToolType_Rotate] = MakeUnique<RotateTool>();
		tools[AetToolType_Scale] = MakeUnique<ScaleTool>();
		tools[AetToolType_Transform] = MakeUnique<TransformTool>();
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

	static AetKeyFrame* GetKeyFrameOrFirst(AetObj* aetObj, int propertyIndex, float inputFrame)
	{
		KeyFrameCollection& keyFrames = aetObj->AnimationData->Properties[propertyIndex];
		bool firstFrame = inputFrame == aetObj->LoopStart;

		if (keyFrames.size() == 1)
			return &keyFrames.front();

		return AetMgr::GetKeyFrameAt(keyFrames, (firstFrame && keyFrames.size() == 1 ? keyFrames.front().Frame : inputFrame));
	}

	ImGuiWindowFlags AetRenderWindow::GetChildWinodwFlags() const
	{
		return ImGuiWindowFlags_None;
	}

	void AetRenderWindow::OnDrawGui()
	{
		constexpr bool showRuler = false;
		constexpr float percentFactor = 100.0f;
		constexpr float itemWidth = 74.0f;
		constexpr float rulerSize = 18.0f;

		DrawToolGui();

		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.0f, 3.0f));
		Gui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 1.0f));
		Gui::PushItemWidth(itemWidth);
		{
			if (!active.IsNull() && active.Type() == AetSelectionType::AetObj && active.GetAetObjRef()->AnimationData != nullptr)
			{
				AetMgr::Interpolate(active.Ptrs.AetObj->AnimationData.get(), &toolProperties, currentFrame);
				toolSize = GetAetObjBoundingSize(active.GetAetObjRef());
			}

			DrawAnimationPropertiesGui();

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

	void AetRenderWindow::PostDrawGui()
	{
		// TODO: Delete temp
		if (false)
		{
			constexpr float step = 25.0f;
			constexpr float rotationStep = 45.0f;
			if (Gui::IsWindowFocused())
			{
				if (Gui::IsKeyPressed(KeyCode_Up)) toolProperties.Origin.y -= step;
				if (Gui::IsKeyPressed(KeyCode_Down)) toolProperties.Origin.y += step;
				if (Gui::IsKeyPressed(KeyCode_Left)) toolProperties.Origin.x -= step;
				if (Gui::IsKeyPressed(KeyCode_Right)) toolProperties.Origin.x += step;
				if (Gui::IsKeyPressed(KeyCode_1)) toolProperties.Rotation += rotationStep;
				if (Gui::IsKeyPressed(KeyCode_2)) toolProperties.Rotation = 0.0f;
				if (Gui::IsKeyPressed(KeyCode_3)) toolProperties.Rotation -= rotationStep;
			}
		}

		AetTool* tool = GetCurrentTool();
		if (!active.IsNull() && active.Type() == AetSelectionType::AetObj)
		{
			auto worldToScreen = [this](vec2 value) { return (camera.WorldToScreenSpace(value) + GetRenderRegion().GetTL()); };
			auto screenToWorld = [this](vec2 value) { return (camera.ScreenToWorldSpace(value - GetRenderRegion().GetTL())); };

			Properties previousProperties = toolProperties;

			tool->SetSpaceConversionFunctions(worldToScreen, screenToWorld);
			tool->UpdatePostDrawGui(&toolProperties, toolSize);

			if (active.GetAetObjRef()->Type == AetObjType::Pic || active.GetAetObjRef()->Type == AetObjType::Eff)
			{
				AetKeyFrame* currentKeyFrames[PropertyType_Count];
				for (int i = 0; i < PropertyType_Count; i++)
					currentKeyFrames[i] = isPlayback ? nullptr : GetKeyFrameOrFirst(active.Ptrs.AetObj, i, glm::round(currentFrame));

				// TODO: if keyframe is nullptr add keyframe (how to handle adding keyframe commands?)
				// NOTE: if the keyframe for the scale for example doesn't exist at the current timeline frame, then modify the value of the 0th keyframe instead (?)

				if (currentKeyFrames[PropertyType_PositionX]) currentKeyFrames[PropertyType_PositionX]->Value = toolProperties.Position.x;
				if (currentKeyFrames[PropertyType_PositionY]) currentKeyFrames[PropertyType_PositionY]->Value = toolProperties.Position.y;

				if (currentKeyFrames[PropertyType_ScaleX]) currentKeyFrames[PropertyType_ScaleX]->Value = toolProperties.Scale.x;
				if (currentKeyFrames[PropertyType_ScaleY]) currentKeyFrames[PropertyType_ScaleY]->Value = toolProperties.Scale.y;
			}
		}

		Gui::WindowContextMenu("AetRenderWindowContextMenu", [this, tool]()
		{
			Gui::TextDisabled("%s  %s", tool->GetIcon(), tool->GetName());
			tool->DrawContextMenu();

			Gui::Separator();

			float cameraZoom = camera.Zoom;
			vec2 zoomMouseOrigin = GetRelativeMouse();

			if (Gui::MenuItem("50% Zoom", nullptr, nullptr, cameraZoom != 0.5f))
				cameraController.SetUpdateCameraZoom(camera, 0.5f, zoomMouseOrigin);
			if (Gui::MenuItem("100% Zoom", nullptr, nullptr, cameraZoom != 1.0f))
				cameraController.SetUpdateCameraZoom(camera, 1.0f, zoomMouseOrigin);
			if (Gui::MenuItem("200% Zoom", nullptr, nullptr, cameraZoom != 2.0f))
				cameraController.SetUpdateCameraZoom(camera, 2.0f, zoomMouseOrigin);
			if (Gui::MenuItem("Fit to Screen", nullptr, nullptr, true))
				CenterFitCamera();
		});
	}

	void AetRenderWindow::OnUpdateInput()
	{
		for (int i = 0; i < AetToolType_Count; i++)
		{
			if (Gui::IsKeyPressed(tools[i]->GetShortcutKey(), false))
				currentToolType = static_cast<AetToolType>(i);
		}
	}

	void AetRenderWindow::OnUpdate()
	{
	}

	void AetRenderWindow::OnRender()
	{
		if (aet != nullptr)
			aetRegionSize = aet->Resolution;

		vec2 relativeMouse = GetRelativeMouse();
		cameraController.Update(camera, relativeMouse);
		GetCurrentTool()->UpdateCamera(camera, relativeMouse);

		renderTarget.Bind();
		{
			Graphics::RenderCommand::SetViewport(renderTarget.GetSize());
			Graphics::RenderCommand::SetClearColor(GetColorVec4(EditorColor_DarkClear));
			Graphics::RenderCommand::Clear(Graphics::ClearTarget_ColorBuffer);

			camera.UpdateMatrices();
			renderer->Begin(camera);
			{
				RenderGrid();

				if (!active.IsNull())
				{
					switch (active.Type())
					{
					case AetSelectionType::AetSet:
						RenderAetSet(active.Ptrs.AetSet);
						break;
					case AetSelectionType::Aet:
						RenderAet(active.Ptrs.Aet);
						break;
					case AetSelectionType::AetLayer:
						RenderAetLayer(active.Ptrs.AetLayer);
						break;
					case AetSelectionType::AetObj:
						RenderAetObj(active.Ptrs.AetObj);
						break;
					case AetSelectionType::AetRegion:
						RenderAetRegion(active.Ptrs.AetRegion);
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
	}

	void AetRenderWindow::OnResize(int width, int height)
	{
		RenderWindowBase::OnResize(width, height);

		vec2 newProjectionSize(width, height);
		camera.Position += (camera.ProjectionSize - newProjectionSize) * 0.5f;;
		camera.ProjectionSize = newProjectionSize;

		// NOTE: Hacky solution to center the camera on the first frame, might wanna center on AetSet load instead
		if (Gui::GetFrameCount() <= 2)
			CenterFitCamera();
	}

	void AetRenderWindow::DrawToolGui()
	{
		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
		Gui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6.0f, 1.0f));

		for (int i = 0; i < AetToolType_Count; i++)
		{
			AetTool* tool = tools[i].get();
			bool isSelected = (tool == GetCurrentTool());

			Gui::PushStyleColor(ImGuiCol_Button, Gui::GetStyleColorVec4(isSelected ? ImGuiCol_Button : ImGuiCol_DockingEmptyBg));

			if (Gui::Button(tool->GetIcon()))
				currentToolType = static_cast<AetToolType>(i);

			if (Gui::IsItemHoveredDelayed())
			{
				const char* shortcutString = GetKeyCodeName(tool->GetShortcutKey());
				char shortcutChar = shortcutString ? toupper(shortcutString[0]) : '?';

				Gui::WideSetTooltip("%s (%c)", tool->GetName(), shortcutChar);
			}

			Gui::PopStyleColor();
			Gui::SameLine();
		}

		Gui::PopStyleVar(2);
	}

	void AetRenderWindow::DrawAnimationPropertiesGui()
	{
		if (!active.IsNull() && active.Type() == AetSelectionType::AetObj && active.GetAetObjRef()->AnimationData != nullptr)
		{
			Array<AetKeyFrame*, PropertyType_Count> currentKeyFrames;
			for (int i = 0; i < PropertyType_Count; i++)
				currentKeyFrames[i] = isPlayback ? nullptr : GetKeyFrameOrFirst(active.Ptrs.AetObj, i, glm::round(currentFrame));

			Gui::ExtendedVerticalSeparator();
			Gui::ComfyInputFloat("##PositionXDragFloat::AetRenderWindow", &toolProperties.Position.x, 1.0f, 0.0f, 0.0f, "X: %.f", !currentKeyFrames[PropertyType_PositionX]);
			Gui::SameLine();
			Gui::ComfyInputFloat("##PositionYDragFloat::AetRenderWindow", &toolProperties.Position.y, 1.0f, 0.0f, 0.0f, "Y: %.f", !currentKeyFrames[PropertyType_PositionY]);
			Gui::SameLine();
			Gui::ExtendedVerticalSeparator();
			Gui::ComfyInputFloat("##OriginXDragFloat::AetRenderWindow", &toolProperties.Origin.x, 1.0f, 0.0f, 0.0f, "X: %.f", !currentKeyFrames[PropertyType_OriginX]);
			Gui::SameLine();
			Gui::ComfyInputFloat("##OriginYDragFloat::AetRenderWindow", &toolProperties.Origin.y, 1.0f, 0.0f, 0.0f, "Y: %.f", !currentKeyFrames[PropertyType_OriginY]);
			Gui::SameLine();
			Gui::ExtendedVerticalSeparator();
			Gui::ComfyInputFloat("##RotationDragFloat::AetRenderWindow", &toolProperties.Rotation, 1.0f, 0.0f, 0.0f, "R: %.2f", !currentKeyFrames[PropertyType_Rotation]);
			Gui::SameLine();
			Gui::ExtendedVerticalSeparator();
			Gui::ComfyInputFloat("##ScaleXDragFloat::AetRenderWindow", &toolProperties.Scale.x, 1.0f, 0.0f, 0.0f, "W: %.2f %%", !currentKeyFrames[PropertyType_ScaleX]);
			Gui::SameLine();
			Gui::ComfyInputFloat("##ScaleYDragFloat::AetRenderWindow", &toolProperties.Scale.y, 1.0f, 0.0f, 0.0f, "H: %.2f %%", !currentKeyFrames[PropertyType_ScaleY]);
			Gui::SameLine();
			Gui::ExtendedVerticalSeparator();
			Gui::ComfyInputFloat("##OpacityDragFloat::AetRenderWindow", &toolProperties.Opacity, 1.0f, 0.00000001f, 100.0f, "O: %.2f %%", !currentKeyFrames[PropertyType_Opacity]);
			Gui::SameLine();
		}
		else
		{
			Gui::Text("	<none> ");
			Gui::SameLine();
		}
	}

	AetTool* AetRenderWindow::GetCurrentTool()
	{
		return tools.at(static_cast<size_t>(currentToolType)).get();
	}

	void AetRenderWindow::CenterFitCamera()
	{
		vec2 viewportSize = GetRenderRegion().GetSize();

		const float aetAspectRatio = aetRegionSize.x / aetRegionSize.y;
		const float viewportAspectRatio = viewportSize.x / viewportSize.y;

		if (aetAspectRatio < viewportAspectRatio)
		{
			camera.Zoom = viewportSize.y / aetRegionSize.y;
			camera.Position = vec2((aetRegionSize.x * camera.Zoom - viewportSize.x) / 2.0f, 0.0f);
		}
		else
		{
			camera.Zoom = viewportSize.x / aetRegionSize.x;
			camera.Position = vec2(0.0f, (aetRegionSize.y * camera.Zoom - viewportSize.y) / 2.0f);
		}

		constexpr float cameraFitZoomMargin = 1.025f;

		camera.UpdateMatrices();
		cameraController.SetUpdateCameraZoom(camera, camera.Zoom / cameraFitZoomMargin, camera.GetProjectionCenter());
	}

	void AetRenderWindow::OnInitialize()
	{
		renderer->Initialize();
	}

	void AetRenderWindow::RenderGrid()
	{
		checkerboardBaseGrid.Position = camera.Position / camera.Zoom;
		checkerboardBaseGrid.Size = renderTarget.GetSize() / camera.Zoom;
		checkerboardBaseGrid.Render(renderer.get());

		const vec2 shadowOffset = vec2(3.5f, 2.5f) / camera.Zoom * 0.5f;
		const vec2 shadowMargin = vec2(2.5f) / camera.Zoom;
		const vec2 shadowPosition = checkerboardGrid.Position + shadowOffset;
		const vec2 shadowSize = aetRegionSize + shadowMargin;
		constexpr vec4 shadowColor(0.0f, 0.0f, 0.0f, 0.15f);
		renderer->Draw(shadowPosition, shadowSize, shadowColor);

		checkerboardGrid.Size = aetRegionSize;
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
		aetRenderer->RenderObjCacheVector(objectCache);
	}

	void AetRenderWindow::RenderAetObj(AetObj* aetObj)
	{
		if (aetObj->Type != AetObjType::Pic && aetObj->Type != AetObjType::Eff)
			return;

		objectCache.clear();
		AetMgr::GetAddObjects(objectCache, aetObj, currentFrame);
		aetRenderer->RenderObjCacheVector(objectCache);
	}

	void AetRenderWindow::RenderAetRegion(AetRegion* aetRegion)
	{
		int32_t spriteIndex = glm::clamp(0, static_cast<int32_t>(currentFrame), aetRegion->SpriteCount() - 1);
		AetSprite* aetSprite = aetRegion->GetSprite(spriteIndex);
		aetRenderer->RenderAetSprite(aetRegion, aetSprite, vec2(0.0f, 0.0f));
	}

	vec2 AetRenderWindow::GetAetObjBoundingSize(const RefPtr<AetObj>& aetObj) const
	{
		const auto& aetRegion = aetObj->GetReferencedRegion();

		if (aetRegion != nullptr)
			return vec2(aetRegion->Width, aetRegion->Height);

		// TODO: Find bounding box (?), or maybe just disallow using the transform tool (?)
		// NOTE: Maybe this is sufficient already (?)
		return aetRegionSize;
	}
}
