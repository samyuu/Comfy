#include "AetTreeView.h"
#include "Editor/Aet/Command/Commands.h"
#include "IO/File.h"
#include "IO/Shell.h"
#include "Input/KeyCode.h"
#include "Misc/StringHelper.h"
#include "Core/Logger.h"

namespace Comfy::Editor
{
	using namespace Graphics;
	using namespace Graphics::Aet;

	constexpr ImGuiTreeNodeFlags SelectableTreeNodeFlags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
	constexpr ImGuiTreeNodeFlags HeaderTreeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | SelectableTreeNodeFlags;
	constexpr ImGuiTreeNodeFlags TreeNodeLeafFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

	AetTreeView::AetTreeView(AetCommandManager* commandManager, AetItemTypePtr* selectedAetItem, AetItemTypePtr* cameraSelectedAetItem)
		: IMutatingEditorComponent(commandManager), selectedAetItem(selectedAetItem), cameraSelectedAetItem(cameraSelectedAetItem)
	{
		assert(selectedAetItem);
		assert(cameraSelectedAetItem);
	}

	AetTreeView::~AetTreeView()
	{
	}

	void AetTreeView::Initialize()
	{
	}

	bool AetTreeView::DrawGui(const std::shared_ptr<AetSet>& aetSet)
	{
		treeViewWindow = Gui::GetCurrentWindow();

		if (aetSet == nullptr)
		{
			if (!scrollPositionStack.empty())
				scrollPositionStack = {};

			return false;
		}

		lastHoveredAetItem = hoveredAetItem;
		hoveredAetItem.Reset();

		// DEBUG: For developement only
		if (selectedAetItem->IsNull() && GetDebugLayerName() != nullptr)
			SetSelectedItems(aetSet->GetScenes().front()->FindLayer(GetDebugLayerName()));

		UpdateScrollButtonInput();

		Gui::PushStyleColor(ImGuiCol_Header, GetColor(EditorColor_TreeViewSelected));
		Gui::PushStyleColor(ImGuiCol_HeaderHovered, GetColor(EditorColor_TreeViewHovered));
		Gui::PushStyleColor(ImGuiCol_HeaderActive, GetColor(EditorColor_TreeViewActive));
		{
			DrawTreeViewBackground();
			DrawTreeNodeAetSet(aetSet);
		}
		Gui::PopStyleColor(3);

		return true;
	}

	void AetTreeView::DrawTreeViewBackground()
	{
		const ImU32 alternativeRowColow = GetColor(EditorColor_AltRow);

		const float itemSpacing = Gui::GetStyle().ItemSpacing.y;
		const float lineHeight = Gui::GetTextLineHeight() + itemSpacing;

		const ImGuiWindow* window = Gui::GetCurrentWindowRead();

		float scrollY = window->Scroll.y;
		const float scrolledOutLines = floorf(scrollY / lineHeight);
		scrollY -= lineHeight * scrolledOutLines;

		const vec2 clipRectMin = vec2(Gui::GetWindowPos().x, Gui::GetWindowPos().y);
		const vec2 clipRectMax = vec2(clipRectMin.x + Gui::GetWindowWidth(), clipRectMin.y + Gui::GetWindowHeight());

		const float yMin = clipRectMin.y - scrollY + Gui::GetCursorPosY();
		const float yMax = clipRectMax.y - scrollY + lineHeight;
		const float xMin = clipRectMin.x + window->Scroll.x + window->ContentsRegionRect.Min.x - window->Pos.x;
		const float xMax = clipRectMin.x + window->Scroll.x + window->ContentsRegionRect.Max.x - window->Pos.x;

		bool isOdd = fmod(scrolledOutLines, 2.0f) == 0.0f;
		for (float y = yMin; y < yMax; y += lineHeight)
		{
			if (y == yMin)
				y -= itemSpacing * 0.5f;

			if (isOdd ^= true)
				window->DrawList->AddRectFilled(vec2(xMin, y), vec2(xMax, y + lineHeight), alternativeRowColow);
		}
	}

	void AetTreeView::DrawTreeNodeAetSet(const std::shared_ptr<AetSet>& aetSet)
	{
		const bool aetSetNodeOpen = Gui::WideTreeNodeEx(aetSet.get(), HeaderTreeNodeFlags, "AetSet: %s", aetSet->Name.c_str());
		Gui::ItemContextMenu("AetSettAetContextMenu##AetTreeView", [this, &aetSet]()
		{
			Gui::Text("AetSet: %s", aetSet->Name.c_str());
			Gui::Separator();

			if (Gui::MenuItem("Save", nullptr, nullptr, false))
			{
				// TEMP: Don't wanna overwrite files during early development stage
				// aetSet->Save(sourceFilePath);
			}

			if (Gui::MenuItem("Save As..."))
			{
				std::string filePath;
				if (IO::Shell::CreateSaveFileDialog(filePath, "Save AetSet file", "dev_ram/aetset", { "AetSet (*.bin)", "*.bin", "All Files (*.*)", "*", }))
					IO::File::Save(filePath, *aetSet);
			}
		});

		if (aetSetNodeOpen)
		{
			if (Gui::IsItemClicked())
				SetSelectedItems(aetSet);

			for (auto& scene : aetSet->GetScenes())
				DrawTreeNodeAet(scene);

			Gui::TreePop();
		}
		else
		{
			ResetSelectedItems();
		}
	}

	void AetTreeView::DrawTreeNodeAet(const std::shared_ptr<Scene>& scene)
	{
		ImGuiTreeNodeFlags aetNodeFlags = HeaderTreeNodeFlags;
		if (scene.get() == selectedAetItem->Ptrs.Scene || scene.get() == lastHoveredAetItem.Ptrs.Scene)
			aetNodeFlags |= ImGuiTreeNodeFlags_Selected;

		const bool aetNodeOpen = Gui::WideTreeNodeEx(scene.get(), aetNodeFlags, "Aet: %s", scene->Name.c_str());

		if (Gui::IsItemClicked())
			SetSelectedItems(scene);

		if (aetNodeOpen)
		{
			if (Gui::WideTreeNodeEx(ICON_AETCOMPS "  Compositions", SelectableTreeNodeFlags | ImGuiTreeNodeFlags_DefaultOpen))
			{
				if (Gui::IsItemClicked())
					ResetSelectedItems();

				scene->RootComposition->GuiData.ThisIndex = -1;
				DrawTreeNodeComposition(scene, scene->RootComposition, true);

				for (i32 i = static_cast<i32>(scene->Compositions.size()) - 1; i >= 0; i--)
				{
					const auto& comp = scene->Compositions[i];

					comp->GuiData.ThisIndex = i;
					DrawTreeNodeComposition(scene, comp, false);
				}

				Gui::TreePop();
			}

			if (Gui::WideTreeNodeEx(ICON_AETVIDEOS "  Videos", SelectableTreeNodeFlags))
			{
				if (Gui::IsItemClicked())
					ResetSelectedItems();

				for (i32 i = 0; i < static_cast<i32>(scene->Videos.size()); i++)
				{
					DrawTreeNodeVideo(scene, scene->Videos[i], i);
				}
				Gui::TreePop();
			}

			Gui::TreePop();
		}
	}

	void AetTreeView::DrawTreeNodeComposition(const std::shared_ptr<Scene>& scene, const std::shared_ptr<Composition>& comp, bool isRoot)
	{
		Gui::PushID(comp.get());

		ImGuiTreeNodeFlags compNodeFlags = SelectableTreeNodeFlags;
		if (comp.get() == selectedAetItem->Ptrs.Composition)
			compNodeFlags |= ImGuiTreeNodeFlags_Selected;
		if (comp->GetLayers().size() < 1)
			compNodeFlags |= ImGuiTreeNodeFlags_Leaf;

		comp->GuiData.TreeViewScrollY = Gui::GetCursorPos().y;

		const vec2 treeNodeCursorPos = Gui::GetCursorScreenPos();

		Gui::SetNextTreeNodeOpen(comp->GuiData.TreeViewNodeOpen);
		comp->GuiData.TreeViewNodeOpen = Gui::WideTreeNodeEx("##AetCompNode", compNodeFlags);

		Gui::ItemContextMenu("AetCompContextMenu##AetTreeView", [&]()
		{
			DrawCompositionContextMenu(scene, comp, isRoot);
		});

		// TODO: Might want to check for mouse released instead (becomes more relevant once TreeNode drag and dropping is implemented)
		if (Gui::IsItemClicked())
			SetSelectedItems(comp);

		bool textHightlighted = false;
		if (!selectedAetItem->IsNull() && selectedAetItem->Type() == AetItemType::Layer)
			textHightlighted = (comp.get() == selectedAetItem->GetLayerRef()->GetCompItem().get());

		if (textHightlighted)
			Gui::PushStyleColor(ImGuiCol_Text, GetColor(EditorColor_TreeViewTextHighlight));

		// NOTE: Node label
		{
			const vec2 nodeLabelCursorPos = treeNodeCursorPos + vec2(GImGui->FontSize + GImGui->Style.FramePadding.x, 0.0f);
			constexpr vec2 iconLabelOffset = vec2(20.0f, 0.0f);

			const auto compName = comp->GetName();
			const char* compNameStart = compName.data();
			const char* compNameEnd = compNameStart + compName.size();

			// NOTE: Composition icon
			Gui::SetCursorScreenPos(nodeLabelCursorPos);
			Gui::TextUnformatted(comp->GuiData.TreeViewNodeOpen ? ICON_AETCOMP_OPEN : ICON_AETCOMP);

			// NOTE: Composition name
			Gui::SetCursorScreenPos(nodeLabelCursorPos + iconLabelOffset);
			Gui::TextUnformatted(compNameStart, compNameEnd);
		}

		if (textHightlighted)
			Gui::PopStyleColor();

		if (cameraSelectedAetItem->Ptrs.Composition == comp.get())
			DrawTreeNodeCameraIcon(treeNodeCursorPos);

		if (comp->GuiData.TreeViewNodeOpen)
		{
			for (auto& layer : comp->GetLayers())
				DrawTreeNodeLayer(scene, comp, layer);

			Gui::TreePop();
		}

		Gui::PopID();
	}

	void AetTreeView::DrawTreeNodeLayer(const std::shared_ptr<Scene>& scene, const std::shared_ptr<Composition>& comp, const std::shared_ptr<Layer>& layer)
	{
		Gui::PushID(layer.get());
		{
			DrawTreeNodeLayerCameraSelectableButton(comp, layer);
			DrawTreeNodeLayerActivityButton(layer);

			const bool isSelected = layer.get() == selectedAetItem->Ptrs.Layer || layer.get() == hoveredAetItem.Ptrs.Layer;
			const bool isCameraSelected = layer.get() == cameraSelectedAetItem->Ptrs.Layer;

			const vec2 treeNodeCursorPos = Gui::GetCursorScreenPos();

			if (Gui::Selectable("##LayerNode", isSelected) && !isCameraSelected)
				SetSelectedItems(layer, comp);

			Gui::ItemContextMenu("LayerContextMenu##AetInspector", [this, &comp, &layer]()
			{
				DrawLayerContextMenu(comp, layer);
			});

			// NOTE: Node label
			{
				constexpr vec2 iconLabelOffset = vec2(20.0f, 0.0f);

				const char* layerNameStart = layer->GetName().c_str();
				const char* layerNameEnd = layerNameStart + layer->GetName().size();

				// NOTE: Item icon
				Gui::SetCursorScreenPos(treeNodeCursorPos);
				Gui::TextUnformatted(GetItemTypeIcon(layer->ItemType));

				// NOTE: Composition name
				Gui::SetCursorScreenPos(treeNodeCursorPos + iconLabelOffset);
				Gui::TextUnformatted(layerNameStart, layerNameEnd);

				// NOTE: Texture mask indicator
				if (layer->LayerVideo != nullptr && layer->LayerVideo->TransferMode.TrackMatte != TrackMatte::NoTrackMatte)
				{
					Gui::SetCursorScreenPos(treeNodeCursorPos + iconLabelOffset + vec2(Gui::CalcTextSize(layerNameStart, layerNameEnd).x, 0.0f));
					Gui::TextUnformatted(textureMaskIndicator);
				}
			}

			if (cameraSelectedAetItem->Ptrs.Layer == layer.get())
				DrawTreeNodeCameraIcon(treeNodeCursorPos);

			if (layer->ItemType == ItemType::Composition && Gui::IsItemHoveredDelayed(ImGuiHoveredFlags_None, compPreviewTooltipHoverDelay) && layer->GetCompItem())
				DrawCompositionPreviewTooltip(layer->GetCompItem());

			layer->GuiData.TreeViewScrollY = Gui::GetCursorPos().y;

			if (layer->ItemType == ItemType::Composition && (Gui::IsItemHovered() || layer.get() == selectedAetItem->Ptrs.Layer))
				hoveredAetItem.SetItem(layer->GetCompItem());
		}
		Gui::PopID();
	}

	void AetTreeView::DrawTreeNodeLayerCameraSelectableButton(const std::shared_ptr<Composition>& comp, const std::shared_ptr<Layer>& layer)
	{
		// TODO: Does not work 100% correctly with all style settings but should be fine for now

		const vec2 cursorPos = Gui::GetCursorScreenPos();
		Gui::SetCursorScreenPos(vec2(GImGui->CurrentWindow->Pos.x + GImGui->Style.FramePadding.x - GImGui->CurrentWindow->Scroll.x, cursorPos.y - 1.0f));
		{
			bool isCameraSelected = layer.get() == cameraSelectedAetItem->Ptrs.Layer;

			const vec2 smallButtonPosition = Gui::GetCursorScreenPos();
			const vec2 smallButtonSize = vec2(cursorPos.x - smallButtonPosition.x, GImGui->FontSize + GImGui->Style.ItemSpacing.y);

			if (Gui::InvisibleButton(ICON_CAMERA, smallButtonSize))
			{
				if (isCameraSelected)
					SetSelectedItems(layer, comp);
				else
					SetSelectedItems(layer);
			}

			// TODO: It'd be nice to have some visual feedback for the hovered item inside the render window
			if (Gui::IsItemHovered())
				Gui::GetWindowDrawList()->AddText(smallButtonPosition + vec2(0.0f, 1.0f), Gui::GetColorU32(ImGuiCol_TextDisabled), ICON_CAMERA);
		}
		Gui::SetCursorScreenPos(cursorPos);
	}

	void AetTreeView::DrawTreeNodeLayerActivityButton(const std::shared_ptr<Layer>& layer)
	{
		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(3.0f, 0.0f));

		ImU32 activeButtonBackgroundColor = Gui::GetColorU32(ImGuiCol_ChildBg, 0.0f);
		Gui::PushStyleColor(ImGuiCol_Button, activeButtonBackgroundColor);
		Gui::PushStyleColor(ImGuiCol_ButtonHovered, activeButtonBackgroundColor);
		Gui::PushStyleColor(ImGuiCol_ButtonActive, activeButtonBackgroundColor);

		const vec2 smallButtonSize = vec2(26.0f, 0.0f);
		if (layer->ItemType == ItemType::Audio)
		{
			if (Gui::ComfySmallButton(layer->GetIsAudible() ? ICON_AUDIBLE : ICON_INAUDIBLE, smallButtonSize))
				ProcessUpdatingAetCommand(GetCommandManager(), LayerChangeFlagsAudible, layer, !layer->GetIsAudible());
		}
		else
		{
			if (Gui::ComfySmallButton(layer->GetIsVisible() ? ICON_VISIBLE : ICON_INVISIBLE, smallButtonSize))
				ProcessUpdatingAetCommand(GetCommandManager(), LayerChangeFlagsVisible, layer, !layer->GetIsVisible());
		}
		Gui::SameLine();

		Gui::PopStyleColor(3);
		Gui::PopStyleVar(1);
	}

	void AetTreeView::DrawTreeNodeVideo(const std::shared_ptr<Scene>& scene, const std::shared_ptr<Video>& video, i32 index)
	{
		Gui::PushID(video.get());

		bool isSelected = video.get() == selectedAetItem->Ptrs.Video;

		if (Gui::WideTreeNodeEx(FormatVideoNodeName(video, index), TreeNodeLeafFlags | (isSelected ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None)))
		{
			if (Gui::IsItemClicked())
				SetSelectedItems(video);
		}

		Gui::PopID();
	}

	bool AetTreeView::DrawCompositionContextMenu(const std::shared_ptr<Scene>& scene, const std::shared_ptr<Composition>& comp, bool isRoot)
	{
		const auto& compName = comp->GetName();
		if (isRoot)
			Gui::Text(ICON_AETCOMP "  %.*s", static_cast<int>(compName.size()), compName.data());
		else
			Gui::Text(ICON_AETCOMP "  %.*s (Comp %d)", static_cast<int>(compName.size()), compName.data(), comp->GuiData.ThisIndex);
		Gui::Separator();

		constexpr bool todoImplemented = false;
		if (todoImplemented && Gui::BeginMenu(ICON_ADD "  Add new Layer..."))
		{
			// TODO: Or maybe this should only be doable inside the timeline itself (?)
			if (Gui::MenuItem(ICON_AETITEMVIDEO "  Image")) {}
			if (Gui::MenuItem(ICON_AETITEMCOMP "  Comp")) {}
			if (Gui::MenuItem(ICON_AETITEMAUDIO "  Sound Effect")) {}
			Gui::EndMenu();
		}

		if (Gui::BeginMenu(ICON_FA_EXTERNAL_LINK_ALT "  Used by...", !isRoot))
		{
			compositionUsagesBuffer.clear();
			AetMgr::FindAddCompositionUsages(scene, comp, compositionUsagesBuffer);

			// NOTE: Count menu item
			Gui::Text("Usage Count: %zu", compositionUsagesBuffer.size());

			// NOTE: Usage menu items
			for (const auto& compUsingLayerPointer : compositionUsagesBuffer)
			{
				const std::shared_ptr<Layer>& compUsingLayer = *compUsingLayerPointer;

				sprintf_s(nodeNameFormatBuffer,
					ICON_AETCOMP "  %.*s,   %s  %s",
					static_cast<int>(compUsingLayer->GetParentComposition()->GetName().size()),
					compUsingLayer->GetParentComposition()->GetName().data(),
					GetItemTypeIcon(compUsingLayer->ItemType),
					compUsingLayer->GetName().c_str());

				if (Gui::MenuItem(nodeNameFormatBuffer))
				{
					SetSelectedItems(compUsingLayer);
					// NOTE: Scroll to parent first to open the node
					ScrollToGuiData(compUsingLayer->GetParentComposition()->GuiData);
					// BUG: The scroll y position won't yet have been set if the comp node was previously closed
					ScrollToGuiData(compUsingLayer->GuiData);
				}
			}

			Gui::EndMenu();
		}

		if (Gui::MenuItem(ICON_MOVEUP "  Move Up", nullptr, nullptr, !isRoot && todoImplemented)) {}
		if (Gui::MenuItem(ICON_MOVEDOWN "  Move Down", nullptr, nullptr, !isRoot && todoImplemented)) {}
		if (Gui::MenuItem(ICON_DELETE "  Delete Composition", nullptr, nullptr, !isRoot && todoImplemented)) {}

		return false;
	}

	bool AetTreeView::DrawLayerContextMenu(const std::shared_ptr<Composition>& comp, const std::shared_ptr<Layer>& layer)
	{
		Gui::Text("%s  %s", GetItemTypeIcon(layer->ItemType), layer->GetName().c_str());

		if (auto compItem = layer->GetCompItem(); compItem != nullptr)
		{
			if (Gui::MenuItem(ICON_FA_ARROW_RIGHT "  Jump to Composition"))
			{
				ScrollToGuiData(compItem->GuiData);
				// NOTE: Make it clear which comp was the jump target
				SetSelectedItems(layer);
			}
		}

		Gui::Separator();

		// TODO:
		constexpr bool todoImplemented = false;
		if (Gui::MenuItem(ICON_MOVEUP "  Move Up", nullptr, nullptr, todoImplemented)) {}
		if (Gui::MenuItem(ICON_MOVEDOWN "  Move Down", nullptr, nullptr, todoImplemented)) {}
		if (Gui::MenuItem(ICON_DELETE "  Delete Layer", nullptr, nullptr, todoImplemented)) {}

		return false;
	}

	void AetTreeView::DrawCompositionPreviewTooltip(const std::shared_ptr<Composition>& comp)
	{
		Gui::WideTooltip([this, &comp]()
		{
			Gui::Text(ICON_AETCOMP "  %s (Comp %d)", comp->GetName().data(), comp->GuiData.ThisIndex);
			Gui::Separator();

			int compIndex = 0;

			for (auto& layer : comp->GetLayers())
			{
				if (compIndex++ > compPreviewMaxConunt)
				{
					Gui::Text(ICON_FA_ELLIPSIS_H);
					break;
				}

				Gui::Text("%s  %s", GetItemTypeIcon(layer->ItemType), layer->GetName().c_str());
			}
		});
	}

	void AetTreeView::DrawTreeNodeCameraIcon(const vec2& treeNodeCursorPos) const
	{
		const vec2 textPosition = vec2(GImGui->CurrentWindow->Pos.x + GImGui->Style.FramePadding.x - GImGui->CurrentWindow->Scroll.x, treeNodeCursorPos.y);
		GImGui->CurrentWindow->DrawList->AddText(textPosition, Gui::GetColorU32(ImGuiCol_Text), ICON_CAMERA);
	}

	const char* AetTreeView::FormatVideoNodeName(const std::shared_ptr<Video>& video, i32 index)
	{
		if (video->Sources.size() >= 1)
		{
			if (video->Sources.size() > 1)
			{
				sprintf_s(nodeNameFormatBuffer, ICON_AETVIDEO "  %s - %s",
					video->GetFront()->Name.c_str(),
					video->GetBack()->Name.c_str());
			}
			else
			{
				sprintf_s(nodeNameFormatBuffer, ICON_AETVIDEO "  %s",
					video->GetFront()->Name.c_str());
			}
		}
		else
		{
			sprintf_s(nodeNameFormatBuffer, ICON_AETPLACEHOLDER "  Video %d (%dx%d)", index, video->Size.x, video->Size.y);
		}

		return nodeNameFormatBuffer;
	}

	void AetTreeView::UpdateScrollButtonInput()
	{
		if (!Gui::IsWindowFocused())
			return;

		if (Gui::IsKeyPressed(Input::KeyCode_Escape))
		{
			if (!selectedAetItem->IsNull())
			{
				switch (selectedAetItem->Type())
				{
				case AetItemType::Composition:
					ScrollToGuiData(selectedAetItem->GetCompositionRef()->GuiData);
					break;
				case AetItemType::Layer:
					ScrollToGuiData(selectedAetItem->GetLayerRef()->GuiData);
					break;
				default:
					break;
				}
			}
		}

		// NOTE: Mouse side button to jump to last scroll position
		if (Gui::IsMouseClicked(3))
		{
			if (!scrollPositionStack.empty())
			{
				Gui::SetScrollY(scrollPositionStack.top());
				scrollPositionStack.pop();
			}
		}
	}

	void AetTreeView::ScrollToGuiData(GuiExtraData& guiData)
	{
		scrollPositionStack.push(treeViewWindow->Scroll.y);

		treeViewWindow->ScrollTarget.y = guiData.TreeViewScrollY;
		treeViewWindow->ScrollTargetCenterRatio.y = scrollTargetCenterRatio;

		guiData.TreeViewNodeOpen = true;
	}

	const char* AetTreeView::GetDebugLayerName()
	{
		return nullptr;
	}
}
