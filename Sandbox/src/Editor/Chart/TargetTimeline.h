#pragma once
#include "Editor/IEditorComponent.h"
#include "Editor/TimelineBase.h"
#include "Editor/AudioController.h"
#include "Audio/AudioEngine.h"
#include "Audio/AudioInstance.h"
#include "Audio/DummySampleProvider.h"
#include "Audio/MemoryAudioStream.h"
#include "Audio/Waveform.h"
#include "BaseWindow.h"
#include "Graphics/Texture.h"
#include "TimelineMap.h"
#include "TargetList.h"
#include "FileSystem/Format/SprSet.h"
#include <memory>

namespace Editor
{
	enum EditorColor;

	class TargetTimeline : public IEditorComponent, public ICallbackReceiver, TimelineBase
	{
	public:
		// Constructors / Destructors:
		// ---------------------------
		TargetTimeline(Application* parent, PvEditor* editor);
		~TargetTimeline();
		// ---------------------------

		// PvEditor Methods:
		// -----------------
		virtual void Initialize() override;
		virtual const char* GetGuiName() const override;
		virtual void DrawGui() override;
		virtual void OnLoad() override;
		virtual void OnPlaybackResumed() override;
		virtual void OnPlaybackPaused() override;
		virtual void OnPlaybackStopped() override;
		virtual void OnAudioCallback() override;
		// -----------------

	protected:
		// ----------------------
		struct
		{
			const char* testSongPath = "rom/sound/sngtst.flac";

			std::vector<TimeSpan> buttonSoundTimesList;
			AudioController audioController;

			bool updateWaveform;
			Waveform songWaveform;
		};
		// ----------------------

		// Timeline:
		// ---------
		struct
		{
			TempoMap tempoMap;
			TimelineMap timelineMap;
			TargetList targets;

			const std::array<const char*, 10> gridDivisionStrings = { "1/1", "1/2", "1/4", "1/8", "1/12", "1/16", "1/24", "1/32", "1/48", "1/64" };
			const std::array<int, 10> gridDivisions = { 1, 2, 4, 8, 12, 16, 24, 32, 48, 64 };
			int gridDivisionIndex = 0;

			int gridDivision = 16;
		};
		// ---------

		// ----------------------
		std::array<float, TargetType_Max> targetYPositions;

		// sankaku | shikaku | batsu | maru | slide_l | slide_r | slide_chain_l | slide_chain_r
		static constexpr int buttonIconsTypeCount = 8;
		static constexpr int buttonIconWidth = 52;

		std::array<ImRect, buttonIconsTypeCount * 2> buttonIconsTextureCoordinates;
		FileSystem::SprSet sprSet;
		Texture2D* buttonIconsTexture;

		bool checkHitsoundsInCallback = false;
		struct { bool Down, WasDown; } buttonPlacementKeyStates[6];
		struct { TargetType Type; int Key; } buttonPlacementMapping[6]
		{
			{ TargetType_Sankaku, 'W'},
			{ TargetType_Shikaku, 'A'},
			{ TargetType_Batsu, 'S'},
			{ TargetType_Maru, 'D'},
			{ TargetType_SlideL, 'Q'},
			{ TargetType_SlideR, 'E'},
		};
		// ----------------------

		// ----------------------
		struct
		{
			// --------------
			bool timeSelectionActive = false;
			TimelineTick timeSelectionStart, timeSelectionEnd;
			// --------------
		};

		// Timeline Button Animation:
		// --------------------------
		const TimeSpan buttonAnimationStartTime = TimeSpan::FromMilliseconds(15.0);
		const TimeSpan buttonAnimationDuration = TimeSpan::FromMilliseconds(60.0);
		const float buttonAnimationScale = 1.5f;
		struct
		{
			TimelineTick Tick;
			TimeSpan ElapsedTime;
		} buttonAnimations[TargetType_Max];
		// --------------------------

		// ----------------------
		const float ICON_SCALE = 1.0f;
		const float ROW_HEIGHT = 42.0f;
		// ----------------------

		// Child Windows:
		// --------------
		void DrawSyncWindow();
		// --------------

		// ----------------
		void InitializeButtonIcons();
		// ----------------

		// ----------------
		void OnUpdate() override;
		void UpdateTimelineMap();
		void UpdateOnCallbackSounds();
		void UpdateOnCallbackPlacementSounds();
		// ----------------

		// Timeline Widgets:
		// -----------------
		void OnDrawTimelineHeaderWidgets() override;
		// Timeline Column:
		// ----------------
		void OnDrawTimelineInfoColumnHeader() override;
		void OnDrawTimelineInfoColumn() override;
		// Timeline Base:
		// --------------
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
		// --------------

		// Timeline Actions:
		// -----------------
		void PlaceOrRemoveTarget(TimelineTick tick, TargetType type);
		void SelectNextGridDivision(int direction);
		// -----------------

		// Timeline Control:
		// -----------------
		virtual TimeSpan GetCursorTime() const override;
		virtual bool GetIsPlayback() const override;
		virtual void PausePlayback() override;
		virtual void ResumePlayback() override;
		virtual void StopPlayback() override;

		virtual float GetTimelineSize() const override;
		void OnTimelineBaseScroll() override;
		// -----------------

		// Conversion Methods:
		// -------------------
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

		int GetGridDivisionIndex() const;
		// -------------------

		// -------------------
		float GetButtonTransparency(float screenX) const;
		int GetButtonIconIndex(const TimelineTarget& target) const;
		void DrawButtonIcon(ImDrawList* drawList, const TimelineTarget& target, ImVec2 position, float scale, float transparency = 1.0f);
		// -------------------
	};
}