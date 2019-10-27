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

	AetInspector::AetInspector(AetCommandManager* commandManager) : IMutatingEditorComponent(commandManager)
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

		switch (selected.Type())
		{
		case AetItemType::AetSet:
			DrawInspectorAetSet(selected.GetAetSetRef());
			break;
		case AetItemType::Aet:
			DrawInspectorAet(selected.GetAetRef());
			break;
		case AetItemType::AetLayer:
			DrawInspectorAetLayer(selected.GetItemParentAet(), selected.GetAetLayerRef());
			break;
		case AetItemType::AetObj:
			DrawInspectorAetObj(selected.GetItemParentAet(), selected.GetAetObjRef());
			break;
		case AetItemType::AetRegion:
			DrawInspectorAetRegion(selected.GetItemParentAet(), selected.GetAetRegionRef());
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

			float frameStart = aet->FrameStart;
			if (Gui::ComfyFloatTextWidget("Start Frame", &frameStart, 1.0f, 10.0f, 0.0f, 0.0f, "%.2f"))
				ProcessUpdatingAetCommand(GetCommandManager(), AetChangeStartFrame, aet, frameStart);

			float frameDuration = aet->FrameDuration;
			if (Gui::ComfyFloatTextWidget("Duration", &frameDuration, 1.0f, 10.0f, 0.0f, 0.0f, "%.2f"))
				ProcessUpdatingAetCommand(GetCommandManager(), AetChangeFrameDuration, aet, frameDuration);

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

	void AetInspector::DrawInspectorAetLayer(Aet* aet, const RefPtr<AetLayer>& aetLayer)
	{
		if (Gui::WideTreeNodeEx(ICON_AETLAYER "  Layer", DefaultOpenPropertiesNodeFlags))
		{
			PushDisableItemFlagIfPlayback();
			CopyStringIntoBuffer(aetLayer->GetName(), layerNameBuffer, sizeof(layerNameBuffer));

			const bool isRoot = aetLayer.get() == aet->GetRootLayer();
			if (Gui::ComfyTextWidget("Name", layerNameBuffer, sizeof(layerNameBuffer), isRoot ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_None))
				ProcessUpdatingAetCommand(GetCommandManager(), AetLayerChangeName, aetLayer, layerNameBuffer);

			// NOTE: Readonly properties
			Gui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			{
				int thisIndex = aetLayer->GuiData.ThisIndex;
				Gui::ComfyIntTextWidget("Index", &thisIndex, 0, 0, ImGuiInputTextFlags_ReadOnly);

				int objectCount = static_cast<int>(aetLayer->size());
				Gui::ComfyIntTextWidget("Object Count", &objectCount, 0, 0, ImGuiInputTextFlags_ReadOnly);
			}
			Gui::PopItemFlag();

			Gui::Separator();

			PopDisableItemFlagIfPlayback();
			Gui::TreePop();
		}
	}

	void AetInspector::DrawInspectorLayerData(Aet* aet, const RefPtr<AetObj>& aetObj, const RefPtr<AetLayer>& aetLayer)
	{
		// TODO: In the future you should not be able to change the layer after creating it because it would leave the previous layer "nameless" (?)

		constexpr int availableLayerNameBufferSize = static_cast<int>(sizeof(layerDataNameBuffer) - 32);

		if (Gui::WideTreeNodeEx(ICON_AETLAYERS "  Layer Data", DefaultOpenPropertiesNodeFlags))
		{
			if (aetLayer != nullptr)
				sprintf_s(layerDataNameBuffer, "%.*s (Layer %d)", availableLayerNameBufferSize, aetLayer->GetName().c_str(), aetLayer->GuiData.ThisIndex);

			PushDisableItemFlagIfPlayback();
			if (Gui::ComfyBeginCombo("Layer", aetLayer == nullptr ? "None (Layer)" : layerDataNameBuffer, ImGuiComboFlags_HeightLarge))
			{
				if (Gui::Selectable("None (Layer)", aetLayer == nullptr))
					ProcessUpdatingAetCommand(GetCommandManager(), AetObjChangeReferenceLayer, aetObj, nullptr);

				for (const RefPtr<AetLayer>& layer : aet->Layers)
				{
					Gui::PushID(layer.get());

					bool isSelected = (aetLayer == layer);
					sprintf_s(layerDataNameBuffer, "%.*s (Layer %d)", availableLayerNameBufferSize, layer->GetName().c_str(), layer->GuiData.ThisIndex);

					if (Gui::Selectable(layerDataNameBuffer, isSelected))
						ProcessUpdatingAetCommand(GetCommandManager(), AetObjChangeReferenceLayer, aetObj, layer);

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

	void AetInspector::DrawInspectorAetObj(Aet* aet, const RefPtr<AetObj>& aetObj)
	{
		if (Gui::WideTreeNodeEx(aetObj.get(), DefaultOpenPropertiesNodeFlags, "%s  Object", GetObjTypeIcon(aetObj->Type)))
		{
			PushDisableItemFlagIfPlayback();
			CopyStringIntoBuffer(aetObj->GetName(), aetObjNameBuffer, sizeof(aetObjNameBuffer));
			if (Gui::ComfyTextWidget("Name", aetObjNameBuffer, sizeof(aetObjNameBuffer)))
				ProcessUpdatingAetCommand(GetCommandManager(), AetObjChangeName, aetObj, aetObjNameBuffer);

			float loopStart = aetObj->LoopStart;
			if (Gui::ComfyFloatTextWidget("Loop Start", &loopStart, 1.0f, 10.0f, 0.0f, 0.0f, "%.2f"))
				ProcessUpdatingAetCommand(GetCommandManager(), AetObjChangeLoopStart, aetObj, loopStart);

			float loopEnd = aetObj->LoopEnd;
			if (Gui::ComfyFloatTextWidget("Loop End", &loopEnd, 1.0f, 10.0f, 0.0f, 0.0f, "%.2f"))
				ProcessUpdatingAetCommand(GetCommandManager(), AetObjChangeLoopEnd, aetObj, loopEnd);

			float startOffset = aetObj->StartOffset;
			if (Gui::ComfyFloatTextWidget("Start Offset", &startOffset, 1.0f, 10.0f, 0.0f, 0.0f, "%.2f"))
				ProcessUpdatingAetCommand(GetCommandManager(), AetObjChangeStartOffset, aetObj, startOffset);

			if (aetObj->Type != AetObjType::Aif)
			{
				constexpr float percentageFactor = 100.0f;
				float playbackPercentage = aetObj->PlaybackSpeed * percentageFactor;

				if (Gui::ComfyFloatTextWidget("Playback Speed", &playbackPercentage, 1.0f, 10.0f, 0.0f, 0.0f, "%.0f%%"))
					ProcessUpdatingAetCommand(GetCommandManager(), AetObjChangePlaybackSpeed, aetObj, playbackPercentage / percentageFactor);
			}

			PopDisableItemFlagIfPlayback();
			Gui::TreePop();
		}

		if (aetObj->Type == AetObjType::Nop)
			return;

		if ((aetObj->Type == AetObjType::Pic))
		{
			Gui::Separator();
			DrawInspectorRegionData(aet, aetObj, aetObj->GetReferencedRegion());
		}

		if ((aetObj->Type == AetObjType::Eff))
		{
			Gui::Separator();
			DrawInspectorLayerData(aet, aetObj, aetObj->GetReferencedLayer());
		}

		if ((aetObj->Type == AetObjType::Pic || aetObj->Type == AetObjType::Eff))
		{
			Gui::Separator();
			DrawInspectorAnimationData(aetObj->AnimationData, aetObj);

			// DEBUG: Quick debug view to inspect individual keyframes
			DrawInspectorDebugAnimationData(aetObj->AnimationData, aetObj);
		}

		Gui::Separator();
		DrawInspectorAetObjMarkers(aetObj, &aetObj->Markers);

		if ((aetObj->Type == AetObjType::Pic || aetObj->Type == AetObjType::Eff))
		{
			Gui::Separator();
			DrawInspectorAetObjParent(aet, aetObj);
		}

		Gui::Separator();
	}

	void AetInspector::DrawInspectorRegionData(Aet* aet, const RefPtr<AetObj>& aetObj, const RefPtr<AetRegion>& aetRegion)
	{
		if (Gui::WideTreeNodeEx(ICON_AETREGIONS "  Region Data", DefaultOpenPropertiesNodeFlags))
		{
			if (aetRegion != nullptr)
			{
				AetSpriteIdentifier* frontSprite = aetRegion->GetFrontSprite();

				if (frontSprite == nullptr)
					sprintf_s(regionDataNameBuffer, "Null (%dx%d)", aetRegion->Size.x, aetRegion->Size.y);
				else
					CopyStringIntoBuffer(frontSprite->Name, regionDataNameBuffer, sizeof(regionDataNameBuffer));
			}

			const char* noSpriteString = "None (Sprite)";

			PushDisableItemFlagIfPlayback();
			if (Gui::ComfyBeginCombo("Sprite", aetRegion == nullptr ? noSpriteString : regionDataNameBuffer, ImGuiComboFlags_HeightLarge))
			{
				if (Gui::Selectable(noSpriteString, aetRegion == nullptr))
					ProcessUpdatingAetCommand(GetCommandManager(), AetObjChangeReferenceRegion, aetObj, nullptr);

				int32_t regionIndex = 0;
				for (const RefPtr<AetRegion>& region : aet->Regions)
				{
					Gui::PushID(region.get());

					bool isSelected = (aetRegion == region);

					AetSpriteIdentifier* frontSprite = region->GetFrontSprite();
					if (frontSprite == nullptr)
						sprintf_s(regionDataNameBuffer, "Region %d (%dx%d)", regionIndex, region->Size.x, region->Size.y);

					if (Gui::Selectable(frontSprite == nullptr ? regionDataNameBuffer : frontSprite->Name.c_str(), isSelected))
						ProcessUpdatingAetCommand(GetCommandManager(), AetObjChangeReferenceRegion, aetObj, region);

					if (isSelected)
						Gui::SetItemDefaultFocus();
					Gui::PopID();
					regionIndex++;
				}

				Gui::ComfyEndCombo();
			}

			PopDisableItemFlagIfPlayback();
			Gui::TreePop();
		}
	}

	void AetInspector::DrawInspectorAnimationData(const RefPtr<AnimationData>& animationData, const RefPtr<AetObj>& aetObj)
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

				DrawInspectorAnimationDataPropertyVec2(aetObj, "Origin", currentFrame, currentProperties.Origin, PropertyType_OriginX, PropertyType_OriginY);
				DrawInspectorAnimationDataPropertyVec2(aetObj, "Position", currentFrame, currentProperties.Position, PropertyType_PositionX, PropertyType_PositionY);
				DrawInspectorAnimationDataProperty(aetObj, "Rotation", currentFrame, currentProperties.Rotation, PropertyType_Rotation);
				DrawInspectorAnimationDataPropertyVec2(aetObj, "Scale", currentFrame, currentProperties.Scale, PropertyType_ScaleX, PropertyType_ScaleY);
				DrawInspectorAnimationDataProperty(aetObj, "Opacity", currentFrame, currentProperties.Opacity, PropertyType_Opacity);
			}

			if (aetObj->Type == AetObjType::Pic)
			{
				if (animationData->UseTextureMask)
					Gui::PushItemFlag(ImGuiItemFlags_Disabled, true);

				if (Gui::ComfyBeginCombo("Blend Mode", AnimationData::GetBlendModeName(animationData->BlendMode), ImGuiComboFlags_HeightLarge))
				{
					// NOTE: Increase the count in case of invalid blend modes
					size_t blendModeCount = glm::max(static_cast<size_t>(animationData->BlendMode), AnimationData::BlendModeNames.size());

					for (int32_t blendModeIndex = 0; blendModeIndex < blendModeCount; blendModeIndex++)
					{
						bool isBlendMode = (static_cast<AetBlendMode>(blendModeIndex) == animationData->BlendMode);
						bool outOfBounds = blendModeIndex >= AnimationData::BlendModeNames.size();

						if (!isBlendMode && (outOfBounds || AnimationData::BlendModeNames[blendModeIndex] == nullptr))
							continue;

						const char* blendModeName = AnimationData::GetBlendModeName(static_cast<AetBlendMode>(blendModeIndex));

						if (Gui::Selectable(blendModeName, isBlendMode))
							ProcessUpdatingAetCommand(GetCommandManager(), AnimationDataChangeBlendMode, animationData, static_cast<AetBlendMode>(blendModeIndex));

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

	void AetInspector::DrawInspectorDebugAnimationData(const RefPtr<AnimationData>& animationData, const RefPtr<AetObj>& aetObj)
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
					if (Gui::WideTreeNode(KeyFrameProperties::PropertyNames.at(i++)))
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

	void AetInspector::DrawInspectorAnimationDataProperty(const RefPtr<AetObj>& aetObj, const char* label, frame_t frame, float& value, int propertyType)
	{
		constexpr float percentFactor = 100.0f;

		assert(aetObj->AnimationData.get() != nullptr);
		const auto& animationData = aetObj->AnimationData;

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
			ProcessUpdatingAetCommand(GetCommandManager(), AnimationDataChangeKeyFrameValue, aetObj, tuple);
		}

		Gui::PopStyleColor();
	}

	void AetInspector::DrawInspectorAnimationDataPropertyVec2(const RefPtr<AetObj>& aetObj, const char* label, frame_t frame, vec2& value, int propertyTypeX, int propertyTypeY)
	{
		constexpr float percentFactor = 100.0f;

		assert(aetObj->AnimationData.get() != nullptr);
		const auto& animationData = aetObj->AnimationData;

		AetKeyFrame* keyFrameX = isPlayback ? nullptr : AetMgr::GetKeyFrameAt(animationData->Properties[propertyTypeX], frame);
		AetKeyFrame* keyFrameY = isPlayback ? nullptr : AetMgr::GetKeyFrameAt(animationData->Properties[propertyTypeY], frame);

		Gui::PushStyleColor(ImGuiCol_FrameBg, ((animationData->Properties[propertyTypeX].size() > 1) || (animationData->Properties[propertyTypeY].size() > 1))
			? (keyFrameX != nullptr || keyFrameY != nullptr)
			? keyFramePropertyColor
			: animatedPropertyColor
			: staticPropertyColor);

		bool scale = (propertyTypeX == PropertyType_ScaleX) && (propertyTypeY == PropertyType_ScaleY);
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
				ProcessUpdatingAetCommand(GetCommandManager(), AnimationDataChangeKeyFrameValue, aetObj, tuple);
			}
			if (value.y != previousValue.y)
			{
				auto tuple = std::make_tuple(static_cast<PropertyType_Enum>(propertyTypeY), frame, value.y);
				ProcessUpdatingAetCommand(GetCommandManager(), AnimationDataChangeKeyFrameValue, aetObj, tuple);
			}
		}

		Gui::PopStyleColor();
	}

	void AetInspector::DrawInspectorAetObjMarkers(const RefPtr<AetObj>& aetObj, std::vector<RefPtr<AetMarker>>* markers)
	{
		if (Gui::WideTreeNodeEx(ICON_MARKERS "  Markers", DefaultOpenPropertiesNodeFlags))
		{
			for (int i = 0; i < markers->size(); i++)
			{
				const RefPtr<AetMarker>& marker = markers->at(i);

				Gui::PushID(marker.get());

				vec2 treeNodeCursorPos = Gui::GetCursorScreenPos();
				bool open = Gui::WideTreeNode("##AetInspectorMarker");

				Gui::ItemContextMenu("AddMarkerContextMenu##AetInspector", [this, &aetObj, &markers, i]()
				{
					if (Gui::MenuItem(ICON_MOVEUP "  Move Up", nullptr, nullptr, i > 0))
					{
						auto tuple = std::tuple<int, int>(i, i - 1);
						ProcessUpdatingAetCommand(GetCommandManager(), AetObjMoveMarker, aetObj, tuple);
					}

					if (Gui::MenuItem(ICON_MOVEDOWN "  Move Down", nullptr, nullptr, i < markers->size() - 1))
					{
						auto tuple = std::tuple<int, int>(i, i + 1);
						ProcessUpdatingAetCommand(GetCommandManager(), AetObjMoveMarker, aetObj, tuple);
					}

					if (Gui::MenuItem(ICON_DELETE "  Delete"))
					{
						ProcessUpdatingAetCommand(GetCommandManager(), AetObjDeleteMarker, aetObj, i);
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
						ProcessUpdatingAetCommand(GetCommandManager(), AetObjChangeMarkerFrame, marker, frame);

					CopyStringIntoBuffer(marker->Name, markerNameBuffer, sizeof(markerNameBuffer));
					if (Gui::ComfyTextWidget("Name", markerNameBuffer, sizeof(markerNameBuffer)))
						ProcessUpdatingAetCommand(GetCommandManager(), AetObjChangeMarkerName, marker, markerNameBuffer);

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
				ProcessUpdatingAetCommand(GetCommandManager(), AetObjAddMarker, aetObj, newMarker);
			}
			PopDisableItemFlagIfPlayback();

			Gui::TreePop();
		}
	}

	bool IsAnyParentRecursive(const AetObj* newObj, const AetObj* aetObj)
	{
		// TODO: Recursively check parent and maybe move this function into the AetMgr
		return (newObj->GetReferencedParentObj() != nullptr && newObj->GetReferencedParentObj() == aetObj);
	}

	void AetInspector::DrawInspectorAetObjParent(Aet* aet, const RefPtr<AetObj>& aetObj)
	{
		if (Gui::WideTreeNodeEx(ICON_PARENT "  Parent", DefaultOpenPropertiesNodeFlags))
		{
			AetObj* parentObj = aetObj->GetReferencedParentObj().get();
			AetLayer* parentLayer = aetObj->GetParentLayer();

			constexpr const char* noParentObjString = "None (Parent)";
			PushDisableItemFlagIfPlayback();

			if (Gui::ComfyBeginCombo("Parent Object", parentObj == nullptr ? noParentObjString : parentObj->GetName().c_str(), ImGuiComboFlags_HeightLarge))
			{
				if (Gui::Selectable(noParentObjString, parentObj == nullptr))
					ProcessUpdatingAetCommand(GetCommandManager(), AetObjChangeObjReferenceParent, aetObj, nullptr);

				for (int32_t objIndex = 0; objIndex < parentLayer->size(); objIndex++)
				{
					const RefPtr<AetObj>& obj = parentLayer->at(objIndex);
					bool isSelected = (obj.get() == parentObj);

					bool isSame = (aetObj.get() == obj.get());
					bool isAnyRecursive = IsAnyParentRecursive(obj.get(), aetObj.get());
					
					Gui::PushID(obj.get());
					if (Gui::Selectable(obj->GetName().c_str(), isSelected, (isSame || isAnyRecursive) ? ImGuiSelectableFlags_Disabled : ImGuiSelectableFlags_None))
						ProcessUpdatingAetCommand(GetCommandManager(), AetObjChangeObjReferenceParent, aetObj, obj);

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

	void AetInspector::DrawInspectorAetRegion(Aet* aet, const RefPtr<AetRegion>& aetRegion)
	{
		Gui::InputInt2("Dimensions", glm::value_ptr(aetRegion->Size));

		ImVec4 color = Gui::ColorConvertU32ToFloat4(aetRegion->Color);
		if (Gui::ColorEdit3("Background##AetRegionColor", (float*)&color, ImGuiColorEditFlags_DisplayHex))
			aetRegion->Color = Gui::ColorConvertFloat4ToU32(color);

		if (Gui::WideTreeNodeEx("Sprites:", DefaultOpenPropertiesNodeFlags))
		{
			for (auto& sprite : aetRegion->GetSprites())
			{
				sprintf_s(spriteNameBuffer, ICON_AETREGION "  %s", sprite.Name.c_str());
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