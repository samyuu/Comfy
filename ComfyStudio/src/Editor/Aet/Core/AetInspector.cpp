#include "AetInspector.h"
#include "Editor/Aet/AetCommands.h"
#include "Editor/Aet/AetIcons.h"
#include "Graphics/Auth2D/Aet/AetUtil.h"
#include "ImGui/Gui.h"
#include "ImGui/Extensions/TexExtensions.h"

namespace Comfy::Studio::Editor
{
	using namespace Graphics;
	using namespace Graphics::Aet;

	namespace
	{
		void CopyStringIntoBuffer(std::string_view string, char* buffer, size_t bufferSize)
		{
			size_t copySize = std::min(string.size(), bufferSize - 1);
			string.copy(buffer, copySize);
			buffer[copySize] = '\0';
		}

		const char* GetBlendModeName(AetBlendMode blendMode)
		{
			size_t blendModeIndex = static_cast<size_t>(blendMode);

			// NOTE: This should never happen
			if (blendModeIndex >= AetBlendModeNames.size())
				return "Invalid Blend Mode";

			return AetBlendModeNames[blendModeIndex];
		}

		bool IsBlendModeSupported(AetBlendMode mode)
		{
			switch (mode)
			{
			case AetBlendMode::Normal:
			case AetBlendMode::Add:
			case AetBlendMode::Multiply:
			case AetBlendMode::Screen:
			case AetBlendMode::Overlay:
				return true;

			default:
				return false;
			}
		}
	}

	constexpr ImGuiTreeNodeFlags DefaultOpenPropertiesNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow;

	AetInspector::AetInspector(Undo::UndoManager& undoManager, Render::Renderer2D& renderer, AetRenderPreviewData& previewData)
		: undoManager(undoManager), renderer(renderer), previewData(previewData)
	{
	}

	void AetInspector::Initialize()
	{
	}

	bool AetInspector::DrawGui(const AetItemTypePtr& selected)
	{
		lastSelectedItem = selected;

		if (selected.Ptrs.Void == nullptr)
			return false;

		// NOTE: Safety clear so no invalid state stays around when deselecting a layer for example
		previewData.Video = nullptr;
		previewData.BlendMode = AetBlendMode::Unknown;

		switch (selected.Type())
		{
		case AetItemType::AetSet:
			DrawInspectorAetSet(selected.GetAetSetRef());
			break;
		case AetItemType::Scene:
			DrawInspectorScene(selected.GetSceneRef());
			break;
		case AetItemType::Composition:
			DrawInspectorComposition(selected.GetItemParentScene(), selected.GetCompositionRef());
			break;
		case AetItemType::Layer:
			DrawInspectorLayer(selected.GetItemParentScene(), selected.GetLayerRef());
			break;
		case AetItemType::Video:
			DrawInspectorVideo(selected.GetItemParentScene(), selected.GetVideoRef());
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

	void AetInspector::DrawInspectorAetSet(const std::shared_ptr<AetSet>& aetSet)
	{
		if (Gui::WideTreeNodeEx(ICON_NAMES "  Aets:", DefaultOpenPropertiesNodeFlags))
		{
			for (auto& scene : aetSet->GetScenes())
				Gui::BulletText(scene->Name.c_str());

			Gui::TreePop();
		}
	}

	void AetInspector::DrawInspectorScene(const std::shared_ptr<Scene>& scene)
	{
		if (Gui::WideTreeNodeEx("Scene", DefaultOpenPropertiesNodeFlags))
		{
			PushDisableItemFlagIfPlayback();
			CopyStringIntoBuffer(scene->Name, aetNameBuffer, sizeof(aetNameBuffer));

			if (Gui::ComfyTextWidget("Name", aetNameBuffer, sizeof(aetNameBuffer)))
				undoManager.AddToEndOfFrameExecutionList<SceneChangeName>(scene, aetNameBuffer);

			float startFrame = scene->StartFrame;
			if (Gui::ComfyFloatTextWidget("Start Frame", &startFrame, 1.0f, 10.0f, 0.0f, 0.0f, "%.2f"))
				undoManager.AddToEndOfFrameExecutionList<SceneChangeStartFrame>(scene, startFrame);

			float endFrame = scene->EndFrame;
			if (Gui::ComfyFloatTextWidget("End Frame", &endFrame, 1.0f, 10.0f, 0.0f, 0.0f, "%.2f"))
				undoManager.AddToEndOfFrameExecutionList<SceneChangeEndFrame>(scene, endFrame);

			float frameRate = scene->FrameRate;
			if (Gui::ComfyFloatTextWidget("Frame Rate", &frameRate, 1.0f, 10.0f, 0.0f, 0.0f, "%.2f"))
				undoManager.AddToEndOfFrameExecutionList<SceneChangeFrameRate>(scene, glm::clamp(frameRate, 1.0f, 1000.0f));

			ivec2 resolution = scene->Resolution;
			if (Gui::ComfyInt2TextWidget("Resolution", glm::value_ptr(resolution)))
				undoManager.AddToEndOfFrameExecutionList<SceneChangeResolution>(scene, resolution);

			vec4 color = Gui::ColorConvertU32ToFloat4(scene->BackgroundColor);
			if (Gui::ComfyColorEdit3("Background", glm::value_ptr(color), ImGuiColorEditFlags_DisplayHex))
				undoManager.AddToEndOfFrameExecutionList<SceneChangeBackgroundColor>(scene, Gui::ColorConvertFloat4ToU32(color));

			PopDisableItemFlagIfPlayback();
			Gui::TreePop();
		}
		Gui::Separator();
	}

	void AetInspector::DrawInspectorComposition(Scene* scene, const std::shared_ptr<Composition>& comp)
	{
		if (Gui::WideTreeNodeEx(ICON_AETCOMP "  Composition", DefaultOpenPropertiesNodeFlags))
		{
			PushDisableItemFlagIfPlayback();
			CopyStringIntoBuffer(comp->GetName(), compNameBuffer, sizeof(compNameBuffer));

			const bool isRoot = comp.get() == scene->GetRootComposition();
			if (Gui::ComfyTextWidget("Name", compNameBuffer, sizeof(compNameBuffer), isRoot ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_None))
				undoManager.AddToEndOfFrameExecutionList<CompositionChangeName>(comp, compNameBuffer);

			// NOTE: Readonly properties
			Gui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			{
				int thisIndex = comp->GuiData.ThisIndex;
				Gui::ComfyIntTextWidget("Index", &thisIndex, 0, 0, ImGuiInputTextFlags_ReadOnly);

				int layerCount = static_cast<int>(comp->GetLayers().size());
				Gui::ComfyIntTextWidget("Layer Count", &layerCount, 0, 0, ImGuiInputTextFlags_ReadOnly);
			}
			Gui::PopItemFlag();

			Gui::Separator();

			PopDisableItemFlagIfPlayback();
			Gui::TreePop();
		}
	}

	void AetInspector::DrawInspectorCompositionData(Scene* scene, const std::shared_ptr<Layer>& layer, const std::shared_ptr<Composition>& comp)
	{
		// TODO: In the future you should not be able to change the composition after creating it because it would leave the previous composition "nameless" (?)

		constexpr int availableCompNameBufferSize = static_cast<int>(sizeof(compDataNameBuffer) - 32);

		if (Gui::WideTreeNodeEx(ICON_AETCOMPS "  Comp Data", DefaultOpenPropertiesNodeFlags))
		{
			if (comp != nullptr)
				sprintf_s(compDataNameBuffer, "%.*s (Comp %d)", availableCompNameBufferSize, comp->GetName().data(), comp->GuiData.ThisIndex);

			PushDisableItemFlagIfPlayback();
			if (Gui::ComfyBeginCombo("Composition", comp == nullptr ? "None (Comp)" : compDataNameBuffer, ImGuiComboFlags_HeightLarge))
			{
				if (Gui::Selectable("None (Composition)", comp == nullptr))
					undoManager.AddToEndOfFrameExecutionList<LayerChangeCompItem>(layer, nullptr);

				for (const std::shared_ptr<Composition>& comp : scene->Compositions)
				{
					Gui::PushID(comp.get());

					bool isSelected = (comp == comp);
					sprintf_s(compDataNameBuffer, "%.*s (Comp %d)", availableCompNameBufferSize, comp->GetName().data(), comp->GuiData.ThisIndex);

					if (Gui::Selectable(compDataNameBuffer, isSelected))
						undoManager.AddToEndOfFrameExecutionList<LayerChangeCompItem>(layer, comp);

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

	void AetInspector::DrawInspectorLayer(Scene* scene, const std::shared_ptr<Layer>& layer)
	{
		if (Gui::WideTreeNodeEx(layer.get(), DefaultOpenPropertiesNodeFlags, "%s  Layer", GetItemTypeIcon(layer->ItemType)))
		{
			PushDisableItemFlagIfPlayback();
			CopyStringIntoBuffer(layer->GetName(), layerNameBuffer, sizeof(layerNameBuffer));
			if (Gui::ComfyTextWidget("Name", layerNameBuffer, sizeof(layerNameBuffer)))
				undoManager.AddToEndOfFrameExecutionList<LayerChangeName>(layer, layerNameBuffer);

			float startFrame = layer->StartFrame;
			if (Gui::ComfyFloatTextWidget("Start Frame", &startFrame, 1.0f, 10.0f, 0.0f, 0.0f, "%.2f"))
				undoManager.AddToEndOfFrameExecutionList<LayerChangeStartFrame>(layer, startFrame);

			float endFrame = layer->EndFrame;
			if (Gui::ComfyFloatTextWidget("End Frame", &endFrame, 1.0f, 10.0f, 0.0f, 0.0f, "%.2f"))
				undoManager.AddToEndOfFrameExecutionList<LayerChangeEndFrame>(layer, endFrame);

			float startOffset = layer->StartOffset;
			if (Gui::ComfyFloatTextWidget("Start Offset", &startOffset, 1.0f, 10.0f, 0.0f, 0.0f, "%.2f"))
				undoManager.AddToEndOfFrameExecutionList<LayerChangeStartOffset>(layer, startOffset);

			if (layer->ItemType != ItemType::Audio)
			{
				constexpr float percentageFactor = 100.0f;
				float timeScale = layer->TimeScale * percentageFactor;

				if (Gui::ComfyFloatTextWidget("Playback Speed", &timeScale, 1.0f, 10.0f, 0.0f, 0.0f, "%.0f%%"))
					undoManager.AddToEndOfFrameExecutionList<LayerChangeTimeScale>(layer, timeScale / percentageFactor);
			}

			PopDisableItemFlagIfPlayback();
			Gui::TreePop();
		}

		if (layer->ItemType == ItemType::None)
			return;

		if (layer->ItemType == ItemType::Video)
		{
			Gui::Separator();
			DrawInspectorVideoData(scene, layer, layer->GetVideoItem());
		}

		if (layer->ItemType == ItemType::Composition)
		{
			Gui::Separator();
			DrawInspectorCompositionData(scene, layer, layer->GetCompItem());
		}

		if (layer->ItemType == ItemType::Video || layer->ItemType == ItemType::Composition)
		{
			Gui::Separator();
			DrawInspectorAnimationData(layer->LayerVideo, layer);

			// DEBUG: Quick debug view to inspect individual keyframes
			DrawInspectorDebugAnimationData(layer->LayerVideo, layer);
		}

		Gui::Separator();
		DrawInspectorLayerMarkers(layer, &layer->Markers);

		if (layer->ItemType == ItemType::Video || layer->ItemType == ItemType::Composition)
		{
			Gui::Separator();
			DrawInspectorLayerParent(scene, layer);
		}

		Gui::Separator();
	}

	void AetInspector::DrawInspectorVideoData(Scene* scene, const std::shared_ptr<Layer>& layer, const std::shared_ptr<Video>& video)
	{
		if (Gui::WideTreeNodeEx(ICON_AETVIDEOS "  Video Data", DefaultOpenPropertiesNodeFlags))
		{
			if (video != nullptr)
			{
				VideoSource* frontSource = video->GetFront();

				if (frontSource == nullptr)
					sprintf_s(videoDataNameBuffer, "Null (%dx%d)", video->Size.x, video->Size.y);
				else
					CopyStringIntoBuffer(frontSource->Name, videoDataNameBuffer, sizeof(videoDataNameBuffer));
			}

			const char* noSpriteString = "None (Sprite)";

			PushDisableItemFlagIfPlayback();
			if (Gui::ComfyBeginCombo("Sprite", video == nullptr ? noSpriteString : videoDataNameBuffer, ImGuiComboFlags_HeightLarge))
			{
				if (Gui::Selectable(noSpriteString, video == nullptr))
					undoManager.AddToEndOfFrameExecutionList<LayerChangeVideoItem>(layer, nullptr);

				i32 videoIndex = 0;
				for (auto& video : scene->Videos)
				{
					Gui::PushID(video.get());

					bool isSelected = (video == video);

					VideoSource* frontSprite = video->GetFront();
					if (frontSprite == nullptr)
						sprintf_s(videoDataNameBuffer, "Video %d (%dx%d)", videoIndex, video->Size.x, video->Size.y);

					if (Gui::Selectable(frontSprite == nullptr ? videoDataNameBuffer : frontSprite->Name.c_str(), isSelected))
						undoManager.AddToEndOfFrameExecutionList<LayerChangeVideoItem>(layer, video);

					if (Gui::IsItemHovered())
						previewData.Video = video.get();

					if (isSelected)
						Gui::SetItemDefaultFocus();
					Gui::PopID();
					videoIndex++;
				}

				Gui::ComfyEndCombo();
			}

			if (!video->Sources.empty())
			{
				if (auto[tex, spr] = renderer.Aet().GetSprite(*video, 0); (tex != nullptr && spr != nullptr))
				{
					constexpr vec2 size = vec2(100.0f, 100.0f);
					Gui::ImageSprTex(tex, spr, size);
				}
			}

			PopDisableItemFlagIfPlayback();
			Gui::TreePop();
		}
	}

	void AetInspector::DrawInspectorAnimationData(const std::shared_ptr<LayerVideo>& animationData, const std::shared_ptr<Layer>& layer)
	{
		if (Gui::WideTreeNodeEx(ICON_ANIMATIONDATA "  Animation Data", DefaultOpenPropertiesNodeFlags))
		{
			PushDisableItemFlagIfPlayback();

			if (animationData != nullptr)
			{
				Transform2D currentTransform = Aet::Util::GetTransformAt(*animationData, currentFrame);

				animatedPropertyColor = GetColorVec4(EditorColor_AnimatedProperty);
				keyFramePropertyColor = GetColorVec4(EditorColor_KeyFrameProperty);
				staticPropertyColor = Gui::GetStyleColorVec4(ImGuiCol_FrameBg);

				DrawInspectorAnimationDataPropertyVec2(layer, "Origin", currentFrame, currentTransform.Origin, Transform2DField_OriginX, Transform2DField_OriginY);
				DrawInspectorAnimationDataPropertyVec2(layer, "Position", currentFrame, currentTransform.Position, Transform2DField_PositionX, Transform2DField_PositionY);
				DrawInspectorAnimationDataProperty(layer, "Rotation", currentFrame, currentTransform.Rotation, Transform2DField_Rotation);
				DrawInspectorAnimationDataPropertyVec2(layer, "Scale", currentFrame, currentTransform.Scale, Transform2DField_ScaleX, Transform2DField_ScaleY);
				DrawInspectorAnimationDataProperty(layer, "Opacity", currentFrame, currentTransform.Opacity, Transform2DField_Opacity);
			}

			if (layer->ItemType == ItemType::Video)
			{
				if (animationData->GetUseTextureMask())
					Gui::PushItemFlag(ImGuiItemFlags_Disabled, true);

				if (Gui::ComfyBeginCombo("Blend Mode", GetBlendModeName(animationData->TransferMode.BlendMode), ImGuiComboFlags_HeightLarge))
				{
					// NOTE: Increase the count in case of invalid blend modes
					size_t blendModeCount = glm::max(static_cast<size_t>(animationData->TransferMode.BlendMode), EnumCount<AetBlendMode>());

					for (i32 blendModeIndex = 0; blendModeIndex < blendModeCount; blendModeIndex++)
					{
						bool isBlendMode = (static_cast<AetBlendMode>(blendModeIndex) == animationData->TransferMode.BlendMode);
						bool outOfBounds = blendModeIndex >= EnumCount<AetBlendMode>();

						if (!isBlendMode && (outOfBounds || !IsBlendModeSupported(static_cast<AetBlendMode>(blendModeIndex))))
							continue;

						const char* blendModeName = GetBlendModeName(static_cast<AetBlendMode>(blendModeIndex));

						if (Gui::Selectable(blendModeName, isBlendMode))
							undoManager.AddToEndOfFrameExecutionList<AnimationDataChangeBlendMode>(animationData, static_cast<AetBlendMode>(blendModeIndex));

						if (Gui::IsItemHovered())
							previewData.BlendMode = static_cast<AetBlendMode>(blendModeIndex);

						if (isBlendMode)
							Gui::SetItemDefaultFocus();
					};

					Gui::ComfyEndCombo();
				}

				if (animationData->GetUseTextureMask())
					Gui::PopItemFlag();

				bool useTextureMask = animationData->GetUseTextureMask();
				if (Gui::ComfyCheckbox("Use Texture Mask", &useTextureMask))
					undoManager.AddToEndOfFrameExecutionList<AnimationDataChangeUseTextureMask>(animationData, useTextureMask);
			}

			PopDisableItemFlagIfPlayback();
			Gui::TreePop();
		}
	}

	void AetInspector::DrawInspectorDebugAnimationData(const std::shared_ptr<LayerVideo>& animationData, const std::shared_ptr<Layer>& layer)
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
				for (Transform2DField i = 0; i < Transform2DField_Count; i++)
				{
					if (Gui::WideTreeNode(LayerVideo2D::FieldNames[i++]))
					{
						for (auto& keyFrame : animationData->Transform[i].Keys)
							Gui::Text("Frame: %f; Value: %f; Curve: %f", keyFrame.Frame, keyFrame.Value, keyFrame.Curve);

						Gui::TreePop();
					}
				}
			}

			Gui::TreePop();
		}
	}

	void AetInspector::DrawInspectorAnimationDataProperty(const std::shared_ptr<Layer>& layer, const char* label, frame_t frame, float& value, Transform2DField field)
	{
		constexpr float percentFactor = 100.0f;

		assert(layer->LayerVideo.get() != nullptr);
		const auto& animationData = layer->LayerVideo;

		KeyFrame* keyFrame = isPlayback ? nullptr : Aet::Util::GetKeyFrameAt(animationData->Transform[field], frame);

		Gui::PushStyleColor(ImGuiCol_FrameBg, ((animationData->Transform[field]->size() > 1))
			? (keyFrame != nullptr)
			? keyFramePropertyColor
			: animatedPropertyColor
			: staticPropertyColor);

		bool isRotation = (field == Transform2DField_Rotation);
		bool isOpacity = (field == Transform2DField_Opacity);

		const char* formatString = isRotation ? u8"%.2f Åã" : isOpacity ? "%.2f%%" : "%.2f";
		float min = 0.0f, max = 0.0f;

		if (isOpacity)
		{
			value *= percentFactor;
			max = 1.0f * percentFactor;
		}

		if (Gui::ComfyFloatTextWidget(label, &value, 1.0f, 10.0f, min, max, formatString, ImGuiInputTextFlags_None, (keyFrame == nullptr)))
		{
			if (isOpacity)
				value = glm::clamp(value * (1.0f / percentFactor), 0.0f, 1.0f);

			auto tuple = std::make_tuple(static_cast<Transform2DField_Enum>(field), frame, value);
			undoManager.AddToEndOfFrameExecutionList<AnimationDataChangeKeyFrameValue>(layer, tuple);
		}

		Gui::PopStyleColor();
	}

	void AetInspector::DrawInspectorAnimationDataPropertyVec2(const std::shared_ptr<Layer>& layer, const char* label, frame_t frame, vec2& value, Transform2DField fieldX, Transform2DField fieldY)
	{
		constexpr float percentFactor = 100.0f;

		assert(layer->LayerVideo.get() != nullptr);
		const auto& animationData = layer->LayerVideo;

		const KeyFrame* keyFrameX = isPlayback ? nullptr : Aet::Util::GetKeyFrameAt(animationData->Transform[fieldX], frame);
		const KeyFrame* keyFrameY = isPlayback ? nullptr : Aet::Util::GetKeyFrameAt(animationData->Transform[fieldY], frame);

		Gui::PushStyleColor(ImGuiCol_FrameBg, ((animationData->Transform[fieldX]->size() > 1) || (animationData->Transform[fieldY]->size() > 1))
			? (keyFrameX != nullptr || keyFrameY != nullptr)
			? keyFramePropertyColor
			: animatedPropertyColor
			: staticPropertyColor);

		const bool isScale = (fieldX == Transform2DField_ScaleX) && (fieldY == Transform2DField_ScaleY);
		const char* formatString = isScale ? "%.2f%%" : "%.2f";

		vec2 previousValue = value;
		if (isScale)
			value *= percentFactor;

		bool disabledText[2] =
		{
			keyFrameX == nullptr,
			keyFrameY == nullptr,
		};

		if (Gui::ComfyFloat2TextWidget(label, glm::value_ptr(value), 1.0f, 0.0f, 0.0f, formatString, ImGuiInputTextFlags_None, disabledText))
		{
			if (isScale)
				value *= (1.0f / percentFactor);

			// NOTE: Should this check for a value change or should this always be adding X and Y KeyFrames at once? 
			//		 The Transform and Move tool will already add two add once so this seems fine

			if (value.x != previousValue.x)
			{
				auto tuple = std::make_tuple(static_cast<Transform2DField_Enum>(fieldX), frame, value.x);
				undoManager.AddToEndOfFrameExecutionList<AnimationDataChangeKeyFrameValue>(layer, tuple);
			}
			if (value.y != previousValue.y)
			{
				auto tuple = std::make_tuple(static_cast<Transform2DField_Enum>(fieldY), frame, value.y);
				undoManager.AddToEndOfFrameExecutionList<AnimationDataChangeKeyFrameValue>(layer, tuple);
			}
		}

		Gui::PopStyleColor();
	}

	void AetInspector::DrawInspectorLayerMarkers(const std::shared_ptr<Layer>& layer, std::vector<std::shared_ptr<Marker>>* markers)
	{
		if (Gui::WideTreeNodeEx(ICON_MARKERS "  Markers", DefaultOpenPropertiesNodeFlags))
		{
			for (int i = 0; i < markers->size(); i++)
			{
				const std::shared_ptr<Marker>& marker = markers->at(i);

				Gui::PushID(marker.get());

				vec2 treeNodeCursorPos = Gui::GetCursorScreenPos();
				bool open = Gui::WideTreeNode("##AetInspectorMarker");

				Gui::ItemContextMenu("AddMarkerContextMenu##AetInspector", [this, &layer, &markers, i]()
				{
					if (Gui::MenuItem(ICON_MOVEUP "  Move Up", nullptr, nullptr, i > 0))
					{
						auto tuple = std::tuple<int, int>(i, i - 1);
						undoManager.AddToEndOfFrameExecutionList<LayerMoveMarker>(layer, tuple);
					}

					if (Gui::MenuItem(ICON_MOVEDOWN "  Move Down", nullptr, nullptr, i < markers->size() - 1))
					{
						auto tuple = std::tuple<int, int>(i, i + 1);
						undoManager.AddToEndOfFrameExecutionList<LayerMoveMarker>(layer, tuple);
					}

					if (Gui::MenuItem(ICON_DELETE "  Delete"))
					{
						undoManager.AddToEndOfFrameExecutionList<LayerDeleteMarker>(layer, i);
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
						undoManager.AddToEndOfFrameExecutionList<LayerChangeMarkerFrame>(marker, frame);

					CopyStringIntoBuffer(marker->Name, markerNameBuffer, sizeof(markerNameBuffer));
					if (Gui::ComfyTextWidget("Name", markerNameBuffer, sizeof(markerNameBuffer)))
						undoManager.AddToEndOfFrameExecutionList<LayerChangeMarkerName>(marker, markerNameBuffer);

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
				auto newMarker = std::make_shared<Marker>(0.0f, newMarkerNameBuffer);
				undoManager.AddToEndOfFrameExecutionList<LayerAddMarker>(layer, newMarker);
			}
			PopDisableItemFlagIfPlayback();

			Gui::TreePop();
		}
	}

	bool IsAnyParentRecursive(const Layer* newLayer, const Layer* layer)
	{
		// TODO: Recursively check parent and maybe move this function into the AetMgr
		return (newLayer->GetRefParentLayer() != nullptr && newLayer->GetRefParentLayer() == layer);
	}

	void AetInspector::DrawInspectorLayerParent(Scene* aet, const std::shared_ptr<Layer>& layer)
	{
		if (Gui::WideTreeNodeEx(ICON_PARENT "  Parent", DefaultOpenPropertiesNodeFlags))
		{
			Layer* parentLayer = layer->GetRefParentLayer().get();
			Composition* parentComp = layer->GetParentComposition();

			constexpr const char* noParentString = "None (Parent)";
			PushDisableItemFlagIfPlayback();

			if (Gui::ComfyBeginCombo("Parent Layer", parentLayer == nullptr ? noParentString : parentLayer->GetName().c_str(), ImGuiComboFlags_HeightLarge))
			{
				if (Gui::Selectable(noParentString, parentLayer == nullptr))
					undoManager.AddToEndOfFrameExecutionList<LayerChangeReferencedParentLayer>(layer, nullptr);

				for (i32 layerIndex = 0; layerIndex < parentComp->GetLayers().size(); layerIndex++)
				{
					const std::shared_ptr<Layer>& iteratorLayer = parentComp->GetLayers().at(layerIndex);
					bool isSelected = (iteratorLayer.get() == parentLayer);

					bool isSame = (layer.get() == iteratorLayer.get());
					bool isAnyRecursive = IsAnyParentRecursive(iteratorLayer.get(), layer.get());

					Gui::PushID(iteratorLayer.get());
					if (Gui::Selectable(iteratorLayer->GetName().c_str(), isSelected, (isSame || isAnyRecursive) ? ImGuiSelectableFlags_Disabled : ImGuiSelectableFlags_None))
						undoManager.AddToEndOfFrameExecutionList<LayerChangeReferencedParentLayer>(layer, iteratorLayer);

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

	void AetInspector::DrawInspectorVideo(Scene* aet, const std::shared_ptr<Video>& video)
	{
		Gui::InputInt2("Dimensions", glm::value_ptr(video->Size));

		ImVec4 color = Gui::ColorConvertU32ToFloat4(video->Color);
		if (Gui::ColorEdit3("Background##AetVideoColor", (float*)&color, ImGuiColorEditFlags_DisplayHex))
			video->Color = Gui::ColorConvertFloat4ToU32(color);

		if (Gui::WideTreeNodeEx("Sprites:", DefaultOpenPropertiesNodeFlags))
		{
			for (auto& source : video->Sources)
			{
				sprintf_s(spriteNameBuffer, ICON_AETVIDEO "  %s", source.Name.c_str());
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
