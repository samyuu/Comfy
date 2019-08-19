#include "AetTimeline.h"
#include "AetEditor.h"

namespace Editor
{
	static_assert(sizeof(PropertyType_Enum) == sizeof(int32_t) && sizeof(KeyFrameIndex::Pair) == sizeof(KeyFrameIndex::PackedValue));

	AetTimeline::AetTimeline()
	{
		infoColumnWidth = 140.0f;
		rowHeight = 16.0f;
		zoomLevel = 5.0f;
	}

	AetTimeline::~AetTimeline()
	{
	}

	void AetTimeline::SetActive(Aet* parent, AetItemTypePtr value)
	{
		aet = parent;
		active = value;

		frameRate = (aet != nullptr) ? aet->FrameRate : 60.0f;
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

	void AetTimeline::DrawTimelineContentKeyFrames()
	{
		if (active.AetObj->AnimationData == nullptr)
			return;

		keyFrameRenderer.DrawKeyFrames(this, active.AetObj->AnimationData->Properties);
	}

	void AetTimeline::OnDrawTimelineHeaderWidgets()
	{
		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 0));

		ImGuiStyle& style = Gui::GetStyle();
		Gui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, style.FramePadding.y));

		TimeSpan cursorTime = GetCursorTime();
		sprintf_s(timeInputBuffer, "%s (%.f/%.f)", cursorTime.FormatTime().c_str(), GetTimelineFrame(cursorTime).Frames(), loopEndFrame.Frames());

		Gui::PushItemWidth(140);
		Gui::InputTextWithHint("##AetTimeline::TimeInput", "00:00.000", timeInputBuffer, sizeof(timeInputBuffer));
		Gui::PopItemWidth();

		Gui::SameLine();
		Gui::Button(ICON_FA_FAST_BACKWARD);
		if (Gui::IsItemActive()) { scrollDelta -= io->DeltaTime * 1000.0f; }

		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

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
		if (Gui::Button(ICON_FA_STOP) && GetIsPlayback())
			StopPlayback();

		Gui::SameLine();
		Gui::Button(ICON_FA_FORWARD);
		if (Gui::IsItemActive()) { scrollDelta += io->DeltaTime * 400.0f; }

		Gui::SameLine();
		Gui::Button(ICON_FA_FAST_FORWARD);
		if (Gui::IsItemActive()) { scrollDelta += io->DeltaTime * 1000.0f; }

		Gui::PopStyleVar(2);

		Gui::SameLine();
		Gui::PushItemWidth(280);
		Gui::SliderFloat(ICON_FA_SEARCH, &zoomLevel, ZOOM_MIN, ZOOM_MAX);
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

		ImDrawList* drawList = Gui::GetWindowDrawList();
		for (int i = 0; i < PropertyType_Count; i++)
		{
			float y = i * rowHeight + 2;
			auto start = ImVec2(2, y) + infoColumnRegion.GetTL();

			drawList->AddText(start, Gui::GetColorU32(ImGuiCol_Text), timelinePropertyNames[i]);
		}
	}

	void AetTimeline::OnDrawTimlineRows()
	{
		// Key Frame Property Rows
		// -----------------------
		for (int i = 0; i <= PropertyType_Count; i++)
		{
			float y = i * rowHeight;
			ImVec2 start = timelineContentRegion.GetTL() + ImVec2(0, y);
			ImVec2 end = start + ImVec2(timelineContentRegion.GetWidth(), 0);

			baseDrawList->AddLine(start, end, GetColor(EditorColor_TimelineRowSeparator));
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
		if (active.VoidPointer != nullptr)
		{
			switch (active.Type())
			{
			case AetSelectionType::Aet:
				loopStartFrame = aet->FrameStart;
				loopEndFrame = aet->FrameDuration;
				break;

			case AetSelectionType::AetLayer:
				loopStartFrame = aet->FrameStart;
				loopEndFrame = 0.0f;

				for (RefPtr<AetObj>& obj : *active.AetLayer)
				{
					if (obj->LoopEnd > loopEndFrame.Frames())
						loopEndFrame = obj->LoopEnd;
				}

				break;

			case AetSelectionType::AetObj:
				loopStartFrame = active.AetObj->LoopStart;
				loopEndFrame = active.AetObj->LoopEnd;
				break;

			case AetSelectionType::AetRegion:
				loopStartFrame = 0;
				loopEndFrame = glm::max(0.0f, active.AetRegion->Frames - 1.0f);
				break;

			default:
				break;
			}
		}

		if (GetIsPlayback())
			cursorTime += io->DeltaTime;

		TimeSpan startTime = GetTimelineTime(loopStartFrame);
		TimeSpan endTime = GetTimelineTime(loopEndFrame);

		if (cursorTime < startTime)
			cursorTime = startTime;

		if (cursorTime > endTime)
		{
			if (loopPlayback)
			{
				cursorTime = startTime;
				SetScrollX(0);
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
		if (active.Type() == AetSelectionType::None || active.VoidPointer == nullptr)
		{
			DrawTimelineContentNone();
		}
		else
		{
			switch (active.Type())
			{
			case AetSelectionType::AetSet:
			case AetSelectionType::Aet:
				DrawTimelineContentNone();
				break;
			case AetSelectionType::AetLayer:
				break;
			case AetSelectionType::AetObj:
				DrawTimelineContentKeyFrames();
				break;
			case AetSelectionType::AetRegion:
				break;
			default:
				break;
			}
		}

		// draw selection region
		const MouseSelectionData& selectonData = timelineController.GetSelectionData();
		if (selectonData.IsSelected())
		{
			bool horizontalSelection = glm::abs((selectonData.StartX - selectonData.EndX).Frames()) > 1.0f;
			bool verticalSelection = glm::abs(selectonData.RowStartIndex - selectonData.RowEndIndex) > 1;

			if (horizontalSelection || verticalSelection)
			{
				const float offset = timelineContentRegion.Min.x - GetScrollX();

				float direction = selectonData.StartX < selectonData.EndX ? +1.0f : -1.0f;
				float padding = 5.0f * direction;

				ImRect selectionRegion = ImRect(
					vec2(glm::round(offset + GetTimelinePosition(selectonData.StartX) - padding), GetRowScreenY(selectonData.RowStartIndex)),
					vec2(glm::round(offset + GetTimelinePosition(selectonData.EndX) + padding), GetRowScreenY(selectonData.RowEndIndex)));

				ImDrawList* windowDrawList = Gui::GetWindowDrawList();
				windowDrawList->AddRectFilled(selectionRegion.Min, selectionRegion.Max, GetColor(EditorColor_TimelineSelection));
				windowDrawList->AddRect(selectionRegion.Min, selectionRegion.Max, GetColor(EditorColor_TimelineSelectionBorder));
			}
		}
	}

	void AetTimeline::PausePlayback()
	{
		isPlayback = false;
	}

	void AetTimeline::ResumePlayback()
	{
		isPlayback = true;
	}

	void AetTimeline::StopPlayback()
	{
		PausePlayback();
	}

	void AetTimeline::UpdateInputCursorClick()
	{
		timelineController.UpdateInput(this);

		if (timelineController.GetUpdateCursorTime())
			cursorTime = timelineController.GetNewCursorTime();
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