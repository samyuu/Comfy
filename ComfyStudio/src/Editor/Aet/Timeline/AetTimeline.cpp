#include "AetTimeline.h"
#include "Editor/Aet/AetEditor.h"
#include "Editor/Aet/AetIcons.h"

namespace Editor
{
	static_assert(sizeof(PropertyType_Enum) == sizeof(int32_t) && sizeof(KeyFrameIndex::Pair) == sizeof(KeyFrameIndex::PackedValue));

	AetTimeline::AetTimeline()
	{
		infoColumnWidth = 140.0f;
		rowHeight = 18.0f;
		zoomLevel = 5.0f;
	}

	AetTimeline::~AetTimeline()
	{
	}

	void AetTimeline::SetActive(AetItemTypePtr value)
	{
		selectedAetItem = value;

		constexpr float defaultFrameRate = 60.0f;
		const Aet* parentAet = value.GetItemParentAet();

		frameRate = (parentAet != nullptr) ? parentAet->FrameRate : defaultFrameRate;
	}

	bool AetTimeline::GetIsPlayback() const
	{
		return isPlayback;
	}

	float AetTimeline::GetTimelineSize() const
	{
		return GetTimelinePosition(loopEndFrame) + timelineContentMarginWidth;
	}

	void AetTimeline::DrawTimelineContentNone()
	{
		ImU32 dimColor = Gui::GetColorU32(ImGuiCol_PopupBg, 0.25f);
		Gui::GetWindowDrawList()->AddRectFilled(timelineBaseRegion.GetTL(), timelineBaseRegion.GetBR(), dimColor);
	}

	void AetTimeline::DrawTimelineContentLayer()
	{
		keyFrameRenderer.DrawLayerObjects(this, selectedAetItem.GetAetLayerRef(), GetCursorFrame().Frames());
	}

	void AetTimeline::DrawTimelineContentObject()
	{
		const auto& aetObj = selectedAetItem.GetAetObjRef();
		if (aetObj->AnimationData == nullptr)
			return;

		keyFrameRenderer.DrawKeyFrames(this, aetObj->AnimationData->Properties);
	}

	void AetTimeline::OnDrawTimelineHeaderWidgets()
	{
		constexpr float percentageFactor = 100.0f;

		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(2.0f, 0.0f));

		ImGuiStyle& style = Gui::GetStyle();
		Gui::PushStyleVar(ImGuiStyleVar_FramePadding, vec2(8.0f, style.FramePadding.y));

		cursorTime.FormatTime(timeInputBuffer, sizeof(timeInputBuffer));
		size_t timeLength = strlen(timeInputBuffer);
		sprintf_s(&timeInputBuffer[timeLength], sizeof(timeInputBuffer) - timeLength, " (%.f/%.f)", GetTimelineFrame(cursorTime).Frames(), loopEndFrame.Frames());

		constexpr float timeWidgetWidth = 138;
		Gui::PushItemWidth(timeWidgetWidth);
		Gui::InputTextWithHint("##AetTimeline::TimeInput", "00:00.000", timeInputBuffer, sizeof(timeInputBuffer));
		Gui::PopItemWidth();

		Gui::SameLine();
		Gui::Button(ICON_FA_FAST_BACKWARD);
		if (Gui::IsItemActive()) { scrollDelta -= io->DeltaTime * 1000.0f; }

		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(0.0f, 0.0f));

		// TODO: jump to last / next keyframe
		Gui::SameLine();
		Gui::Button(ICON_FA_BACKWARD);
		if (Gui::IsItemActive()) { scrollDelta -= io->DeltaTime * 400.0f; }

		Gui::SameLine();

		if (GetIsPlayback())
		{
			if (Gui::Button(ICON_FA_PAUSE))
				PausePlayback();
		}
		else
		{
			if (Gui::Button(ICON_FA_PLAY))
				ResumePlayback();
		}

		Gui::SameLine();
		if (Gui::Button(ICON_FA_STOP))
		{
			StopPlayback();
			scrollDelta = -GetTimelineSize();
		}

		Gui::SameLine();
		Gui::Button(ICON_FA_FORWARD);
		if (Gui::IsItemActive()) { scrollDelta += io->DeltaTime * 400.0f; }

		Gui::SameLine();
		Gui::Button(ICON_FA_FAST_FORWARD);
		if (Gui::IsItemActive()) { scrollDelta += io->DeltaTime * 1000.0f; }

		Gui::PopStyleVar(2);

		Gui::SameLine();
		Gui::PushItemWidth(120.0f);
		{
			float zoomPercentage = zoomLevel * percentageFactor;

			if (Gui::SliderFloat(ICON_FA_SEARCH, &zoomPercentage, ZOOM_MIN * percentageFactor, ZOOM_MAX * percentageFactor, "%.2f %%"))
				zoomLevel = zoomPercentage * (1.0f / percentageFactor);

			if (Gui::IsItemHoveredDelayed())
				Gui::WideSetTooltip("Zoom Level");
		}
		Gui::PopItemWidth();

		Gui::SameLine();
		Gui::PushItemWidth(120.0f);
		{
			float playbackSpeedPercentage = playbackSpeedFactor * percentageFactor;

			if (Gui::SliderFloat(ICON_FA_CLOCK, &playbackSpeedPercentage, 1.0f, 400.0f, "%.2f %%"))
				playbackSpeedFactor = playbackSpeedPercentage * (1.0f / percentageFactor);

			if (Gui::IsItemHoveredDelayed())
				Gui::WideSetTooltip("Playback Speed");
		}
		Gui::PopItemWidth();

		Gui::PopStyleVar(1);

		Gui::SameLine();
		Gui::Checkbox("Loop Animation", &loopPlayback);
	}

	void AetTimeline::OnDrawTimelineInfoColumnHeader()
	{
		TimelineBase::OnDrawTimelineInfoColumnHeader();
	}

	void AetTimeline::OnDrawTimelineInfoColumn()
	{
		TimelineBase::OnDrawTimelineInfoColumn();

		constexpr float textPadding = 1.0f;
		constexpr float startPadding = 4.0f;

		ImDrawList* drawList = Gui::GetWindowDrawList();
		ImU32 textColor = Gui::GetColorU32(ImGuiCol_Text);

		if (selectedAetItem.IsNull() || selectedAetItem.Type() == AetItemType::Aet)
		{
			// textColor = Gui::GetColorU32(ImGuiCol_TextDisabled);
			// drawList->AddText(infoColumnRegion.GetTL() + vec2(8, 2), textColor, "Select an Object...");
		}
		else if (selectedAetItem.Type() == AetItemType::AetObj)
		{
			if (selectedAetItem.GetAetObjRef()->Type == AetObjType::Aif)
				textColor = Gui::GetColorU32(ImGuiCol_TextDisabled);

			// NOTE: Results in a 5 pixel spacing
			constexpr float nameTypeDistance = 58.0f;
			constexpr float nameTypeSeparatorSpacing = 8.0f;

			// NOTE: Draw Text
			for (int i = 0; i < PropertyType_Count; i++)
			{
				const float y = i * rowHeight;
				vec2 start = vec2(startPadding, y + textPadding) + infoColumnRegion.GetTL();

				drawList->AddText(start, textColor, timelinePropertyNameTypes[i]);
				start.x += nameTypeDistance;
				drawList->AddText(start, textColor, timelinePropertyNameTypeSeparator);
				start.x += nameTypeSeparatorSpacing;
				drawList->AddText(start, textColor, timelinePropertyNames[i]);
			}

			// NOTE: Draw Separators
			for (int i = 0; i <= PropertyType_Count; i++)
			{
				const float y = i * rowHeight;
				const vec2 topLeftSeparator = infoColumnRegion.GetTL() + vec2(0.0f, y);
				const vec2 topRightSeparator = infoColumnRegion.GetTR() + vec2(0.0f, y);
				drawList->AddLine(topLeftSeparator, topRightSeparator, GetColor(EditorColor_TempoMapBg));
			}
		}
		else if (selectedAetItem.Type() == AetItemType::AetLayer)
		{
			constexpr float typeIconDistance = 19.0f;

			// TODO: Quick test
			AetLayer* layer = selectedAetItem.GetAetLayerRef().get();
			for (int i = 0; i < static_cast<int>(layer->size()); i++)
			{
				float y = i * rowHeight;
				vec2 start = vec2(startPadding, y + textPadding) + infoColumnRegion.GetTL();

				const auto& object = layer->at(i);

				drawList->AddText(start, textColor, GetObjTypeIcon(object->Type));
				start.x += typeIconDistance;
				drawList->AddText(start, textColor, object->GetName().c_str());
			}

			// NOTE: Draw Separators
			for (int i = 0; i <= static_cast<int>(layer->size()); i++)
			{
				const float y = i * rowHeight;
				const vec2 topLeftSeparator = infoColumnRegion.GetTL() + vec2(0.0f, y);
				const vec2 topRightSeparator = infoColumnRegion.GetTR() + vec2(0.0f, y);
				drawList->AddLine(topLeftSeparator, topRightSeparator, GetColor(EditorColor_TempoMapBg));
			}
		}
	}

	void AetTimeline::OnDrawTimlineRows()
	{
		if (selectedAetItem.IsNull())
			return;

		int rowCount = (selectedAetItem.Type() == AetItemType::AetLayer) ?
			static_cast<int>(selectedAetItem.GetAetLayerRef()->size()) :
			static_cast<int>(PropertyType_Count);

		// TODO: Scroll offset and scrollbar, use Gui::Scrollbar (?) might have to make a completely custom one and don't forget to push clipping rects

		ImU32 rowColor = GetColor(EditorColor_TimelineRowSeparator);

		for (int i = 0; i <= rowCount; i++)
		{
			const float y = i * rowHeight;
			vec2 start = timelineContentRegion.GetTL() + vec2(0.0f, y);
			vec2 end = start + vec2(timelineContentRegion.GetWidth(), 0.0f);

			baseDrawList->AddLine(start, end, rowColor);
		}
	}

	void AetTimeline::OnDrawTimlineDivisors()
	{
		FrameTimeline::OnDrawTimlineDivisors();
	}

	void AetTimeline::OnDrawTimlineBackground()
	{
	}

	void AetTimeline::OnUpdate()
	{
		if (!selectedAetItem.IsNull())
		{
			const auto* parentAet = selectedAetItem.GetItemParentAet();

			switch (selectedAetItem.Type())
			{
			case AetItemType::Aet:
				loopStartFrame = parentAet->FrameStart;
				loopEndFrame = parentAet->FrameDuration;
				break;

			case AetItemType::AetLayer:
				loopStartFrame = parentAet->FrameStart;
				loopEndFrame = 0.0f;

				for (const RefPtr<AetObj>& obj : *selectedAetItem.Ptrs.AetLayer)
				{
					if (obj->LoopEnd > loopEndFrame.Frames())
						loopEndFrame = obj->LoopEnd;
				}

				break;

			case AetItemType::AetObj:
				loopStartFrame = selectedAetItem.Ptrs.AetObj->LoopStart;
				loopEndFrame = selectedAetItem.Ptrs.AetObj->LoopEnd;
				break;

			case AetItemType::AetRegion:
				loopStartFrame = 0.0f;
				loopEndFrame = glm::max(0.0f, selectedAetItem.Ptrs.AetRegion->SpriteCount() - 1.0f);
				break;

			default:
				break;
			}
		}

		if (GetIsPlayback())
		{
			UpdateCursorPlaybackTime();
		}

		TimeSpan startTime = GetTimelineTime(loopStartFrame);
		TimeSpan endTime = GetTimelineTime(loopEndFrame);

		if (cursorTime < startTime)
			cursorTime = startTime;

		if (cursorTime > endTime)
		{
			if (loopPlayback)
			{
				cursorTime = startTime;
				SetScrollX(0.0f);
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
		if (selectedAetItem.Type() == AetItemType::None || selectedAetItem.IsNull())
		{
			DrawTimelineContentNone();
		}
		else
		{
			switch (selectedAetItem.Type())
			{
			case AetItemType::AetSet:
			case AetItemType::Aet:
				DrawTimelineContentNone();
				break;
			case AetItemType::AetLayer:
				DrawTimelineContentLayer();
				break;
			case AetItemType::AetObj:
				DrawTimelineContentObject();
				break;
			case AetItemType::AetRegion:
				break;
			default:
				break;
			}
		}

		// draw selection region
		const MouseSelectionData& selectionData = timelineController.GetSelectionData();
		if (selectionData.IsSelected())
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
		bool horizontalSelection = glm::abs((selectionData.StartX - selectionData.EndX).Frames()) > 1.0f;
		bool verticalSelection = glm::abs(selectionData.RowStartIndex - selectionData.RowEndIndex) > 1;

		if (horizontalSelection || verticalSelection)
		{
			const float offset = timelineContentRegion.Min.x - GetScrollX();

			float direction = selectionData.StartX < selectionData.EndX ? +1.0f : -1.0f;
			float padding = 5.0f * direction;

			ImRect selectionRegion = ImRect(
				vec2(glm::round(offset + GetTimelinePosition(selectionData.StartX) - padding), GetRowScreenY(selectionData.RowStartIndex)),
				vec2(glm::round(offset + GetTimelinePosition(selectionData.EndX) + padding), GetRowScreenY(selectionData.RowEndIndex)));

			ImDrawList* windowDrawList = Gui::GetWindowDrawList();
			windowDrawList->AddRectFilled(selectionRegion.Min, selectionRegion.Max, GetColor(EditorColor_TimelineSelection));
			windowDrawList->AddRect(selectionRegion.Min, selectionRegion.Max, GetColor(EditorColor_TimelineSelectionBorder));
		}
	}

	void AetTimeline::UpdateCursorPlaybackTime()
	{
		cursorTime += io->DeltaTime * playbackSpeedFactor;
	}

	void AetTimeline::RoundCursorTimeToNearestFrame()
	{
		frame_t cursorFrames = GetCursorFrame().Frames();
		frame_t roundedCursorFrames = glm::round(cursorFrames);
		cursorTime = GetTimelineTime(TimelineFrame(roundedCursorFrames));
	}

	float AetTimeline::GetRowScreenY(int index) const
	{
		return (timelineContentRegion.Min.y + static_cast<float>(rowHeight * index));
	}

	int AetTimeline::GetRowIndexFromScreenY(float screenY) const
	{
		return glm::max(0, static_cast<int>(glm::floor((screenY - timelineContentRegion.Min.y) / rowHeight)));
	}
}