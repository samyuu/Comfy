#pragma once
#include "Editor/Aet/AetSelection.h"
#include "KeyFrameRenderer.h"
#include "AetTimelineController.h"
#include "Editor/Timeline/FrameTimeline.h"
#include "Graphics/Auth2D/AetMgr.h"
#include "Graphics/Auth2D/AetSet.h"

namespace Editor
{
	class AetEditor;

	struct KeyFrameIndex
	{
		union
		{
			struct PropertyKeyFrameIndexPair
			{
				Graphics::PropertyType_Enum Property;
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

		// NOTE: Height per item
		inline float GetRowItemHeight() const { return rowItemHeight; };

	private:
		enum class TimelineMode
		{
			DopeSheet,
			Curves
		};

		TimelineMode currentTimelineMode = TimelineMode::DopeSheet;
		
		// TODO: Should be replaced with a pointer like with the other components
		AetItemTypePtr selectedAetItem = {};

		const float rowItemHeight = 18.0f;
		bool isPlayback = false;
		bool loopPlayback = true;

		// NOTE: Speed at factor at which the playback time is incremented without editing any AetObj state
		float playbackSpeedFactor = 1.0f;
		static constexpr float playbackSpeedMin = 0.01f;
		static constexpr float playbackSpeedMax = 4.00f;

	private:
		KeyFrameRenderer keyFrameRenderer = {};
		AetTimelineController timelineController = { this };

	private:
		float GetTimelineSize() const override;
		float GetTimelineHeight() const override;

		void DrawTimelineContentNone();
		void DrawTimelineContent();

		void OnInitialize() override;
		void OnDrawTimelineHeaderWidgets() override;
		
		void OnDrawTimelineInfoColumnHeader() override;
		void OnDrawTimelineInfoColumn() override;
		void DrawTimelineInfoColumnLayer(const Graphics::AetLayer* workingLayer, const Graphics::AetObj* selectedObject) const;

		const Graphics::AetLayer* GetWorkingLayer() const;
		int GetTimelineRowCount() const;

		void OnDrawTimlineRows() override;
		void OnDrawTimlineDivisors() override;
		void OnDrawTimlineBackground() override;
		void OnDrawTimelineScrollBarRegion() override;
		
		void OnUpdate() override;
		void OnUpdateInput() override;
		
		void OnDrawTimelineContents() override;
		void PausePlayback() override;
		void ResumePlayback() override;
		void StopPlayback() override;

		void UpdateInputCursorClick();
		void DrawMouseSelection(const MouseSelectionData& selectionData);

		void UpdateCursorPlaybackTime();
		void RoundCursorTimeToNearestFrame();

	private:
		// TODO: Implement
		// Vector<KeyFrameIndex> selectedKeyFrames;

		static constexpr const char* settingsPopupName = "TimelineSettingsPopup::AetTimeline";
		static constexpr const char* timelinePropertyNameTypeSeparator = ":";

		static constexpr std::array<std::pair<const char*, const char*>, static_cast<size_t>(Graphics::PropertyType_Count)> timelinePropertyTypeNames =
		{
			std::make_pair("Transform", "Origin.X"),
			std::make_pair("Transform", "Origin.Y"),
			std::make_pair("Transform", "Position.X"),
			std::make_pair("Transform", "Position.Y"),
			std::make_pair("Transform", "Rotation"),
			std::make_pair("Transform", "Scale.X"),
			std::make_pair("Transform", "Scale.Y"),
			std::make_pair("Color",	    "Opacity"),
		};
	};
}