#include "AetTimeline.h"
#include "Editor/Aet/AetEditor.h"
#include "Editor/Aet/AetIcons.h"

namespace Editor
{
	static_assert(sizeof(PropertyType_Enum) == sizeof(int32_t) && sizeof(KeyFrameIndex::Pair) == sizeof(KeyFrameIndex::PackedValue));

	AetTimeline::AetTimeline()
	{
		// NOTE: Around 240.0f seems to be the standard for many programs
		infoColumnWidth = 240.0f;
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
		return;
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

	void AetTimeline::OnInitialize()
	{
		keyFrameRenderer.Initialize();
	}

	void AetTimeline::OnDrawTimelineHeaderWidgets()
	{
		return;
	}

	void AetTimeline::OnDrawTimelineInfoColumnHeader()
	{
		TimelineBase::OnDrawTimelineInfoColumnHeader();

		// TODO: Does not work with all style configurations
		assert(infoColumnWidth == 240.0f);

		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(0.0f, 0.0f));
		Gui::PushStyleVar(ImGuiStyleVar_FramePadding, vec2(8.0f, 8.0f));

		constexpr vec4 transparent = vec4(0.0f);
		Gui::PushStyleColor(ImGuiCol_Button, transparent);
		{
			const bool isFirstFrame = (cursorTime <= GetTimelineTime(loopStartFrame));
			const bool isLastFrame = (cursorTime >= GetTimelineTime(loopEndFrame));
			const bool isPlayback = GetIsPlayback();

			constexpr float borderSize = 1.0f;
			Gui::SetCursorPosX(Gui::GetCursorPosX() + borderSize);

			// NOTE: First frame button
			{
				Gui::PushItemDisabledAndTextColorIf(isFirstFrame || isPlayback);
				if (Gui::Button(ICON_FA_FAST_BACKWARD))
				{
					cursorTime = GetTimelineTime(loopStartFrame);
					RoundCursorTimeToNearestFrame();
					scrollDelta = -GetTimelineSize();
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
					scrollDelta = -GetTimelineSize();
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
					scrollDelta = +GetTimelineSize();
				}
				Gui::PopItemDisabledAndTextColorIf(isLastFrame || isPlayback);
				Gui::SetWideItemTooltip("Go to last frame");
			}

			// TODO:
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
			constexpr float percentageFactor = 100.0f;

			// TODO: Come up with a neat comfy layout
			Gui::Text("TODO:");

			float playbackSpeedPercentage = playbackSpeedFactor * percentageFactor;
			if (Gui::SliderFloat("Playback Speed", &playbackSpeedPercentage, 1.0f, 400.0f, "%.2f %%"))
				playbackSpeedFactor = playbackSpeedPercentage * (1.0f / percentageFactor);

			Gui::Checkbox("Loop Animation", &loopPlayback);

			Gui::EndPopup();
		}
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
				const float y = i * rowItemHeight;
				vec2 start = vec2(startPadding, y + textPadding) + infoColumnRegion.GetTL();

				const auto[type, name] = timelinePropertyTypeNames[i];
				drawList->AddText(start, textColor, type);
				start.x += nameTypeDistance;
				drawList->AddText(start, textColor, timelinePropertyNameTypeSeparator);
				start.x += nameTypeSeparatorSpacing;
				drawList->AddText(start, textColor, name);
			}

			// NOTE: Draw Separators
			for (int i = 0; i <= PropertyType_Count; i++)
			{
				const float y = i * rowItemHeight;
				const vec2 topLeftSeparator = infoColumnRegion.GetTL() + vec2(0.0f, y);
				const vec2 topRightSeparator = infoColumnRegion.GetTR() + vec2(0.0f, y);
				drawList->AddLine(topLeftSeparator, topRightSeparator, Gui::GetColorU32(ImGuiCol_Border));
			}
		}
		else if (selectedAetItem.Type() == AetItemType::AetLayer)
		{
			constexpr float typeIconDistance = 19.0f;

			// TODO: Quick test
			AetLayer* layer = selectedAetItem.GetAetLayerRef().get();
			for (int i = 0; i < static_cast<int>(layer->size()); i++)
			{
				float y = i * rowItemHeight;
				vec2 start = vec2(startPadding, y + textPadding) + infoColumnRegion.GetTL();

				const auto& object = layer->at(i);

				drawList->AddText(start, textColor, GetObjTypeIcon(object->Type));
				start.x += typeIconDistance;
				drawList->AddText(start, textColor, object->GetName().c_str());
			}

			// NOTE: Draw Separators
			for (int i = 0; i <= static_cast<int>(layer->size()); i++)
			{
				const float y = i * rowItemHeight;
				const vec2 topLeftSeparator = infoColumnRegion.GetTL() + vec2(0.0f, y);
				const vec2 topRightSeparator = infoColumnRegion.GetTR() + vec2(0.0f, y);
				drawList->AddLine(topLeftSeparator, topRightSeparator, Gui::GetColorU32(ImGuiCol_Border));
			}
		}
	}

	void AetTimeline::OnDrawTimlineRows()
	{
		if (selectedAetItem.IsNull())
			return;

		if (currentTimelineMode != TimelineMode::DopeSheet)
			return;

		const int rowCount = (selectedAetItem.Type() == AetItemType::AetLayer) ?
			static_cast<int>(selectedAetItem.GetAetLayerRef()->size()) :
			static_cast<int>(PropertyType_Count);

		// TODO: Scroll offset and scrollbar, use Gui::Scrollbar (?) might have to make a completely custom one and don't forget to push clipping rects
		const ImU32 rowColor = GetColor(EditorColor_TimelineRowSeparator);

		for (int i = 0; i <= rowCount; i++)
		{
			const float y = i * rowItemHeight;
			const vec2 start = timelineContentRegion.GetTL() + vec2(0.0f, y);
			const vec2 end = start + vec2(timelineContentRegion.GetWidth(), 0.0f);

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

	void AetTimeline::OnDrawTimelineScrollBarRegion()
	{
		constexpr float timeDragTextOffset = 10.0f;
		constexpr float timeDragTextWidth = 60.0f + 26.0f;
		Gui::SetCursorPosX(Gui::GetCursorPosX() + timeDragTextOffset);

		Gui::PushStyleVar(ImGuiStyleVar_FramePadding, vec2(Gui::GetStyle().FramePadding.x, 0.0f));

		// NOTE: Time drag text
		{
			char cursorTimeBuffer[32];
			cursorTime.FormatTime(cursorTimeBuffer, sizeof(cursorTimeBuffer));

			float cursorFrame = GetCursorFrame().Frames();

			if (Gui::ComfyDragText("TimeDragText::AetTimeline", cursorTimeBuffer, &cursorFrame, 1.0f, 0.0f, 0.0f, timeDragTextWidth))
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
					constexpr float modeButtonWidth = 72.0f;
					if (Gui::Button(label, vec2(modeButtonWidth, timelineScrollbarSize.y)))
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

	static frame_t GetAetLayerLastFrame(const AetLayer* aetLayer)
	{
		frame_t lastFrame = 0;
		for (auto& object : *aetLayer)
		{
			if (object->LoopEnd > lastFrame)
				lastFrame = object->LoopEnd;
		}
		return lastFrame;
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
				//loopStartFrame = parentAet->FrameStart;
				//loopEndFrame = 0.0f;

				//for (const RefPtr<AetObj>& obj : *selectedAetItem.Ptrs.AetLayer)
				//{
				//	if (obj->LoopEnd > loopEndFrame.Frames())
				//		loopEndFrame = obj->LoopEnd;
				//}

				loopStartFrame = 0.0f;
				loopEndFrame = GetAetLayerLastFrame(selectedAetItem.Ptrs.AetLayer);

				break;

			case AetItemType::AetObj:

				//loopStartFrame = parentAet->FrameStart;

				//loopStartFrame = selectedAetItem.Ptrs.AetObj->LoopStart;
				//loopEndFrame = selectedAetItem.Ptrs.AetObj->LoopEnd;

				loopStartFrame = parentAet->FrameStart;
				loopEndFrame = selectedAetItem.Ptrs.AetObj->LoopEnd;

				//loopStartFrame = 0.0f;
				//loopEndFrame = GetAetLayerLastFrame(selectedAetItem.Ptrs.AetObj->GetParentLayer());

				break;

			case AetItemType::AetRegion:
				loopStartFrame = 0.0f;
				loopEndFrame = glm::max(0.0f, selectedAetItem.Ptrs.AetRegion->SpriteCount() - 1.0f);
				break;

			default:
				break;
			}

			// TODO: Is this how it should work? Hence the reason to edit these values in the first place PeepoShrug
			// TODO: Maybe this should be an option inside an options context menu button like loop playback and playback speed (?)
			if (selectedAetItem.Type() != AetItemType::AetRegion && false)
			{
				loopStartFrame = parentAet->FrameStart;
				loopEndFrame = parentAet->FrameDuration;
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
		const bool horizontalSelection = glm::abs((selectionData.StartX - selectionData.EndX).Frames()) > 1.0f;
		const bool verticalSelection = glm::abs(selectionData.RowStartIndex - selectionData.RowEndIndex) > 1;

		if (horizontalSelection || verticalSelection)
		{
			const float offset = timelineContentRegion.Min.x - GetScrollX();

			const float direction = selectionData.StartX < selectionData.EndX ? +1.0f : -1.0f;
			const float padding = 5.0f * direction;

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
		const auto& io = Gui::GetIO();
		cursorTime += io.DeltaTime * playbackSpeedFactor;
	}

	void AetTimeline::RoundCursorTimeToNearestFrame()
	{
		const frame_t cursorFrames = GetCursorFrame().Frames();
		const frame_t roundedCursorFrames = glm::round(cursorFrames);
		cursorTime = GetTimelineTime(TimelineFrame(roundedCursorFrames));
	}

	float AetTimeline::GetRowScreenY(int index) const
	{
		return (timelineContentRegion.Min.y + static_cast<float>(rowItemHeight * index));
	}

	int AetTimeline::GetRowIndexFromScreenY(float screenY) const
	{
		return glm::max(0, static_cast<int>(glm::floor((screenY - timelineContentRegion.Min.y) / rowItemHeight)));
	}
}