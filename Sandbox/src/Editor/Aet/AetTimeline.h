#pragma once
#include "../FrameTimeline.h"
#include "../../FileSystem/Format/AetSet.h"

namespace Editor
{
	using namespace FileSystem;
	class AetEditor;

	class AetTimeline : public FrameTimeline
	{
	public:
		AetTimeline();
		~AetTimeline();

		AetObj* GetAetobj() const;
		void SetAetObj(AetLyo* parent, AetObj* value);

	private:
		AetLyo* activeAetLyo = nullptr;
		AetObj* aetObj = nullptr;
		float rowHeight;
		bool isPlayback = false;
		bool loopPlayback = false;

		bool GetIsPlayback() const override;
		float GetTimelineSize() const override;

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