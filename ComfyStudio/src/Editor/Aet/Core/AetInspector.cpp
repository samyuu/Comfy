#include "AetInspector.h"
#include "Editor/Aet/Command/Commands.h"
#include "Editor/Aet/AetIcons.h"
#include "Graphics/Auth2D/AetMgr.h"
#include "ImGui/Gui.h"

namespace Editor
{
	using namespace Graphics;

	namespace
	{
		void CopyStringIntoBuffer(const std::string& string, char* buffer, size_t bufferSize)
		{
			size_t copySize = std::min(string.size(), bufferSize - 1);
			string.copy(buffer, copySize);
			buffer[copySize] = '\0';
		}
	}

	constexpr ImGuiTreeNodeFlags DefaultOpenPropertiesNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow;

	AetInspector::AetInspector(AetCommandManager* commandManager, AetRenderPreviewData* previewData) 
		: IMutatingEditorComponent(commandManager), previewData(previewData)
	{
	}

	AetInspector::~AetInspector()
	{
	}

	void AetInspector::Initialize()
	{
	}

	bool AetInspector::DrawGui(const AetItemTypePtr& selected)
	{
		lastSelectedItem = selected;

		if (selected.Ptrs.VoidPointer == nullptr)
			return false;

		// NOTE: Safety clear so no invalid state stays around when deselecting a layer for example
		previewData->Surface = nullptr;
		previewData->BlendMode = AetBlendMode::Unknown;

		switch (selected.Type())
		{
		case AetItemType::AetSet:
			DrawInspectorAetSet(selected.GetAetSetRef());
			break;
		case AetItemType::Aet:
			DrawInspectorAet(selected.GetAetRef());
			break;
		case AetItemType::Composition:
			DrawInspectorComposition(selected.GetItemParentAet(), selected.GetAetCompositionRef());
			break;
		case AetItemType::Layer:
			DrawInspectorLayer(selected.GetItemParentAet(), selected.GetLayerRef());
			break;
		case AetItemType::Surface:
			DrawInspectorSurface(selected.GetItemParentAet(), selected.GetSurfaceRef());
			break;
		default:
			break;
		}

		return true;
	}

	void AetInspector::SetIsPlayback(bool value)
	{
		isPlayback = value;
	}

	float AetInspector::SetCurrentFrame(float value)
	{
		return currentFrame = value;
	}

	void AetInspector::DrawInspectorAetSet(const RefPtr<AetSet>& aetSet)
	{
		if (Gui::WideTreeNodeEx(ICON_NAMES "  Aets:", DefaultOpenPropertiesNodeFlags))
		{
			for (const RefPtr<Aet>& aet : *aetSet)
				Gui::BulletText(aet->Name.c_str());

			Gui::TreePop();
		}
	}

	void AetInspector::DrawInspectorAet(const RefPtr<Aet>& aet)
	{
		if (Gui::WideTreeNodeEx("Aet", DefaultOpenPropertiesNodeFlags))
		{
			PushDisableItemFlagIfPlayback();
			CopyStringIntoBuffer(aet->Name, aetNameBuffer, sizeof(aetNameBuffer));
			
			if (Gui::ComfyTextWidget("Name", aetNameBuffer, sizeof(aetNameBuffer)))
				ProcessUpdatingAetCommand(GetCommandManager(), AetChangeName, aet, aetNameBuffer);

			float startFrame = aet->StartFrame;
			if (Gui::ComfyFloatTextWidget("Start Frame", &startFrame, 1.0f, 10.0f, 0.0f, 0.0f, "%.2f"))
				ProcessUpdatingAetCommand(GetCommandManager(), AetChangeStartFrame, aet, startFrame);

			float endFrame = aet->EndFrame;
			if (Gui::ComfyFloatTextWidget("End Frame", &endFrame, 1.0f, 10.0f, 0.0f, 0.0f, "%.2f"))
				ProcessUpdatingAetCommand(GetCommandManager(), AetChangeEndFrame, aet, endFrame);

			float frameRate = aet->FrameRate;
			if (Gui::ComfyFloatTextWidget("Frame Rate", &frameRate, 1.0f, 10.0f, 0.0f, 0.0f, "%.2f"))
				ProcessUpdatingAetCommand(GetCommandManager(), AetChangeFrameRate, aet, glm::clamp(frameRate, 1.0f, 1000.0f));

			ivec2 resolution = aet->Resolution;
			if (Gui::ComfyInt2TextWidget("Resolution", glm::value_ptr(resolution)))
				ProcessUpdatingAetCommand(GetCommandManager(), AetChangeResolution, aet, resolution);

			vec4 color = Gui::ColorConvertU32ToFloat4(aet->BackgroundColor);
			if (Gui::ComfyColorEdit3("Background", glm::value_ptr(color), ImGuiColorEditFlags_DisplayHex))
				ProcessUpdatingAetCommand(GetCommandManager(), AetChangeBackgroundColor, aet, Gui::ColorConvertFloat4ToU32(color));

			PopDisableItemFlagIfPlayback();
			Gui::TreePop();
		}
		Gui::Separator();
	}

	void AetInspector::DrawInspectorComposition(Aet* aet, const RefPtr<AetComposition>& comp)
	{
		if (Gui::WideTreeNodeEx(ICON_AETCOMP "  Composition", DefaultOpenPropertiesNodeFlags))
		{
			PushDisableItemFlagIfPlayback();
			CopyStringIntoBuffer(comp->GetName(), compNameBuffer, sizeof(compNameBuffer));

			const bool isRoot = comp.get() == aet->GetRootComposition();
			if (Gui::ComfyTextWidget("Name", compNameBuffer, sizeof(compNameBuffer), isRoot ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_None))
				ProcessUpdatingAetCommand(GetCommandManager(), CompositionChangeName, comp, compNameBuffer);

			// NOTE: Readonly properties
			Gui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			{
				int thisIndex = comp->GuiData.ThisIndex;
				Gui::ComfyIntTextWidget("Index", &thisIndex, 0, 0, ImGuiInputTextFlags_ReadOnly);

				int layerCount = static_cast<int>(comp->size());
				Gui::ComfyIntTextWidget("Layer Count", &layerCount, 0, 0, ImGuiInputTextFlags_ReadOnly);
			}
			Gui::PopItemFlag();

			Gui::Separator();

			PopDisableItemFlagIfPlayback();
			Gui::TreePop();
		}
	}

	void AetInspector::DrawInspectorCompositionData(Aet* aet, const RefPtr<AetLayer>& layer, const RefPtr<AetComposition>& comp)
	{
		// TODO: In the future you should not be able to change the composition after creating it because it would leave the previous composition "nameless" (?)

		constexpr int availableCompNameBufferSize = static_cast<int>(sizeof(compDataNameBuffer) - 32);

		if (Gui::WideTreeNodeEx(ICON_AETCOMPS "  Comp Data", DefaultOpenPropertiesNodeFlags))
		{
			if (comp != nullptr)
				sprintf_s(compDataNameBuffer, "%.*s (Comp %d)", availableCompNameBufferSize, comp->GetName().c_str(), comp->GuiData.ThisIndex);

			PushDisableItemFlagIfPlayback();
			if (Gui::ComfyBeginCombo("Composition", comp == nullptr ? "None (Comp)" : compDataNameBuffer, ImGuiComboFlags_HeightLarge))
			{
				if (Gui::Selectable("None (Composition)", comp == nullptr))
					ProcessUpdatingAetCommand(GetCommandManager(), LayerChangeReferenceComposition, layer, nullptr);

				for (const RefPtr<AetComposition>& comp : aet->Compositions)
				{
					Gui::PushID(comp.get());

					bool isSelected = (comp == comp);
					sprintf_s(compDataNameBuffer, "%.*s (Comp %d)", availableCompNameBufferSize, comp->GetName().c_str(), comp->GuiData.ThisIndex);

					if (Gui::Selectable(compDataNameBuffer, isSelected))
						ProcessUpdatingAetCommand(GetCommandManager(), LayerChangeReferenceComposition, layer, comp);

					if (isSelected)
						Gui::SetItemDefaultFocus();
					Gui::PopID();
				}

				Gui::ComfyEndCombo();
			}

			PopDisableItemFlagIfPlayback();
			Gui::TreePop();
		}
	}

	void AetInspector::DrawInspectorLayer(Aet* aet, const RefPtr<AetLayer>& layer)
	{
		if (Gui::WideTreeNodeEx(layer.get(), DefaultOpenPropertiesNodeFlags, "%s  Layer", GetLayerTypeIcon(layer->Type)))
		{
			PushDisableItemFlagIfPlayback();
			CopyStringIntoBuffer(layer->GetName(), layerNameBuffer, sizeof(layerNameBuffer));
			if (Gui::ComfyTextWidget("Name", layerNameBuffer, sizeof(layerNameBuffer)))
				ProcessUpdatingAetCommand(GetCommandManager(), LayerChangeName, layer, layerNameBuffer);

			float startFrame = layer->StartFrame;
			if (Gui::ComfyFloatTextWidget("Start Frame", &startFrame, 1.0f, 10.0f, 0.0f, 0.0f, "%.2f"))
				ProcessUpdatingAetCommand(GetCommandManager(), LayerChangeStartFrame, layer, startFrame);

			float endFrame = layer->EndFrame;
			if (Gui::ComfyFloatTextWidget("End Frame", &endFrame, 1.0f, 10.0f, 0.0f, 0.0f, "%.2f"))
				ProcessUpdatingAetCommand(GetCommandManager(), LayerChangeEndFrame, layer, endFrame);

			float startOffset = layer->StartOffset;
			if (Gui::ComfyFloatTextWidget("Start Offset", &startOffset, 1.0f, 10.0f, 0.0f, 0.0f, "%.2f"))
				ProcessUpdatingAetCommand(GetCommandManager(), LayerChangeStartOffset, layer, startOffset);

			if (layer->Type != AetLayerType::Aif)
			{
				constexpr float percentageFactor = 100.0f;
				float playbackPercentage = layer->PlaybackSpeed * percentageFactor;

				if (Gui::ComfyFloatTextWidget("Playback Speed", &playbackPercentage, 1.0f, 10.0f, 0.0f, 0.0f, "%.0f%%"))
					ProcessUpdatingAetCommand(GetCommandManager(), LayerChangePlaybackSpeed, layer, playbackPercentage / percentageFactor);
			}

			PopDisableItemFlagIfPlayback();
			Gui::TreePop();
		}

		if (layer->Type == AetLayerType::Nop)
			return;

		if ((layer->Type == AetLayerType::Pic))
		{
			Gui::Separator();
			DrawInspectorSurfaceData(aet, layer, layer->GetReferencedSurface());
		}

		if ((layer->Type == AetLayerType::Eff))
		{
			Gui::Separator();
			DrawInspectorCompositionData(aet, layer, layer->GetReferencedComposition());
		}

		if ((layer->Type == AetLayerType::Pic || layer->Type == AetLayerType::Eff))
		{
			Gui::Separator();
			DrawInspectorAnimationData(layer->AnimationData, layer);

			// DEBUG: Quick debug view to inspect individual keyframes
			DrawInspectorDebugAnimationData(layer->AnimationData, layer);
		}

		Gui::Separator();
		DrawInspectorLayerMarkers(layer, &layer->Markers);

		if ((layer->Type == AetLayerType::Pic || layer->Type == AetLayerType::Eff))
		{
			Gui::Separator();
			DrawInspectorLayerParent(aet, layer);
		}

		Gui::Separator();
	}

	void AetInspector::DrawInspectorSurfaceData(Aet* aet, const RefPtr<AetLayer>& layer, const RefPtr<AetSurface>& surface)
	{
		if (Gui::WideTreeNodeEx(ICON_AETSURFACES "  Surface Data", DefaultOpenPropertiesNodeFlags))
		{
			if (surface != nullptr)
			{
				AetSpriteIdentifier* frontSprite = surface->GetFrontSprite();

				if (frontSprite == nullptr)
					sprintf_s(surfaceDataNameBuffer, "Null (%dx%d)", surface->Size.x, surface->Size.y);
				else
					CopyStringIntoBuffer(frontSprite->Name, surfaceDataNameBuffer, sizeof(surfaceDataNameBuffer));
			}

			const char* noSpriteString = "None (Sprite)";

			PushDisableItemFlagIfPlayback();
			if (Gui::ComfyBeginCombo("Sprite", surface == nullptr ? noSpriteString : surfaceDataNameBuffer, ImGuiComboFlags_HeightLarge))
			{
				if (Gui::Selectable(noSpriteString, surface == nullptr))
					ProcessUpdatingAetCommand(GetCommandManager(), LayerChangeReferenceSurface, layer, nullptr);

				int32_t surfaceIndex = 0;
				for (const RefPtr<AetSurface>& surface : aet->Surfaces)
				{
					Gui::PushID(surface.get());

					bool isSelected = (surface == surface);

					AetSpriteIdentifier* frontSprite = surface->GetFrontSprite();
					if (frontSprite == nullptr)
						sprintf_s(surfaceDataNameBuffer, "Surface %d (%dx%d)", surfaceIndex, surface->Size.x, surface->Size.y);

					if (Gui::Selectable(frontSprite == nullptr ? surfaceDataNameBuffer : frontSprite->Name.c_str(), isSelected))
						ProcessUpdatingAetCommand(GetCommandManager(), LayerChangeReferenceSurface, layer, surface);

					if (Gui::IsItemHovered())
						previewData->Surface = surface.get();

					if (isSelected)
						Gui::SetItemDefaultFocus();
					Gui::PopID();
					surfaceIndex++;
				}

				Gui::ComfyEndCombo();
			}

			PopDisableItemFlagIfPlayback();
			Gui::TreePop();
		}
	}

	void AetInspector::DrawInspectorAnimationData(const RefPtr<AetAnimationData>& animationData, const RefPtr<AetLayer>& layer)
	{
		if (Gui::WideTreeNodeEx(ICON_ANIMATIONDATA "  Animation Data", DefaultOpenPropertiesNodeFlags))
		{
			PushDisableItemFlagIfPlayback();

			if (animationData != nullptr)
			{
				static Properties currentProperties;
				AetMgr::Interpolate(animationData.get(), &currentProperties, currentFrame);

				animatedPropertyColor = GetColorVec4(EditorColor_AnimatedProperty);
				keyFramePropertyColor = GetColorVec4(EditorColor_KeyFrameProperty);
				staticPropertyColor = Gui::GetStyleColorVec4(ImGuiCol_FrameBg);

				DrawInspectorAnimationDataPropertyVec2(layer, "Origin", currentFrame, currentProperties.Origin, PropertyType_OriginX, PropertyType_OriginY);
				DrawInspectorAnimationDataPropertyVec2(layer, "Position", currentFrame, currentProperties.Position, PropertyType_PositionX, PropertyType_PositionY);
				DrawInspectorAnimationDataProperty(layer, "Rotation", currentFrame, currentProperties.Rotation, PropertyType_Rotation);
				DrawInspectorAnimationDataPropertyVec2(layer, "Scale", currentFrame, currentProperties.Scale, PropertyType_ScaleX, PropertyType_ScaleY);
				DrawInspectorAnimationDataProperty(layer, "Opacity", currentFrame, currentProperties.Opacity, PropertyType_Opacity);
			}

			if (layer->Type == AetLayerType::Pic)
			{
				if (animationData->UseTextureMask)
					Gui::PushItemFlag(ImGuiItemFlags_Disabled, true);

				if (Gui::ComfyBeginCombo("Blend Mode", AetAnimationData::GetBlendModeName(animationData->BlendMode), ImGuiComboFlags_HeightLarge))
				{
					// NOTE: Increase the count in case of invalid blend modes
					size_t blendModeCount = glm::max(static_cast<size_t>(animationData->BlendMode), AetAnimationData::BlendModeNames.size());

					for (int32_t blendModeIndex = 0; blendModeIndex < blendModeCount; blendModeIndex++)
					{
						bool isBlendMode = (static_cast<AetBlendMode>(blendModeIndex) == animationData->BlendMode);
						bool outOfBounds = blendModeIndex >= AetAnimationData::BlendModeNames.size();

						if (!isBlendMode && (outOfBounds || AetAnimationData::BlendModeNames[blendModeIndex] == nullptr))
							continue;

						const char* blendModeName = AetAnimationData::GetBlendModeName(static_cast<AetBlendMode>(blendModeIndex));

						if (Gui::Selectable(blendModeName, isBlendMode))
							ProcessUpdatingAetCommand(GetCommandManager(), AnimationDataChangeBlendMode, animationData, static_cast<AetBlendMode>(blendModeIndex));

						if (Gui::IsItemHovered())
							previewData->BlendMode = static_cast<AetBlendMode>(blendModeIndex);

						if (isBlendMode)
							Gui::SetItemDefaultFocus();
					};

					Gui::ComfyEndCombo();
				}

				if (animationData->UseTextureMask)
					Gui::PopItemFlag();

				bool useTextureMask = animationData->UseTextureMask;
				if (Gui::ComfyCheckbox("Use Texture Mask", &useTextureMask))
					ProcessUpdatingAetCommand(GetCommandManager(), AnimationDataChangeUseTextureMask, animationData, useTextureMask);
			}

			PopDisableItemFlagIfPlayback();
			Gui::TreePop();
		}
	}

	void AetInspector::DrawInspectorDebugAnimationData(const RefPtr<AetAnimationData>& animationData, const RefPtr<AetLayer>& layer)
	{
		// DEBUG:
		static bool showDebugView = false;
		
		if (Gui::GetIO().KeyCtrl && Gui::IsKeyPressedMap(ImGuiKey_Home, false))
			showDebugView ^= true;

		if (!showDebugView)
			return;

		Gui::Separator();
		if (Gui::WideTreeNodeEx(ICON_ANIMATIONDATA "  DEBUG Animation Data", DefaultOpenPropertiesNodeFlags))
		{
			if (animationData != nullptr)
			{
				int i = 0;
				for (auto& property : animationData->Properties)
				{
					if (Gui::WideTreeNode(AetKeyFrameProperties::PropertyNames.at(i++)))
					{
						for (auto& keyFrame : property)
							Gui::Text("Frame: %f; Value: %f; Curve: %f", keyFrame.Frame, keyFrame.Value, keyFrame.Curve);

						Gui::TreePop();
					}
				}
			}

			Gui::TreePop();
		}
	}

	void AetInspector::DrawInspectorAnimationDataProperty(const RefPtr<AetLayer>& layer, const char* label, frame_t frame, float& value, int propertyType)
	{
		constexpr float percentFactor = 100.0f;

		assert(layer->AnimationData.get() != nullptr);
		const auto& animationData = layer->AnimationData;

		AetKeyFrame* keyFrame = isPlayback ? nullptr : AetMgr::GetKeyFrameAt(animationData->Properties[propertyType], frame);

		Gui::PushStyleColor(ImGuiCol_FrameBg, ((animationData->Properties[propertyType].size() > 1))
			? (keyFrame != nullptr)
			? keyFramePropertyColor
			: animatedPropertyColor
			: staticPropertyColor);

		bool rotation = (propertyType == PropertyType_Rotation);
		bool opacity = (propertyType == PropertyType_Opacity);

		const char* formatString = rotation ? u8"%.2f Åã" : opacity ? "%.2f%%" : "%.2f";
		float min = 0.0f, max = 0.0f;

		if (opacity)
		{
			value *= percentFactor;
			max = 1.0f * percentFactor;
		}

		if (Gui::ComfyFloatTextWidget(label, &value, 1.0f, 10.0f, min, max, formatString, ImGuiInputTextFlags_None, (keyFrame == nullptr)))
		{
			if (opacity)
				value = glm::clamp(value * (1.0f / percentFactor), 0.0f, 1.0f);

			auto tuple = std::make_tuple(static_cast<PropertyType_Enum>(propertyType), frame, value);
			ProcessUpdatingAetCommand(GetCommandManager(), AnimationDataChangeKeyFrameValue, layer, tuple);
		}

		Gui::PopStyleColor();
	}

	void AetInspector::DrawInspectorAnimationDataPropertyVec2(const RefPtr<AetLayer>& Layer, const char* label, frame_t frame, vec2& value, int propertyTypeX, int propertyTypeY)
	{
		constexpr float percentFactor = 100.0f;

		assert(Layer->AnimationData.get() != nullptr);
		const auto& animationData = Layer->AnimationData;

		const AetKeyFrame* keyFrameX = isPlayback ? nullptr : AetMgr::GetKeyFrameAt(animationData->Properties[propertyTypeX], frame);
		const AetKeyFrame* keyFrameY = isPlayback ? nullptr : AetMgr::GetKeyFrameAt(animationData->Properties[propertyTypeY], frame);

		Gui::PushStyleColor(ImGuiCol_FrameBg, ((animationData->Properties[propertyTypeX].size() > 1) || (animationData->Properties[propertyTypeY].size() > 1))
			? (keyFrameX != nullptr || keyFrameY != nullptr)
			? keyFramePropertyColor
			: animatedPropertyColor
			: staticPropertyColor);

		const bool scale = (propertyTypeX == PropertyType_ScaleX) && (propertyTypeY == PropertyType_ScaleY);
		const char* formatString = scale ? "%.2f%%" : "%.2f";

		vec2 previousValue = value;
		if (scale)
			value *= percentFactor;

		bool disabledText[2] =
		{
			keyFrameX == nullptr,
			keyFrameY == nullptr,
		};

		if (Gui::ComfyFloat2TextWidget(label, glm::value_ptr(value), 1.0f, 0.0f, 0.0f, formatString, ImGuiInputTextFlags_None, disabledText))
		{
			if (scale)
				value *= (1.0f / percentFactor);

			// NOTE: Should this check for a value change or should this always be adding X and Y KeyFrames at once? 
			//		 The Transform and Move tool will already add two add once so this seems fine

			if (value.x != previousValue.x)
			{
				auto tuple = std::make_tuple(static_cast<PropertyType_Enum>(propertyTypeX), frame, value.x);
				ProcessUpdatingAetCommand(GetCommandManager(), AnimationDataChangeKeyFrameValue, Layer, tuple);
			}
			if (value.y != previousValue.y)
			{
				auto tuple = std::make_tuple(static_cast<PropertyType_Enum>(propertyTypeY), frame, value.y);
				ProcessUpdatingAetCommand(GetCommandManager(), AnimationDataChangeKeyFrameValue, Layer, tuple);
			}
		}

		Gui::PopStyleColor();
	}

	void AetInspector::DrawInspectorLayerMarkers(const RefPtr<AetLayer>& layer, std::vector<RefPtr<AetMarker>>* markers)
	{
		if (Gui::WideTreeNodeEx(ICON_MARKERS "  Markers", DefaultOpenPropertiesNodeFlags))
		{
			for (int i = 0; i < markers->size(); i++)
			{
				const RefPtr<AetMarker>& marker = markers->at(i);

				Gui::PushID(marker.get());

				vec2 treeNodeCursorPos = Gui::GetCursorScreenPos();
				bool open = Gui::WideTreeNode("##AetInspectorMarker");

				Gui::ItemContextMenu("AddMarkerContextMenu##AetInspector", [this, &layer, &markers, i]()
				{
					if (Gui::MenuItem(ICON_MOVEUP "  Move Up", nullptr, nullptr, i > 0))
					{
						auto tuple = std::tuple<int, int>(i, i - 1);
						ProcessUpdatingAetCommand(GetCommandManager(), LayerMoveMarker, layer, tuple);
					}

					if (Gui::MenuItem(ICON_MOVEDOWN "  Move Down", nullptr, nullptr, i < markers->size() - 1))
					{
						auto tuple = std::tuple<int, int>(i, i + 1);
						ProcessUpdatingAetCommand(GetCommandManager(), LayerMoveMarker, layer, tuple);
					}

					if (Gui::MenuItem(ICON_DELETE "  Delete"))
					{
						ProcessUpdatingAetCommand(GetCommandManager(), LayerDeleteMarker, layer, i);
					}
				});

				treeNodeCursorPos.x += GImGui->FontSize + GImGui->Style.FramePadding.x;
				Gui::SetCursorScreenPos(treeNodeCursorPos);
				Gui::Text("Frame: %.2f", marker->Frame);

				Gui::SetCursorScreenPos(treeNodeCursorPos + vec2(Gui::GetWindowWidth() * 0.35f, 0.0f));
				Gui::Text("%s", marker->Name.c_str());

				if (open)
				{
					PushDisableItemFlagIfPlayback();
					float frame = marker->Frame;
					if (Gui::ComfyFloatTextWidget("Frame", &frame, 1.0f, 10.0f, 0.0f, 0.0f, "%.2f"))
						ProcessUpdatingAetCommand(GetCommandManager(), LayerChangeMarkerFrame, marker, frame);

					CopyStringIntoBuffer(marker->Name, markerNameBuffer, sizeof(markerNameBuffer));
					if (Gui::ComfyTextWidget("Name", markerNameBuffer, sizeof(markerNameBuffer)))
						ProcessUpdatingAetCommand(GetCommandManager(), LayerChangeMarkerName, marker, markerNameBuffer);

					PopDisableItemFlagIfPlayback();
					Gui::TreePop();
				}
				Gui::PopID();
			}

			if (markers->size() > 0)
				Gui::Separator();

			PushDisableItemFlagIfPlayback();
			if (Gui::ComfyCenteredButton("Add Marker"))
			{
				char newMarkerNameBuffer[32];
				sprintf_s(newMarkerNameBuffer, "marker_%02zd", markers->size());
				auto newMarker = MakeRef<AetMarker>(0.0f, newMarkerNameBuffer);
				ProcessUpdatingAetCommand(GetCommandManager(), LayerAddMarker, layer, newMarker);
			}
			PopDisableItemFlagIfPlayback();

			Gui::TreePop();
		}
	}

	bool IsAnyParentRecursive(const AetLayer* newLayer, const AetLayer* layer)
	{
		// TODO: Recursively check parent and maybe move this function into the AetMgr
		return (newLayer->GetReferencedParentLayer() != nullptr && newLayer->GetReferencedParentLayer() == layer);
	}

	void AetInspector::DrawInspectorLayerParent(Aet* aet, const RefPtr<AetLayer>& layer)
	{
		if (Gui::WideTreeNodeEx(ICON_PARENT "  Parent", DefaultOpenPropertiesNodeFlags))
		{
			AetLayer* parentLayer = layer->GetReferencedParentLayer().get();
			AetComposition* parentComp = layer->GetParentComposition();

			constexpr const char* noParentString = "None (Parent)";
			PushDisableItemFlagIfPlayback();

			if (Gui::ComfyBeginCombo("Parent Layer", parentLayer == nullptr ? noParentString : parentLayer->GetName().c_str(), ImGuiComboFlags_HeightLarge))
			{
				if (Gui::Selectable(noParentString, parentLayer == nullptr))
					ProcessUpdatingAetCommand(GetCommandManager(), LayerChangeReferencedParentLayer, layer, nullptr);

				for (int32_t layerIndex = 0; layerIndex < parentComp->size(); layerIndex++)
				{
					const RefPtr<AetLayer>& iteratorLayer = parentComp->at(layerIndex);
					bool isSelected = (iteratorLayer.get() == parentLayer);

					bool isSame = (layer.get() == iteratorLayer.get());
					bool isAnyRecursive = IsAnyParentRecursive(iteratorLayer.get(), layer.get());
					
					Gui::PushID(iteratorLayer.get());
					if (Gui::Selectable(iteratorLayer->GetName().c_str(), isSelected, (isSame || isAnyRecursive) ? ImGuiSelectableFlags_Disabled : ImGuiSelectableFlags_None))
						ProcessUpdatingAetCommand(GetCommandManager(), LayerChangeReferencedParentLayer, layer, iteratorLayer);

					if (isSelected)
						Gui::SetItemDefaultFocus();
					Gui::PopID();
				}
				Gui::ComfyEndCombo();
			}

			PopDisableItemFlagIfPlayback();
			Gui::TreePop();
		}
	}

	void AetInspector::DrawInspectorSurface(Aet* aet, const RefPtr<AetSurface>& surface)
	{
		Gui::InputInt2("Dimensions", glm::value_ptr(surface->Size));

		ImVec4 color = Gui::ColorConvertU32ToFloat4(surface->Color);
		if (Gui::ColorEdit3("Background##AetSurfaceColor", (float*)&color, ImGuiColorEditFlags_DisplayHex))
			surface->Color = Gui::ColorConvertFloat4ToU32(color);

		if (Gui::WideTreeNodeEx("Sprites:", DefaultOpenPropertiesNodeFlags))
		{
			for (auto& sprite : surface->GetSprites())
			{
				sprintf_s(spriteNameBuffer, ICON_AETSURFACE "  %s", sprite.Name.c_str());
				Gui::Selectable(spriteNameBuffer);
			}
			Gui::TreePop();
		}
	}

	void AetInspector::PushDisableItemFlagIfPlayback()
	{
		if (isPlayback)
			Gui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	}

	void AetInspector::PopDisableItemFlagIfPlayback()
	{
		if (isPlayback)
			Gui::PopItemFlag();
	}
}