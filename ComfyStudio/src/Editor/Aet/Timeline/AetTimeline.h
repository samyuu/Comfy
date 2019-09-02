#pragma once
#include "Editor/Aet/AetSelection.h"
#include "KeyFrameRenderer.h"
#include "AetTimelineController.h"
#include "Editor/Timeline/FrameTimeline.h"
#include "Graphics/Auth2D/AetMgr.h"
#include "FileSystem/Format/AetSet.h"

namespace Editor
{
	using namespace FileSystem;
	using namespace Graphics::Auth2D;
	class AetEditor;

	struct KeyFrameIndex
	{
		union
		{
			struct PropertyKeyFrameIndexPair
			{
				PropertyType_Enum Property;
				int32_t KeyFrame;
			} Pair;
			int64_t PackedValue;
		};
	};

	class AetTimeline : public FrameTimeline
	{
	public:
		AetTimeline();
		~AetTimeline();

		void SetActive(Aet* parent, AetItemTypePtr value);
		bool GetIsPlayback() const override;

	public:
		// screen position of row index
		float GetRowScreenY(int index) const;

		// row index at input height
		int GetRowIndexFromScreenY(float screenY) const;

		inline float GetRowHeight() const { return rowHeight; };

	private:
		Aet* aet = nullptr;
		AetItemTypePtr active;

		float rowHeight;
		bool isPlayback = false;
		bool loopPlayback = true;

		char timeInputBuffer[32];

	private:
		KeyFrameRenderer keyFrameRenderer;
		AetTimelineController timelineController;

	private:
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
		
	private:
		Vector<KeyFrameIndex> selectedKeyFrames;

		Array<const char*, static_cast<size_t>(PropertyType_Count)> timelinePropertyNames =
		{
			"Transform  :  Origin.X",
			"Transform  :  Origin.Y",
			"Transform  :  Position.X",
			"Transform  :  Position.Y",
			"Transform  :  Rotation",
			"Transform  :  Scale.X",
			"Transform  :  Scale.Y",
			"Color               Opacity",
		};
	};
}