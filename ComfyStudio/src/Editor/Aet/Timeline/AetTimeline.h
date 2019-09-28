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

		void SetActive(AetItemTypePtr value);
		bool GetIsPlayback() const override;

	public:
		// NOTE: Screen position of row index
		float GetRowScreenY(int index) const;

		// NOTE: Row index at input height
		int GetRowIndexFromScreenY(float screenY) const;

		inline float GetRowHeight() const { return rowHeight; };

	private:
		AetItemTypePtr selectedAetItem;

		float rowHeight;
		bool isPlayback = false;
		bool loopPlayback = true;

		char timeInputBuffer[32];

		// NOTE: Speed at factor at which the playback time is incremented without editing any AetObj state
		float playbackSpeedFactor = 1.0f;

	private:
		KeyFrameRenderer keyFrameRenderer;
		AetTimelineController timelineController = { this };

	private:
		float GetTimelineSize() const override;

		void DrawTimelineContentNone();
		void DrawTimelineContentLayer();
		void DrawTimelineContentObject();

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
		void DrawMouseSelection(const MouseSelectionData& selectionData);

		void UpdateCursorPlaybackTime();

	private:
		Vector<KeyFrameIndex> selectedKeyFrames;

		static constexpr const char* timelinePropertyNameTypeSeparator = ":";

		const Array<const char*, static_cast<size_t>(PropertyType_Count)> timelinePropertyNameTypes =
		{
			"Transform",
			"Transform",
			"Transform",
			"Transform",
			"Transform",
			"Transform",
			"Transform",
			"Color",
		};

		const Array<const char*, static_cast<size_t>(PropertyType_Count)> timelinePropertyNames =
		{
			"Origin.X",
			"Origin.Y",
			"Position.X",
			"Position.Y",
			"Rotation",
			"Scale.X",
			"Scale.Y",
			"Opacity",
		};
	};
}