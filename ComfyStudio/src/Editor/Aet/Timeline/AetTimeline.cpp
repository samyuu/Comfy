#include "AetTimeline.h"
#include "Editor/Aet/AetEditor.h"
#include "Editor/Aet/AetIcons.h"

namespace Comfy::Studio::Editor
{
	using namespace Graphics;
	using namespace Graphics::Aet;

	static_assert(sizeof(Transform2DField_Enum) == sizeof(i32) && sizeof(KeyFrameIndex::Pair) == sizeof(KeyFrameIndex::PackedValue));

	AetTimeline::AetTimeline()
	{
		// NOTE: Around 240.0f seems to be the standard for many programs
		constexpr f32 fixedInfoColumnWidth = 240.0f;
		infoColumnWidth = fixedInfoColumnWidth;

		infoColumnScrollStep = rowItemHeight * 3.0f;

		constexpr f32 defaultZoomLevel = 5.0f;
		zoomLevel = defaultZoomLevel;
	}

	AetTimeline::~AetTimeline()
	{
	}

	void AetTimeline::SetActive(AetItemTypePtr value)
	{
		selectedAetItem = value;

		constexpr f32 defaultFrameRate = 60.0f;
		const Scene* parentAet = value.GetItemParentScene();

		frameRate = (parentAet != nullptr) ? parentAet->FrameRate : defaultFrameRate;
	}

	bool AetTimeline::GetIsPlayback() const
	{
		return isPlayback;
	}

	f32 AetTimeline::GetTimelineSize() const
	{
		return GetTimelinePosition(loopEndFrame) + timelineContentMarginWidth;
	}

	f32 AetTimeline::GetTimelineHeight() const
	{
		const Composition* workingComp = GetWorkingComposition();
		if (workingComp == nullptr)
			return 0.0f;

		i32 rowCount = static_cast<i32>(workingComp->GetLayers().size());
		for (const auto& layer : workingComp->GetLayers())
		{
			if (layer->GuiData.TimelineNodeOpen)
				rowCount += Transform2DField_Count;
		}

		constexpr f32 borderSize = 1.0f;
		const f32 horizontalSize = rowCount * rowItemHeight - regions.InfoColumnContent.GetHeight() + borderSize;

		return (horizontalSize < 0.0f) ? 0.0f : horizontalSize;
	}

	void AetTimeline::DrawTimelineContentNone()
	{
		return;
	}

	void AetTimeline::DrawTimelineContent()
	{
		Gui::PushClipRect(regions.Content.GetTL(), regions.Content.GetBR(), true);
		keyFrameRenderer.DrawContent(*this, GetWorkingComposition());
		Gui::PopClipRect();
	}

	void AetTimeline::OnDrawTimelineHeaderWidgets()
	{
		return;
	}

	void AetTimeline::OnDrawTimelineInfoColumnHeader()
	{
		TimelineBase::OnDrawTimelineInfoColumnHeader();

		// TODO: This does not work with all style configurations
		assert(infoColumnWidth == 240.0f);

		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(0.0f, 0.0f));
		Gui::PushStyleVar(ImGuiStyleVar_FramePadding, vec2(8.0f, 8.0f));

		constexpr vec4 transparent = vec4(0.0f);
		Gui::PushStyleColor(ImGuiCol_Button, transparent);
		{
			const f32 timelineStartPosition = GetTimelinePosition(loopStartFrame - 1.0f);
			const f32 timelineEndPosition = GetTimelinePosition(loopEndFrame + 1.0f) - regions.Content.GetWidth() + 1.0f;

			const bool isFirstFrame = (cursorTime <= GetTimelineTime(loopStartFrame));
			const bool isLastFrame = (cursorTime >= GetTimelineTime(loopEndFrame));
			const bool isPlayback = GetIsPlayback();

			constexpr f32 borderSize = 1.0f;
			Gui::SetCursorPosX(Gui::GetCursorPosX() + borderSize);

			// NOTE: First frame button
			{
				Gui::PushItemDisabledAndTextColorIf(isFirstFrame || isPlayback);
				if (Gui::Button(ICON_FA_FAST_BACKWARD))
				{
					cursorTime = GetTimelineTime(loopStartFrame);
					RoundCursorTimeToNearestFrame();
					SetScrollTargetX(timelineStartPosition);
				}
				Gui::PopItemDisabledAndTextColorIf(isFirstFrame || isPlayback);
				Gui::SetWideItemTooltip("Go to first frame");
			}

			// NOTE: Previous frame button
			{
				Gui::SameLine();
				Gui::PushItemDisabledAndTextColorIf(isFirstFrame || isPlayback);
				if (Gui::Button(ICON_FA_BACKWARD))
				{
					cursorTime = GetTimelineTime(GetCursorFrame() - 1.0f);
					RoundCursorTimeToNearestFrame();
				}
				Gui::PopItemDisabledAndTextColorIf(isFirstFrame || isPlayback);
				Gui::SetWideItemTooltip("Go to previous frame");
			}

			// NOTE: Playback toggle button
			{
				Gui::SameLine();
				if (Gui::Button(isPlayback ? ICON_FA_PAUSE : ICON_FA_PLAY))
				{
					isPlayback ? PausePlayback() : ResumePlayback();
				}
				Gui::SetWideItemTooltip("Toggle playback");
			}

			// NOTE: Playback stop button
			{
				Gui::SameLine();
				Gui::PushItemDisabledAndTextColorIf(!isPlayback && isFirstFrame);
				if (Gui::Button(ICON_FA_STOP))
				{
					StopPlayback();
					SetScrollTargetX(timelineStartPosition);
				}
				Gui::PopItemDisabledAndTextColorIf(!isPlayback && isFirstFrame);
				Gui::SetWideItemTooltip("Stop playback");
			}

			// NOTE: Next frame button
			{
				Gui::SameLine();
				Gui::PushItemDisabledAndTextColorIf(isLastFrame || isPlayback);
				if (Gui::Button(ICON_FA_FORWARD))
				{
					cursorTime = GetTimelineTime(GetCursorFrame() + 1);
					RoundCursorTimeToNearestFrame();
				}
				Gui::PopItemDisabledAndTextColorIf(isLastFrame || isPlayback);
				Gui::SetWideItemTooltip("Go to next frame");
			}

			// NOTE: Last frame button
			{
				Gui::SameLine();
				Gui::PushItemDisabledAndTextColorIf(isLastFrame || isPlayback);
				if (Gui::Button(ICON_FA_FAST_FORWARD))
				{
					cursorTime = GetTimelineTime(loopEndFrame - 1.0f);
					RoundCursorTimeToNearestFrame();
					SetScrollTargetX(timelineEndPosition);
				}
				Gui::PopItemDisabledAndTextColorIf(isLastFrame || isPlayback);
				Gui::SetWideItemTooltip("Go to last frame");
			}

			// TODO: Filler button for now, what functionality should go here?
			{
				Gui::SameLine();
				Gui::Button(ICON_FA_ADJUST);
				Gui::SetWideItemTooltip("???");
			}

			// NOTE: Settings button
			{
				Gui::SameLine();
				if (Gui::Button(ICON_FA_COG))
					Gui::OpenPopup(settingsPopupName);
				Gui::SetWideItemTooltip("Timeline settings");
			}
		}
		Gui::PopStyleColor(1);
		Gui::PopStyleVar(2);

		// NOTE: Settings popup
		if (Gui::WideBeginPopup(settingsPopupName))
		{
			constexpr f32 percentageFactor = 100.0f;

			// TODO: Come up with a neat comfy layout
			Gui::Text("TODO:");

			f32 playbackSpeedPercentage = playbackSpeedFactor * percentageFactor;
			if (Gui::SliderFloat("Playback Speed", &playbackSpeedPercentage, (playbackSpeedMin * percentageFactor), (playbackSpeedMax * percentageFactor), "%.2f %%"))
				playbackSpeedFactor = playbackSpeedPercentage * (1.0f / percentageFactor);

			Gui::Checkbox("Loop Animation", &loopPlayback);

			Gui::EndPopup();
		}
	}

	void AetTimeline::OnDrawTimelineInfoColumn()
	{
		TimelineBase::OnDrawTimelineInfoColumn();

		if (selectedAetItem.Type() != AetItemType::Layer && selectedAetItem.Type() != AetItemType::Composition)
			return;

		const Composition* workingComp = GetWorkingComposition();
		const Layer* selectedLayer = (selectedAetItem.Type() == AetItemType::Layer) ? selectedAetItem.Ptrs.Layer : nullptr;

		Gui::PushClipRect(regions.InfoColumnContent.GetTL(), regions.InfoColumnContent.GetBR(), true);
		DrawTimelineInfoColumnComposition(workingComp, selectedLayer);
		Gui::PopClipRect();
	}

	void AetTimeline::DrawTimelineInfoColumnComposition(const Composition* workingComp, const Layer* selectedLayer) const
	{
		if (workingComp == nullptr)
			return;

#if 1 // DEBUG: Should be controlled by tree node expansion arrows
		for (auto& layer : workingComp->GetLayers())
			layer->GuiData.TimelineNodeOpen = (layer.get() == selectedLayer);
#endif

		// TODO: Same behavior for AetLayer and AetComposition.
		//		 Each layer should have a name row with the StartFrame and EndFrame
		//		 as well as a collapsable tree node for the transform properties
		//		 the "tree nodes" on on info column should be clickable (draggable to reorder in the future)
		//		 and reflect the selected layer state

		auto drawRowSeparator = [this](i32 index)
		{
			const vec2 yOffset = vec2(0.0f, index * rowItemHeight - GetScrollY());
			Gui::GetWindowDrawList()->AddLine(regions.InfoColumnContent.GetTL() + yOffset, regions.InfoColumnContent.GetTR() + yOffset, Gui::GetColorU32(ImGuiCol_Border));
		};

		auto drawRowText = [this](i32 index, const char* text, f32 xTextOffset = 0.0f)
		{
			constexpr f32 startPadding = 4.0f;
			constexpr f32 textPadding = 1.0f;

			const vec2 position = vec2(startPadding + xTextOffset, index * rowItemHeight - GetScrollY() + textPadding) + regions.InfoColumnContent.GetTL();

			Gui::GetWindowDrawList()->AddText(position, Gui::GetColorU32(ImGuiCol_Text), text);
		};

		// TODO: Theses should probably be member functions
		auto drawLayer = [&](i32 rowIndex, const std::shared_ptr<Layer>& layer)
		{
			// TODO: Adjust spacing and implement tree node expansion arrow
			constexpr f32 typeIconDistance = 20.0f;

			vec2 position = vec2(GImGui->Style.FramePadding.x, rowIndex * rowItemHeight - GetScrollY() + 1.0f) + regions.InfoColumnContent.GetTL();

			if (layer.get() == selectedLayer)
			{
				ImRect selectedRegion = ImRect(position - vec2(GImGui->Style.FramePadding.x, 0.0f), position + vec2(infoColumnWidth - GImGui->Style.FramePadding.x, rowItemHeight));
				selectedRegion.Min += vec2(1.0f, 0.0f);
				selectedRegion.Max -= vec2(1.0f, 1.0f);

				Gui::GetWindowDrawList()->AddRectFilled(selectedRegion.Min, selectedRegion.Max, GetColor(EditorColor_TreeViewActive));
			}

			Gui::RenderArrow(Gui::GetWindowDrawList(), position + vec2(0.0f, GImGui->FontSize * 0.15f), Gui::GetColorU32(ImGuiCol_Text), layer->GuiData.TimelineNodeOpen ? ImGuiDir_Down : ImGuiDir_Right, 0.70f);
			position.x += GImGui->FontSize + GImGui->Style.ItemSpacing.x - 5.0f;

			Gui::GetWindowDrawList()->AddText(position, Gui::GetColorU32(ImGuiCol_Text), GetItemTypeIcon(layer->ItemType));
			position.x += typeIconDistance;

			Gui::GetWindowDrawList()->AddText(position, Gui::GetColorU32(ImGuiCol_Text), layer->GetName().c_str());
		};

		auto drawLayerTransformProperties = [&](i32& rowIndex, const std::shared_ptr<Layer>& layer)
		{
			for (Transform2DField i = 0; i < Transform2DField_Count; i++)
			{
				const auto[type, name] = timelinePropertyTypeNames[i];

				// NOTE: Results in a 5 pixel spacing
				constexpr f32 nameTypeDistance = 58.0f;
				constexpr f32 nameTypeSeparatorSpacing = 8.0f;
				constexpr f32 textOffset = 39.0f - 20.0f;

				f32 x = textOffset;
				drawRowText(rowIndex, type, x);

				x += nameTypeDistance;
				drawRowText(rowIndex, timelinePropertyNameTypeSeparator, x);

				x += nameTypeSeparatorSpacing;
				drawRowText(rowIndex, name, x);

				drawRowSeparator(++rowIndex);
			}
		};

		i32 rowIndex = 0;
		for (const auto& layer : workingComp->GetLayers())
		{
			drawLayer(rowIndex, layer);
			drawRowSeparator(++rowIndex);

			if (!layer->GuiData.TimelineNodeOpen)
				continue;

			drawLayerTransformProperties(rowIndex, layer);
		}
	}

	const Composition* AetTimeline::GetWorkingComposition() const
	{
		switch (selectedAetItem.Type())
		{
		case AetItemType::Layer:
			return selectedAetItem.Ptrs.Layer->GetParentComposition();

		case AetItemType::Composition:
			return selectedAetItem.Ptrs.Composition;

		default:
			return nullptr;
		}
	}

	i32 AetTimeline::GetTimelineRowCount() const
	{
		const Composition* workingComp = GetWorkingComposition();

		if (workingComp == nullptr)
			return 1;

		i32 rowCount = static_cast<i32>(workingComp->GetLayers().size());
		{
			for (const auto& layer : workingComp->GetLayers())
			{
				if (layer->GuiData.TimelineNodeOpen)
					rowCount += Transform2DField_Count;
			}
		}
		return rowCount;
	}

	void AetTimeline::OnDrawTimlineRows()
	{
		if (selectedAetItem.IsNull() || currentTimelineMode != TimelineMode::DopeSheet)
			return;

		const ImU32 rowColor = GetColor(EditorColor_TimelineRowSeparator);
		const f32 scrollY = GetScrollY();

		Gui::PushClipRect(regions.Content.GetTL(), regions.Content.GetBR(), true);

		const i32 rowCount = GetTimelineRowCount();
		for (i32 i = 0; i <= rowCount; i++)
		{
			const f32 y = i * rowItemHeight - scrollY;
			const vec2 start = regions.Content.GetTL() + vec2(0.0f, y);
			const vec2 end = start + vec2(regions.Content.GetWidth(), 0.0f);

			baseWindowDrawList->AddLine(start, end, rowColor);
		}

		Gui::PopClipRect();
	}

	void AetTimeline::OnDrawTimlineDivisors()
	{
		FrameTimeline::OnDrawTimlineDivisors();
	}

	void AetTimeline::OnDrawTimlineBackground()
	{
	}

	void AetTimeline::OnDrawTimelineScrollBarRegion()
	{
		constexpr f32 timeDragTextOffset = 10.0f;
		constexpr f32 timeDragTextWidth = 60.0f + 26.0f;
		Gui::SetCursorPosX(Gui::GetCursorPosX() + timeDragTextOffset);

		Gui::PushStyleVar(ImGuiStyleVar_FramePadding, vec2(Gui::GetStyle().FramePadding.x, 0.0f));

		// NOTE: Time drag text
		{
			f32 cursorFrame = GetCursorFrame().Frames();
			if (Gui::ComfyDragText("TimeDragText::AetTimeline", cursorTime.FormatTime().data(), &cursorFrame, 1.0f, 0.0f, 0.0f, timeDragTextWidth))
			{
				cursorTime = GetTimelineTime(std::clamp(TimelineFrame(cursorFrame), loopStartFrame, loopEndFrame));
				RoundCursorTimeToNearestFrame();
			}

			Gui::SetWideItemTooltip("Frame: %.2f (%.2f fps)", GetCursorFrame().Frames(), frameRate);
		}

		// NOTE: Mode buttons (Dopesheet / Curves)
		{
			const auto modeButton = [this](const char* label, const TimelineMode mode)
			{
				Gui::PushStyleColor(ImGuiCol_Button, Gui::GetStyleColorVec4(currentTimelineMode == mode ? ImGuiCol_ButtonHovered : ImGuiCol_Button));
				{
					constexpr f32 modeButtonWidth = 72.0f;
					if (Gui::Button(label, vec2(modeButtonWidth, scrollbarSize.y)))
						currentTimelineMode = mode;
				}
				Gui::PopStyleColor(1);
			};

			Gui::SameLine();
			modeButton("Dope Sheet", TimelineMode::DopeSheet);
			Gui::SameLine();
			modeButton("Curves", TimelineMode::Curves);
		}

		Gui::PopStyleVar(1);
	}

	static frame_t GetCompositionLastFrame(const Composition* comp)
	{
		frame_t lastFrame = 0;
		for (auto& layer : comp->GetLayers())
		{
			if (layer->EndFrame > lastFrame)
				lastFrame = layer->EndFrame;
		}
		return lastFrame;
	}

	void AetTimeline::OnUpdate()
	{
		if (!selectedAetItem.IsNull())
		{
			const auto parentAet = selectedAetItem.GetItemParentScene();
			const auto itemType = selectedAetItem.Type();

			if (itemType == AetItemType::Scene || (itemType == AetItemType::Composition && selectedAetItem.GetCompositionRef()->IsRootComposition()))
			{
				loopStartFrame = parentAet->StartFrame;
				loopEndFrame = parentAet->EndFrame;
			}
			else if (itemType == AetItemType::Video)
			{
				loopStartFrame = 0.0f;
				loopEndFrame = glm::max(0.0f, static_cast<f32>(selectedAetItem.Ptrs.Video->Sources.size()) - 1.0f);
			}
			else if (const Composition* workingComp = GetWorkingComposition(); itemType == AetItemType::Composition || itemType == AetItemType::Layer && workingComp != nullptr)
			{
				loopStartFrame = 0.0f;
				loopEndFrame = GetCompositionLastFrame(workingComp);
			}
			else
			{
				loopStartFrame = 0.0f;
				loopEndFrame = 0.0f;
			}
		}
		else
		{
			loopStartFrame = 0.0f;
			loopEndFrame = 0.0f;
		}

		if (GetIsPlayback())
		{
			UpdateCursorPlaybackTime();
		}

		const TimeSpan startTime = GetTimelineTime(loopStartFrame);
		const TimeSpan endTime = GetTimelineTime(loopEndFrame);

		if (cursorTime < startTime)
			cursorTime = startTime;

		if (cursorTime > endTime)
		{
			if (loopPlayback)
			{
				cursorTime = startTime;
				SetScrollTargetX(0.0f);
			}
			else
			{
				cursorTime = endTime;
			}
		}
	}

	void AetTimeline::OnUpdateInput()
	{
		UpdateInputCursorClick();
	}

	void AetTimeline::OnDrawTimelineContents()
	{
		if (currentTimelineMode != TimelineMode::DopeSheet)
			return;

		if (selectedAetItem.Type() == AetItemType::None || selectedAetItem.IsNull())
		{
			DrawTimelineContentNone();
		}
		else
		{
			switch (selectedAetItem.Type())
			{
			case AetItemType::AetSet:
			case AetItemType::Scene:
			case AetItemType::Video:
				DrawTimelineContentNone();
				break;

			case AetItemType::Layer:
			case AetItemType::Composition:
				DrawTimelineContent();
				break;

			default:
				break;
			}
		}

		// NOTE: Draw selection region
		if (const auto& selectionData = timelineController.GetSelectionData(); selectionData.IsSelected())
			DrawMouseSelection(selectionData);
	}

	void AetTimeline::PausePlayback()
	{
		isPlayback = false;
		RoundCursorTimeToNearestFrame();
	}

	void AetTimeline::ResumePlayback()
	{
		isPlayback = true;
	}

	void AetTimeline::StopPlayback()
	{
		if (GetIsPlayback())
			PausePlayback();

		cursorTime = GetTimelineTime(loopStartFrame);
	}

	void AetTimeline::UpdateInputCursorClick()
	{
		timelineController.UpdateInput();

		if (timelineController.GetUpdateCursorTime())
			cursorTime = timelineController.GetNewCursorTime();
	}

	void AetTimeline::DrawMouseSelection(const MouseSelectionData& selectionData)
	{
		const bool horizontalSelection = glm::abs((selectionData.StartX - selectionData.EndX).Frames()) > 1.0f;
		const bool verticalSelection = glm::abs(selectionData.RowStartIndex - selectionData.RowEndIndex) > 1;

		if (horizontalSelection || verticalSelection)
		{
			const f32 offset = regions.Content.Min.x - GetScrollX();

			const f32 direction = selectionData.StartX < selectionData.EndX ? +1.0f : -1.0f;
			const f32 padding = 5.0f * direction;

			ImRect selectionRegion = ImRect(
				vec2(glm::round(offset + GetTimelinePosition(selectionData.StartX) - padding), GetRowScreenY(selectionData.RowStartIndex)),
				vec2(glm::round(offset + GetTimelinePosition(selectionData.EndX) + padding), GetRowScreenY(selectionData.RowEndIndex)));

			Gui::PushClipRect(regions.Content.GetTL(), regions.Content.GetBR(), true);
			ImDrawList* windowDrawList = Gui::GetWindowDrawList();
			windowDrawList->AddRectFilled(selectionRegion.Min, selectionRegion.Max, GetColor(EditorColor_TimelineSelection));
			windowDrawList->AddRect(selectionRegion.Min, selectionRegion.Max, GetColor(EditorColor_TimelineSelectionBorder));
			Gui::PopClipRect();
		}
	}

	void AetTimeline::UpdateCursorPlaybackTime()
	{
		const auto& io = Gui::GetIO();
		cursorTime += TimeSpan::FromSeconds(io.DeltaTime * playbackSpeedFactor);
	}

	void AetTimeline::RoundCursorTimeToNearestFrame()
	{
		const frame_t cursorFrames = GetCursorFrame().Frames();
		const frame_t roundedCursorFrames = glm::round(cursorFrames);
		cursorTime = GetTimelineTime(TimelineFrame(roundedCursorFrames));
	}

	f32 AetTimeline::GetRowScreenY(i32 index) const
	{
		return (regions.Content.Min.y + static_cast<f32>(index * rowItemHeight)) - GetScrollY();
	}

	i32 AetTimeline::GetRowIndexFromScreenY(f32 screenY) const
	{
		return glm::max(0, static_cast<i32>(glm::floor((screenY - regions.Content.Min.y + GetScrollY()) / rowItemHeight)));
	}
}
