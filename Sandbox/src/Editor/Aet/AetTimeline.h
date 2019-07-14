#pragma once
#include "Selection.h"
#include "Editor/FrameTimeline.h"
#include "FileSystem/Format/AetSet.h"

namespace Editor
{
	using namespace FileSystem;
	class AetEditor;

	class AetTimeline : public FrameTimeline
	{
	public:
		AetTimeline();
		~AetTimeline();

		void SetActive(Aet* parent, AetItemTypePtr value);

	private:
		Aet* aet = nullptr;
		AetItemTypePtr active;

		float rowHeight;
		bool isPlayback = false;
		bool loopPlayback = true;

		bool GetIsPlayback() const override;
		float GetTimelineSize() const override;

		void DrawTimelineContentNone();
		void DrawTimelineContentKeyFrames();

		void OnDrawTimelineHeaderWidgets() override;
		void OnDrawTimelineInfoColumnHeader() override;
		void OnDrawTimelineInfoColumn() override;
		void OnDrawTimlineRows() override;
		void OnDrawTimlineDivisors() override;
		void OnDrawTimlineBackground() override;
		void OnUpdate() override;
		void OnUpdateInput() override;
		void OnDrawTimelineContents() override;
		void PausePlayback() override;
		void ResumePlayback() override;
		void StopPlayback() override;

		void UpdateInputCursorClick();
	};
}