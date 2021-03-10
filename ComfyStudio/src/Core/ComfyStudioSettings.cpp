#include "ComfyStudioSettings.h"
#include "IO/JSON.h"

namespace Comfy::Studio
{
	using namespace Editor;

	// NOTE: Underscore denote object boundaries, all IDs should follow the snake_case naming convention
	//		 and are global std::string objects to avoid reconstructing them inside inner loops
	namespace IDs
	{
		const std::string TargetProperties_Position = "position";
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
		const std::string LastSessionWindowState_ActiveEditorComponent = "active_editor_component";

		const std::string RecentFiles = "recent_files";
		const std::string RecentFiles_ChartFiles = "chart_files";
	}

	bool ComfyStudioAppSettings::LoadFromFile(std::string_view filePath)
	{
		const std::optional<json> loadedJson = IO::LoadJson(filePath);
		if (!loadedJson.has_value())
			return false;

		const json& appJson = loadedJson.value();

		if (const json* windowStateJson = JsonFind(appJson, AppIDs::LastSessionWindowState))
		{
			LastSessionWindowState.RestoreRegion = JsonTryGetIVec4(JsonFind(*windowStateJson, AppIDs::LastSessionWindowState_RestoreRegion));
			LastSessionWindowState.Position = JsonTryGetIVec2(JsonFind(*windowStateJson, AppIDs::LastSessionWindowState_Position));
			LastSessionWindowState.Size = JsonTryGetIVec2(JsonFind(*windowStateJson, AppIDs::LastSessionWindowState_Size));
			LastSessionWindowState.IsFullscreen = JsonTryGetBool(JsonFind(*windowStateJson, AppIDs::LastSessionWindowState_IsFullscreen));
			LastSessionWindowState.IsMaximized = JsonTryGetBool(JsonFind(*windowStateJson, AppIDs::LastSessionWindowState_IsMaximized));
			LastSessionWindowState.ActiveEditorComponent = JsonTryGetStr(JsonFind(*windowStateJson, AppIDs::LastSessionWindowState_ActiveEditorComponent));
		}

		if (const json* recentFilesJson = JsonFind(appJson, AppIDs::RecentFiles))
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
		json appJson = json::object();

		json& windowStateJson = appJson[AppIDs::LastSessionWindowState];
		JsonTrySetIVec4(windowStateJson[AppIDs::LastSessionWindowState_RestoreRegion], LastSessionWindowState.RestoreRegion);
		JsonTrySetIVec2(windowStateJson[AppIDs::LastSessionWindowState_Position], LastSessionWindowState.Position);
		JsonTrySetIVec2(windowStateJson[AppIDs::LastSessionWindowState_Size], LastSessionWindowState.Size);
		JsonTrySetBool(windowStateJson[AppIDs::LastSessionWindowState_IsFullscreen], LastSessionWindowState.IsFullscreen);
		JsonTrySetBool(windowStateJson[AppIDs::LastSessionWindowState_IsMaximized], LastSessionWindowState.IsMaximized);
		JsonTrySetStr(windowStateJson[AppIDs::LastSessionWindowState_ActiveEditorComponent], LastSessionWindowState.ActiveEditorComponent);

		json& recentFilesJson = appJson[AppIDs::RecentFiles];
		json& chartFilesJson = recentFilesJson[AppIDs::RecentFiles_ChartFiles];
		chartFilesJson = json::array();
		std::for_each(RecentFiles.ChartFiles.View().rbegin(), RecentFiles.ChartFiles.View().rend(), [&](auto& path) { chartFilesJson.emplace_back(path); });

		IO::SaveJson(filePath, appJson);
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

				SequencePreset { SequencePresetType::BezierPath, SequencePresetButtonType::SingleLine, "Small Heart", {}, SequencePreset::BezierPathData {} },
				SequencePreset { SequencePresetType::BezierPath, SequencePresetButtonType::SingleLine, "Triangle Clockwise", {}, SequencePreset::BezierPathData {} },
				SequencePreset { SequencePresetType::BezierPath, SequencePresetButtonType::SingleLine, "Triangle Counterclockwise", {}, SequencePreset::BezierPathData {} },
				SequencePreset { SequencePresetType::BezierPath, SequencePresetButtonType::SingleLine, "Dummy A", {}, SequencePreset::BezierPathData {} },
				SequencePreset { SequencePresetType::BezierPath, SequencePresetButtonType::SingleLine, "Dummy B", {}, SequencePreset::BezierPathData {} },
				SequencePreset { SequencePresetType::BezierPath, SequencePresetButtonType::SingleLine, "Dummy C", {}, SequencePreset::BezierPathData {} },
				SequencePreset { SequencePresetType::BezierPath, SequencePresetButtonType::SingleLine, "Dummy D", {}, SequencePreset::BezierPathData {} },
			};
		}

		std::optional<TargetProperties> JsonTryGetTargetProperties(const json& j)
		{
			TargetProperties result;

			if (const json* foundJson = JsonFind(j, IDs::TargetProperties_Position))
				result.Position = JsonTryGetVec2(*foundJson).value_or(vec2(0.0f));
			else
				return std::nullopt;

			if (const json* foundJson = JsonFind(j, IDs::TargetProperties_Angle))
				result.Angle = JsonTryGetF32(*foundJson).value_or(0.0f);
			else
				return std::nullopt;

			if (const json* foundJson = JsonFind(j, IDs::TargetProperties_Frequency))
				result.Frequency = JsonTryGetF32(*foundJson).value_or(0.0f);
			else
				return std::nullopt;

			if (const json* foundJson = JsonFind(j, IDs::TargetProperties_Amplitude))
				result.Amplitude = JsonTryGetF32(*foundJson).value_or(0.0f);
			else
				return std::nullopt;

			if (const json* foundJson = JsonFind(j, IDs::TargetProperties_Distance))
				result.Distance = JsonTryGetF32(*foundJson).value_or(0.0f);
			else
				return std::nullopt;

			return result;
		}

		void JsonSetTargetProperties(json& j, const TargetProperties& v)
		{
			JsonSetVec2(j[IDs::TargetProperties_Position], v.Position);
			j[IDs::TargetProperties_Angle] = v.Angle;
			j[IDs::TargetProperties_Frequency] = v.Frequency;
			j[IDs::TargetProperties_Amplitude] = v.Amplitude;
			j[IDs::TargetProperties_Distance] = v.Distance;
		}

	}

	namespace UserIDs
	{
		const std::string TargetPreview = "target_preview";
		const std::string TargetPreview_ShowButtons = "show_buttons";
		const std::string TargetPreview_ShowGrid = "show_grid";
		const std::string TargetPreview_ShowHoldInfo = "show_hold_info";
		const std::string TargetPreview_ShowBackgroundCheckerboard = "show_background_checkerboard";
		const std::string TargetPreview_BackgroundDimPercentage = "background_dim_percentage";
		const std::string TargetPreview_PostHitLingerDurationTicks = "post_hit_linger_duration_ticks";
		const std::string TargetPreview_UsePracticeBackground = "use_practice_background";

		const std::string TargetPreset = "target_preset";
		const std::string TargetPreset_StaticSyncPresets = "static_sync_preset";
		const std::string TargetPreset_StaticSyncPresets_Name = "name";
		const std::string TargetPreset_StaticSyncPresets_Targets = "targets";
		const std::string TargetPreset_StaticSyncPresets_Targets_ButtonType = "button_type";
		const std::string TargetPreset_StaticSyncPresets_Targets_Properties = "properties";

		const std::string TargetPreset_SequencePresets = "sequence_preset";
		const std::string TargetPreset_SequencePresets_GuiButtonType = "gui_button_type";
		const std::string TargetPreset_SequencePresets_Name = "name";
		const std::string TargetPreset_SequencePresets_Circle = "circle";
		const std::string TargetPreset_SequencePresets_Circle_DurationTicks = "duration_ticks";
		const std::string TargetPreset_SequencePresets_Circle_Radius = "radius";
		const std::string TargetPreset_SequencePresets_Circle_Direction = "direction";
		const std::string TargetPreset_SequencePresets_Circle_Center = "center";
		const std::string TargetPreset_SequencePresets_BezierPath = "bezier_path";

		const std::string TargetPreset_InspectorDropdown = "inspector_dropdown";
		const std::string TargetPreset_InspectorDropdown_PositionsX = "positions_x";
		const std::string TargetPreset_InspectorDropdown_PositionsY = "positions_y";
		const std::string TargetPreset_InspectorDropdown_Angles = "angles";
		const std::string TargetPreset_InspectorDropdown_Frequencies = "frequencies";
		const std::string TargetPreset_InspectorDropdown_Amplitudes = "amplitudes";
		const std::string TargetPreset_InspectorDropdown_Distances = "distances";
	}

	bool ComfyStudioUserSettings::LoadFromFile(std::string_view filePath)
	{
		const std::optional<json> loadedJson = IO::LoadJson(filePath);
		if (!loadedJson.has_value())
			return false;

		const json& userJson = loadedJson.value();

		if (const json* targetPreviewJson = JsonFind(userJson, UserIDs::TargetPreview))
		{
			TargetPreview.ShowButtons = JsonTryGetBool(JsonFind(*targetPreviewJson, UserIDs::TargetPreview_ShowButtons)).value_or(false);
			TargetPreview.ShowGrid = JsonTryGetBool(JsonFind(*targetPreviewJson, UserIDs::TargetPreview_ShowGrid)).value_or(false);
			TargetPreview.ShowHoldInfo = JsonTryGetBool(JsonFind(*targetPreviewJson, UserIDs::TargetPreview_ShowHoldInfo)).value_or(false);
			TargetPreview.ShowBackgroundCheckerboard = JsonTryGetBool(JsonFind(*targetPreviewJson, UserIDs::TargetPreview_ShowBackgroundCheckerboard)).value_or(false);
			TargetPreview.BackgroundDim = static_cast<f32>(JsonTryGetI32(JsonFind(*targetPreviewJson, UserIDs::TargetPreview_BackgroundDimPercentage)).value_or(0)) / 100.0f;
			TargetPreview.PostHitLingerDuration = BeatTick::FromTicks(JsonTryGetI32(JsonFind(*targetPreviewJson, UserIDs::TargetPreview_PostHitLingerDurationTicks)).value_or(0));
			TargetPreview.UsePracticeBackground = JsonTryGetBool(JsonFind(*targetPreviewJson, UserIDs::TargetPreview_UsePracticeBackground)).value_or(false);
		}

		if (const json* targetPresetJson = JsonFind(userJson, UserIDs::TargetPreset))
		{
			if (const json* syncPresetsJson = JsonFind(*targetPresetJson, UserIDs::TargetPreset_StaticSyncPresets))
			{
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
									targetData.Properties = JsonTryGetTargetProperties(*propertiesJson).value_or(TargetProperties {});
							}
						}
					}
				}
			}

			if (const json* sequencePresetsJson = JsonFind(*targetPresetJson, UserIDs::TargetPreset_SequencePresets))
			{
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
						sequencePreset.Circle.Center = JsonTryGetVec2(JsonFind(*circleJson, UserIDs::TargetPreset_SequencePresets_Circle_Center)).value_or(vec2(0.0f));
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

		return true;
	}

	void ComfyStudioUserSettings::SaveToFile(std::string_view filePath) const
	{
		json userJson = json::object();

		json& targetPreviewJson = userJson[UserIDs::TargetPreview];
		targetPreviewJson[UserIDs::TargetPreview_ShowButtons] = TargetPreview.ShowButtons;
		targetPreviewJson[UserIDs::TargetPreview_ShowGrid] = TargetPreview.ShowGrid;
		targetPreviewJson[UserIDs::TargetPreview_ShowHoldInfo] = TargetPreview.ShowHoldInfo;
		targetPreviewJson[UserIDs::TargetPreview_ShowBackgroundCheckerboard] = TargetPreview.ShowBackgroundCheckerboard;
		targetPreviewJson[UserIDs::TargetPreview_BackgroundDimPercentage] = static_cast<i32>(glm::round(TargetPreview.BackgroundDim * 100.0f));
		targetPreviewJson[UserIDs::TargetPreview_PostHitLingerDurationTicks] = TargetPreview.PostHitLingerDuration.Ticks();
		targetPreviewJson[UserIDs::TargetPreview_UsePracticeBackground] = TargetPreview.UsePracticeBackground;

		json& targetPresetJson = userJson[UserIDs::TargetPreset];

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
				JsonSetVec2(circleJson[UserIDs::TargetPreset_SequencePresets_Circle_Center], sequencePreset.Circle.Center);
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

		IO::SaveJson(filePath, userJson);
	}

	void ComfyStudioUserSettings::RestoreDefault()
	{
		*this = {};

		TargetPreview.ShowButtons = true;
		TargetPreview.ShowGrid = true;
		TargetPreview.ShowHoldInfo = true;
		TargetPreview.ShowBackgroundCheckerboard = true;
		TargetPreview.BackgroundDim = 0.35f;
		TargetPreview.PostHitLingerDuration = BeatTick::FromBeats(1);
		TargetPreview.UsePracticeBackground = false;

		TargetPreset.StaticSyncPresets = GetDefaultStaticSyncPresets();
		TargetPreset.SequencePresets = GetDefaultSequencePresets();
		TargetPreset.InspectorDropdown.Amplitudes = { 450.0f, 500.0f, 600.0f, 750.0f, 800.0f, 1250.0f, 1500.0f };
		TargetPreset.InspectorDropdown.Distances = { 880.0f, 960.0f, 1200.0f, 1212.0f, 1440.0f };
	}
}
