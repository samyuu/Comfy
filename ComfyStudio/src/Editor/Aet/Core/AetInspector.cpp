#include "AetInspector.h"
#include "Editor/Aet/Command/Commands.h"
#include "Editor/Aet/AetIcons.h"
#include "Graphics/Auth2D/AetMgr.h"
#include "ImGui/Gui.h"

namespace Editor
{
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
		if (lastSelectedItem.Ptrs.VoidPointer != selected.Ptrs.VoidPointer)
			newParentObjLayerIndex = -1;

		lastSelectedItem = selected;

		if (selected.Ptrs.VoidPointer == nullptr)
			return false;

		Aet* parentAet = selected.GetItemParentAet();

		switch (selected.Type())
		{
		case AetItemType::AetSet:
			DrawInspectorAetSet(selected.GetAetSetRef());
			break;
		case AetItemType::Aet:
			DrawInspectorAet(selected.GetAetRef());
			break;
		case AetItemType::AetLayer:
			DrawInspectorAetLayer(parentAet, selected.GetAetLayerRef());
			break;
		case AetItemType::AetObj:
			DrawInspectorAetObj(parentAet, selected.GetAetObjRef());
			break;
		case AetItemType::AetRegion:
			DrawInspectorAetRegion(parentAet, selected.GetAetRegionRef());
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
		if (Gui::WideTreeNodeEx(ICON_NAMES "  Aets:", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (const RefPtr<Aet>& aet : *aetSet)
				Gui::BulletText(aet->Name.c_str());

			Gui::TreePop();
		}
	}

	void AetInspector::DrawInspectorAet(const RefPtr<Aet>& aet)
	{
		if (Gui::WideTreeNodeEx("Aet", ImGuiTreeNodeFlags_DefaultOpen))
		{
			strcpy_s(aetNameBuffer, aet->Name.c_str());

			if (Gui::ComfyTextWidget("Name", aetNameBuffer, sizeof(aetNameBuffer)))
				ProcessUpdatingAetCommand(GetCommandManager(), AetChangeName, aet, aetNameBuffer);

			float frameStart = aet->FrameStart;
			if (Gui::ComfyFloatWidget("Start Frame", &frameStart, 1.0f, 10.0f, "%.2f"))
				ProcessUpdatingAetCommand(GetCommandManager(), AetChangeStartFrame, aet, frameStart);

			float frameDuration = aet->FrameDuration;
			if (Gui::ComfyFloatWidget("Duration", &frameDuration, 1.0f, 10.0f, "%.2f"))
				ProcessUpdatingAetCommand(GetCommandManager(), AetChangeFrameDuration, aet, frameDuration);

			float frameRate = aet->FrameRate;
			if (Gui::ComfyFloatWidget("Frame Rate", &frameRate, 1.0f, 10.0f, "%.2f"))
				ProcessUpdatingAetCommand(GetCommandManager(), AetChangeFrameRate, aet, glm::clamp(frameRate, 1.0f, 1000.0f));

			ivec2 resolution = aet->Resolution;
			if (Gui::ComfyInt2Widget("Resolution", glm::value_ptr(resolution)))
				ProcessUpdatingAetCommand(GetCommandManager(), AetChangeResolution, aet, resolution);

			vec4 color = Gui::ColorConvertU32ToFloat4(aet->BackgroundColor);
			if (Gui::ComfyColorEdit3("Background", glm::value_ptr(color), ImGuiColorEditFlags_DisplayHex))
				ProcessUpdatingAetCommand(GetCommandManager(), AetChangeBackgroundColor, aet, Gui::ColorConvertFloat4ToU32(color));

			Gui::TreePop();
		}
		Gui::Separator();
	}

	void AetInspector::DrawInspectorAetLayer(Aet* aet, const RefPtr<AetLayer>& aetLayer)
	{
		if (Gui::WideTreeNodeEx(ICON_NAMES "  Given Names:", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (auto& name : aetLayer->GetGivenNames())
				Gui::BulletText(name.c_str());

			Gui::TreePop();
		}

		if (Gui::WideTreeNodeEx(ICON_AETLAYER "  Objects:", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (const RefPtr<AetObj>& aetObj : *aetLayer)
				Gui::BulletText("%s  %s", GetObjTypeIcon(aetObj->Type), aetObj->GetName().c_str());

			Gui::TreePop();
		}
	}

	void AetInspector::DrawInspectorLayerData(Aet* aet, const RefPtr<AetObj>& aetObj, const RefPtr<AetLayer>& aetLayer)
	{
		// TODO: In the future you should not be able to change the layer after creating it because it would leave the previous layer "nameless" (?)

		if (Gui::WideTreeNodeEx(ICON_AETLAYERS "  Layer Data", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (aetLayer != nullptr)
				sprintf_s(layerDataNameBuffer, "Layer %d (%s)", aetLayer->GuiData.ThisIndex, aetLayer->GetCommaSeparatedNames().c_str());

			if (Gui::ComfyBeginCombo("Layer", aetLayer == nullptr ? "None (Layer)" : layerDataNameBuffer, ImGuiComboFlags_HeightLarge))
			{
				if (Gui::Selectable("None (Layer)", aetLayer == nullptr))
					ProcessUpdatingAetCommand(GetCommandManager(), AetObjChangeReferenceLayer, aetObj, nullptr);

				int layerIndex = 0;
				for (const RefPtr<AetLayer>& layer : aet->AetLayers)
				{
					if (aet->AetLayers.back() == layer)
						break;

					Gui::PushID(layer.get());

					bool isSelected = (aetLayer == layer);
					sprintf_s(layerDataNameBuffer, "Layer %d (%s)", layerIndex++, layer->GetCommaSeparatedNames().c_str());

					if (Gui::Selectable(layerDataNameBuffer, isSelected))
						ProcessUpdatingAetCommand(GetCommandManager(), AetObjChangeReferenceLayer, aetObj, layer);

					if (isSelected)
						Gui::SetItemDefaultFocus();
					Gui::PopID();
				}

				Gui::ComfyEndCombo();
			}
			Gui::TreePop();
		}
	}

	void AetInspector::DrawInspectorAetObj(Aet* aet, const RefPtr<AetObj>& aetObj)
	{
		if (Gui::WideTreeNodeEx(aetObj.get(), ImGuiTreeNodeFlags_DefaultOpen, "%s  Object", GetObjTypeIcon(aetObj->Type)))
		{
			strcpy_s(aetObjNameBuffer, aetObj->GetName().c_str());
			if (Gui::ComfyTextWidget("Name", aetObjNameBuffer, sizeof(aetObjNameBuffer)))
				ProcessUpdatingAetCommand(GetCommandManager(), AetObjChangeName, aetObj, aetObjNameBuffer);

			float loopStart = aetObj->LoopStart;
			if (Gui::ComfyFloatWidget("Loop Start", &loopStart, 1.0f, 10.0f, "%.2f"))
				ProcessUpdatingAetCommand(GetCommandManager(), AetObjChangeLoopStart, aetObj, loopStart);

			float loopEnd = aetObj->LoopEnd;
			if (Gui::ComfyFloatWidget("Loop End", &loopEnd, 1.0f, 10.0f, "%.2f"))
				ProcessUpdatingAetCommand(GetCommandManager(), AetObjChangeLoopEnd, aetObj, loopEnd);

			float startFrame = aetObj->StartFrame;
			if (Gui::ComfyFloatWidget("Start Frame", &startFrame, 1.0f, 10.0f, "%.2f"))
				ProcessUpdatingAetCommand(GetCommandManager(), AetObjChangeStartFrame, aetObj, startFrame);

			if (aetObj->Type != AetObjType::Aif)
			{
				constexpr float percentageFactor = 100.0f;
				float playbackPercentage = aetObj->PlaybackSpeed * percentageFactor;

				if (Gui::ComfyFloatWidget("Playback Speed", &playbackPercentage, 10.0f, 100.0f, "%.0f%%"))
					ProcessUpdatingAetCommand(GetCommandManager(), AetObjChangePlaybackSpeed, aetObj, playbackPercentage / percentageFactor);
			}

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

			// TODO: Temp remove
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
		if (Gui::WideTreeNodeEx(ICON_AETREGIONS "  Region Data", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (aetRegion != nullptr)
			{
				AetSprite* frontSprite = aetRegion->GetFrontSprite();

				if (frontSprite == nullptr)
					sprintf_s(regionDataNameBuffer, "Null (%dx%d)", aetRegion->Width, aetRegion->Height);
				else
					strcpy_s(regionDataNameBuffer, frontSprite->Name.c_str());
			}

			const char* noSpriteString = "None (Sprite)";

			if (Gui::ComfyBeginCombo("Sprite", aetRegion == nullptr ? noSpriteString : regionDataNameBuffer, ImGuiComboFlags_HeightLarge))
			{
				if (Gui::Selectable(noSpriteString, aetRegion == nullptr))
					ProcessUpdatingAetCommand(GetCommandManager(), AetObjChangeReferenceRegion, aetObj, nullptr);

				int32_t regionIndex = 0;
				for (const RefPtr<AetRegion>& region : aet->AetRegions)
				{
					Gui::PushID(region.get());

					bool isSelected = (aetRegion == region);

					AetSprite* frontSprite = region->GetFrontSprite();
					if (frontSprite == nullptr)
						sprintf_s(regionDataNameBuffer, "Region %d (%dx%d)", regionIndex, region->Width, region->Height);

					if (Gui::Selectable(frontSprite == nullptr ? regionDataNameBuffer : frontSprite->Name.c_str(), isSelected))
						ProcessUpdatingAetCommand(GetCommandManager(), AetObjChangeReferenceRegion, aetObj, region);

					if (isSelected)
						Gui::SetItemDefaultFocus();
					Gui::PopID();
					regionIndex++;
				}

				Gui::ComfyEndCombo();
			}

			Gui::TreePop();
		}
	}

	AetKeyFrame* AetInspector::GetKeyFrameIfExact(const RefPtr<AetObj>& aetObj, int propertyIndex, float inputFrame) const
	{
		KeyFrameCollection& keyFrames = aetObj->AnimationData->Properties[propertyIndex];
		bool isFirstFrame = (inputFrame == aetObj->LoopStart);

		if (isFirstFrame && keyFrames.size() == 1)
			inputFrame = keyFrames.front().Frame;

		for (auto& keyFrame : keyFrames)
		{
			if (keyFrame.Frame == inputFrame)
				return &keyFrame;
		}

		return nullptr;
	}

	void AetInspector::DrawInspectorAnimationData(const RefPtr<AnimationData>& animationData, const RefPtr<AetObj>& aetObj)
	{
		if (Gui::WideTreeNodeEx(ICON_ANIMATIONDATA "  Animation Data", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (animationData != nullptr)
			{
				using namespace Graphics::Auth2D;

				static Properties currentProperties;
				AetMgr::Interpolate(animationData.get(), &currentProperties, currentFrame);

				AetKeyFrame* currentKeyFrames[PropertyType_Count];
				for (int i = 0; i < PropertyType_Count; i++)
					currentKeyFrames[i] = !isPlayback ? nullptr : GetKeyFrameIfExact(aetObj, i, glm::round(currentFrame));

				animatedPropertyColor = GetColorVec4(EditorColor_AnimatedProperty);
				keyFramePropertyColor = GetColorVec4(EditorColor_KeyFrameProperty);
				staticPropertyColor = Gui::GetStyleColorVec4(ImGuiCol_FrameBg);

				// TODO: Auto insert keyframe mode

				DrawInspectorAnimationDataPropertyVec2(animationData, "Origin", currentProperties.Origin, PropertyType_OriginX, PropertyType_OriginY, currentKeyFrames);
				DrawInspectorAnimationDataPropertyVec2(animationData, "Position", currentProperties.Position, PropertyType_PositionX, PropertyType_PositionY, currentKeyFrames);
				DrawInspectorAnimationDataProperty(animationData, "Rotation", currentProperties.Rotation, PropertyType_Rotation, currentKeyFrames);
				DrawInspectorAnimationDataPropertyVec2(animationData, "Scale", currentProperties.Scale, PropertyType_ScaleX, PropertyType_ScaleY, currentKeyFrames);
				DrawInspectorAnimationDataProperty(animationData, "Opacity", currentProperties.Opacity, PropertyType_Opacity, currentKeyFrames);
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

			Gui::TreePop();
		}
	}

	void AetInspector::DrawInspectorDebugAnimationData(const RefPtr<AnimationData>& animationData, const RefPtr<AetObj>& aetObj)
	{
		if (Gui::WideTreeNodeEx(ICON_ANIMATIONDATA "  DEBUG Data", ImGuiTreeNodeFlags_Selected))
		{
			if (animationData != nullptr)
			{
				int i = 0;
				for (auto& property : animationData->Properties)
				{
					if (Gui::WideTreeNode(KeyFrameProperties::PropertyNames.at(i++)))
					{
						for (auto& keyFrame : property)
							Gui::Text("Frame: %.2f; Value: %.2f; Curve: %.2f", keyFrame.Frame, keyFrame.Value, keyFrame.Interpolation);

						Gui::TreePop();
					}
				}
			}

			Gui::TreePop();
		}
	}

	static bool GetIsAnimationPropertyDisabled(AetKeyFrame* currentKeyFrames[], int propetyType, int propetyTypeAlt = -1)
	{
		return !(currentKeyFrames[propetyType] || (propetyTypeAlt >= 0 && currentKeyFrames[propetyTypeAlt]));
	}

	void AetInspector::DrawInspectorAnimationDataProperty(const RefPtr<AnimationData>& animationData, const char* label, float& value, int propertyType, AetKeyFrame* keyFrames[])
	{
		using namespace Graphics::Auth2D;
		constexpr float percentFactor = 100.0f;

		Gui::PushStyleColor(ImGuiCol_FrameBg, ((animationData->Properties[propertyType].size() > 1))
			? (keyFrames[propertyType])
			? keyFramePropertyColor
			: animatedPropertyColor
			: staticPropertyColor);

		bool rotation = (propertyType == PropertyType_Rotation);
		bool opacity = (propertyType == PropertyType_Opacity);

		const char* formatString = rotation ? u8"%.2f Åã" : opacity ? "%.0f%%" : "%.2f";

		float previousValue = value;

		if (opacity)
			value *= percentFactor;

		if (Gui::ComfyFloatWidget(label, &value, 1.0f, 10.0f, formatString, ImGuiInputTextFlags_None, GetIsAnimationPropertyDisabled(keyFrames, propertyType, propertyType)))
		{
			if (opacity)
				value = glm::clamp(value * (1.0f / percentFactor), 0.0f, 1.0f);

			if (keyFrames[propertyType] && value != previousValue)
			{
				auto tuple = std::make_tuple(static_cast<PropertyType_Enum>(propertyType), keyFrames[propertyType]->Frame, value);
				ProcessUpdatingAetCommand(GetCommandManager(), AnimationDataChangeKeyFrameValue, animationData, tuple);
			}
		}

		Gui::PopStyleColor();
	}

	void AetInspector::DrawInspectorAnimationDataPropertyVec2(const RefPtr<AnimationData>& animationData, const char* label, vec2& value, int propertyTypeX, int propertyTypeY, AetKeyFrame* keyFrames[])
	{
		using namespace Graphics::Auth2D;
		constexpr float percentFactor = 100.0f;

		Gui::PushStyleColor(ImGuiCol_FrameBg, ((animationData->Properties[propertyTypeX].size() > 1) || (animationData->Properties[propertyTypeY].size() > 1))
			? (keyFrames[propertyTypeX] || keyFrames[propertyTypeY])
			? keyFramePropertyColor
			: animatedPropertyColor
			: staticPropertyColor);

		bool scale = (propertyTypeX == PropertyType_ScaleX) && (propertyTypeY == PropertyType_ScaleY);
		const char* formatString = scale ? "%.2f%%" : "%.2f";

		vec2 previousValue = value;

		if (scale)
			value *= percentFactor;

		if (Gui::ComfyFloat2Widget(label, glm::value_ptr(value), formatString, ImGuiInputTextFlags_None, GetIsAnimationPropertyDisabled(keyFrames, propertyTypeX, propertyTypeY)))
		{
			if (scale)
				value *= (1.0f / percentFactor);

			if (keyFrames[propertyTypeX] && value.x != previousValue.x)
			{
				auto tuple = std::make_tuple(static_cast<PropertyType_Enum>(propertyTypeX), keyFrames[propertyTypeX]->Frame, value.x);
				ProcessUpdatingAetCommand(GetCommandManager(), AnimationDataChangeKeyFrameValue, animationData, tuple);
			}

			if (keyFrames[propertyTypeY] && value.y != previousValue.y)
			{
				auto tuple = std::make_tuple(static_cast<PropertyType_Enum>(propertyTypeY), keyFrames[propertyTypeY]->Frame, value.y);
				ProcessUpdatingAetCommand(GetCommandManager(), AnimationDataChangeKeyFrameValue, animationData, tuple);
			}
		}

		Gui::PopStyleColor();
	}

	void AetInspector::DrawInspectorAetObjMarkers(const RefPtr<AetObj>& aetObj, Vector<RefPtr<AetMarker>>* markers)
	{
		if (Gui::WideTreeNodeEx(ICON_MARKERS "  Markers", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (int i = 0; i < markers->size(); i++)
			{
				const RefPtr<AetMarker>& marker = markers->at(i);

				Gui::PushID(marker.get());

				ImVec2 treeNodeCursorPos = Gui::GetCursorScreenPos();
				bool open = Gui::WideTreeNode("##AetInspectorMarker");

				Gui::ItemContextMenu("AddMarkerContextMenu##AetInspector", [this, &aetObj, &markers, i]()
				{
					if (Gui::MenuItem(ICON_MOVEUP "  Move Up", nullptr, nullptr, i > 0))
					{
						auto tuple = std::tuple<int, int>(i, i - 1);
						ProcessAetCommand(GetCommandManager(), AetObjMoveMarker, aetObj, tuple);
					}

					if (Gui::MenuItem(ICON_MOVEDOWN "  Move Down", nullptr, nullptr, i < markers->size() - 1))
					{
						auto tuple = std::tuple<int, int>(i, i + 1);
						ProcessAetCommand(GetCommandManager(), AetObjMoveMarker, aetObj, tuple);
					}

					if (Gui::MenuItem(ICON_DELETE "  Delete"))
					{
						ProcessAetCommand(GetCommandManager(), AetObjDeleteMarker, aetObj, i);
					}
				});

				treeNodeCursorPos.x += GImGui->FontSize + GImGui->Style.FramePadding.x;
				Gui::SetCursorScreenPos(treeNodeCursorPos);
				Gui::Text("Frame: %.2f", marker->Frame);

				Gui::SetCursorScreenPos(treeNodeCursorPos + ImVec2(Gui::GetWindowWidth() * 0.35f, 0.0f));
				Gui::Text("%s", marker->Name.c_str());

				if (open)
				{
					float frame = marker->Frame;
					if (Gui::ComfyFloatWidget("Frame", &frame, 1.0f, 10.0f, "%.2f"))
						ProcessUpdatingAetCommand(GetCommandManager(), AetObjChangeMarkerFrame, marker, frame);

					strcpy_s(markerNameBuffer, marker->Name.c_str());
					if (Gui::ComfyTextWidget("Name", markerNameBuffer, sizeof(markerNameBuffer)))
						ProcessUpdatingAetCommand(GetCommandManager(), AetObjChangeMarkerName, marker, markerNameBuffer);

					Gui::TreePop();
				}
				Gui::PopID();
			}

			if (markers->size() > 0)
				Gui::Separator();

			if (Gui::ComfyCenteredButton("Add Marker"))
			{
				char newMarkerBuffer[32];
				sprintf_s(newMarkerBuffer, "marker_%02zd", markers->size());
				auto newMarker = MakeRef<AetMarker>(0.0f, newMarkerBuffer);
				ProcessAetCommand(GetCommandManager(), AetObjAddMarker, aetObj, newMarker);
			}
			Gui::TreePop();
		}
	}

	void AetInspector::DrawInspectorAetObjParent(Aet* aet, const RefPtr<AetObj>& aetObj)
	{
		if (Gui::WideTreeNodeEx(ICON_PARENT "  Parent", ImGuiTreeNodeFlags_DefaultOpen))
		{
			AetObj* parentObj = aetObj->GetReferencedParentObj().get();
			AetLayer* parentObjLayer = parentObj != nullptr ? parentObj->GetParentLayer() : nullptr;

			if (parentObj != nullptr)
				newParentObjLayerIndex = parentObjLayer->GuiData.ThisIndex;

			const char* noParentLayerString = "None (Parent Layer)";
			const char* noParentObjString = "None (Parent Object)";

			if (newParentObjLayerIndex >= 0)
			{
				parentObjLayer = aet->AetLayers[newParentObjLayerIndex].get();
				sprintf_s(parentObjDataNameBuffer, "Layer %d (%s)", parentObjLayer->GuiData.ThisIndex, parentObjLayer->GetCommaSeparatedNames().c_str());
			}
			else
			{
				strcpy_s(parentObjDataNameBuffer, noParentLayerString);
			}

			if (Gui::ComfyBeginCombo("Parent Layer", parentObjDataNameBuffer, ImGuiComboFlags_HeightLarge))
			{
				if (Gui::Selectable(noParentLayerString, parentObjLayer == nullptr))
				{
					if (aetObj->GetReferencedParentObj() != nullptr)
						ProcessUpdatingAetCommand(GetCommandManager(), AetObjChangeObjReferenceParent, aetObj, nullptr);
					newParentObjLayerIndex = -1;
				}

				for (int32_t layerIndex = 0; layerIndex < aet->AetLayers.size(); layerIndex++)
				{
					auto& layer = aet->AetLayers[layerIndex];

					bool isSelected = (layerIndex == newParentObjLayerIndex);
					sprintf_s(parentObjDataNameBuffer, "Layer %d (%s)", layerIndex, layer->GetCommaSeparatedNames().c_str());

					Gui::PushID(layer.get());
					if (Gui::Selectable(parentObjDataNameBuffer, isSelected))
					{
						if (aetObj->GetReferencedParentObj() != nullptr)
							ProcessUpdatingAetCommand(GetCommandManager(), AetObjChangeObjReferenceParent, aetObj, nullptr);

						newParentObjLayerIndex = layerIndex;
					}

					if (isSelected)
						Gui::SetItemDefaultFocus();
					Gui::PopID();
				}

				Gui::ComfyEndCombo();
			}

			if (Gui::ComfyBeginCombo("Parent Object", parentObj == nullptr ? noParentObjString : parentObj->GetName().c_str(), ImGuiComboFlags_HeightLarge))
			{
				if (Gui::Selectable(noParentObjString, parentObj == nullptr))
					ProcessUpdatingAetCommand(GetCommandManager(), AetObjChangeObjReferenceParent, aetObj, nullptr);

				if (parentObjLayer != nullptr)
				{
					for (int32_t objIndex = 0; objIndex < parentObjLayer->size(); objIndex++)
					{
						const RefPtr<AetObj>& obj = parentObjLayer->at(objIndex);
						bool isSelected = (obj.get() == parentObj);

						Gui::PushID(obj.get());
						if (Gui::Selectable(obj->GetName().c_str(), isSelected))
							ProcessUpdatingAetCommand(GetCommandManager(), AetObjChangeObjReferenceParent, aetObj, obj);

						if (isSelected)
							Gui::SetItemDefaultFocus();
						Gui::PopID();
					}
				}
				Gui::ComfyEndCombo();

			}
			Gui::TreePop();
		}
	}

	void AetInspector::DrawInspectorAetRegion(Aet* aet, const RefPtr<AetRegion>& aetRegion)
	{
		Gui::InputScalarN("Dimensions", ImGuiDataType_S16, &aetRegion->Width, 2);

		ImVec4 color = Gui::ColorConvertU32ToFloat4(aetRegion->Color);
		if (Gui::ColorEdit3("Background##AetRegionColor", (float*)&color, ImGuiColorEditFlags_DisplayHex))
			aetRegion->Color = Gui::ColorConvertFloat4ToU32(color);

		if (Gui::WideTreeNodeEx("Sprites:", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (auto& sprite : aetRegion->GetSprites())
			{
				sprintf_s(spriteNameBuffer, ICON_AETREGION "  %s", sprite.Name.c_str());
				Gui::Selectable(spriteNameBuffer);
			}
			Gui::TreePop();
		}
	}
}