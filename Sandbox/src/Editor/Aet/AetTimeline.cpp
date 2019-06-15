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

	AetObj* AetTimeline::GetAetobj() const
	{
		return aetObj;
	}

	void AetTimeline::SetAetObj(AetObj* value)
	{
		aetObj = value;
	}

	bool AetTimeline::GetIsPlayback() const
	{
		return isPlayback;
	}

	float AetTimeline::GetTimelineSize() const
	{
		return GetTimelinePosition(loopEndFrame);
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

		//ImGui::SameLine();
		//ImGui::PushItemWidth(80);
		//{
		//	if (gridDivisions[gridDivisionIndex] != gridDivision)
		//		gridDivisionIndex = GetGridDivisionIndex();

		//	if (ImGui::Combo("Grid Precision", &gridDivisionIndex, gridDivisionStrings.data(), gridDivisionStrings.size()))
		//		gridDivision = gridDivisions[gridDivisionIndex];
		//}
		//ImGui::PopItemWidth();

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
		for (int i = 0; i < KeyFrameProperties::PropertyNames.size(); i++)
		{
			float y = i * rowHeight + 2;
			auto start = ImVec2(2, y) + infoColumnRegion.GetTL();

			drawList->AddText(start, ImGui::GetColorU32(ImGuiCol_Text), KeyFrameProperties::PropertyNames[i]);
		}
	}

	void AetTimeline::OnDrawTimlineRows()
	{
		// Key Frame Property Rows
		// -----------------------
		for (int i = 0; i <= KeyFrameProperties::PropertyNames.size(); i++)
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
		if (aetObj != nullptr)
		{
			loopStartFrame = aetObj->LoopStart;
			loopEndFrame = aetObj->LoopEnd;
		}

		if (isPlayback)
		{
			cursorTime += io->DeltaTime;

			TimeSpan startTime = GetTimelineTime(loopStartFrame);
			TimeSpan endTime = GetTimelineTime(loopEndFrame);

			if (cursorTime > endTime)
				cursorTime = loopPlayback ? startTime : endTime;
		}
	}

	void AetTimeline::OnUpdateInput()
	{
		UpdateInputCursorClick();
	}

	void AetTimeline::OnDrawTimelineContents()
	{
		if (aetObj == nullptr)
			return;

		KeyFrameProperties* properties = aetObj->AnimationData.Properties.get();
		std::vector<KeyFrame>* keyFrames = &properties->OriginX;

		for (int i = 0; i < KeyFrameProperties::PropertyNames.size(); i++)
		{
			float y = (i * rowHeight) + (rowHeight / 2);

			for (auto& keyFrame : *keyFrames)
			{
				ImVec2 start = timelineContentRegion.GetTL() + ImVec2(GetTimelinePosition(TimelineFrame(keyFrame.Frame)) - GetScrollX(), y);
				baseDrawList->AddCircleFilled(start, 6.0f, GetColor(EditorColor_KeyFrame));
			}

			keyFrames++;
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
		if (!ImGui::IsMouseHoveringWindow() || !timelineContentRegion.Contains(ImGui::GetMousePos()))
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

			cursorTime = newTime;
		}
	}
}