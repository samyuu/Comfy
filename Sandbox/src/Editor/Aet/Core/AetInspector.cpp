#include "AetInspector.h"
#include "Editor/Aet/Command/Commands.h"
#include "Editor/Aet/AetIcons.h"
#include "ImGui/Gui.h"

namespace Editor
{
	namespace Command
	{
		class ChangeAetName;
	}

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
				GetCommandManager()->EnqueueCommand<Command::Aet::ChangeName>(aet, aetNameBuffer);

			float frameStart = aet->FrameStart;
			if (Gui::ComfyFloatWidget("Start Frame", &frameStart, 1.0f, 10.0f, "%.2f", ImGuiInputTextFlags_EnterReturnsTrue))
				GetCommandManager()->EnqueueCommand<Command::Aet::ChangeStartFrame>(aet, frameStart);

			float frameDuration = aet->FrameDuration;
			if (Gui::ComfyFloatWidget("Duration", &frameDuration, 1.0f, 10.0f, "%.2f", ImGuiInputTextFlags_EnterReturnsTrue))
				GetCommandManager()->EnqueueCommand<Command::Aet::ChangeFrameDuration>(aet, frameDuration);
			
			float frameRate = aet->FrameRate;
			if (Gui::ComfyFloatWidget("Frame Rate", &frameRate, 1.0f, 10.0f, "%.2f", ImGuiInputTextFlags_EnterReturnsTrue))
				GetCommandManager()->EnqueueCommand<Command::Aet::ChangeFrameRate>(aet, glm::clamp(frameRate, 1.0f, 1000.0f));

			int resolution[2] = { aet->Width, aet->Height };
			if (Gui::ComfyInt2Widget("Resolution", resolution, ImGuiInputTextFlags_EnterReturnsTrue))
				GetCommandManager()->EnqueueCommand<Command::Aet::ChangeResolution>(aet, resolution[0], resolution[1]);

			vec4 color = Gui::ColorConvertU32ToFloat4(aet->BackgroundColor);
			if (Gui::ComfyColorEdit3("Background", glm::value_ptr(color), ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_DisplayHex))
				GetCommandManager()->EnqueueCommand<Command::Aet::ChangeBackgroundColor>(aet, Gui::ColorConvertFloat4ToU32(color));

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
					aetObj->SetReferencedLayer(nullptr);

				int layerIndex = 0;
				for (RefPtr<AetLayer>& layer : aet->AetLayers)
				{
					if (&aet->AetLayers.back() == &layer)
						break;

					Gui::PushID(&layer);

					bool isSelected = (aetLayer == layer);
					sprintf_s(layerDataNameBuffer, "Layer %d (%s)", layerIndex++, layer->GetCommaSeparatedNames());

					if (Gui::Selectable(layerDataNameBuffer, isSelected))
						aetObj->SetReferencedLayer(layer);

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
				GetCommandManager()->EnqueueCommand<Command::AetObj::ChangeName>(aetObj, aetObjNameBuffer);

			float loopStart = aetObj->LoopStart;
			if (Gui::ComfyFloatWidget("Loop Start", &loopStart, 1.0f, 10.0f, "%.2f", ImGuiInputTextFlags_EnterReturnsTrue))
				GetCommandManager()->EnqueueCommand<Command::AetObj::ChangeLoopStart>(aetObj, loopStart);

			float loopEnd = aetObj->LoopEnd;
			if (Gui::ComfyFloatWidget("Loop End", &loopEnd, 1.0f, 10.0f, "%.2f", ImGuiInputTextFlags_EnterReturnsTrue))
				GetCommandManager()->EnqueueCommand<Command::AetObj::ChangeLoopEnd>(aetObj, loopEnd);

			float startFrame = aetObj->StartFrame;
			if (Gui::ComfyFloatWidget("Start Frame", &startFrame, 1.0f, 10.0f, "%.2f", ImGuiInputTextFlags_EnterReturnsTrue))
				GetCommandManager()->EnqueueCommand<Command::AetObj::ChangeStartFrame>(aetObj, startFrame);

			if (aetObj->Type != AetObjType::Aif)
			{
				constexpr float percentageFactor = 100.0f;
				float playbackPercentage = aetObj->PlaybackSpeed * percentageFactor;

				if (Gui::ComfyFloatWidget("Playback Speed", &playbackPercentage, 10.0f, 100.0f, "%.2f %%", ImGuiInputTextFlags_EnterReturnsTrue))
					GetCommandManager()->EnqueueCommand<Command::AetObj::ChangePlaybackSpeed>(aetObj, playbackPercentage / percentageFactor);
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
			DrawInspectorAnimationData(aetObj->AnimationData.get(), aetObj->Type);
		}

		Gui::Separator();
		DrawInspectorAetObjMarkers(&aetObj->Markers);

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
					aetObj->SetReferencedRegion(nullptr);

				int32_t regionIndex = 0;
				for (RefPtr<AetRegion>& region : aet->AetRegions)
				{
					Gui::PushID(&region);

					bool isSelected = (aetRegion == region);

					AetSprite* frontSprite = region->GetFrontSprite();
					if (frontSprite == nullptr)
						sprintf_s(regionDataNameBuffer, "Region %d (%dx%d)", regionIndex, region->Width, region->Height);

					if (Gui::Selectable(frontSprite == nullptr ? regionDataNameBuffer : frontSprite->Name.c_str(), isSelected))
						aetObj->SetReferencedRegion(region);

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

	void AetInspector::DrawInspectorAnimationData(AnimationData* animationData, AetObjType objType)
	{
		if (Gui::WideTreeNodeEx(ICON_ANIMATIONDATA "  Animation Data", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (Gui::WideTreeNode("Properties"))
			{
				if (animationData != nullptr)
				{
					DrawInspectorKeyFrameProperties(&animationData->Properties);
				}
				else
				{
					Gui::BulletText("None");
				}
				Gui::TreePop();
			}

			if (Gui::WideTreeNode("Perspective Properties"))
			{
				if (animationData->PerspectiveProperties != nullptr)
				{
					DrawInspectorKeyFrameProperties(animationData->PerspectiveProperties.get());
				}
				else
				{
					Gui::BulletText("None");
				}
				Gui::TreePop();
			}

			if (objType == AetObjType::Pic)
			{
				int32_t blendMode = static_cast<int32_t>(animationData->BlendMode);
				if (Gui::ComfyBeginCombo("Blend Mode", AnimationData::BlendModeNames[blendMode], ImGuiComboFlags_HeightLarge))
				{
					for (int32_t blendModeIndex = 0; blendModeIndex < AnimationData::BlendModeNames.size(); blendModeIndex++)
					{
						if (AnimationData::BlendModeNames[blendModeIndex] == nullptr)
							continue;

						if (Gui::Selectable(AnimationData::BlendModeNames[blendModeIndex], blendModeIndex == blendMode))
							animationData->BlendMode = static_cast<AetBlendMode>(blendModeIndex);

						if (blendModeIndex == blendMode)
							Gui::SetItemDefaultFocus();
					};

					Gui::ComfyEndCombo();
				}

				Gui::ComfyCheckbox("Use Texture Mask", &animationData->UseTextureMask);
			}

			Gui::TreePop();
		}
	}

	void AetInspector::DrawInspectorKeyFrameProperties(KeyFrameProperties* properties)
	{
		for (size_t i = 0; i < properties->size(); i++)
			DrawInspectorKeyFrames(KeyFrameProperties::PropertyNames[i], &properties->at(i));
	}

	void AetInspector::DrawInspectorKeyFrames(const char* name, std::vector<AetKeyFrame>* keyFrames)
	{
		if (Gui::WideTreeNode(name))
		{
			if (keyFrames->size() == 1)
			{
				Gui::PushID((void*)keyFrames);
				Gui::ComfyFloatWidget("Value", &keyFrames->front().Value, 0.1f, 1.0f);
				Gui::PopID();
			}
			else
			{
				for (AetKeyFrame& keyFrame : *keyFrames)
				{
					Gui::PushID((void*)&keyFrame.Frame);
					Gui::DragFloat3("F-V-I", &keyFrame.Frame);
					Gui::PopID();
				}
			}
			Gui::TreePop();
		}
	}

	void AetInspector::DrawInspectorAetObjMarkers(std::vector<AetMarker>* markers)
	{
		if (Gui::WideTreeNodeEx(ICON_MARKERS "  Markers", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (int i = 0; i < markers->size(); i++)
			{
				AetMarker& marker = markers->at(i);

				Gui::PushID((void*)&marker);

				ImVec2 treeNodeCursorPos = Gui::GetCursorScreenPos();
				bool open = Gui::WideTreeNode("##AetInspectorMarker");

				Gui::ItemContextMenu("AddMarkerContextMenu##AetInspector", [&markers, &i]()
				{
					if (Gui::MenuItem(ICON_MOVEUP "  Move Up", nullptr, nullptr, i > 0))
						std::iter_swap(markers->begin() + i, markers->begin() + i - 1);

					if (Gui::MenuItem(ICON_MOVEDOWN "  Move Down", nullptr, nullptr, i < markers->size() - 1))
						std::iter_swap(markers->begin() + i, markers->begin() + i + 1);

					if (Gui::MenuItem(ICON_DELETE "  Delete"))
					{
						markers->erase(markers->begin() + i);
						i--;
					}
				});

				treeNodeCursorPos.x += GImGui->FontSize + GImGui->Style.FramePadding.x;
				Gui::SetCursorScreenPos(treeNodeCursorPos);
				Gui::Text("Frame: %.2f", marker.Frame);
				Gui::SetCursorScreenPos(treeNodeCursorPos + ImVec2(Gui::GetWindowWidth() * 0.35f, 0.0f));
				Gui::Text("%s", marker.Name.c_str());

				if (open)
				{
					Gui::ComfyFloatWidget("Frame", &marker.Frame, 1.0f, 10.0f);
					strcpy_s(markerNameBuffer, marker.Name.c_str());

					if (Gui::ComfyTextWidget("Name", markerNameBuffer, sizeof(markerNameBuffer)))
						marker.Name = std::string(markerNameBuffer);

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
				markers->emplace_back(0.0f, newMarkerBuffer);
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
					aetObj->SetParentObj(nullptr);
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
						aetObj->SetParentObj(nullptr);
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
					aetObj->SetParentObj(nullptr);

				if (parentObjLayer != nullptr)
				{
					for (int32_t objIndex = 0; objIndex < parentObjLayer->size(); objIndex++)
					{
						auto& obj = parentObjLayer->at(objIndex);
						bool isSelected = (obj.get() == parentObj);

						Gui::PushID(&obj);
						if (Gui::Selectable(obj->GetName().c_str(), isSelected))
							aetObj->SetParentObj(obj);

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