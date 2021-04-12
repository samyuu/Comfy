#include "ComfyStudioSettings.h"
#include "IO/JSON.h"
#include "Core/Logger.h"

namespace Comfy::Studio
{
	using namespace Editor;

	// NOTE: Underscore denote object boundaries, all IDs should follow the snake_case naming convention
	//		 and are global std::string objects to avoid reconstructing them inside inner loops
	namespace IDs
	{
		const std::string FileVersion = ".file_version";

		const std::string TargetProperties_PositionX = "position_x";
		const std::string TargetProperties_PositionY = "position_y";
		const std::string TargetProperties_Angle = "angle";
		const std::string TargetProperties_Frequency = "frequency";
		const std::string TargetProperties_Amplitude = "amplitude";
		const std::string TargetProperties_Distance = "distance";
	}

	namespace AppIDs
	{
		const std::string LastSessionWindowState = "last_session_window_state";
		const std::string LastSessionWindowState_RestoreRegion = "restore_region";
		const std::string LastSessionWindowState_Position = "position";
		const std::string LastSessionWindowState_Size = "size";
		const std::string LastSessionWindowState_IsFullscreen = "is_fullscreen";
		const std::string LastSessionWindowState_IsMaximized = "is_maximized";
		const std::string LastSessionWindowState_SwapInterval = "swap_interval";
		const std::string LastSessionWindowState_ActiveEditorComponent = "active_editor_component";

		const std::string RecentFiles = "recent_files";
		const std::string RecentFiles_ChartFiles = "chart_files";
	}

	namespace
	{
		std::optional<SemanticVersion> TryGetJsonSettingsFileVersionFromRoot(const json& rootJson)
		{
			if (auto v = JsonTryGetStr(JsonFind(rootJson, IDs::FileVersion)); v.has_value())
				return SemanticVersion::FromString(v.value());
			else
				return std::nullopt;
		}

		void SetJsonSettingsFileVersionForRoot(json& rootJson, const SemanticVersion& currentVersion)
		{
			rootJson[IDs::FileVersion] = currentVersion.ToString();
		}
	}

	bool ComfyStudioAppSettings::LoadFromFile(std::string_view filePath)
	{
		const std::optional<json> loadedJson = IO::LoadJson(filePath);
		if (!loadedJson.has_value())
			return false;

		const json& rootJson = loadedJson.value();
		const auto fileVersion = TryGetJsonSettingsFileVersionFromRoot(rootJson).value_or(SemanticVersion {});
		if (fileVersion.Major > CurrentVersion.Major)
		{
			Logger::LogErrorLine(__FUNCTION__"(): Unsupported AppSettings version detected: \"%s\". Current version: \"%s\"", fileVersion.ToString().c_str(), CurrentVersion.ToString().c_str());
			return false;
		}

		if (const json* windowStateJson = JsonFind(rootJson, AppIDs::LastSessionWindowState))
		{
			LastSessionWindowState.RestoreRegion = JsonTryGetIVec4(JsonFind(*windowStateJson, AppIDs::LastSessionWindowState_RestoreRegion));
			LastSessionWindowState.Position = JsonTryGetIVec2(JsonFind(*windowStateJson, AppIDs::LastSessionWindowState_Position));
			LastSessionWindowState.Size = JsonTryGetIVec2(JsonFind(*windowStateJson, AppIDs::LastSessionWindowState_Size));
			LastSessionWindowState.IsFullscreen = JsonTryGetBool(JsonFind(*windowStateJson, AppIDs::LastSessionWindowState_IsFullscreen));
			LastSessionWindowState.IsMaximized = JsonTryGetBool(JsonFind(*windowStateJson, AppIDs::LastSessionWindowState_IsMaximized));
			LastSessionWindowState.SwapInterval = JsonTryGetI32(JsonFind(*windowStateJson, AppIDs::LastSessionWindowState_SwapInterval));
			LastSessionWindowState.ActiveEditorComponent = JsonTryGetStr(JsonFind(*windowStateJson, AppIDs::LastSessionWindowState_ActiveEditorComponent));
		}

		if (const json* recentFilesJson = JsonFind(rootJson, AppIDs::RecentFiles))
		{
			if (const json* chartFilesJson = JsonFind(*recentFilesJson, AppIDs::RecentFiles_ChartFiles))
			{
				std::for_each(chartFilesJson->rbegin(), chartFilesJson->rend(), [&](const json& jsonIt)
				{
					if (auto v = JsonTryGetStr(jsonIt); v.has_value())
						RecentFiles.ChartFiles.Add(v.value());
				});
			}
		}

		return true;
	}

	void ComfyStudioAppSettings::SaveToFile(std::string_view filePath) const
	{
		json rootJson = json::object();
		SetJsonSettingsFileVersionForRoot(rootJson, CurrentVersion);

		json& windowStateJson = rootJson[AppIDs::LastSessionWindowState];
		JsonTrySetIVec4(windowStateJson[AppIDs::LastSessionWindowState_RestoreRegion], LastSessionWindowState.RestoreRegion);
		JsonTrySetIVec2(windowStateJson[AppIDs::LastSessionWindowState_Position], LastSessionWindowState.Position);
		JsonTrySetIVec2(windowStateJson[AppIDs::LastSessionWindowState_Size], LastSessionWindowState.Size);
		JsonTrySetBool(windowStateJson[AppIDs::LastSessionWindowState_IsFullscreen], LastSessionWindowState.IsFullscreen);
		JsonTrySetBool(windowStateJson[AppIDs::LastSessionWindowState_IsMaximized], LastSessionWindowState.IsMaximized);
		JsonTrySetI32(windowStateJson[AppIDs::LastSessionWindowState_SwapInterval], LastSessionWindowState.SwapInterval);
		JsonTrySetStr(windowStateJson[AppIDs::LastSessionWindowState_ActiveEditorComponent], LastSessionWindowState.ActiveEditorComponent);

		json& recentFilesJson = rootJson[AppIDs::RecentFiles];
		json& chartFilesJson = recentFilesJson[AppIDs::RecentFiles_ChartFiles];
		chartFilesJson = json::array();
		std::for_each(RecentFiles.ChartFiles.View().rbegin(), RecentFiles.ChartFiles.View().rend(), [&](auto& path) { chartFilesJson.emplace_back(path); });

		IO::SaveJson(filePath, rootJson);
	}

	void ComfyStudioAppSettings::RestoreDefault()
	{
		*this = {};
	}

	namespace
	{
		template <size_t TargetCount>
		StaticSyncPreset ConstructStaticSyncPreset(std::string name, std::array<PresetTargetData, TargetCount> targetData)
		{
			StaticSyncPreset result = {};
			result.Name = std::move(name);
			result.TargetCount = static_cast<u32>(TargetCount);
			for (size_t i = 0; i < TargetCount; i++)
				result.Targets[i] = targetData[i];
			return result;
		}

		std::vector<StaticSyncPreset> GetDefaultStaticSyncPresets()
		{
			constexpr size_t testPresetCount = 8;

			std::vector<StaticSyncPreset> presets;
			presets.reserve(testPresetCount);

			presets.push_back(ConstructStaticSyncPreset("Smallest Sideways Square", std::array
				{
					PresetTargetData { ButtonType::Triangle, { vec2(960.0f, 480.0f), 0.0f, 0.0f, 500.0f, 1040.0f } },
					PresetTargetData { ButtonType::Square, { vec2(864.0f, 576.0f), -90.0f, 0.0f, 500.0f, 1040.0f } },
					PresetTargetData { ButtonType::Cross, { vec2(960.0f, 672.0f), 180.0f, 0.0f, 500.0f, 1040.0f } },
					PresetTargetData { ButtonType::Circle, { vec2(1056.0f, 576.0f), +90.0f, 0.0f, 500.0f, 1040.0f } },
				}));
			presets.push_back(ConstructStaticSyncPreset("Small Sideways Square", std::array
				{
					PresetTargetData { ButtonType::Triangle, { vec2(960.0f, 336.0f), 0.0f, 0.0f, 500.0f, 960.0f } },
					PresetTargetData { ButtonType::Square, { vec2(744.0f, 552.0f), -90.0f, 0.0f, 500.0f, 960.0f } },
					PresetTargetData { ButtonType::Cross, { vec2(960.0f, 768.0f), 180.0f, 0.0f, 500.0f, 960.0f } },
					PresetTargetData { ButtonType::Circle, { vec2(1176.0f, 552.0f), 90.0f, 0.0f, 500.0f, 960.0f } },
				}));
			presets.push_back(ConstructStaticSyncPreset("Large Sideways Square", std::array
				{
					PresetTargetData { ButtonType::Triangle, { vec2(960.0f, 192.0f), 0.0f, 0.0f, 500.0f, 800.0f } },
					PresetTargetData { ButtonType::Square, { vec2(384.0f, 528.0f), -90.0f, 0.0f, 500.0f, 800.0f } },
					PresetTargetData { ButtonType::Cross, { vec2(960.0f, 864.0f), 180.0f, 0.0f, 500.0f, 800.0f } },
					PresetTargetData { ButtonType::Circle, { vec2(1536.0f, 528.0f), 90.0f, 0.0f, 500.0f, 800.0f } },
				}));

			presets.push_back(ConstructStaticSyncPreset("Curved Small Square", std::array
				{
					PresetTargetData { ButtonType::Triangle, { vec2(912.0f, 480.0f), -45.0f, 1.0f, 3000.0f, 2000.0f } },
					PresetTargetData { ButtonType::Square, { vec2(1008.0f, 480.0f),45.0f, -1.0f, 3000.0f, 2000.0f } },
					PresetTargetData { ButtonType::Cross, { vec2(912.0f, 576.0f), -135.0f, -1.0f, 3000.0f, 2000.0f } },
					PresetTargetData { ButtonType::Circle, { vec2(1008.0f, 576.0f), 135.0f, 1.0f, 3000.0f, 2000.0f } },
				}));

			presets.push_back(ConstructStaticSyncPreset("Parallelogram", std::array
				{
					PresetTargetData { ButtonType::Triangle, { vec2(240.0f, 240.0f), -90.0f, 0.0f, 500.0f, 880.0f } },
					PresetTargetData { ButtonType::Square, { vec2(720.0f, 768.0f), -90.0f, 0.0f, 500.0f, 880.0f } },
					PresetTargetData { ButtonType::Cross, { vec2(1200.0f, 240.0f), 90.0f, 0.0f, 500.0f, 880.0f } },
					PresetTargetData { ButtonType::Circle, { vec2(1680.0f, 768.0f), 90.0f, 0.0f, 500.0f, 880.0f } },
				}));

			presets.push_back(ConstructStaticSyncPreset("Fancy Square Stuff [0]", std::array
				{
					PresetTargetData { ButtonType::Triangle, { vec2(960.0f, 432.0f), 45.0f, -2.0f, 1000.0f, 1040.0f } },
					PresetTargetData { ButtonType::Square, { vec2(864.0f, 528.0f), -45.0f, -2.0f, 1000.0f, 1040.0f } },
					PresetTargetData { ButtonType::Cross, { vec2(960.0f, 624.0f), -135.0f, -2.0f, 1000.0f, 1040.0f } },
					PresetTargetData { ButtonType::Circle, { vec2(1056.0f, 528.0f), 135.0f, -2.0f, 1000.0f, 1040.0f } },
				}));
			presets.push_back(ConstructStaticSyncPreset("Fancy Square Stuff [1]", std::array
				{
					PresetTargetData { ButtonType::Triangle, { vec2(960.0f, 336.0f), 90.0f, -2.0f, 1000.0f, 1040.0f } },
					PresetTargetData { ButtonType::Square, { vec2(768.0f, 528.0f), 0.0f, -2.0f, 1000.0f, 1040.0f } },
					PresetTargetData { ButtonType::Cross, { vec2(960.0f, 720.0f), -90.0f, -2.0f, 1000.0f, 1040.0f } },
					PresetTargetData { ButtonType::Circle, { vec2(1152.0f, 528.0f), 180.0f, -2.0f, 1000.0f, 1040.0f } },
				}));
			presets.push_back(ConstructStaticSyncPreset("Fancy Square Stuff [2]", std::array
				{
					PresetTargetData { ButtonType::Triangle, { vec2(960.0f, 168.0f), 165.0f, -2.0f, 1000.0f, 2080.0f } },
					PresetTargetData { ButtonType::Square, { vec2(600.0f, 528.0f), 75.0f, -2.0f, 1000.0f, 2080.0f } },
					PresetTargetData { ButtonType::Cross, { vec2(960.0f, 888.0f), -15.0f, -2.0f, 1000.0f, 2080.0f } },
					PresetTargetData { ButtonType::Circle, { vec2(1320.0f, 528.0f), -105.0f, -2.0f, 1000.0f, 2080.0f } },
				}));

			assert(presets.size() == testPresetCount);
			return presets;
		}

		std::vector<SequencePreset> GetDefaultSequencePresets()
		{
			constexpr vec2 circleCenterSmall = vec2(960.0f, 552.0f);
			constexpr vec2 circleCenterMedium = vec2(960.0f, 552.0f + 24.0f - 48.0f);
			constexpr vec2 circleCenterLarge = circleCenterMedium;
			constexpr f32 circleRadiusSmall = 120.0f;
			constexpr f32 circleRadiusMedium = 192.0f;
			constexpr f32 circleRadiusLarge = 240.0f;

			return
			{
				SequencePreset
				{
					SequencePresetType::Circle,
					SequencePresetButtonType::SameLine,
					"Small Circle CCW",
					SequencePreset::CircleData { BeatTick::FromBars(1), circleRadiusSmall, SequencePresetCircleCounterclockwiseDirection, circleCenterSmall },
					{},
				},
				SequencePreset
				{
					SequencePresetType::Circle,
					SequencePresetButtonType::SameLine,
					"Small Circle CW",
					SequencePreset::CircleData { BeatTick::FromBars(1), circleRadiusSmall, SequencePresetCircleClockwiseDirection, circleCenterSmall },
					{},
				},
				SequencePreset
				{
					SequencePresetType::Circle,
					SequencePresetButtonType::SameLine,
					"Medium Circle CCW",
					SequencePreset::CircleData { BeatTick::FromBeats(6), circleRadiusMedium, SequencePresetCircleCounterclockwiseDirection, circleCenterMedium },
					{},
				},
				SequencePreset
				{
					SequencePresetType::Circle,
					SequencePresetButtonType::SameLine,
					"Medium Circle CW",
					SequencePreset::CircleData { BeatTick::FromBeats(6), circleRadiusMedium, SequencePresetCircleClockwiseDirection, circleCenterMedium },
					{},
				},
				SequencePreset
				{
					SequencePresetType::Circle,
					SequencePresetButtonType::SameLine,
					"Large Circle CCW",
					SequencePreset::CircleData { BeatTick::FromBars(2), circleRadiusLarge, SequencePresetCircleCounterclockwiseDirection, circleCenterLarge },
					{},
				},
				SequencePreset
				{
					SequencePresetType::Circle,
					SequencePresetButtonType::SameLine,
					"Large Circle CW",
					SequencePreset::CircleData { BeatTick::FromBars(2), circleRadiusLarge, SequencePresetCircleClockwiseDirection, circleCenterLarge },
					{},
				},

#if 0 // TODO: Add once bezier path presets are implemented
				SequencePreset { SequencePresetType::BezierPath, SequencePresetButtonType::SingleLine, "Small Heart", {}, SequencePreset::BezierPathData {} },
				SequencePreset { SequencePresetType::BezierPath, SequencePresetButtonType::SingleLine, "Triangle Clockwise", {}, SequencePreset::BezierPathData {} },
				SequencePreset { SequencePresetType::BezierPath, SequencePresetButtonType::SingleLine, "Triangle Counterclockwise", {}, SequencePreset::BezierPathData {} },
#endif
				SequencePreset { SequencePresetType::BezierPath, SequencePresetButtonType::SingleLine, "Dummy A", {}, SequencePreset::BezierPathData {} },
				SequencePreset { SequencePresetType::BezierPath, SequencePresetButtonType::SingleLine, "Dummy B", {}, SequencePreset::BezierPathData {} },
				SequencePreset { SequencePresetType::BezierPath, SequencePresetButtonType::SingleLine, "Dummy C", {}, SequencePreset::BezierPathData {} },
				SequencePreset { SequencePresetType::BezierPath, SequencePresetButtonType::SingleLine, "Dummy D", {}, SequencePreset::BezierPathData {} },
			};
		}

		TargetProperties JsonTryGetTargetProperties(const json& j)
		{
			TargetProperties result;
			result.Position.x = JsonTryGetF32(JsonFind(j, IDs::TargetProperties_PositionX)).value_or(0.0f);
			result.Position.y = JsonTryGetF32(JsonFind(j, IDs::TargetProperties_PositionY)).value_or(0.0f);
			result.Angle = JsonTryGetF32(JsonFind(j, IDs::TargetProperties_Angle)).value_or(0.0f);
			result.Frequency = JsonTryGetF32(JsonFind(j, IDs::TargetProperties_Frequency)).value_or(0.0f);
			result.Amplitude = JsonTryGetF32(JsonFind(j, IDs::TargetProperties_Amplitude)).value_or(0.0f);
			result.Distance = JsonTryGetF32(JsonFind(j, IDs::TargetProperties_Distance)).value_or(0.0f);
			return result;
		}

		void JsonSetTargetProperties(json& j, const TargetProperties& v)
		{
			j[IDs::TargetProperties_PositionX] = v.Position.x;
			j[IDs::TargetProperties_PositionY] = v.Position.y;
			j[IDs::TargetProperties_Angle] = v.Angle;
			j[IDs::TargetProperties_Frequency] = v.Frequency;
			j[IDs::TargetProperties_Amplitude] = v.Amplitude;
			j[IDs::TargetProperties_Distance] = v.Distance;
		}

	}

	namespace UserIDs
	{
		const std::string System = "system";
		const std::string System_Video = "video";

		const std::string System_Audio = "audio";
		const std::string System_Audio_SongVolume = "song_volume";
		const std::string System_Audio_ButtonSoundVolume = "button_sound_volume";
		const std::string System_Audio_SoundEffectVolume = "sound_effect_volume";
		const std::string System_Audio_MetronomeVolume = "metronome_volume";
		const std::string System_Audio_OpenDeviceOnStartup = "open_device_on_startup";
		const std::string System_Audio_CloseDeviceOnIdleFocusLoss = "close_device_on_idle_focus_loss";
		const std::string System_Audio_RequestExclusiveDeviceAccess = "request_exclusive_device_access";

		const std::string System_Gui = "gui";
		const std::string System_Gui_ShowTestMenu = "show_test_menu";
		const std::string System_Gui_AntiAliasedLines = "anti_aliased_lines";
		const std::string System_Gui_AntiAliasedFill = "anti_aliased_fill";
		const std::string System_Gui_TargetDistanceGuideCircleSegments = "target_distance_guide_circle_segments";
		const std::string System_Gui_TargetDistanceGuideMaxCount = "target_distance_guide_max_count";
		const std::string System_Gui_TargetButtonPathCurveSegments = "target_button_path_curve_segments";
		const std::string System_Gui_TargetButtonPathMaxCount = "target_button_path_max_count";

		const std::string Input = "input";
		const std::string Input_ControllerLayoutMappings = "controller_layout_mappings";
		const std::string Input_Bindings = "bindings";
		const std::string Input_PlaytestBindings = "playtest_bindings";

		const std::string TargetPreview = "target_preview";
		const std::string TargetPreview_ShowButtons = "show_buttons";
		const std::string TargetPreview_ShowGrid = "show_grid";
		const std::string TargetPreview_ShowHoldInfo = "show_hold_info";
		const std::string TargetPreview_ShowBackgroundCheckerboard = "show_background_checkerboard";
		const std::string TargetPreview_BackgroundDim = "background_dim";
		const std::string TargetPreview_PostHitLingerDurationTicks = "post_hit_linger_duration_ticks";
		const std::string TargetPreview_DisplayPracticeBackground = "display_practice_background";

		const std::string PositionTool = "position_tool";
		const std::string PositionTool_ShowDistanceGuides = "show_distance_guides";
		const std::string PositionTool_ShowTargetGrabTooltip = "show_target_grab_tooltip";
		const std::string PositionTool_UseAxisSnapGuides = "use_axis_snap_guides";
		const std::string PositionTool_AxisSnapGuideDistanceThreshold = "axis_snap_guide_distance_threshold";
		const std::string PositionTool_PositionMouseSnap = "position_mouse_snap";
		const std::string PositionTool_PositionMouseSnapRough = "position_mouse_snap_rough";
		const std::string PositionTool_PositionMouseSnapPrecise = "position_mouse_snap_precise";
		const std::string PositionTool_PositionKeyMoveStep = "position_key_move_step";
		const std::string PositionTool_PositionKeyMoveStepRough = "position_key_move_step_rough";
		const std::string PositionTool_PositionKeyMoveStepPrecise = "position_key_move_step_precise";
		const std::string PositionTool_MouseRowCenterDistanceThreshold = "mouse_row_center_distance_threshold";
		const std::string PositionTool_PositionInterpolationCommandSnap = "position_interpolation_command_snap";
		const std::string PositionTool_DiagonalMouseRowLayouts = "diagonal_mouse_row_layouts";
		const std::string PositionTool_DiagonalMouseRowLayouts_PerBeatDiagonalSpacingX = "per_beat_diagonal_spacing_x";
		const std::string PositionTool_DiagonalMouseRowLayouts_PerBeatDiagonalSpacingY = "per_beat_diagonal_spacing_y";
		const std::string PositionTool_DiagonalMouseRowLayouts_DisplayName = "display_name";

		const std::string PathTool = "path_tool";
		const std::string PathTool_AngleMouseSnap = "angle_mouse_snap";
		const std::string PathTool_AngleMouseSnapRough = "angle_mouse_snap_rough";
		const std::string PathTool_AngleMouseSnapPrecise = "angle_mouse_snap_precise";
		const std::string PathTool_AngleMouseScrollDirection = "angle_mouse_scroll_direction";
		const std::string PathTool_AngleMouseScrollStep = "angle_mouse_scroll_step";
		const std::string PathTool_AngleMouseScrollRough = "angle_mouse_scroll_step_rough";
		const std::string PathTool_AngleMouseScrollPrecise = "angle_mouse_scroll_step_precise";
		const std::string PathTool_AngleMouseMovementDistanceThreshold = "angle_mouse_movement_distance_threshold";
		const std::string PathTool_AngleMouseTargetCenterDistanceThreshold = "angle_mouse_target_center_distance_threshold";

		const std::string TargetPreset = "target_preset";
		const std::string TargetPreset_StaticSyncPresets = "static_sync_presets";
		const std::string TargetPreset_StaticSyncPresets_Name = "name";
		const std::string TargetPreset_StaticSyncPresets_Targets = "targets";
		const std::string TargetPreset_StaticSyncPresets_Targets_ButtonType = "button_type";
		const std::string TargetPreset_StaticSyncPresets_Targets_Properties = "properties";

		const std::string TargetPreset_SequencePresets = "sequence_presets";
		const std::string TargetPreset_SequencePresets_GuiButtonType = "gui_button_type";
		const std::string TargetPreset_SequencePresets_Name = "name";
		const std::string TargetPreset_SequencePresets_Circle = "circle";
		const std::string TargetPreset_SequencePresets_Circle_DurationTicks = "duration_ticks";
		const std::string TargetPreset_SequencePresets_Circle_Radius = "radius";
		const std::string TargetPreset_SequencePresets_Circle_Direction = "direction";
		const std::string TargetPreset_SequencePresets_Circle_CenterX = "center_x";
		const std::string TargetPreset_SequencePresets_Circle_CenterY = "center_y";
		const std::string TargetPreset_SequencePresets_BezierPath = "bezier_path";

		const std::string TargetPreset_InspectorDropdown = "inspector_dropdown";
		const std::string TargetPreset_InspectorDropdown_PositionsX = "positions_x";
		const std::string TargetPreset_InspectorDropdown_PositionsY = "positions_y";
		const std::string TargetPreset_InspectorDropdown_Angles = "angles";
		const std::string TargetPreset_InspectorDropdown_Frequencies = "frequencies";
		const std::string TargetPreset_InspectorDropdown_Amplitudes = "amplitudes";
		const std::string TargetPreset_InspectorDropdown_Distances = "distances";

		const std::string ChartProperties = "chart_properties";
		const std::string ChartProperties_ChartCreatorDefaultName = "chart_creator_default_name";

		const std::string BPMCalculator = "bpm_calculator";
		const std::string BPMCalculator_AutoResetEnabled = "auto_reset_enabled";
		const std::string BPMCalculator_ApplyToTempoMap = "apply_to_tempo_map";
		const std::string BPMCalculator_TapSoundType = "tap_sound_type";

		const std::string Playtest = "playtest";
		const std::string Playtest_EnterFullscreenOnMaximizedStart = "enter_fullscreen_on_maximized_start";
		const std::string Playtest_AutoHideCursor = "auto_hide_cursor";

		template <typename Func>
		void ForEachMultiBindingWithID(/* const */ ComfyStudioUserSettings& userData, Func func)
		{
			func(userData.Input.App_ToggleFullscreen, "app_toggle_fullscreen");
			func(userData.Input.App_Dialog_YesOrOk, "app_dialog_yes_or_ok");
			func(userData.Input.App_Dialog_No, "app_dialog_no");
			func(userData.Input.App_Dialog_Cancel, "app_dialog_cancel");
			func(userData.Input.App_Dialog_SelectNextTab, "app_dialog_select_next_tab");
			func(userData.Input.App_Dialog_SelectPreviousTab, "app_dialog_select_previous_tab");
			func(userData.Input.ChartEditor_ChartNew, "chart_editor_chart_new");
			func(userData.Input.ChartEditor_ChartOpen, "chart_editor_chart_open");
			func(userData.Input.ChartEditor_ChartSave, "chart_editor_chart_save");
			func(userData.Input.ChartEditor_ChartSaveAs, "chart_editor_chart_save_as");
			func(userData.Input.ChartEditor_ChartOpenDirectory, "chart_editor_chart_open_directory");
			func(userData.Input.ChartEditor_Undo, "chart_editor_undo");
			func(userData.Input.ChartEditor_Redo, "chart_editor_redo");
			func(userData.Input.ChartEditor_OpenSettings, "chart_editor_open_settings");
			func(userData.Input.ChartEditor_StartPlaytestFromStart, "chart_editor_start_playtest_from_start");
			func(userData.Input.ChartEditor_StartPlaytestFromCursor, "chart_editor_start_playtest_from_cursor");
			func(userData.Input.Timeline_CenterCursor, "timeline_center_cursor");
			func(userData.Input.Timeline_TogglePlayback, "timeline_toggle_playback");
			func(userData.Input.Timeline_StopPlayback, "timeline_stop_playback");
			func(userData.Input.TargetTimeline_Cut, "target_timeline_cut");
			func(userData.Input.TargetTimeline_Copy, "target_timeline_copy");
			func(userData.Input.TargetTimeline_Paste, "target_timeline_paste");
			func(userData.Input.TargetTimeline_MoveCursorLeft, "target_timeline_move_cursor_left");
			func(userData.Input.TargetTimeline_MoveCursorRight, "target_timeline_move_cursor_right");
			func(userData.Input.TargetTimeline_IncreaseGridPrecision, "target_timeline_increase_grid_precision");
			func(userData.Input.TargetTimeline_DecreaseGridPrecision, "target_timeline_decrease_grid_precision");
			func(userData.Input.TargetTimeline_StartEndRangeSelection, "target_timeline_start_end_range_selection");
			func(userData.Input.TargetTimeline_DeleteSelection, "target_timeline_delete_selection");
			func(userData.Input.TargetTimeline_IncreasePlaybackSpeed, "target_timeline_increase_playback_speed");
			func(userData.Input.TargetTimeline_DecreasePlaybackSpeed, "target_timeline_decrease_playback_speed");
			func(userData.Input.TargetTimeline_ToggleMetronome, "target_timeline_toggle_metronome");
			func(userData.Input.TargetTimeline_ToggleTargetHolds, "target_timeline_toggle_target_holds");
			func(userData.Input.TargetTimeline_PlaceTriangle, "target_timeline_place_triangle");
			func(userData.Input.TargetTimeline_PlaceSquare, "target_timeline_place_square");
			func(userData.Input.TargetTimeline_PlaceCross, "target_timeline_place_cross");
			func(userData.Input.TargetTimeline_PlaceCircle, "target_timeline_place_circle");
			func(userData.Input.TargetTimeline_PlaceSlideL, "target_timeline_place_slide_l");
			func(userData.Input.TargetTimeline_PlaceSlideR, "target_timeline_place_slide_r");
			func(userData.Input.TargetPreview_JumpToPreviousTarget, "target_preview_jump_to_previous_target");
			func(userData.Input.TargetPreview_JumpToNextTarget, "target_preview_jump_to_next_target");
			func(userData.Input.TargetPreview_TogglePlayback, "target_preview_toggle_playback");
			func(userData.Input.TargetPreview_SelectPositionTool, "target_preview_select_position_tool");
			func(userData.Input.TargetPreview_SelectPathTool, "target_preview_select_path_tool");
			func(userData.Input.TargetPreview_PositionTool_MoveUp, "target_preview_position_tool_move_up");
			func(userData.Input.TargetPreview_PositionTool_MoveLeft, "target_preview_position_tool_move_left");
			func(userData.Input.TargetPreview_PositionTool_MoveDown, "target_preview_position_tool_move_down");
			func(userData.Input.TargetPreview_PositionTool_MoveRight, "target_preview_position_tool_move_right");
			func(userData.Input.TargetPreview_PositionTool_FlipHorizontal, "target_preview_position_tool_flip_horizontal");
			func(userData.Input.TargetPreview_PositionTool_FlipHorizontalLocal, "target_preview_position_tool_flip_horizontal_local");
			func(userData.Input.TargetPreview_PositionTool_FlipVertical, "target_preview_position_tool_flip_vertical");
			func(userData.Input.TargetPreview_PositionTool_FlipVerticalLocal, "target_preview_position_tool_flip_vertical_local");
			func(userData.Input.TargetPreview_PositionTool_PositionInRow, "target_preview_position_tool_position_in_row");
			func(userData.Input.TargetPreview_PositionTool_PositionInRowBack, "target_preview_position_tool_position_in_row_back");
			func(userData.Input.TargetPreview_PositionTool_InterpolateLinear, "target_preview_position_tool_interpolate_linear");
			func(userData.Input.TargetPreview_PositionTool_InterpolateCircular, "target_preview_position_tool_interpolate_circular");
			func(userData.Input.TargetPreview_PositionTool_InterpolateCircularFlip, "target_preview_position_tool_interpolate_circular_flip");
			func(userData.Input.TargetPreview_PositionTool_StackPositions, "target_preview_position_tool_stack_positions");
			func(userData.Input.TargetPreview_PathTool_InvertFrequencies, "target_preview_path_tool_invert_frequencies");
			func(userData.Input.TargetPreview_PathTool_InterpolateAnglesClockwise, "target_preview_path_tool_interpolate_angles_clockwise");
			func(userData.Input.TargetPreview_PathTool_InterpolateAnglesCounterclockwise, "target_preview_path_tool_interpolate_angles");
			func(userData.Input.TargetPreview_PathTool_InterpolateDistances, "target_preview_path_tool_interpolate_distances");
			func(userData.Input.TargetPreview_PathTool_ApplyAngleIncrementsPositive, "target_preview_path_tool_apply_angle_increments_positive");
			func(userData.Input.TargetPreview_PathTool_ApplyAngleIncrementsPositiveBack, "target_preview_path_tool_apply_angle_increments_positive_back");
			func(userData.Input.TargetPreview_PathTool_ApplyAngleIncrementsNegative, "target_preview_path_tool_apply_angle_increments_negative");
			func(userData.Input.TargetPreview_PathTool_ApplyAngleIncrementsNegativeBack, "target_preview_path_tool_apply_angle_increments_negative_back");
			func(userData.Input.BPMCalculator_Tap, "bpm_calculator_tap");
			func(userData.Input.BPMCalculator_Reset, "bpm_calculator_reset");
			func(userData.Input.Playtest_ReturnToEditorCurrent, "playtest_return_to_editor_current");
			func(userData.Input.Playtest_ReturnToEditorPrePlaytest, "playtest_return_to_editor_pre_playtest");
			func(userData.Input.Playtest_ToggleAutoplay, "playtest_toggle_autoplay");
			func(userData.Input.Playtest_TogglePause, "playtest_toggle_pause");
			func(userData.Input.Playtest_RestartFromResetPoint, "playtest_restart_from_reset_point");
			func(userData.Input.Playtest_MoveResetPointBackward, "playtest_move_reset_point_backward");
			func(userData.Input.Playtest_MoveResetPointForward, "playtest_move_reset_point_forward");
		}
	}

	bool ComfyStudioUserSettings::LoadFromFile(std::string_view filePath)
	{
		const std::optional<json> loadedJson = IO::LoadJson(filePath);
		if (!loadedJson.has_value())
			return false;

		const json& rootJson = loadedJson.value();
		const auto fileVersion = TryGetJsonSettingsFileVersionFromRoot(rootJson).value_or(SemanticVersion {});
		if (fileVersion.Major > CurrentVersion.Major)
		{
			Logger::LogErrorLine(__FUNCTION__"(): Unsupported UserSettings version detected: \"%s\". Current version: \"%s\"", fileVersion.ToString().c_str(), CurrentVersion.ToString().c_str());
			return false;
		}

		// NOTE: Restore default so that unspecified objects still start off with reasonable values, thereby improving forward compatibility in case the json is from an older version.
		//		 To compensate all parser code needs to clear out all vectors to avoid duplicate entries and only assign to trivial types if the corresponding json entry was found
		RestoreDefault();

		if (const json* systemJson = JsonFind(rootJson, UserIDs::System))
		{
			if (const json* videoJson = JsonFind(*systemJson, UserIDs::System_Video))
			{
			}

			if (const json* audioJson = JsonFind(*systemJson, UserIDs::System_Audio))
			{
				JsonTryAssign(System.Audio.SongVolume, JsonTryGetF32(JsonFind(*audioJson, UserIDs::System_Audio_SongVolume)));
				JsonTryAssign(System.Audio.ButtonSoundVolume, JsonTryGetF32(JsonFind(*audioJson, UserIDs::System_Audio_ButtonSoundVolume)));
				JsonTryAssign(System.Audio.SoundEffectVolume, JsonTryGetF32(JsonFind(*audioJson, UserIDs::System_Audio_SoundEffectVolume)));
				JsonTryAssign(System.Audio.MetronomeVolume, JsonTryGetF32(JsonFind(*audioJson, UserIDs::System_Audio_MetronomeVolume)));
				JsonTryAssign(System.Audio.OpenDeviceOnStartup, JsonTryGetBool(JsonFind(*audioJson, UserIDs::System_Audio_OpenDeviceOnStartup)));
				JsonTryAssign(System.Audio.CloseDeviceOnIdleFocusLoss, JsonTryGetBool(JsonFind(*audioJson, UserIDs::System_Audio_CloseDeviceOnIdleFocusLoss)));
				JsonTryAssign(System.Audio.RequestExclusiveDeviceAccess, JsonTryGetBool(JsonFind(*audioJson, UserIDs::System_Audio_RequestExclusiveDeviceAccess)));
			}

			if (const json* guiJson = JsonFind(*systemJson, UserIDs::System_Gui))
			{
				JsonTryAssign(System.Gui.ShowTestMenu, JsonTryGetBool(JsonFind(*guiJson, UserIDs::System_Gui_ShowTestMenu)));
				JsonTryAssign(System.Gui.AntiAliasedLines, JsonTryGetBool(JsonFind(*guiJson, UserIDs::System_Gui_AntiAliasedLines)));
				JsonTryAssign(System.Gui.AntiAliasedFill, JsonTryGetBool(JsonFind(*guiJson, UserIDs::System_Gui_AntiAliasedFill)));
				JsonTryAssign(System.Gui.TargetDistanceGuideCircleSegments, JsonTryGetI32(JsonFind(*guiJson, UserIDs::System_Gui_TargetDistanceGuideCircleSegments)));
				JsonTryAssign(System.Gui.TargetDistanceGuideMaxCount, JsonTryGetI32(JsonFind(*guiJson, UserIDs::System_Gui_TargetDistanceGuideMaxCount)));
				JsonTryAssign(System.Gui.TargetButtonPathCurveSegments, JsonTryGetI32(JsonFind(*guiJson, UserIDs::System_Gui_TargetButtonPathCurveSegments)));
				JsonTryAssign(System.Gui.TargetButtonPathMaxCount, JsonTryGetI32(JsonFind(*guiJson, UserIDs::System_Gui_TargetButtonPathMaxCount)));
			}
		}

		if (const json* inputJson = JsonFind(rootJson, UserIDs::Input))
		{
			if (const json* layoutMappingsJson = JsonFind(*inputJson, UserIDs::Input_ControllerLayoutMappings))
			{
				Input.ControllerLayoutMappings.clear();
				Input.ControllerLayoutMappings.reserve(layoutMappingsJson->size());
				for (const json& layoutMappingJson : *layoutMappingsJson)
				{
					auto layoutMapping = Input::ControllerLayoutMappingFromStorageString(JsonTryGetStrView(layoutMappingJson).value_or(""));
					if (Input::IsValidControllerID(layoutMapping.ProductID))
						Input.ControllerLayoutMappings.push_back(std::move(layoutMapping));
				}
			}

			if (const json* bindingsJson = JsonFind(*inputJson, UserIDs::Input_Bindings))
			{
				UserIDs::ForEachMultiBindingWithID(*this, [&](Input::MultiBinding& multiBinding, auto&& multiBindingID)
				{
					if (const json* multiBindingJson = JsonFind(*bindingsJson, multiBindingID))
					{
						multiBinding.BindingCount = 0;
						for (const json& bindingJson : *multiBindingJson)
						{
							if (multiBinding.BindingCount < multiBinding.Bindings.size() && bindingJson.is_string())
							{
								auto& binding = multiBinding.Bindings[multiBinding.BindingCount++];
								binding = Input::BindingFromStorageString(JsonTryGetStrView(bindingJson).value_or(""));

								if (binding.IsEmpty())
									multiBinding.BindingCount--;
							}
						}
					}
				});
			}

			if (const json* playtestBindingsJson = JsonFind(*inputJson, UserIDs::Input_PlaytestBindings))
			{
				Input.PlaytestBindings.clear();
				Input.PlaytestBindings.reserve(playtestBindingsJson->size());
				for (const json& playtestBindingJson : *playtestBindingsJson)
				{
					if (auto playtestBinding = PlayTestBindingFromStorageString(JsonTryGetStrView(playtestBindingJson).value_or("")); !playtestBinding.IsEmpty())
						Input.PlaytestBindings.push_back(std::move(playtestBinding));
				}
			}
		}

		if (const json* targetPreviewJson = JsonFind(rootJson, UserIDs::TargetPreview))
		{
			JsonTryAssign(TargetPreview.ShowButtons, JsonTryGetBool(JsonFind(*targetPreviewJson, UserIDs::TargetPreview_ShowButtons)));
			JsonTryAssign(TargetPreview.ShowGrid, JsonTryGetBool(JsonFind(*targetPreviewJson, UserIDs::TargetPreview_ShowGrid)));
			JsonTryAssign(TargetPreview.ShowHoldInfo, JsonTryGetBool(JsonFind(*targetPreviewJson, UserIDs::TargetPreview_ShowHoldInfo)));
			JsonTryAssign(TargetPreview.ShowBackgroundCheckerboard, JsonTryGetBool(JsonFind(*targetPreviewJson, UserIDs::TargetPreview_ShowBackgroundCheckerboard)));
			JsonTryAssign(TargetPreview.BackgroundDim, JsonTryGetF32(JsonFind(*targetPreviewJson, UserIDs::TargetPreview_BackgroundDim)));
			if (auto v = JsonTryGetI32(JsonFind(*targetPreviewJson, UserIDs::TargetPreview_PostHitLingerDurationTicks)); v.has_value())
				TargetPreview.PostHitLingerDuration = BeatTick::FromTicks(v.value());
			JsonTryAssign(TargetPreview.DisplayPracticeBackground, JsonTryGetBool(JsonFind(*targetPreviewJson, UserIDs::TargetPreview_DisplayPracticeBackground)));
		}

		if (const json* positionToolJson = JsonFind(rootJson, UserIDs::PositionTool))
		{
			JsonTryAssign(PositionTool.ShowDistanceGuides, JsonTryGetBool(JsonFind(*positionToolJson, UserIDs::PositionTool_ShowDistanceGuides)));
			JsonTryAssign(PositionTool.ShowTargetGrabTooltip, JsonTryGetBool(JsonFind(*positionToolJson, UserIDs::PositionTool_ShowTargetGrabTooltip)));
			JsonTryAssign(PositionTool.UseAxisSnapGuides, JsonTryGetBool(JsonFind(*positionToolJson, UserIDs::PositionTool_UseAxisSnapGuides)));
			JsonTryAssign(PositionTool.AxisSnapGuideDistanceThreshold, JsonTryGetF32(JsonFind(*positionToolJson, UserIDs::PositionTool_AxisSnapGuideDistanceThreshold)));
			JsonTryAssign(PositionTool.PositionMouseSnap, JsonTryGetF32(JsonFind(*positionToolJson, UserIDs::PositionTool_PositionMouseSnap)));
			JsonTryAssign(PositionTool.PositionMouseSnapRough, JsonTryGetF32(JsonFind(*positionToolJson, UserIDs::PositionTool_PositionMouseSnapRough)));
			JsonTryAssign(PositionTool.PositionMouseSnapPrecise, JsonTryGetF32(JsonFind(*positionToolJson, UserIDs::PositionTool_PositionMouseSnapPrecise)));
			JsonTryAssign(PositionTool.PositionKeyMoveStep, JsonTryGetF32(JsonFind(*positionToolJson, UserIDs::PositionTool_PositionKeyMoveStep)));
			JsonTryAssign(PositionTool.PositionKeyMoveStepRough, JsonTryGetF32(JsonFind(*positionToolJson, UserIDs::PositionTool_PositionKeyMoveStepRough)));
			JsonTryAssign(PositionTool.PositionKeyMoveStepPrecise, JsonTryGetF32(JsonFind(*positionToolJson, UserIDs::PositionTool_PositionKeyMoveStepPrecise)));
			JsonTryAssign(PositionTool.MouseRowCenterDistanceThreshold, JsonTryGetF32(JsonFind(*positionToolJson, UserIDs::PositionTool_MouseRowCenterDistanceThreshold)));
			JsonTryAssign(PositionTool.PositionInterpolationCommandSnap, JsonTryGetF32(JsonFind(*positionToolJson, UserIDs::PositionTool_PositionInterpolationCommandSnap)));
			if (const json* diagonalRowLayoutsJson = JsonFind(*positionToolJson, UserIDs::PositionTool_DiagonalMouseRowLayouts))
			{
				PositionTool.DiagonalMouseRowLayouts.clear();
				PositionTool.DiagonalMouseRowLayouts.reserve(diagonalRowLayoutsJson->size());
				for (const json& rowLayoutJson : *diagonalRowLayoutsJson)
				{
					auto& rowLayout = PositionTool.DiagonalMouseRowLayouts.emplace_back();
					rowLayout.PerBeatDiagonalSpacing.x = JsonTryGetF32(JsonFind(rowLayoutJson, UserIDs::PositionTool_DiagonalMouseRowLayouts_PerBeatDiagonalSpacingX)).value_or(0.0f);
					rowLayout.PerBeatDiagonalSpacing.y = JsonTryGetF32(JsonFind(rowLayoutJson, UserIDs::PositionTool_DiagonalMouseRowLayouts_PerBeatDiagonalSpacingY)).value_or(0.0f);
					rowLayout.DisplayName = std::move(JsonTryGetStr(JsonFind(rowLayoutJson, UserIDs::PositionTool_DiagonalMouseRowLayouts_DisplayName)).value_or(""));
				}
			}
		}

		if (const json* pathToolJson = JsonFind(rootJson, UserIDs::PathTool))
		{
			JsonTryAssign(PathTool.AngleMouseSnap, JsonTryGetF32(JsonFind(*pathToolJson, UserIDs::PathTool_AngleMouseSnap)));
			JsonTryAssign(PathTool.AngleMouseSnapRough, JsonTryGetF32(JsonFind(*pathToolJson, UserIDs::PathTool_AngleMouseSnapRough)));
			JsonTryAssign(PathTool.AngleMouseSnapPrecise, JsonTryGetF32(JsonFind(*pathToolJson, UserIDs::PathTool_AngleMouseSnapPrecise)));
			JsonTryAssign(PathTool.AngleMouseScrollDirection, JsonTryGetF32(JsonFind(*pathToolJson, UserIDs::PathTool_AngleMouseScrollDirection)));
			JsonTryAssign(PathTool.AngleMouseScrollStep, JsonTryGetF32(JsonFind(*pathToolJson, UserIDs::PathTool_AngleMouseScrollStep)));
			JsonTryAssign(PathTool.AngleMouseScrollRough, JsonTryGetF32(JsonFind(*pathToolJson, UserIDs::PathTool_AngleMouseScrollRough)));
			JsonTryAssign(PathTool.AngleMouseScrollPrecise, JsonTryGetF32(JsonFind(*pathToolJson, UserIDs::PathTool_AngleMouseScrollPrecise)));
			JsonTryAssign(PathTool.AngleMouseMovementDistanceThreshold, JsonTryGetF32(JsonFind(*pathToolJson, UserIDs::PathTool_AngleMouseMovementDistanceThreshold)));
			JsonTryAssign(PathTool.AngleMouseTargetCenterDistanceThreshold, JsonTryGetF32(JsonFind(*pathToolJson, UserIDs::PathTool_AngleMouseTargetCenterDistanceThreshold)));
		}

		if (const json* targetPresetJson = JsonFind(rootJson, UserIDs::TargetPreset))
		{
			if (const json* syncPresetsJson = JsonFind(*targetPresetJson, UserIDs::TargetPreset_StaticSyncPresets))
			{
				TargetPreset.StaticSyncPresets.clear();
				TargetPreset.StaticSyncPresets.reserve(syncPresetsJson->size());
				for (const json& syncPresetJson : *syncPresetsJson)
				{
					auto& syncPreset = TargetPreset.StaticSyncPresets.emplace_back();
					syncPreset.Name = std::move(JsonTryGetStr(JsonFind(syncPresetJson, UserIDs::TargetPreset_StaticSyncPresets_Name)).value_or(""));

					if (const json* targetDataArrayJson = JsonFind(syncPresetJson, UserIDs::TargetPreset_StaticSyncPresets_Targets))
					{
						for (const json& targetDataJson : *targetDataArrayJson)
						{
							if (syncPreset.TargetCount < syncPreset.Targets.size())
							{
								auto& targetData = syncPreset.Targets[syncPreset.TargetCount++];
								targetData.Type = static_cast<ButtonType>(JsonTryGetI32(JsonFind(targetDataJson, UserIDs::TargetPreset_StaticSyncPresets_Targets_ButtonType)).value_or(0));

								if (const json* propertiesJson = JsonFind(targetDataJson, UserIDs::TargetPreset_StaticSyncPresets_Targets_Properties))
									targetData.Properties = JsonTryGetTargetProperties(*propertiesJson);
							}
						}
					}
				}
			}

			if (const json* sequencePresetsJson = JsonFind(*targetPresetJson, UserIDs::TargetPreset_SequencePresets))
			{
				TargetPreset.SequencePresets.clear();
				TargetPreset.SequencePresets.reserve(sequencePresetsJson->size());
				for (const json& sequencePresetJson : *sequencePresetsJson)
				{
					auto& sequencePreset = TargetPreset.SequencePresets.emplace_back();
					sequencePreset.ButtonType = static_cast<SequencePresetButtonType>(JsonTryGetI32(JsonFind(sequencePresetJson, UserIDs::TargetPreset_SequencePresets_GuiButtonType)).value_or(0));
					sequencePreset.Name = std::move(JsonTryGetStr(JsonFind(sequencePresetJson, UserIDs::TargetPreset_SequencePresets_Name)).value_or(""));

					if (const json* circleJson = JsonFind(sequencePresetJson, UserIDs::TargetPreset_SequencePresets_Circle))
					{
						sequencePreset.Type = SequencePresetType::Circle;
						sequencePreset.Circle.Duration = BeatTick::FromTicks(JsonTryGetI32(JsonFind(*circleJson, UserIDs::TargetPreset_SequencePresets_Circle_DurationTicks)).value_or(0));
						sequencePreset.Circle.Radius = JsonTryGetF32(JsonFind(*circleJson, UserIDs::TargetPreset_SequencePresets_Circle_Radius)).value_or(0.0f);
						sequencePreset.Circle.Direction = JsonTryGetF32(JsonFind(*circleJson, UserIDs::TargetPreset_SequencePresets_Circle_Direction)).value_or(0.0f);
						sequencePreset.Circle.Center.x = JsonTryGetF32(JsonFind(*circleJson, UserIDs::TargetPreset_SequencePresets_Circle_CenterX)).value_or(0.0f);
						sequencePreset.Circle.Center.y = JsonTryGetF32(JsonFind(*circleJson, UserIDs::TargetPreset_SequencePresets_Circle_CenterY)).value_or(0.0f);
					}
					else if (const json* bezierPathJson = JsonFind(sequencePresetJson, UserIDs::TargetPreset_SequencePresets_BezierPath))
					{
						// TODO: ...
						sequencePreset.Type = SequencePresetType::BezierPath;
					}
				}
			}

			if (const json* inspectorDropdownJson = JsonFind(*targetPresetJson, UserIDs::TargetPreset_InspectorDropdown))
			{
				auto parseVector = [inspectorDropdownJson](auto&& id, auto& outVector)
				{
					if (const json* arrayJson = JsonFind(*inspectorDropdownJson, id))
					{
						outVector.clear();
						outVector.reserve(arrayJson->size());
						for (const json& itemJson : *arrayJson)
						{
							if constexpr (std::is_floating_point_v<typename std::remove_reference_t<decltype(outVector)>::value_type>)
								outVector.push_back(JsonTryGetF32(itemJson).value_or(0.0f));
							else
								outVector.push_back(JsonTryGetI32(itemJson).value_or(0));
						}
					}
				};

				parseVector(UserIDs::TargetPreset_InspectorDropdown_PositionsX, TargetPreset.InspectorDropdown.PositionsX);
				parseVector(UserIDs::TargetPreset_InspectorDropdown_PositionsY, TargetPreset.InspectorDropdown.PositionsY);
				parseVector(UserIDs::TargetPreset_InspectorDropdown_Angles, TargetPreset.InspectorDropdown.Angles);
				parseVector(UserIDs::TargetPreset_InspectorDropdown_Frequencies, TargetPreset.InspectorDropdown.Frequencies);
				parseVector(UserIDs::TargetPreset_InspectorDropdown_Amplitudes, TargetPreset.InspectorDropdown.Amplitudes);
				parseVector(UserIDs::TargetPreset_InspectorDropdown_Distances, TargetPreset.InspectorDropdown.Distances);
			}
		}

		if (const json* chartPropertiesJson = JsonFind(rootJson, UserIDs::ChartProperties))
		{
			JsonTryAssign(ChartProperties.ChartCreatorDefaultName, JsonTryGetStr(JsonFind(*chartPropertiesJson, UserIDs::ChartProperties_ChartCreatorDefaultName)));
		}

		if (const json* bpmCalculatorJson = JsonFind(rootJson, UserIDs::BPMCalculator))
		{
			JsonTryAssign(BPMCalculator.AutoResetEnabled, JsonTryGetBool(JsonFind(*bpmCalculatorJson, UserIDs::BPMCalculator_AutoResetEnabled)));
			JsonTryAssign(BPMCalculator.ApplyToTempoMap, JsonTryGetBool(JsonFind(*bpmCalculatorJson, UserIDs::BPMCalculator_ApplyToTempoMap)));
			if (auto v = JsonTryGetI32(JsonFind(*bpmCalculatorJson, UserIDs::BPMCalculator_TapSoundType)); v.has_value())
				BPMCalculator.TapSoundType = static_cast<BPMTapSoundType>(v.value());
		}

		if (const json* playtestJson = JsonFind(rootJson, UserIDs::Playtest))
		{
			JsonTryAssign(Playtest.EnterFullscreenOnMaximizedStart, JsonTryGetBool(JsonFind(*playtestJson, UserIDs::Playtest_EnterFullscreenOnMaximizedStart)));
			JsonTryAssign(Playtest.AutoHideCursor, JsonTryGetBool(JsonFind(*playtestJson, UserIDs::Playtest_AutoHideCursor)));
		}

		return true;
	}

	void ComfyStudioUserSettings::SaveToFile(std::string_view filePath) const
	{
		json rootJson = json::object();
		SetJsonSettingsFileVersionForRoot(rootJson, CurrentVersion);

		json& systemJson = rootJson[UserIDs::System];
		{
			json& videoJson = systemJson[UserIDs::System_Video];
			videoJson = json::object();

			json& audioJson = systemJson[UserIDs::System_Audio];
			audioJson[UserIDs::System_Audio_SongVolume] = System.Audio.SongVolume;
			audioJson[UserIDs::System_Audio_ButtonSoundVolume] = System.Audio.ButtonSoundVolume;
			audioJson[UserIDs::System_Audio_SoundEffectVolume] = System.Audio.SoundEffectVolume;
			audioJson[UserIDs::System_Audio_MetronomeVolume] = System.Audio.MetronomeVolume;
			audioJson[UserIDs::System_Audio_OpenDeviceOnStartup] = System.Audio.OpenDeviceOnStartup;
			audioJson[UserIDs::System_Audio_CloseDeviceOnIdleFocusLoss] = System.Audio.CloseDeviceOnIdleFocusLoss;
			audioJson[UserIDs::System_Audio_RequestExclusiveDeviceAccess] = System.Audio.RequestExclusiveDeviceAccess;

			json& guiJson = systemJson[UserIDs::System_Gui];
			guiJson[UserIDs::System_Gui_ShowTestMenu] = System.Gui.ShowTestMenu;
			guiJson[UserIDs::System_Gui_AntiAliasedLines] = System.Gui.AntiAliasedLines;
			guiJson[UserIDs::System_Gui_AntiAliasedFill] = System.Gui.AntiAliasedFill;
			guiJson[UserIDs::System_Gui_TargetDistanceGuideCircleSegments] = System.Gui.TargetDistanceGuideCircleSegments;
			guiJson[UserIDs::System_Gui_TargetDistanceGuideMaxCount] = System.Gui.TargetDistanceGuideMaxCount;
			guiJson[UserIDs::System_Gui_TargetButtonPathCurveSegments] = System.Gui.TargetButtonPathCurveSegments;
			guiJson[UserIDs::System_Gui_TargetButtonPathMaxCount] = System.Gui.TargetButtonPathMaxCount;
		}

		json& inputJson = rootJson[UserIDs::Input];
		{
			json& layoutMappingsJson = inputJson[UserIDs::Input_ControllerLayoutMappings];
			layoutMappingsJson = json::array();
			for (const auto& layoutMapping : Input.ControllerLayoutMappings)
				layoutMappingsJson.emplace_back(Input::ControllerLayoutMappingToStorageString(layoutMapping));

			json& bindingsJson = inputJson[UserIDs::Input_Bindings];
			UserIDs::ForEachMultiBindingWithID(*const_cast<ComfyStudioUserSettings*>(this), [&](const Input::MultiBinding& multiBinding, auto&& multiBindingID)
			{
				json& multiBindingJson = bindingsJson[multiBindingID];
				multiBindingJson = json::array();
				for (const auto& binding : multiBinding)
					multiBindingJson.emplace_back(Input::BindingToStorageString(binding).data());
			});

			json& playtestBindingsJson = inputJson[UserIDs::Input_PlaytestBindings];
			playtestBindingsJson = json::array();
			for (const auto& playtestBinding : Input.PlaytestBindings)
				playtestBindingsJson.emplace_back(PlayTestBindingToStorageString(playtestBinding).data());
		}

		json& targetPreviewJson = rootJson[UserIDs::TargetPreview];
		{
			targetPreviewJson[UserIDs::TargetPreview_ShowButtons] = TargetPreview.ShowButtons;
			targetPreviewJson[UserIDs::TargetPreview_ShowGrid] = TargetPreview.ShowGrid;
			targetPreviewJson[UserIDs::TargetPreview_ShowHoldInfo] = TargetPreview.ShowHoldInfo;
			targetPreviewJson[UserIDs::TargetPreview_ShowBackgroundCheckerboard] = TargetPreview.ShowBackgroundCheckerboard;
			targetPreviewJson[UserIDs::TargetPreview_BackgroundDim] = TargetPreview.BackgroundDim;
			targetPreviewJson[UserIDs::TargetPreview_PostHitLingerDurationTicks] = TargetPreview.PostHitLingerDuration.Ticks();
			targetPreviewJson[UserIDs::TargetPreview_DisplayPracticeBackground] = TargetPreview.DisplayPracticeBackground;
		}

		json& positionToolJson = rootJson[UserIDs::PositionTool];
		{
			positionToolJson[UserIDs::PositionTool_ShowDistanceGuides] = PositionTool.ShowDistanceGuides;
			positionToolJson[UserIDs::PositionTool_ShowTargetGrabTooltip] = PositionTool.ShowTargetGrabTooltip;
			positionToolJson[UserIDs::PositionTool_UseAxisSnapGuides] = PositionTool.UseAxisSnapGuides;
			positionToolJson[UserIDs::PositionTool_AxisSnapGuideDistanceThreshold] = PositionTool.AxisSnapGuideDistanceThreshold;
			positionToolJson[UserIDs::PositionTool_PositionMouseSnap] = PositionTool.PositionMouseSnap;
			positionToolJson[UserIDs::PositionTool_PositionMouseSnapRough] = PositionTool.PositionMouseSnapRough;
			positionToolJson[UserIDs::PositionTool_PositionMouseSnapPrecise] = PositionTool.PositionMouseSnapPrecise;
			positionToolJson[UserIDs::PositionTool_PositionKeyMoveStep] = PositionTool.PositionKeyMoveStep;
			positionToolJson[UserIDs::PositionTool_PositionKeyMoveStepRough] = PositionTool.PositionKeyMoveStepRough;
			positionToolJson[UserIDs::PositionTool_PositionKeyMoveStepPrecise] = PositionTool.PositionKeyMoveStepPrecise;
			positionToolJson[UserIDs::PositionTool_MouseRowCenterDistanceThreshold] = PositionTool.MouseRowCenterDistanceThreshold;
			positionToolJson[UserIDs::PositionTool_PositionInterpolationCommandSnap] = PositionTool.PositionInterpolationCommandSnap;
			json& diagonalRowLayoutsJson = positionToolJson[UserIDs::PositionTool_DiagonalMouseRowLayouts];
			diagonalRowLayoutsJson = json::array();
			for (const auto& rowLayout : PositionTool.DiagonalMouseRowLayouts)
			{
				json& rowLayoutJson = diagonalRowLayoutsJson.emplace_back(json::object());
				rowLayoutJson[UserIDs::PositionTool_DiagonalMouseRowLayouts_PerBeatDiagonalSpacingX] = rowLayout.PerBeatDiagonalSpacing.x;
				rowLayoutJson[UserIDs::PositionTool_DiagonalMouseRowLayouts_PerBeatDiagonalSpacingY] = rowLayout.PerBeatDiagonalSpacing.y;
				rowLayoutJson[UserIDs::PositionTool_DiagonalMouseRowLayouts_DisplayName] = rowLayout.DisplayName;
			}
		}

		json& pathToolJson = rootJson[UserIDs::PathTool];
		{
			pathToolJson[UserIDs::PathTool_AngleMouseSnap] = PathTool.AngleMouseSnap;
			pathToolJson[UserIDs::PathTool_AngleMouseSnapRough] = PathTool.AngleMouseSnapRough;
			pathToolJson[UserIDs::PathTool_AngleMouseSnapPrecise] = PathTool.AngleMouseSnapPrecise;
			pathToolJson[UserIDs::PathTool_AngleMouseScrollDirection] = PathTool.AngleMouseScrollDirection;
			pathToolJson[UserIDs::PathTool_AngleMouseScrollStep] = PathTool.AngleMouseScrollStep;
			pathToolJson[UserIDs::PathTool_AngleMouseScrollRough] = PathTool.AngleMouseScrollRough;
			pathToolJson[UserIDs::PathTool_AngleMouseScrollPrecise] = PathTool.AngleMouseScrollPrecise;
			pathToolJson[UserIDs::PathTool_AngleMouseMovementDistanceThreshold] = PathTool.AngleMouseMovementDistanceThreshold;
			pathToolJson[UserIDs::PathTool_AngleMouseTargetCenterDistanceThreshold] = PathTool.AngleMouseTargetCenterDistanceThreshold;
		}

		json& targetPresetJson = rootJson[UserIDs::TargetPreset];
		{
			json& syncPresetsJson = targetPresetJson[UserIDs::TargetPreset_StaticSyncPresets];
			syncPresetsJson = json::array();
			for (const auto& syncPreset : TargetPreset.StaticSyncPresets)
			{
				json& syncPresetJson = syncPresetsJson.emplace_back(json::object());
				json& targetDataArrayJson = syncPresetJson[UserIDs::TargetPreset_StaticSyncPresets_Targets];

				syncPresetJson[UserIDs::TargetPreset_StaticSyncPresets_Name] = syncPreset.Name;
				for (size_t i = 0; i < syncPreset.TargetCount; i++)
				{
					const auto& targetData = syncPreset.Targets[i];
					json& targetDataJson = targetDataArrayJson.emplace_back(json::object());

					targetDataJson[UserIDs::TargetPreset_StaticSyncPresets_Targets_ButtonType] = static_cast<i32>(targetData.Type);
					JsonSetTargetProperties(targetDataJson[UserIDs::TargetPreset_StaticSyncPresets_Targets_Properties], targetData.Properties);
				}
			}

			json& sequencePresetsJson = targetPresetJson[UserIDs::TargetPreset_SequencePresets];
			sequencePresetsJson = json::array();
			for (const auto& sequencePreset : TargetPreset.SequencePresets)
			{
				json& sequencePresetJson = sequencePresetsJson.emplace_back(json::object());
				sequencePresetJson[UserIDs::TargetPreset_SequencePresets_GuiButtonType] = static_cast<i32>(sequencePreset.ButtonType);
				sequencePresetJson[UserIDs::TargetPreset_SequencePresets_Name] = sequencePreset.Name;

				if (sequencePreset.Type == SequencePresetType::Circle)
				{
					json& circleJson = sequencePresetJson[UserIDs::TargetPreset_SequencePresets_Circle];
					circleJson[UserIDs::TargetPreset_SequencePresets_Circle_DurationTicks] = sequencePreset.Circle.Duration.Ticks();
					circleJson[UserIDs::TargetPreset_SequencePresets_Circle_Radius] = sequencePreset.Circle.Radius;
					circleJson[UserIDs::TargetPreset_SequencePresets_Circle_Direction] = sequencePreset.Circle.Direction;
					circleJson[UserIDs::TargetPreset_SequencePresets_Circle_CenterX] = sequencePreset.Circle.Center.x;
					circleJson[UserIDs::TargetPreset_SequencePresets_Circle_CenterY] = sequencePreset.Circle.Center.y;
				}
				else if (sequencePreset.Type == SequencePresetType::BezierPath)
				{
					json& bezierPathJson = sequencePresetJson[UserIDs::TargetPreset_SequencePresets_BezierPath];
					// TODO: ...
					bezierPathJson = json::object();
				}
			}

			json& inspectorDropdownJson = targetPresetJson[UserIDs::TargetPreset_InspectorDropdown];
			inspectorDropdownJson[UserIDs::TargetPreset_InspectorDropdown_PositionsX] = TargetPreset.InspectorDropdown.PositionsX;
			inspectorDropdownJson[UserIDs::TargetPreset_InspectorDropdown_PositionsY] = TargetPreset.InspectorDropdown.PositionsY;
			inspectorDropdownJson[UserIDs::TargetPreset_InspectorDropdown_Angles] = TargetPreset.InspectorDropdown.Angles;
			inspectorDropdownJson[UserIDs::TargetPreset_InspectorDropdown_Frequencies] = TargetPreset.InspectorDropdown.Frequencies;
			inspectorDropdownJson[UserIDs::TargetPreset_InspectorDropdown_Amplitudes] = TargetPreset.InspectorDropdown.Amplitudes;
			inspectorDropdownJson[UserIDs::TargetPreset_InspectorDropdown_Distances] = TargetPreset.InspectorDropdown.Distances;
		}

		json& chartPropertiesJson = rootJson[UserIDs::ChartProperties];
		{
			chartPropertiesJson[UserIDs::ChartProperties_ChartCreatorDefaultName] = ChartProperties.ChartCreatorDefaultName;
		}

		json& bpmCalculatorJson = rootJson[UserIDs::BPMCalculator];
		{
			bpmCalculatorJson[UserIDs::BPMCalculator_AutoResetEnabled] = BPMCalculator.AutoResetEnabled;
			bpmCalculatorJson[UserIDs::BPMCalculator_ApplyToTempoMap] = BPMCalculator.ApplyToTempoMap;
			bpmCalculatorJson[UserIDs::BPMCalculator_TapSoundType] = static_cast<i32>(BPMCalculator.TapSoundType);
		}

		json& playtestJson = rootJson[UserIDs::Playtest];
		{
			playtestJson[UserIDs::Playtest_EnterFullscreenOnMaximizedStart] = Playtest.EnterFullscreenOnMaximizedStart;
			playtestJson[UserIDs::Playtest_AutoHideCursor] = Playtest.AutoHideCursor;
		}

		IO::SaveJson(filePath, rootJson);
	}

	void ComfyStudioUserSettings::RestoreDefault()
	{
		*this = {};

		System.Audio.SongVolume = 1.0f;
		System.Audio.ButtonSoundVolume = 1.0f;
		System.Audio.SoundEffectVolume = 1.0f;
		System.Audio.MetronomeVolume = 1.0f;
		System.Audio.OpenDeviceOnStartup = false;
		System.Audio.CloseDeviceOnIdleFocusLoss = true;
		System.Audio.RequestExclusiveDeviceAccess = true;

		System.Gui.ShowTestMenu = false;
		System.Gui.AntiAliasedLines = true;
		System.Gui.AntiAliasedFill = true;
		System.Gui.TargetDistanceGuideCircleSegments = 64;
		System.Gui.TargetDistanceGuideMaxCount = 64;
		System.Gui.TargetButtonPathCurveSegments = 32;
		System.Gui.TargetButtonPathMaxCount = 64;

		{
			using namespace Input;

			Input.ControllerLayoutMappings.clear();
			if (const auto[ds4Layouts, ds4LayoutsCount] = Input::GetKnownDS4LayoutMappingsView(); ds4Layouts != nullptr)
			{
				Input.ControllerLayoutMappings.reserve(ds4LayoutsCount);
				for (size_t i = 0; i < ds4LayoutsCount; i++)
					Input.ControllerLayoutMappings.push_back(ds4Layouts[i]);
			}

			Input.App_ToggleFullscreen = MultiBinding(Binding(KeyCode_F11));
			Input.App_Dialog_YesOrOk = MultiBinding(Binding(KeyCode_Enter));
			Input.App_Dialog_No = MultiBinding(Binding(KeyCode_Backspace));
			Input.App_Dialog_Cancel = MultiBinding(Binding(KeyCode_Escape));
			Input.App_Dialog_SelectNextTab = MultiBinding(Binding(KeyCode_Tab, KeyModifiers_Ctrl));
			Input.App_Dialog_SelectPreviousTab = MultiBinding(Binding(KeyCode_Tab, KeyModifiers_CtrlShift));

			Input.ChartEditor_ChartNew = MultiBinding(Binding(KeyCode_N, KeyModifiers_Ctrl));
			Input.ChartEditor_ChartOpen = MultiBinding(Binding(KeyCode_O, KeyModifiers_Ctrl));
			Input.ChartEditor_ChartSave = MultiBinding(Binding(KeyCode_S, KeyModifiers_Ctrl));
			Input.ChartEditor_ChartSaveAs = MultiBinding(Binding(KeyCode_S, KeyModifiers_CtrlShift));
			Input.ChartEditor_ChartOpenDirectory = {};
			Input.ChartEditor_Undo = MultiBinding(Binding(KeyCode_Z, KeyModifiers_Ctrl));
			Input.ChartEditor_Redo = MultiBinding(Binding(KeyCode_Y, KeyModifiers_Ctrl));
			Input.ChartEditor_OpenSettings = MultiBinding(Binding(KeyCode_OEMComma, KeyModifiers_Ctrl));
			Input.ChartEditor_StartPlaytestFromStart = MultiBinding(Binding(KeyCode_F5));
			Input.ChartEditor_StartPlaytestFromCursor = MultiBinding(Binding(KeyCode_F6));

			Input.Timeline_CenterCursor = MultiBinding(Binding(KeyCode_Escape));
			Input.Timeline_TogglePlayback = MultiBinding(Binding(KeyCode_Space));
			Input.Timeline_StopPlayback = MultiBinding(Binding(KeyCode_Escape));

			Input.TargetTimeline_Cut = MultiBinding(Binding(KeyCode_X, KeyModifiers_Ctrl));
			Input.TargetTimeline_Copy = MultiBinding(Binding(KeyCode_C, KeyModifiers_Ctrl));
			Input.TargetTimeline_Paste = MultiBinding(Binding(KeyCode_V, KeyModifiers_Ctrl));
			Input.TargetTimeline_MoveCursorLeft = MultiBinding(Binding(KeyCode_Left));
			Input.TargetTimeline_MoveCursorRight = MultiBinding(Binding(KeyCode_Right));
			Input.TargetTimeline_IncreaseGridPrecision = MultiBinding(Binding(KeyCode_Up), Binding(KeyCode_MouseX2));
			Input.TargetTimeline_DecreaseGridPrecision = MultiBinding(Binding(KeyCode_Down), Binding(KeyCode_MouseX1));
			Input.TargetTimeline_StartEndRangeSelection = MultiBinding(Binding(KeyCode_Tab));
			Input.TargetTimeline_DeleteSelection = MultiBinding(Binding(KeyCode_Delete));
			Input.TargetTimeline_IncreasePlaybackSpeed = MultiBinding(Binding(KeyCode_OEMPeriod));
			Input.TargetTimeline_DecreasePlaybackSpeed = MultiBinding(Binding(KeyCode_OEMComma));
			Input.TargetTimeline_ToggleMetronome = MultiBinding(Binding(KeyCode_M));
			Input.TargetTimeline_ToggleTargetHolds = MultiBinding(Binding(KeyCode_F));
			Input.TargetTimeline_PlaceTriangle = MultiBinding(Binding(KeyCode_W), Binding(KeyCode_I));
			Input.TargetTimeline_PlaceSquare = MultiBinding(Binding(KeyCode_A), Binding(KeyCode_J));
			Input.TargetTimeline_PlaceCross = MultiBinding(Binding(KeyCode_S), Binding(KeyCode_K));
			Input.TargetTimeline_PlaceCircle = MultiBinding(Binding(KeyCode_D), Binding(KeyCode_L));
			Input.TargetTimeline_PlaceSlideL = MultiBinding(Binding(KeyCode_Q), Binding(KeyCode_U));
			Input.TargetTimeline_PlaceSlideR = MultiBinding(Binding(KeyCode_E), Binding(KeyCode_O));

			Input.TargetPreview_JumpToPreviousTarget = MultiBinding(Binding(KeyCode_Q));
			Input.TargetPreview_JumpToNextTarget = MultiBinding(Binding(KeyCode_E));
			Input.TargetPreview_TogglePlayback = Input.Timeline_TogglePlayback;
			Input.TargetPreview_SelectPositionTool = MultiBinding(Binding(KeyCode_1));
			Input.TargetPreview_SelectPathTool = MultiBinding(Binding(KeyCode_2));
			Input.TargetPreview_PositionTool_MoveUp = MultiBinding(Binding(KeyCode_W), Binding(KeyCode_Up));
			Input.TargetPreview_PositionTool_MoveLeft = MultiBinding(Binding(KeyCode_A), Binding(KeyCode_Left));
			Input.TargetPreview_PositionTool_MoveDown = MultiBinding(Binding(KeyCode_S), Binding(KeyCode_Down));
			Input.TargetPreview_PositionTool_MoveRight = MultiBinding(Binding(KeyCode_D), Binding(KeyCode_Right));
			Input.TargetPreview_PositionTool_FlipHorizontal = MultiBinding(Binding(KeyCode_H));
			Input.TargetPreview_PositionTool_FlipHorizontalLocal = MultiBinding(Binding(KeyCode_H, KeyModifiers_Alt));
			Input.TargetPreview_PositionTool_FlipVertical = MultiBinding(Binding(KeyCode_J));
			Input.TargetPreview_PositionTool_FlipVerticalLocal = MultiBinding(Binding(KeyCode_J, KeyModifiers_Alt));
			Input.TargetPreview_PositionTool_PositionInRow = MultiBinding(Binding(KeyCode_U));
			Input.TargetPreview_PositionTool_PositionInRowBack = MultiBinding(Binding(KeyCode_U, KeyModifiers_Alt));
			Input.TargetPreview_PositionTool_InterpolateLinear = MultiBinding(Binding(KeyCode_I));
			Input.TargetPreview_PositionTool_InterpolateCircular = MultiBinding(Binding(KeyCode_O));
			Input.TargetPreview_PositionTool_InterpolateCircularFlip = MultiBinding(Binding(KeyCode_O, KeyModifiers_Alt));
			Input.TargetPreview_PositionTool_StackPositions = MultiBinding(Binding(KeyCode_P));
			Input.TargetPreview_PathTool_InvertFrequencies = MultiBinding(Binding(KeyCode_R));
			Input.TargetPreview_PathTool_InterpolateAnglesClockwise = MultiBinding(Binding(KeyCode_U));
			Input.TargetPreview_PathTool_InterpolateAnglesCounterclockwise = MultiBinding(Binding(KeyCode_I));
			Input.TargetPreview_PathTool_InterpolateDistances = MultiBinding(Binding(KeyCode_O));
			Input.TargetPreview_PathTool_ApplyAngleIncrementsPositive = MultiBinding(Binding(KeyCode_F));
			Input.TargetPreview_PathTool_ApplyAngleIncrementsPositiveBack = MultiBinding(Binding(KeyCode_F, KeyModifiers_Alt));
			Input.TargetPreview_PathTool_ApplyAngleIncrementsNegative = MultiBinding(Binding(KeyCode_V));
			Input.TargetPreview_PathTool_ApplyAngleIncrementsNegativeBack = MultiBinding(Binding(KeyCode_V, KeyModifiers_Alt));

			Input.BPMCalculator_Tap = MultiBinding(Binding(KeyCode_Space));
			Input.BPMCalculator_Reset = MultiBinding(Binding(KeyCode_Escape));

			Input.Playtest_ReturnToEditorCurrent = MultiBinding(Binding(KeyCode_Escape));
			Input.Playtest_ReturnToEditorPrePlaytest = MultiBinding(Binding(KeyCode_Escape, KeyModifiers_Shift));
			Input.Playtest_ToggleAutoplay = MultiBinding(Binding(KeyCode_F1));
			Input.Playtest_TogglePause = MultiBinding(Binding(KeyCode_Space), Binding(Button::Start));
			Input.Playtest_RestartFromResetPoint = MultiBinding(Binding(KeyCode_Enter), Binding(Button::RightStickClick));
			Input.Playtest_MoveResetPointBackward = MultiBinding(Binding(KeyCode_Tab, KeyModifiers_Shift), Binding(Button::LeftStickClick));
			Input.Playtest_MoveResetPointForward = MultiBinding(Binding(KeyCode_Tab), Binding(Button::TouchPad));

			Input.PlaytestBindings =
			{
				PlayTestInputBinding { ButtonTypeFlags_Triangle, PlayTestSlidePositionType::None, Input::KeyCode_W },
				PlayTestInputBinding { ButtonTypeFlags_Square, PlayTestSlidePositionType::None, Input::KeyCode_A },
				PlayTestInputBinding { ButtonTypeFlags_Cross, PlayTestSlidePositionType::None, Input::KeyCode_S },
				PlayTestInputBinding { ButtonTypeFlags_Circle, PlayTestSlidePositionType::None, Input::KeyCode_D },
				PlayTestInputBinding { ButtonTypeFlags_SlideL, PlayTestSlidePositionType::Left, Input::KeyCode_Q },
				PlayTestInputBinding { ButtonTypeFlags_SlideR, PlayTestSlidePositionType::Left, Input::KeyCode_E },

				PlayTestInputBinding { ButtonTypeFlags_Triangle, PlayTestSlidePositionType::None, Input::KeyCode_I },
				PlayTestInputBinding { ButtonTypeFlags_Square, PlayTestSlidePositionType::None, Input::KeyCode_J },
				PlayTestInputBinding { ButtonTypeFlags_Cross, PlayTestSlidePositionType::None, Input::KeyCode_K },
				PlayTestInputBinding { ButtonTypeFlags_Circle, PlayTestSlidePositionType::None, Input::KeyCode_L },
				PlayTestInputBinding { ButtonTypeFlags_SlideL, PlayTestSlidePositionType::Right, Input::KeyCode_U },
				PlayTestInputBinding { ButtonTypeFlags_SlideR, PlayTestSlidePositionType::Right, Input::KeyCode_O },

				PlayTestInputBinding { ButtonTypeFlags_Triangle, PlayTestSlidePositionType::None, Input::Button::FaceUp },
				PlayTestInputBinding { ButtonTypeFlags_Square, PlayTestSlidePositionType::None, Input::Button::FaceLeft },
				PlayTestInputBinding { ButtonTypeFlags_Cross, PlayTestSlidePositionType::None, Input::Button::FaceDown },
				PlayTestInputBinding { ButtonTypeFlags_Circle, PlayTestSlidePositionType::None, Input::Button::FaceRight },

				PlayTestInputBinding { ButtonTypeFlags_Triangle, PlayTestSlidePositionType::None, Input::Button::DPadUp },
				PlayTestInputBinding { ButtonTypeFlags_Square, PlayTestSlidePositionType::None, Input::Button::DPadLeft },
				PlayTestInputBinding { ButtonTypeFlags_Cross, PlayTestSlidePositionType::None, Input::Button::DPadDown },
				PlayTestInputBinding { ButtonTypeFlags_Circle, PlayTestSlidePositionType::None, Input::Button::DPadRight },

				PlayTestInputBinding { ButtonTypeFlags_SlideL, PlayTestSlidePositionType::Left, Input::Button::LeftBumper },
				PlayTestInputBinding { ButtonTypeFlags_SlideR, PlayTestSlidePositionType::Right, Input::Button::RightBumper },
				PlayTestInputBinding { ButtonTypeFlags_SlideL, PlayTestSlidePositionType::Left, Input::Button::LeftStickLeft },
				PlayTestInputBinding { ButtonTypeFlags_SlideR, PlayTestSlidePositionType::Left, Input::Button::LeftStickRight },
				PlayTestInputBinding { ButtonTypeFlags_SlideL, PlayTestSlidePositionType::Right, Input::Button::RightStickLeft },
				PlayTestInputBinding { ButtonTypeFlags_SlideR, PlayTestSlidePositionType::Right, Input::Button::RightStickRight },

				PlayTestInputBinding { ButtonTypeFlags_NormalAll, PlayTestSlidePositionType::None, Input::Button::LeftTrigger },
				PlayTestInputBinding { ButtonTypeFlags_NormalAll, PlayTestSlidePositionType::None, Input::Button::RightTrigger },
			};
		}

		TargetPreview.ShowButtons = true;
		TargetPreview.ShowGrid = true;
		TargetPreview.ShowHoldInfo = true;
		TargetPreview.ShowBackgroundCheckerboard = true;
		TargetPreview.BackgroundDim = 0.35f;
		TargetPreview.PostHitLingerDuration = BeatTick::FromBeats(1);
		TargetPreview.DisplayPracticeBackground = false;

		PositionTool.ShowDistanceGuides = true;
		PositionTool.ShowTargetGrabTooltip = true;
		PositionTool.UseAxisSnapGuides = true;
		PositionTool.AxisSnapGuideDistanceThreshold = Rules::GridStepDistance / 2.0f;
		PositionTool.PositionMouseSnap = Rules::NormalStepDistance;
		PositionTool.PositionMouseSnapRough = Rules::GridStepDistance;
		PositionTool.PositionMouseSnapPrecise = 1.0f;
		PositionTool.PositionKeyMoveStep = Rules::NormalStepDistance;
		PositionTool.PositionKeyMoveStepRough = Rules::GridStepDistance;
		PositionTool.PositionKeyMoveStepPrecise = Rules::PreciseStepDistance;
		PositionTool.MouseRowCenterDistanceThreshold = 9.0f;
		PositionTool.PositionInterpolationCommandSnap = Rules::NormalStepDistance;
		PositionTool.DiagonalMouseRowLayouts =
		{
			{ Rules::DefaultPerBeatDiagonalSpacing, "Default" },
		};

		PathTool.AngleMouseSnap = 1.0f;
		PathTool.AngleMouseSnapRough = 15.0f;
		PathTool.AngleMouseSnapPrecise = 0.1f;
		PathTool.AngleMouseScrollDirection = -1.0f;
		PathTool.AngleMouseScrollStep = 1.0f;
		PathTool.AngleMouseScrollRough = 5.0f;
		PathTool.AngleMouseScrollPrecise = 0.1f;
		PathTool.AngleMouseMovementDistanceThreshold = 3.0f;
		PathTool.AngleMouseTargetCenterDistanceThreshold = 4.0f;

		TargetPreset.StaticSyncPresets = GetDefaultStaticSyncPresets();
		TargetPreset.SequencePresets = GetDefaultSequencePresets();
		TargetPreset.InspectorDropdown.Amplitudes = { 450.0f, 500.0f, 600.0f, 750.0f, 800.0f, 1250.0f, 1500.0f };
		TargetPreset.InspectorDropdown.Distances = { 880.0f, 960.0f, 1200.0f, 1212.0f, 1440.0f };

		BPMCalculator.AutoResetEnabled = true;
		BPMCalculator.ApplyToTempoMap = false;
		BPMCalculator.TapSoundType = BPMTapSoundType::MetronomeBeat;

		Playtest.EnterFullscreenOnMaximizedStart = true;
		Playtest.AutoHideCursor = true;
	}
}
