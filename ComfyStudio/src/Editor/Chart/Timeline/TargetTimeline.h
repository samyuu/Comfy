#pragma once
#include "Editor/Core/IEditorComponent.h"
#include "Editor/Timeline/TimelineBase.h"
#include "Editor/Common/ButtonSoundController.h"
#include "Editor/Chart/Chart.h"
#include "Graphics/Auth2D/SprSet.h"
#include "Audio/Audio.h"
#include "Input/Input.h"

namespace Comfy::Studio::Editor
{
	enum EditorColor;
	class ChartEditor;

	class TargetTimeline : public TimelineBase
	{
	public:
		TargetTimeline(ChartEditor& parent);
		~TargetTimeline() = default;

	public:
		void OnSongLoaded();
		void OnPlaybackResumed();
		void OnPlaybackPaused();
		void OnPlaybackStopped();

	public:
		TimelineTick GetGridTick() const;
		TimelineTick FloorToGrid(TimelineTick tick) const;
		TimelineTick RoundToGrid(TimelineTick tick) const;

		float GetTimelinePosition(TimeSpan time) const override;
		float GetTimelinePosition(TimelineTick tick) const;

		TimelineTick GetTimelineTick(TimeSpan time) const;
		TimelineTick GetTimelineTick(float position) const;

		TimeSpan GetTimelineTime(TimelineTick tick) const;
		TimeSpan GetTimelineTime(float position) const override;

		TimelineTick GetCursorTick() const;
		TimelineTick GetCursorMouseXTick() const;

	public:
		int GetGridDivisionIndex() const;

	public:
		void UpdateTimelineMap();

	protected:
		Chart* workingChart;
		ChartEditor& chartEditor;

	protected:
		std::unique_ptr<Audio::CallbackReceiver> callbackReceiver = nullptr;

		std::vector<TimeSpan> buttonSoundTimesList;
		ButtonSoundController buttonSoundController;

		bool updateWaveform;
		Audio::Waveform songWaveform;

	protected:
		static constexpr std::array<const char*, 10> gridDivisionStrings = { "1/1", "1/2", "1/4", "1/8", "1/12", "1/16", "1/24", "1/32", "1/48", "1/64" };
		static constexpr std::array<int, 10> gridDivisions = { 1, 2, 4, 8, 12, 16, 24, 32, 48, 64 };

		int gridDivisionIndex = 0;
		int gridDivision = 16;

	protected:
		std::array<float, TargetType_Max> targetYPositions;

		// NOTE: sankaku | shikaku | batsu | maru | slide_l | slide_r | slide_chain_l | slide_chain_r
		static constexpr int buttonIconsTypeCount = 8;
		static constexpr int buttonIconWidth = 52;

		std::array<ImRect, buttonIconsTypeCount * 2> buttonIconsTextureCoordinates;

		std::unique_ptr<Graphics::SprSet> sprSet;
		std::shared_ptr<Graphics::Tex> buttonIconsTexture = nullptr;

		bool checkHitsoundsInCallback = false;
		struct { bool Down, WasDown; } buttonPlacementKeyStates[12];
		static constexpr struct { TargetType Type; Input::KeyCode Key; } buttonPlacementMapping[12]
		{
			{ TargetType_Sankaku, Input::KeyCode_W },
			{ TargetType_Shikaku, Input::KeyCode_A },
			{ TargetType_Batsu, Input::KeyCode_S },
			{ TargetType_Maru, Input::KeyCode_D },
			{ TargetType_SlideL, Input::KeyCode_Q },
			{ TargetType_SlideR, Input::KeyCode_E },

			{ TargetType_Sankaku, Input::KeyCode_I },
			{ TargetType_Shikaku, Input::KeyCode_J },
			{ TargetType_Batsu, Input::KeyCode_K },
			{ TargetType_Maru, Input::KeyCode_L },
			{ TargetType_SlideL, Input::KeyCode_U },
			{ TargetType_SlideR, Input::KeyCode_O },
		};

	protected:
		char timeInputBuffer[TimeSpan::RequiredFormatBufferSize] = "00:00.000";

	protected:
		bool timeSelectionActive = false;
		TimelineTick timeSelectionStart, timeSelectionEnd;

	protected:
		const TimeSpan buttonAnimationStartTime = TimeSpan::FromMilliseconds(15.0);
		const TimeSpan buttonAnimationDuration = TimeSpan::FromMilliseconds(60.0);
		const float buttonAnimationScale = 1.5f;
		struct
		{
			TimelineTick Tick;
			TimeSpan ElapsedTime;
		} buttonAnimations[TargetType_Max];

	protected:
		const float iconScale = 1.0f;
		const float rowHeight = 36;

	protected:
		void OnInitialize() override;
		void InitializeButtonIcons();

	protected:
		void OnUpdate() override;
		void UpdateOnCallbackSounds();
		void UpdateOnCallbackPlacementSounds();

	protected:
		void OnDrawTimelineHeaderWidgets() override;

	protected:
		void OnDrawTimelineInfoColumnHeader() override;
		void OnDrawTimelineInfoColumn() override;

	protected:
		void OnDrawTimlineRows() override;
		void OnDrawTimlineDivisors() override;
		void OnDrawTimlineBackground() override;
		void DrawWaveform();
		void DrawTimelineTempoMap();
		void DrawTimelineTargets();
		void DrawTimelineCursor() override;
		void DrawTimeSelection();
		void OnUpdateInput() override;
		void OnDrawTimelineContents() override;
		void UpdateInputCursorClick();
		void UpdateInputTargetPlacement();

	protected:
		void PlaceOrRemoveTarget(TimelineTick tick, TargetType type);
		void SelectNextGridDivision(int direction);

	protected:
		TimeSpan GetCursorTime() const override;
		bool GetIsPlayback() const override;
		void PausePlayback() override;
		void ResumePlayback() override;
		void StopPlayback() override;

		float GetTimelineSize() const override;
		void OnTimelineBaseScroll() override;

	protected:
		float GetButtonTransparency(float screenX) const;
		int GetButtonIconIndex(const TimelineTarget& target) const;
		void DrawButtonIcon(ImDrawList* drawList, const TimelineTarget& target, vec2 position, float scale, float transparency = 1.0f);
	};
}
