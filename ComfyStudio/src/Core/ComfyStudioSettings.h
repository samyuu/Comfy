#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Editor/Common/RecentFilesList.h"
#include "Editor/Chart/TargetPropertyPresets.h"
#include "Editor/Chart/BPMCalculatorWindow.h"

namespace Comfy::Studio
{
	constexpr std::string_view ComfyStudioAppSettingsFilePath = "settings_app.json";
	constexpr std::string_view ComfyStudioUserSettingsFilePath = "settings_user.json";

	// NOTE: Loaded at startup and saved on exit
	struct ComfyStudioAppSettings
	{
		bool LoadFromFile(std::string_view filePath = ComfyStudioAppSettingsFilePath);
		void SaveToFile(std::string_view filePath = ComfyStudioAppSettingsFilePath) const;
		void RestoreDefault();

		struct
		{
			std::optional<ivec4> RestoreRegion;
			std::optional<ivec2> Position;
			std::optional<ivec2> Size;
			std::optional<bool> IsFullscreen;
			std::optional<bool> IsMaximized;
			std::optional<std::string> ActiveEditorComponent;
		} LastSessionWindowState;

		struct
		{
			Editor::RecentFilesList ChartFiles;
		} RecentFiles;
	};

	// NOTE: Loaded at startup but only saved when manually edited by the user via a settings window
	struct ComfyStudioUserSettings
	{
		bool LoadFromFile(std::string_view filePath = ComfyStudioUserSettingsFilePath);
		void SaveToFile(std::string_view filePath = ComfyStudioUserSettingsFilePath) const;
		void RestoreDefault();

		// NOTE: Explicit mutable accessor to make the syntax very clear. Changes then need to be explicitly written back to disk
		inline ComfyStudioUserSettings& Mutable() const { return *const_cast<ComfyStudioUserSettings*>(this); }

		struct
		{
			struct
			{
				// i32 WindowSwapInterval; // ... or should this be part of AppData (?)
				bool EnterFullscreenOnMaximizedPlaytestStart;
				bool PlaytestAutoHideCursor;
			} Video;

			struct
			{
				// bool ReleaseAudioDevicecOnFocusLoss;
				// ... Audio backend stuff
				f32 SongVolume;
				f32 ButtonSoundVolume;
				f32 SoundEffectVolume;
				f32 MetronomeVolume;
			} Audio;

			struct
			{
				// bool AntiAliasedLines;
				// bool AntiAliasedFill;
				// ... Curve and circle line counts used by TargetRenderWindow / TargetTools
			} Gui;
		} System;

		struct
		{
		} Input;

		struct
		{
			// bool UnsavedChangesWarningDialog;
			// bool RememberRecentFiles;

			// bool AutoSaveEnabled;
			// TimeSpan AutoSaveInterval;
		} SaveAndLoad;

		struct
		{
			bool ShowButtons;
			bool ShowGrid;
			bool ShowHoldInfo;
			bool ShowBackgroundCheckerboard;
			f32 BackgroundDim;
			Editor::BeatTick PostHitLingerDuration;
			bool DisplayPracticeBackground;
		} TargetPreview;

		struct
		{
			std::vector<Editor::StaticSyncPreset> StaticSyncPresets;
			std::vector<Editor::SequencePreset> SequencePresets;

			struct
			{
				std::vector<f32> PositionsX;
				std::vector<f32> PositionsY;
				std::vector<f32> Angles;
				std::vector<i32> Frequencies;
				std::vector<f32> Amplitudes;
				std::vector<f32> Distances;
			} InspectorDropdown;
		} TargetPreset;

		struct
		{
			std::string ChartCreatorDefaultName;
		} ChartProperties;

		struct
		{
			bool AutoResetEnabled;
			bool ApplyToTempoMap;
			Editor::BPMTapSoundType TapSoundType;
		} BPMCalculator;
	};

	// NOTE: Changes are always saved so no need for const protection or manual saves
	inline ComfyStudioAppSettings GlobalAppData = {};

	// NOTE: Global variable for cleaner syntax but const to avoid accidental changes outside an explicit settings window without saving back to disk.
	//		 Modifications should then be done via the Mutable() accessor method.
	inline const ComfyStudioUserSettings GlobalUserData = {};
}
