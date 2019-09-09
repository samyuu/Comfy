#include "AetInspector.h"
#include "Editor/Aet/Command/Commands.h"
#include "Editor/Aet/AetIcons.h"
#include "Graphics/Auth2D/AetMgr.h"
#include "ImGui/Gui.h"

namespace Editor
{
	AetInspector::AetInspector(AetCommandManager* commandManager) : IMutableAetEditorComponent(commandManager)
	{
	}

	AetInspector::~AetInspector()
	{
	}

	void AetInspector::Initialize()
	{
	}

	bool AetInspector::DrawGui(Aet* aet, const AetItemTypePtr& selected)
	{
		if (lastSelectedItem.Ptrs.VoidPointer != selected.Ptrs.VoidPointer)
			newParentObjLayerIndex = -1;

		lastSelectedItem = selected;

		if (selected.Ptrs.VoidPointer == nullptr)
			return false;

		switch (selected.Type())
		{
		case AetSelectionType::AetSet:
			DrawInspectorAetSet(selected.GetAetSetRef());
			break;
		case AetSelectionType::Aet:
			DrawInspectorAet(selected.GetAetRef());
			break;
		case AetSelectionType::AetLayer:
			DrawInspectorAetLayer(aet, selected.GetAetLayerRef());
			break;
		case AetSelectionType::AetObj:
			DrawInspectorAetObj(aet, selected.GetAetObjRef());
			break;
		case AetSelectionType::AetRegion:
			DrawInspectorAetRegion(aet, selected.GetAetRegionRef());
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
			for (RefPtr<Aet>& aet : *aetSet)
				Gui::BulletText(aet->Name.c_str());

			Gui::TreePop();
		}
	}

	void AetInspector::DrawInspectorAet(const RefPtr<Aet>& aet)
	{
		if (Gui::WideTreeNodeEx("Aet", ImGuiTreeNodeFlags_DefaultOpen))
		{
			strcpy_s(aetNameBuffer, aet->Name.c_str());

			if (Gui::ComfyTextWidget("Name", aetNameBuffer, sizeof(aetNameBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
				GetCommandManager()->EnqueueCommand<Command::AetChangeName>(aet, aetNameBuffer);

			float frameStart = aet->FrameStart;
			if (Gui::ComfyFloatWidget("Start Frame", &frameStart, 1.0f, 10.0f, "%.2f", ImGuiInputTextFlags_EnterReturnsTrue))
				GetCommandManager()->EnqueueCommand<Command::AetChangeStartFrame>(aet, frameStart);

			float frameDuration = aet->FrameDuration;
			if (Gui::ComfyFloatWidget("Duration", &frameDuration, 1.0f, 10.0f, "%.2f", ImGuiInputTextFlags_EnterReturnsTrue))
				GetCommandManager()->EnqueueCommand<Command::AetChangeFrameDuration>(aet, frameDuration);

			float frameRate = aet->FrameRate;
			if (Gui::ComfyFloatWidget("Frame Rate", &frameRate, 1.0f, 10.0f, "%.2f", ImGuiInputTextFlags_EnterReturnsTrue))
				GetCommandManager()->EnqueueCommand<Command::AetChangeFrameRate>(aet, glm::clamp(frameRate, 1.0f, 1000.0f));

			ivec2 resolution = aet->Resolution;
			if (Gui::ComfyInt2Widget("Resolution", glm::value_ptr(resolution), ImGuiInputTextFlags_EnterReturnsTrue))
				GetCommandManager()->EnqueueCommand<Command::AetChangeResolution>(aet, resolution);

			vec4 color = Gui::ColorConvertU32ToFloat4(aet->BackgroundColor);
			if (Gui::ComfyColorEdit3("Background", glm::value_ptr(color), ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_DisplayHex))
				GetCommandManager()->EnqueueCommand<Command::AetChangeBackgroundColor>(aet, Gui::ColorConvertFloat4ToU32(color));

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
			for (RefPtr<AetObj>& aetObj : *aetLayer)
				Gui::BulletText("%s  %s", GetObjTypeIcon(aetObj->Type), aetObj->GetName().c_str());

			Gui::TreePop();
		}
	}

	void AetInspector::DrawInspectorLayerData(Aet* aet, const RefPtr<AetObj>& aetObj, const RefPtr<AetLayer>& aetLayer)
	{
		if (Gui::WideTreeNodeEx(ICON_AETLAYERS "  Layer Data", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (aetLayer != nullptr)
				sprintf_s(layerDataNameBuffer, "Layer %d (%s)", aetLayer->GetThisIndex(), aetLayer->GetCommaSeparatedNames());

			if (Gui::ComfyBeginCombo("Layer", aetLayer == nullptr ? "None (Layer)" : layerDataNameBuffer, ImGuiComboFlags_HeightLarge))
			{
				if (Gui::Selectable("None (Layer)", aetLayer == nullptr))
					GetCommandManager()->EnqueueCommand<Command::AetObjChangeReferenceLayer>(aetObj, nullptr);

				int layerIndex = 0;
				for (RefPtr<AetLayer>& layer : aet->AetLayers)
				{
					if (&aet->AetLayers.back() == &layer)
						break;

					Gui::PushID(&layer);

					bool isSelected = (aetLayer == layer);
					sprintf_s(layerDataNameBuffer, "Layer %d (%s)", layerIndex++, layer->GetCommaSeparatedNames());

					if (Gui::Selectable(layerDataNameBuffer, isSelected))
						GetCommandManager()->EnqueueCommand<Command::AetObjChangeReferenceLayer>(aetObj, layer);

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
			if (Gui::ComfyTextWidget("Name", aetObjNameBuffer, sizeof(aetObjNameBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
				GetCommandManager()->EnqueueCommand<Command::AetObjChangeName>(aetObj, aetObjNameBuffer);

			float loopStart = aetObj->LoopStart;
			if (Gui::ComfyFloatWidget("Loop Start", &loopStart, 1.0f, 10.0f, "%.2f", ImGuiInputTextFlags_EnterReturnsTrue))
				GetCommandManager()->EnqueueCommand<Command::AetObjChangeLoopStart>(aetObj, loopStart);

			float loopEnd = aetObj->LoopEnd;
			if (Gui::ComfyFloatWidget("Loop End", &loopEnd, 1.0f, 10.0f, "%.2f", ImGuiInputTextFlags_EnterReturnsTrue))
				GetCommandManager()->EnqueueCommand<Command::AetObjChangeLoopEnd>(aetObj, loopEnd);

			float startFrame = aetObj->StartFrame;
			if (Gui::ComfyFloatWidget("Start Frame", &startFrame, 1.0f, 10.0f, "%.2f", ImGuiInputTextFlags_EnterReturnsTrue))
				GetCommandManager()->EnqueueCommand<Command::AetObjChangeStartFrame>(aetObj, startFrame);

			if (aetObj->Type != AetObjType::Aif)
			{
				constexpr float percentageFactor = 100.0f;
				float playbackPercentage = aetObj->PlaybackSpeed * percentageFactor;

				if (Gui::ComfyFloatWidget("Playback Speed", &playbackPercentage, 10.0f, 100.0f, "%.0f%%", ImGuiInputTextFlags_EnterReturnsTrue))
					GetCommandManager()->EnqueueCommand<Command::AetObjChangePlaybackSpeed>(aetObj, playbackPercentage / percentageFactor);
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
					GetCommandManager()->EnqueueCommand<Command::AetObjChangeReferenceRegion>(aetObj, nullptr);

				int32_t regionIndex = 0;
				for (RefPtr<AetRegion>& region : aet->AetRegions)
				{
					Gui::PushID(&region);

					bool isSelected = (aetRegion == region);

					AetSprite* frontSprite = region->GetFrontSprite();
					if (frontSprite == nullptr)
						sprintf_s(regionDataNameBuffer, "Region %d (%dx%d)", regionIndex, region->Width, region->Height);

					if (Gui::Selectable(frontSprite == nullptr ? regionDataNameBuffer : frontSprite->Name.c_str(), isSelected))
						GetCommandManager()->EnqueueCommand<Command::AetObjChangeReferenceRegion>(aetObj, region);

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

	std::pair<AetKeyFrame*, int> AetInspector::GetKeyFrameAndIndex(const RefPtr<AetObj>& aetObj, int propertyIndex, float inputFrame) const
	{
		KeyFrameCollection& keyFrames = aetObj->AnimationData->Properties[propertyIndex];
		bool firstFrame = inputFrame == aetObj->LoopStart;

		if (firstFrame && keyFrames.size() == 1)
			inputFrame = keyFrames.front().Frame;

		AetKeyFrame* foundKeyFrame = nullptr;
		int index = 0;

		for (auto& keyFrame : keyFrames)
		{
			if (glm::round(keyFrame.Frame) == inputFrame)
			{
				foundKeyFrame = &keyFrame;
				break;
			}
			index++;
		}

		return { foundKeyFrame, index };
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
				int currentKeyFrameIndices[PropertyType_Count];

				for (int i = 0; i < PropertyType_Count; i++)
				{
					if (isPlayback)
					{
						currentKeyFrames[i] = nullptr;
					}
					else
					{
						auto[keyFrame, index] = GetKeyFrameAndIndex(aetObj, i, glm::round(currentFrame));

						currentKeyFrames[i] = keyFrame;
						currentKeyFrameIndices[i] = index;
					}
				}

				animatedPropertyColor = GetColorVec4(EditorColor_AnimatedProperty);
				keyFramePropertyColor = GetColorVec4(EditorColor_KeyFrameProperty);
				staticPropertyColor = Gui::GetStyleColorVec4(ImGuiCol_FrameBg);

				// TODO: Auto insert keyframe mode

				DrawInspectorAnimationDataPropertyVec2(animationData, "Origin", currentProperties.Origin, PropertyType_OriginX, PropertyType_OriginY, currentKeyFrames, currentKeyFrameIndices);
				DrawInspectorAnimationDataPropertyVec2(animationData, "Position", currentProperties.Position, PropertyType_PositionX, PropertyType_PositionY, currentKeyFrames, currentKeyFrameIndices);
				DrawInspectorAnimationDataProperty(animationData, "Rotation", currentProperties.Rotation, PropertyType_Rotation, currentKeyFrames, currentKeyFrameIndices);
				DrawInspectorAnimationDataPropertyVec2(animationData, "Scale", currentProperties.Scale, PropertyType_ScaleX, PropertyType_ScaleY, currentKeyFrames, currentKeyFrameIndices);
				DrawInspectorAnimationDataProperty(animationData, "Opacity", currentProperties.Opacity, PropertyType_Opacity, currentKeyFrames, currentKeyFrameIndices);
			}

			if (aetObj->Type == AetObjType::Pic)
			{
				int32_t blendMode = static_cast<int32_t>(animationData->BlendMode);
				if (Gui::ComfyBeginCombo("Blend Mode", AnimationData::BlendModeNames[blendMode], ImGuiComboFlags_HeightLarge))
				{
					for (int32_t blendModeIndex = 0; blendModeIndex < AnimationData::BlendModeNames.size(); blendModeIndex++)
					{
						if (AnimationData::BlendModeNames[blendModeIndex] == nullptr)
							continue;

						if (Gui::Selectable(AnimationData::BlendModeNames[blendModeIndex], blendModeIndex == blendMode))
							GetCommandManager()->EnqueueCommand<Command::AnimationDataChangeBlendMode>(animationData, static_cast<AetBlendMode>(blendModeIndex));

						if (blendModeIndex == blendMode)
							Gui::SetItemDefaultFocus();
					};

					Gui::ComfyEndCombo();
				}

				bool useTextureMask = animationData->UseTextureMask;
				if (Gui::ComfyCheckbox("Use Texture Mask", &useTextureMask))
					GetCommandManager()->EnqueueCommand<Command::AnimationDataChangeUseTextureMask>(animationData, useTextureMask);
			}

			Gui::TreePop();
		}
	}

	static bool GetIsAnimationPropertyDisabled(AetKeyFrame* currentKeyFrames[], int propetyType, int propetyTypeAlt = -1)
	{
		return !(currentKeyFrames[propetyType] || (propetyTypeAlt >= 0 && currentKeyFrames[propetyTypeAlt]));
	}

	void AetInspector::DrawInspectorAnimationDataProperty(const RefPtr<AnimationData>& animationData, const char* label, float& value, int propertyType, AetKeyFrame* keyFrames[], int keyFrameIndices[])
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

		const char* formatString = rotation ? u8"%.2f ‹" : opacity ? "%.0f%%" : "%.2f";

		float previousValue = value;

		if (opacity)
			value *= percentFactor;

		// TODO: Remove step and stepFast to avoid filling undo history with meaningless items
		if (Gui::ComfyFloatWidget(label, &value, 1.0f, 10.0f, formatString, ImGuiInputTextFlags_EnterReturnsTrue, GetIsAnimationPropertyDisabled(keyFrames, propertyType, propertyType)))
		{
			if (opacity)
				value = glm::clamp(value * (1.0f / percentFactor), 0.0f, 1.0f);

			if (keyFrames[propertyType] && value != previousValue)
				GetCommandManager()->EnqueueCommand<Command::AnimationDataChangeKeyFrameValue>(animationData, std::make_tuple(static_cast<PropertyType_Enum>(propertyType), keyFrameIndices[propertyType], value));
		}

		Gui::PopStyleColor();
	}

	void AetInspector::DrawInspectorAnimationDataPropertyVec2(const RefPtr<AnimationData>& animationData, const char* label, vec2& value, int propertyTypeX, int propertyTypeY, AetKeyFrame* keyFrames[], int keyFrameIndices[])
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

		if (Gui::ComfyFloat2Widget(label, glm::value_ptr(value), formatString, ImGuiInputTextFlags_EnterReturnsTrue, GetIsAnimationPropertyDisabled(keyFrames, propertyTypeX, propertyTypeY)))
		{
			if (scale)
				value *= (1.0f / percentFactor);

			if (keyFrames[propertyTypeX] && value.x != previousValue.x)
				GetCommandManager()->EnqueueCommand<Command::AnimationDataChangeKeyFrameValue>(animationData, std::make_tuple(static_cast<PropertyType_Enum>(propertyTypeX), keyFrameIndices[propertyTypeX], value.x));

			if (keyFrames[propertyTypeY] && value.y != previousValue.y)
				GetCommandManager()->EnqueueCommand<Command::AnimationDataChangeKeyFrameValue>(animationData, std::make_tuple(static_cast<PropertyType_Enum>(propertyTypeY), keyFrameIndices[propertyTypeY], value.y));
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
						GetCommandManager()->EnqueueCommand<Command::AetObjMoveMarker>(aetObj, std::tuple<int, int>(i, i - 1));

					if (Gui::MenuItem(ICON_MOVEDOWN "  Move Down", nullptr, nullptr, i < markers->size() - 1))
						GetCommandManager()->EnqueueCommand<Command::AetObjMoveMarker>(aetObj, std::tuple<int, int>(i, i + 1));

					if (Gui::MenuItem(ICON_DELETE "  Delete"))
						GetCommandManager()->EnqueueCommand<Command::AetObjDeleteMarker>(aetObj, i);
				});

				treeNodeCursorPos.x += GImGui->FontSize + GImGui->Style.FramePadding.x;
				Gui::SetCursorScreenPos(treeNodeCursorPos);
				Gui::Text("Frame: %.2f", marker->Frame);

				Gui::SetCursorScreenPos(treeNodeCursorPos + ImVec2(Gui::GetWindowWidth() * 0.35f, 0.0f));
				Gui::Text("%s", marker->Name.c_str());

				if (open)
				{
					float frame = marker->Frame;
					if (Gui::ComfyFloatWidget("Frame", &frame, 1.0f, 10.0f, "%.2f", ImGuiInputTextFlags_EnterReturnsTrue))
						GetCommandManager()->EnqueueCommand<Command::AetObjChangeMarkerFrame>(marker, frame);

					strcpy_s(markerNameBuffer, marker->Name.c_str());
					if (Gui::ComfyTextWidget("Name", markerNameBuffer, sizeof(markerNameBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
						GetCommandManager()->EnqueueCommand<Command::AetObjChangeMarkerName>(marker, markerNameBuffer);

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
				GetCommandManager()->EnqueueCommand<Command::AetObjAddMarker>(aetObj, MakeRef<AetMarker>(0.0f, newMarkerBuffer));
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
				newParentObjLayerIndex = parentObjLayer->GetThisIndex();

			const char* noParentLayerString = "None (Parent Layer)";
			const char* noParentObjString = "None (Parent Object)";

			if (newParentObjLayerIndex >= 0)
			{
				parentObjLayer = aet->AetLayers[newParentObjLayerIndex].get();
				sprintf_s(parentObjDataNameBuffer, "Layer %d (%s)", parentObjLayer->GetThisIndex(), parentObjLayer->GetCommaSeparatedNames());
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
						GetCommandManager()->EnqueueCommand<Command::AetObjChangeObjReferenceParent>(aetObj, nullptr);
					newParentObjLayerIndex = -1;
				}

				for (int32_t layerIndex = 0; layerIndex < aet->AetLayers.size(); layerIndex++)
				{
					auto& layer = aet->AetLayers[layerIndex];

					bool isSelected = (layerIndex == newParentObjLayerIndex);
					sprintf_s(parentObjDataNameBuffer, "Layer %d (%s)", layerIndex, layer->GetCommaSeparatedNames());

					Gui::PushID(&layer);
					if (Gui::Selectable(parentObjDataNameBuffer, isSelected))
					{
						if (aetObj->GetReferencedParentObj() != nullptr)
							GetCommandManager()->EnqueueCommand<Command::AetObjChangeObjReferenceParent>(aetObj, nullptr);
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
					GetCommandManager()->EnqueueCommand<Command::AetObjChangeObjReferenceParent>(aetObj, nullptr);

				if (parentObjLayer != nullptr)
				{
					for (int32_t objIndex = 0; objIndex < parentObjLayer->size(); objIndex++)
					{
						const RefPtr<AetObj>& obj = parentObjLayer->at(objIndex);
						bool isSelected = (obj.get() == parentObj);

						Gui::PushID(&obj);
						if (Gui::Selectable(obj->GetName().c_str(), isSelected))
							GetCommandManager()->EnqueueCommand<Command::AetObjChangeObjReferenceParent>(aetObj, obj);

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