#include "AetTimeline.h"
#include "AetEditor.h"

namespace Editor
{
	AetTimeline::AetTimeline()
	{
		infoColumnWidth = 58.0f;
		rowHeight = 22.0f;
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
		return GetTimelinePosition(loopEndFrame);
	}

	void AetTimeline::DrawTimelineContentKeyFrameDoubleX(const vec2& position) const
	{
		baseDrawList->AddTriangleFilled(
			position - vec2(keyFrameSize, 0.0f),
			position - vec2(0.0f, keyFrameSize),
			position + vec2(0.0f, keyFrameSize),
			GetColor(EditorColor_KeyFrame));
	}

	void AetTimeline::DrawTimelineContentKeyFrameDoubleY(const vec2& position) const
	{
		baseDrawList->AddTriangleFilled(
			position + vec2(keyFrameSize, 0.0f),
			position - vec2(0.0f, keyFrameSize),
			position + vec2(0.0f, keyFrameSize),
			GetColor(EditorColor_KeyFrame));
	}

	void AetTimeline::DrawTimelineContentKeyFrame(const vec2& position, KeyFrameType type) const
	{
		switch (type)
		{
		case KeyFrameType::Single:
			DrawTimelineContentKeyFrameDoubleX(position);
			DrawTimelineContentKeyFrameDoubleY(position);
			break;
		case KeyFrameType::DoubleX:
			DrawTimelineContentKeyFrameDoubleX(position);
			break;
		case KeyFrameType::DoubleY:
			DrawTimelineContentKeyFrameDoubleY(position);
			break;
		default:
			break;
		}
	}

	void AetTimeline::DrawTimelineContentNone()
	{
		ImU32 dimColor = ImGui::GetColorU32(ImGuiCol_PopupBg, 0.25f);
		ImGui::GetWindowDrawList()->AddRectFilled(timelineBaseRegion.GetTL(), timelineBaseRegion.GetBR(), dimColor);
	}

	void AetTimeline::DrawTimelineContentKeyFrames()
	{
		if (active.AetObj->AnimationData == nullptr)
			return;

		vec2 timelineTL = timelineContentRegion.GetTL() - vec2(GetScrollX(), 0.0f);
		float y = (rowHeight / 2.0f);

		const KeyFrameProperties* properties = &active.AetObj->AnimationData->Properties;
		for (int i = 0; i < properties->size(); i++)
		{
			KeyFrameType type;

			switch (i)
			{
			case PropertyType_OriginX:
			case PropertyType_PositionX:
			case PropertyType_ScaleX:
				type = KeyFrameType::DoubleX;
				break;

			case PropertyType_OriginY:
			case PropertyType_PositionY:
			case PropertyType_ScaleY:
				type = KeyFrameType::DoubleY;
				break;

			case PropertyType_Rotation:
			case PropertyType_Opacity:
			default:
				type = KeyFrameType::Single;
				break;
			}

			const KeyFrameCollection& keyFrames = properties->at(i);
			for (const auto& keyFrame : keyFrames)
			{
				TimelineFrame keyFrameFrame = keyFrames.size() == 1 ? loopStartFrame : keyFrame.Frame;
				vec2 position = timelineTL + vec2(GetTimelinePosition(keyFrameFrame), y);

				DrawTimelineContentKeyFrame(position, type);
			}

			if (type == KeyFrameType::Single || type == KeyFrameType::DoubleY)
				y += rowHeight;
		}
	}

	void AetTimeline::OnDrawTimelineHeaderWidgets()
	{
		static char timeInputBuffer[32];

		TimeSpan cursorTime = GetCursorTime();
		sprintf_s(timeInputBuffer, "%s (%.f/%.f)", cursorTime.FormatTime().c_str(), GetTimelineFrame(cursorTime).Frames(), loopEndFrame.Frames());

		ImGui::PushItemWidth(140);
		ImGui::InputTextWithHint("##time_input", "00:00.000", timeInputBuffer, sizeof(timeInputBuffer));
		ImGui::PopItemWidth();

		ImGui::SameLine();
		if (ImGui::Button("Stop") && GetIsPlayback())
			StopPlayback();

		ImGui::SameLine();
		if (ImGui::Button("Pause") && GetIsPlayback())
			PausePlayback();

		ImGui::SameLine();
		if (ImGui::Button("Play") && !GetIsPlayback())
			ResumePlayback();

		ImGui::SameLine();
		ImGui::Button("|<");
		if (ImGui::IsItemActive()) { scrollDelta -= io->DeltaTime * 1000.0f; }

		ImGui::SameLine();
		ImGui::Button(">|");
		if (ImGui::IsItemActive()) { scrollDelta += io->DeltaTime * 1000.0f; }

		ImGui::SameLine();
		ImGui::PushItemWidth(280);
		ImGui::SliderFloat("Zoom Level", &zoomLevel, ZOOM_MIN, ZOOM_MAX);
		ImGui::PopItemWidth();

		ImGui::SameLine();
		ImGui::Checkbox("Loop Animation", &loopPlayback);
	}

	void AetTimeline::OnDrawTimelineInfoColumnHeader()
	{
		TimelineBase::OnDrawTimelineInfoColumnHeader();
	}

	void AetTimeline::OnDrawTimelineInfoColumn()
	{
		TimelineBase::OnDrawTimelineInfoColumn();

		auto drawList = ImGui::GetWindowDrawList();
		for (int i = 0; i < static_cast<size_t>(PropertyType::Count); i++)
		{
			float y = i * rowHeight + 2;
			auto start = ImVec2(2, y) + infoColumnRegion.GetTL();

			drawList->AddText(start, ImGui::GetColorU32(ImGuiCol_Text), timelinePropertyNames[i]);
		}
	}

	void AetTimeline::OnDrawTimlineRows()
	{
		// Key Frame Property Rows
		// -----------------------
		for (int i = 0; i <= static_cast<size_t>(PropertyType::Count); i++)
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
			case AetSelectionType::AetLayer:
				loopStartFrame = aet->FrameStart;
				loopEndFrame = aet->FrameDuration;
				break;

			case AetSelectionType::AetObj:
				loopStartFrame = active.AetObj->LoopStart;
				loopEndFrame = active.AetObj->LoopEnd;
				break;

			case AetSelectionType::AetRegion:
				loopStartFrame = 0;
				loopEndFrame = glm::max(0.0f, static_cast<float>(active.AetRegion->SpriteSize()) - 1.0f);
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
			return;
		}

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
		if (!ImGui::IsWindowFocused() || !timelineContentRegion.Contains(ImGui::GetMousePos()))
			return;

		// Cursor Mouse Down:
		// ------------------
		if (ImGui::IsMouseDown(0) && !io->KeyShift)
		{
			const TimelineFrame cursorMouseFrame = GetCursorMouseXFrame();
			TimeSpan previousTime = GetCursorTime();
			TimeSpan newTime = GetTimelineTime(cursorMouseFrame);

			if (previousTime == newTime)
				return;

			TimeSpan endTime = GetTimelineTime(loopEndFrame);
			if (newTime > endTime)
				newTime = endTime;

			cursorTime = newTime;
		}
	}
}