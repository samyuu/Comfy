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
	using namespace Graphics;

	AetRenderWindow::AetRenderWindow(AetCommandManager* commandManager, SpriteGetterFunction* spriteGetter, AetItemTypePtr* selectedAetItem, AetItemTypePtr* cameraSelectedAetItem, AetRenderPreviewData* previewData)
		: IMutatingEditorComponent(commandManager), selectedAetItem(selectedAetItem), cameraSelectedAetItem(cameraSelectedAetItem), previewData(previewData)
	{
		assert(spriteGetter != nullptr);
		assert(selectedAetItem != nullptr);
		assert(cameraSelectedAetItem != nullptr);
		assert(previewData != nullptr);

		// NOTE: The checkerboard pattern is still visible because of the framebuffer clear color blend
		const vec4 baseColor = checkerboardGrid.ColorAlt * 0.5f;
		checkerboardBaseGrid.Color = baseColor;
		checkerboardBaseGrid.ColorAlt = baseColor;

		renderer = MakeUnique<D3D_Renderer2D>();
		aetRenderer = MakeUnique<AetRenderer>(renderer.get());
		aetRenderer->SetSpriteGetterFunction(spriteGetter);
		aetRenderer->SetCallback([this](const AetMgr::ObjCache& obj, const vec2& positionOffset, float opacity) { return OnObjRender(obj, positionOffset, opacity); });
		aetRenderer->SetMaskCallback([this](const AetMgr::ObjCache& objMask, const AetMgr::ObjCache& obj, const vec2& positionOffset, float opacity) { return OnObjMaskRender(objMask, obj, positionOffset, opacity); });

		mousePicker = MakeUnique<ObjectMousePicker>(objectCache, windowHoveredOnMouseClick, selectedAetItem, cameraSelectedAetItem);

		tools[AetToolType_Hand] = MakeUnique<HandTool>();
		tools[AetToolType_Move] = MakeUnique<MoveTool>();
		tools[AetToolType_Rotate] = MakeUnique<RotateTool>();
		tools[AetToolType_Scale] = MakeUnique<ScaleTool>();
		tools[AetToolType_Transform] = MakeUnique<TransformTool>();
	}

	AetRenderWindow::~AetRenderWindow()
	{
	}

	void AetRenderWindow::SetIsPlayback(bool value)
	{
		isPlayback = value;
	}

	float AetRenderWindow::SetCurrentFrame(float value)
	{
		return currentFrame = value;
	}

	ImGuiWindowFlags AetRenderWindow::GetChildWinodwFlags() const
	{
		return ImGuiWindowFlags_None;
	}

	void AetRenderWindow::OnDrawGui()
	{
		constexpr float percentFactor = 100.0f;
		constexpr float itemWidth = 74.0f;
		constexpr float rulerSize = 18.0f;

		DrawToolSelectionHeaderGui();

		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(2.0f, 3.0f));
		Gui::PushStyleVar(ImGuiStyleVar_FramePadding, vec2(4.0f, 1.0f));
		Gui::PushItemWidth(itemWidth);
		{
			if (!selectedAetItem->IsNull() && selectedAetItem->Type() == AetItemType::Layer && selectedAetItem->GetLayerRef()->AnimationData != nullptr)
			{
				const auto& selctedLayer = selectedAetItem->GetLayerRef();
				AetMgr::Interpolate(selctedLayer->AnimationData.get(), &toolProperties, currentFrame);

				// BUG: This is problematic because the tool ignores this offset when moving
				int32_t recursionCount = 0;
				AetMgr::OffsetByParentProperties(toolProperties, selctedLayer->GetReferencedParentLayer().get(), currentFrame, recursionCount);

				toolSize = GetLayerBoundingSize(selctedLayer);
			}

			DrawTooltipHeaderGui();

			Gui::SetCursorPosX(Gui::GetWindowWidth() - itemWidth - 2);

			float zoomPercentage = camera.Zoom * percentFactor;
			if (Gui::ComfyInputFloat("##ZoomDragFloat::AetRenderWindow", &zoomPercentage, 0.15f, cameraController.ZoomMin * percentFactor, cameraController.ZoomMax * percentFactor, "%.2f %%"))
				cameraController.SetUpdateCameraZoom(camera, zoomPercentage * (1.0f / percentFactor), camera.GetProjectionCenter());
		}
		Gui::PopItemWidth();
		Gui::PopStyleVar(2);

		// TODO: This needs major work, should be toggable and render lines between edges and mouse cursor (?)
		constexpr bool showRuler = false;
		if (showRuler)
		{
			ImU32 rulerColor = Gui::GetColorU32(ImGuiCol_ScrollbarBg);
			ImU32 rulerSeparatorColor = ImU32(0xFF212121);

			Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(0.0f));
			vec2 rulerTopLeft = Gui::GetCursorScreenPos();
			vec2 rulerTopRight = rulerTopLeft + vec2(Gui::GetContentRegionAvail().x, 0.0);
			vec2 rulerBotLeft = rulerTopLeft + vec2(0.0f, Gui::GetContentRegionAvail().y);

			auto drawList = Gui::GetWindowDrawList();

			vec2 size = vec2(Gui::GetContentRegionAvail().x, rulerSize);
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
		AetTool* tool = GetCurrentTool();
		if (!selectedAetItem->IsNull() && selectedAetItem->Type() == AetItemType::Layer)
		{
			const auto worldToScreen = [this](vec2 value) { return (camera.WorldToScreenSpace(value) + GetRenderRegion().GetTL()); };
			const auto screenToWorld = [this](vec2 value) { return (camera.ScreenToWorldSpace(value - GetRenderRegion().GetTL())); };

			const Properties previousProperties = toolProperties;

			tool->SetSpaceConversionFunctions(worldToScreen, screenToWorld);
			tool->UpdatePostDrawGui(&toolProperties, toolSize);

			if (!isPlayback && selectedAetItem->GetLayerRef()->Type != AetLayerType::Aif)
			{
				const auto& layer = selectedAetItem->GetLayerRef();
				tool->ProcessCommands(GetCommandManager(), layer, currentFrame, toolProperties, previousProperties);
			}
		}

		if (currentToolType != AetToolType_Hand)
			UpdateMousePickControls();

		if (Gui::IsMouseClicked(0))
			windowHoveredOnMouseClick = Gui::IsWindowHovered();
		else if (Gui::IsMouseReleased(0))
			windowHoveredOnMouseClick = false;

		Gui::WindowContextMenu("AetRenderWindowContextMenu", [this, tool]()
		{
			Gui::TextDisabled("%s  %s", tool->GetIcon(), tool->GetName());

			tool->DrawContextMenu();
			Gui::Separator();

			bool allowSelection = !cameraSelectedAetItem->IsNull() && cameraSelectedAetItem->Type() == AetItemType::Composition;
			if (Gui::BeginMenu("Select", allowSelection))
			{
				for (const auto& layer : *cameraSelectedAetItem->GetAetCompositionRef())
				{
					bool alreadySelected = layer.get() == selectedAetItem->Ptrs.Layer;

					Gui::PushID(layer.get());
					if (Gui::MenuItem(layer->GetName().c_str(), nullptr, nullptr, !alreadySelected))
						selectedAetItem->SetItem(layer);
					Gui::PopID();
				}
				Gui::EndMenu();
			}
			Gui::Separator();

			const float cameraZoom = camera.Zoom;
			const vec2 zoomMouseOrigin = GetRelativeMouse();

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
		if (!selectedAetItem->IsNull() && selectedAetItem->GetItemParentAet() != nullptr)
			aetRegionSize = selectedAetItem->GetItemParentAet()->Resolution;

		const vec2 relativeMouse = GetRelativeMouse();
		cameraController.Update(camera, relativeMouse);
		GetCurrentTool()->UpdateCamera(camera, relativeMouse);

		renderTarget->Bind();
		{
			D3D.SetViewport(renderTarget->GetSize());
			renderTarget->Clear(GetColorVec4(EditorColor_DarkClear));

			camera.UpdateMatrices();
			renderer->Begin(camera);
			{
				RenderBackground();

				const AetItemTypePtr* visibleItem = cameraSelectedAetItem;
				if (!visibleItem->IsNull())
				{
					switch (visibleItem->Type())
					{
					case AetItemType::AetSet:
						RenderAetSet(visibleItem->Ptrs.AetSet);
						break;
					case AetItemType::Aet:
						RenderAet(visibleItem->Ptrs.Aet);
						break;
					case AetItemType::Composition:
						RenderComposition(visibleItem->Ptrs.Composition);
						break;
					case AetItemType::Layer:
						RenderLayer(visibleItem->Ptrs.Layer);
						break;
					case AetItemType::Surface:
						RenderSurface(visibleItem->Ptrs.Surface);
						break;
					case AetItemType::None:
					default:
						break;
					}
				}
			}
			renderer->End();
		}
		renderTarget->UnBind();
	}

	void AetRenderWindow::OnResize(ivec2 size)
	{
		RenderWindowBase::OnResize(size);

		const vec2 newProjectionSize(size);
		camera.Position += (camera.ProjectionSize - newProjectionSize) * 0.5f;;
		camera.ProjectionSize = newProjectionSize;

		// HACK: Hacky solution to center the camera on the first frame, might wanna center on AetSet load instead
		if (Gui::GetFrameCount() <= 2)
			CenterFitCamera();
	}

	void AetRenderWindow::DrawToolSelectionHeaderGui()
	{
		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(0.0f, 0.0f));
		Gui::PushStyleVar(ImGuiStyleVar_FramePadding, vec2(6.0f, 1.0f));

		for (int i = 0; i < AetToolType_Count; i++)
		{
			const AetTool* tool = tools[i].get();
			const bool isSelected = (tool == GetCurrentTool());

			Gui::PushStyleColor(ImGuiCol_Button, Gui::GetStyleColorVec4(isSelected ? ImGuiCol_Button : ImGuiCol_DockingEmptyBg));

			if (Gui::Button(tool->GetIcon()))
				currentToolType = static_cast<AetToolType>(i);

			if (Gui::IsItemHoveredDelayed())
			{
				const char* shortcutString = GetKeyCodeName(tool->GetShortcutKey());
				char shortcutChar = shortcutString ? static_cast<char>(toupper(shortcutString[0])) : '?';

				Gui::WideSetTooltip("%s (%c)", tool->GetName(), shortcutChar);
			}

			Gui::PopStyleColor();
			Gui::SameLine();
		}

		Gui::PopStyleVar(2);
	}

	void AetRenderWindow::DrawTooltipHeaderGui()
	{
		if (!selectedAetItem->IsNull() && selectedAetItem->Type() == AetItemType::Layer && selectedAetItem->GetLayerRef()->AnimationData != nullptr)
		{
			// TODO: Tool specific widgets
			// TODO: TransformTool could have a origin / position preset selection box here
			// tool->DrawTooltipHeader();

			/*
			Gui::ExtendedVerticalSeparator();
			Gui::Text("  <%s> ", selectedAetItem->GetLayerRef()->GetName().c_str());
			Gui::SameLine();
			*/
		}
		else
		{
			Gui::ExtendedVerticalSeparator();
			Gui::TextUnformatted("	<none>  ");
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

	void AetRenderWindow::UpdateMousePickControls()
	{
		if (cameraSelectedAetItem->IsNull() || cameraSelectedAetItem->Type() != AetItemType::Composition)
		{
			allowedMousePickerInputLastFrame = allowMousePickerInput = false;
			return;
		}

		allowedMousePickerInputLastFrame = allowMousePickerInput;
		allowMousePickerInput = Gui::IsWindowHovered() && !GetCurrentTool()->MouseFocusCaptured();

		if (allowMousePickerInput && allowedMousePickerInputLastFrame)
		{
			const vec2 mouseWorldSpace = camera.ScreenToWorldSpace(GetRelativeMouse());
			mousePicker->UpdateMouseInput(mouseWorldSpace);
		}
	}

	void AetRenderWindow::OnInitialize()
	{
		D3D_SetObjectDebugName(renderTarget->GetResourceView(), "AetRenderWindow::RenderTarget");
	}

	void AetRenderWindow::RenderBackground()
	{
		checkerboardBaseGrid.GridSize = CheckerboardGrid::DefaultGridSize * 1.2f;
		checkerboardBaseGrid.Position = camera.Position / camera.Zoom;
		checkerboardBaseGrid.Size = vec2(renderTarget->GetSize()) / camera.Zoom;
		checkerboardBaseGrid.Render(renderer.get());

		const vec2 shadowOffset = vec2(3.5f, 2.5f) / camera.Zoom * 0.5f;
		const vec2 shadowMargin = vec2(2.5f) / camera.Zoom;
		const vec2 shadowPosition = checkerboardGrid.Position + shadowOffset;
		const vec2 shadowSize = aetRegionSize + shadowMargin;
		constexpr vec4 shadowColor(0.0f, 0.0f, 0.0f, 0.15f);
		renderer->Draw(shadowPosition, shadowSize, shadowColor);

		checkerboardGrid.GridSize = CheckerboardGrid::DefaultGridSize;
		checkerboardGrid.Size = aetRegionSize;
		checkerboardGrid.Render(renderer.get());
	}

	void AetRenderWindow::RenderAetSet(const AetSet* aetSet)
	{
	}

	void AetRenderWindow::RenderAet(const Aet* aet)
	{
	}

	void AetRenderWindow::RenderComposition(const AetComposition* comp)
	{
		objectCache.clear();
		AetMgr::GetAddObjects(objectCache, comp, currentFrame);
		aetRenderer->RenderObjCacheVector(objectCache);
	}

	void AetRenderWindow::RenderLayer(const AetLayer* layer)
	{
		if (layer->Type != AetLayerType::Pic && layer->Type != AetLayerType::Eff)
			return;

		objectCache.clear();
		AetMgr::GetAddObjects(objectCache, layer, currentFrame);
		aetRenderer->RenderObjCacheVector(objectCache);
	}

	void AetRenderWindow::RenderSurface(const AetSurface* surface)
	{
		int32_t spriteIndex = glm::clamp(0, static_cast<int32_t>(currentFrame), surface->SpriteCount() - 1);
		const AetSpriteIdentifier* aetSprite = surface->GetSprite(spriteIndex);
		aetRenderer->RenderAetSprite(surface, aetSprite, vec2(0.0f, 0.0f));
	}

	vec2 AetRenderWindow::GetLayerBoundingSize(const RefPtr<AetLayer>& layer) const
	{
		const auto& surface = layer->GetReferencedSurface();

		if (surface != nullptr)
			return surface->Size;

		// TODO: Find bounding box (?), or maybe just disallow using the transform tool (?)
		// NOTE: ~~Maybe this is sufficient already (?)~~
		return aetRegionSize;
	}

	bool AetRenderWindow::OnObjRender(const AetMgr::ObjCache& obj, const vec2& positionOffset, float opacity)
	{
		const bool visible = obj.Surface != nullptr && obj.Visible;
		const bool selected = selectedAetItem->Ptrs.Layer == obj.Source;

		if (!visible || !selected)
			return false;

		const AetSurface* surface = (previewData->Surface != nullptr) ? previewData->Surface : obj.Surface;

		const Txp* txp;
		const Spr* spr;
		const bool validSprite = aetRenderer->GetSprite(surface->GetSprite(obj.SpriteIndex), &txp, &spr);

		if (!validSprite)
			return false;

		renderer->Draw(
			txp->Texture2D.get(),
			spr->PixelRegion,
			obj.Properties.Position + positionOffset,
			obj.Properties.Origin,
			obj.Properties.Rotation,
			obj.Properties.Scale,
			vec4(1.0f, 1.0f, 1.0f, obj.Properties.Opacity * opacity),
			(previewData->BlendMode != AetBlendMode::Unknown) ? previewData->BlendMode : obj.BlendMode);

		return true;
	}

	bool AetRenderWindow::OnObjMaskRender(const AetMgr::ObjCache& maskObj, const AetMgr::ObjCache& obj, const vec2& positionOffset, float opacity)
	{
		const bool visible = maskObj.Surface != nullptr && obj.Surface != nullptr && obj.Visible;

		if (!visible || selectedAetItem->IsNull())
			return false;

		const bool selected = obj.Source == selectedAetItem->Ptrs.Layer;
		const bool maskSelected = maskObj.Source == selectedAetItem->Ptrs.Layer;

		if (!selected && !maskSelected)
			return false;

		const AetSurface* surface = (previewData->Surface != nullptr && selected) ? previewData->Surface : obj.Surface;
		const AetSurface* maskSurface = (previewData->Surface != nullptr && maskSelected) ? previewData->Surface : maskObj.Surface;

		const Txp* maskTxp;
		const Spr* maskSpr;
		const bool validMaskSprite = aetRenderer->GetSprite(maskSurface->GetSprite(maskObj.SpriteIndex), &maskTxp, &maskSpr);

		const Txp* txp;
		const Spr* spr;
		const bool validSprite = aetRenderer->GetSprite(surface->GetSprite(obj.SpriteIndex), &txp, &spr);

		if (!validMaskSprite || !validSprite)
			return false;

		renderer->Draw(
			maskTxp->Texture2D.get(),
			maskSpr->PixelRegion,
			maskObj.Properties.Position,
			maskObj.Properties.Origin,
			maskObj.Properties.Rotation,
			maskObj.Properties.Scale,
			txp->Texture2D.get(),
			spr->PixelRegion,
			obj.Properties.Position + positionOffset,
			obj.Properties.Origin,
			obj.Properties.Rotation,
			obj.Properties.Scale,
			vec4(1.0f, 1.0f, 1.0f, maskObj.Properties.Opacity * obj.Properties.Opacity * opacity),
			(maskSelected && previewData->BlendMode != AetBlendMode::Unknown) ? previewData->BlendMode : maskObj.BlendMode);

		return true;
	}
}
