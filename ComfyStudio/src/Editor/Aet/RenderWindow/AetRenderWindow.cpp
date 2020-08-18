#include "AetRenderWindow.h"
#include "Tools/HandTool.h"
#include "Tools/MoveTool.h"
#include "Tools/RotateTool.h"
#include "Tools/ScaleTool.h"
#include "Tools/TransformTool.h"
#include "Editor/Aet/AetIcons.h"
#include "Editor/Core/Theme.h"
#include "Misc/StringUtil.h"
#include "Input/Input.h"
#include "Time/TimeSpan.h"

namespace Comfy::Studio::Editor
{
	using namespace Graphics;
	using namespace Graphics::Aet;

	AetRenderWindow::AetRenderWindow(Undo::UndoManager& undoManager, Render::Renderer2D& renderer, AetItemTypePtr& selectedAetItem, AetItemTypePtr& cameraSelectedAetItem, AetRenderPreviewData& previewData)
		: undoManager(undoManager), renderer(renderer), selectedAetItem(selectedAetItem), cameraSelectedAetItem(cameraSelectedAetItem), previewData(previewData)
	{
		// NOTE: The checkerboard pattern is still visible because of the framebuffer clear color blend
		const vec4 baseColor = (checkerboardGrid.ColorAlt * 0.5f);
		checkerboardBaseGrid.Color = baseColor;
		checkerboardBaseGrid.ColorAlt = baseColor;

		renderTarget = Render::Renderer2D::CreateRenderTarget();

		renderer.Aet().SetObjCallback([&](const Graphics::Aet::Util::Obj& obj, vec2 positionOffset, float opacity) -> bool
		{
			return OnObjRender(obj, positionOffset, opacity);
		});

		renderer.Aet().SetObjMaskCallback([&](const Graphics::Aet::Util::Obj& maskObj, const Graphics::Aet::Util::Obj& obj, vec2 positionOffset, float opacity) -> bool
		{
			return OnObjMaskRender(maskObj, obj, positionOffset, opacity);
		});

		renderer.Aet().SetRenderNullVideos(true);

		mousePicker = std::make_unique<ObjectMousePicker>(objectCache, windowHoveredOnMouseClick, selectedAetItem, cameraSelectedAetItem);

		tools[AetToolType_Hand] = std::make_unique<HandTool>();
		tools[AetToolType_Move] = std::make_unique<MoveTool>();
		tools[AetToolType_Rotate] = std::make_unique<RotateTool>();
		tools[AetToolType_Scale] = std::make_unique<ScaleTool>();
		tools[AetToolType_Transform] = std::make_unique<TransformTool>();
	}

	void AetRenderWindow::SetIsPlayback(bool value)
	{
		isPlayback = value;
	}

	float AetRenderWindow::SetCurrentFrame(float value)
	{
		return currentFrame = value;
	}

	ImTextureID AetRenderWindow::GetTextureID() const
	{
		return (renderTarget != nullptr) ? renderTarget->GetTextureID() : nullptr;
	}

	void AetRenderWindow::UpdateMousePickControls()
	{
		if (cameraSelectedAetItem.IsNull() || cameraSelectedAetItem.Type() != AetItemType::Composition)
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

	ImGuiWindowFlags AetRenderWindow::GetRenderTextureChildWindowFlags() const
	{
		return ImGuiWindowFlags_None;
	}

	void AetRenderWindow::OnFirstFrame()
	{
	}

	void AetRenderWindow::PreRenderTextureGui()
	{
		constexpr float percentFactor = 100.0f;
		constexpr float itemWidth = 74.0f;
		constexpr float rulerSize = 18.0f;

		ToolSelectionHeaderGui();

		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(2.0f, 3.0f));
		Gui::PushStyleVar(ImGuiStyleVar_FramePadding, vec2(4.0f, 1.0f));
		Gui::PushItemWidth(itemWidth);
		{
			if (!selectedAetItem.IsNull() && selectedAetItem.Type() == AetItemType::Layer && selectedAetItem.GetLayerRef()->LayerVideo != nullptr)
			{
				const auto& selctedLayer = selectedAetItem.GetLayerRef();
				toolTransform = Aet::Util::GetTransformAt(*selctedLayer->LayerVideo, currentFrame);

				// BUG: This is problematic because the tool ignores this offset when moving
				i32 recursionCount = 0;
				Aet::Util::ApplyParentTransform(toolTransform, selctedLayer->GetRefParentLayer().get(), currentFrame, recursionCount);

				toolSize = GetLayerBoundingSize(selctedLayer);
			}

			TooltipHeaderGui();

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

	void AetRenderWindow::PostRenderTextureGui()
	{
		AetTool* tool = GetCurrentTool();
		if (!selectedAetItem.IsNull() && selectedAetItem.Type() == AetItemType::Layer)
		{
			const auto worldToScreen = [this](vec2 value) { return (camera.WorldToScreenSpace(value) + GetRenderRegion().GetTL()); };
			const auto screenToWorld = [this](vec2 value) { return (camera.ScreenToWorldSpace(value - GetRenderRegion().GetTL())); };

			const Transform2D previousTransform = toolTransform;

			tool->SetSpaceConversionFunctions(worldToScreen, screenToWorld);
			tool->UpdatePostDrawGui(&toolTransform, toolSize);

			if (!isPlayback && selectedAetItem.GetLayerRef()->ItemType != ItemType::Audio)
			{
				const auto& layer = selectedAetItem.GetLayerRef();
				tool->ProcessCommands(undoManager, layer, currentFrame, toolTransform, previousTransform);
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

			bool allowSelection = !cameraSelectedAetItem.IsNull() && cameraSelectedAetItem.Type() == AetItemType::Composition;
			if (Gui::BeginMenu("Select", allowSelection))
			{
				for (const auto& layer : cameraSelectedAetItem.GetCompositionRef()->GetLayers())
				{
					bool alreadySelected = layer.get() == selectedAetItem.Ptrs.Layer;

					Gui::PushID(layer.get());
					if (Gui::MenuItem(layer->GetName().c_str(), nullptr, nullptr, !alreadySelected))
						selectedAetItem.SetItem(layer);
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

	void AetRenderWindow::OnResize(ivec2 newSize)
	{
		renderTarget->Param.Resolution = newSize;

		const auto newProjectionSize = vec2(newSize);
		camera.Position += (camera.ProjectionSize - newProjectionSize) * 0.5f;
		camera.ProjectionSize = newProjectionSize;

		// HACK: Hacky solution to center the camera on the first frame, might wanna center on AetSet load instead
		if (Gui::GetFrameCount() <= 2)
			CenterFitCamera();
	}

	void AetRenderWindow::OnRender()
	{
		if (Gui::IsWindowFocused())
		{
			for (int i = 0; i < AetToolType_Count; i++)
			{
				if (Gui::IsKeyPressed(tools[i]->GetShortcutKey(), false))
					currentToolType = static_cast<AetToolType>(i);
			}
		}

		if (!selectedAetItem.IsNull() && selectedAetItem.GetItemParentScene() != nullptr)
			aetRegionSize = selectedAetItem.GetItemParentScene()->Resolution;

		const vec2 relativeMouse = GetRelativeMouse();
		cameraController.Update(camera, relativeMouse);
		GetCurrentTool()->UpdateCamera(camera, relativeMouse);

		renderTarget->Param.ClearColor = GetColorVec4(EditorColor_DarkClear);
		renderer.Begin(camera, *renderTarget);
		{
			RenderBackground();

			const AetItemTypePtr& visibleItem = cameraSelectedAetItem;
			if (!visibleItem.IsNull())
			{
				switch (visibleItem.Type())
				{
				case AetItemType::AetSet:
					RenderAetSet(visibleItem.Ptrs.AetSet);
					break;
				case AetItemType::Scene:
					RenderScene(visibleItem.Ptrs.Scene);
					break;
				case AetItemType::Composition:
					RenderComposition(visibleItem.Ptrs.Composition);
					break;
				case AetItemType::Layer:
					RenderLayer(visibleItem.Ptrs.Layer);
					break;
				case AetItemType::Video:
					RenderVideo(visibleItem.Ptrs.Video);
					break;
				case AetItemType::None:
				default:
					break;
				}
			}
		}
		renderer.End();
	}

	void AetRenderWindow::ToolSelectionHeaderGui()
	{
		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(0.0f, 0.0f));
		Gui::PushStyleVar(ImGuiStyleVar_FramePadding, vec2(6.0f, 1.0f));

		for (int i = 0; i < AetToolType_Count; i++)
		{
			const AetTool& tool = *tools[i];
			const bool isSelected = (&tool == GetCurrentTool());

			Gui::PushStyleColor(ImGuiCol_Button, Gui::GetStyleColorVec4(isSelected ? ImGuiCol_Button : ImGuiCol_DockingEmptyBg));

			if (Gui::Button(tool.GetIcon()))
				currentToolType = static_cast<AetToolType>(i);

			if (Gui::IsItemHoveredDelayed())
			{
				const char* shortcutString = Input::GetKeyCodeName(tool.GetShortcutKey());
				char shortcutChar = shortcutString ? static_cast<char>(toupper(shortcutString[0])) : '?';

				Gui::WideSetTooltip("%s (%c)", tool.GetName(), shortcutChar);
			}

			Gui::PopStyleColor();
			Gui::SameLine();
		}

		Gui::PopStyleVar(2);
	}

	void AetRenderWindow::TooltipHeaderGui()
	{
		if (!selectedAetItem.IsNull() && selectedAetItem.Type() == AetItemType::Layer && selectedAetItem.GetLayerRef()->LayerVideo != nullptr)
		{
			// TODO: Tool specific widgets
			// TODO: TransformTool could have a origin / position preset selection box here
			// tool->DrawTooltipHeader();

			/*
			Gui::ExtendedVerticalSeparator();
			Gui::Text("  <%s> ", selectedAetItem.GetLayerRef()->GetName().c_str());
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

	void AetRenderWindow::RenderBackground()
	{
		checkerboardBaseGrid.GridSize = CheckerboardGrid::DefaultGridSize * 1.2f;
		checkerboardBaseGrid.Position = camera.Position / camera.Zoom;
		checkerboardBaseGrid.Size = vec2(renderTarget->Param.Resolution) / camera.Zoom;
		checkerboardBaseGrid.Render(renderer);

		const vec2 shadowOffset = vec2(3.5f, 2.5f) / camera.Zoom * 0.5f;
		const vec2 shadowMargin = vec2(2.5f) / camera.Zoom;
		const vec2 shadowPosition = checkerboardGrid.Position + shadowOffset;
		const vec2 shadowSize = aetRegionSize + shadowMargin;
		constexpr vec4 shadowColor(0.0f, 0.0f, 0.0f, 0.15f);
		renderer.Draw(Render::RenderCommand2D(shadowPosition, shadowSize, shadowColor));

		checkerboardGrid.GridSize = CheckerboardGrid::DefaultGridSize;
		checkerboardGrid.Size = aetRegionSize;
		checkerboardGrid.Render(renderer);
	}

	void AetRenderWindow::RenderAetSet(const AetSet* aetSet)
	{
	}

	void AetRenderWindow::RenderScene(const Scene* scene)
	{
	}

	void AetRenderWindow::RenderComposition(const Composition* comp)
	{
		objectCache.clear();
		Aet::Util::GetAddObjectsAt(objectCache, *comp, currentFrame);
		renderer.Aet().DrawObjCache(objectCache);
	}

	void AetRenderWindow::RenderLayer(const Layer* layer)
	{
		if (layer->ItemType != ItemType::Video && layer->ItemType != ItemType::Composition)
			return;

		objectCache.clear();
		Aet::Util::GetAddObjectsAt(objectCache, *layer, currentFrame);
		renderer.Aet().DrawObjCache(objectCache);
	}

	void AetRenderWindow::RenderVideo(const Video* video)
	{
		const int spriteIndex = glm::clamp(0, static_cast<int>(currentFrame), static_cast<int>(video->Sources.size()) - 1);
		renderer.Aet().DrawVideo(*video, spriteIndex, vec2(0.0f, 0.0f));
	}

	vec2 AetRenderWindow::GetLayerBoundingSize(const std::shared_ptr<Layer>& layer) const
	{
		const auto& video = layer->GetVideoItem();

		if (video != nullptr)
			return video->Size;

		// TODO: Find bounding box (?), or maybe just disallow using the transform tool (?)
		// NOTE: ~~Maybe this is sufficient already (?)~~
		return aetRegionSize;
	}

	bool AetRenderWindow::OnObjRender(const Aet::Util::Obj& obj, vec2 positionOffset, float opacity)
	{
		if (obj.Video == nullptr || !obj.IsVisible)
			return false;

		const auto* video = (previewData.Video != nullptr) ? previewData.Video : obj.Video;
		auto[tex, spr] = renderer.Aet().GetSprite(video, obj.SpriteFrame);

		const auto finalPosition = obj.Transform.Position + positionOffset;
		const auto finalOpacity = obj.Transform.Opacity * opacity;

		if (tex == nullptr || spr == nullptr)
			return false;

		const auto command = Render::RenderCommand2D(
			tex,
			obj.Transform.Origin,
			finalPosition,
			obj.Transform.Rotation,
			obj.Transform.Scale,
			spr->PixelRegion,
			(previewData.BlendMode != AetBlendMode::Unknown) ? previewData.BlendMode : obj.BlendMode,
			finalOpacity);

		renderer.Draw(command);
		return true;
	}

	bool AetRenderWindow::OnObjMaskRender(const Aet::Util::Obj& maskObj, const Aet::Util::Obj& obj, vec2 positionOffset, float opacity)
	{
		if (maskObj.Video == nullptr || obj.Video == nullptr || !obj.IsVisible)
			return false;

		const bool isSelected = (obj.SourceLayer == selectedAetItem.Ptrs.Layer);
		const bool isMaskSelected = (maskObj.SourceLayer == selectedAetItem.Ptrs.Layer);

		if (!isSelected && !isMaskSelected)
			return false;

		const auto* maskVideo = (isMaskSelected && previewData.Video != nullptr) ? previewData.Video : maskObj.Video;
		const auto* video = (isSelected && previewData.Video != nullptr) ? previewData.Video : obj.Video;

		auto[maskTex, maskSpr] = renderer.Aet().GetSprite(maskObj.Video, maskObj.SpriteFrame);
		auto[tex, spr] = renderer.Aet().GetSprite(obj.Video, obj.SpriteFrame);

		const auto finalOpacity = maskObj.Transform.Opacity * obj.Transform.Opacity * opacity;

		if (maskTex == nullptr || maskSpr == nullptr || tex == nullptr || spr == nullptr)
			return false;

		const auto command = Render::RenderCommand2D(
			tex,
			obj.Transform.Origin,
			obj.Transform.Position + positionOffset,
			obj.Transform.Rotation,
			obj.Transform.Scale,
			spr->PixelRegion,
			obj.BlendMode,
			finalOpacity);

		const auto maskCommand = Render::RenderCommand2D(
			maskTex,
			maskObj.Transform.Origin,
			maskObj.Transform.Position + positionOffset,
			maskObj.Transform.Rotation,
			maskObj.Transform.Scale,
			maskSpr->PixelRegion,
			(previewData.BlendMode != AetBlendMode::Unknown) ? previewData.BlendMode : maskObj.BlendMode,
			finalOpacity);

		renderer.Draw(command, maskCommand);
		return true;
	}
}
