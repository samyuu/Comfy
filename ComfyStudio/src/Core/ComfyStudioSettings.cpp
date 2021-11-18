#include "ComfyStudioSettings.h"
#include "Editor/Chart/ChartEditor.h"
#include "IO/JSON.h"
#include "IO/File.h"
#include "Core/Logger.h"

namespace Comfy::Studio
{
	using namespace Editor;

	// NOTE: Underscore denote object boundaries, all IDs should follow the snake_case naming convention
	namespace IDs
	{
		constexpr std::string_view FileVersion = ".file_version";

		constexpr std::string_view TargetProperties_PositionX = "position_x";
		constexpr std::string_view TargetProperties_PositionY = "position_y";
		constexpr std::string_view TargetProperties_Angle = "angle";
		constexpr std::string_view TargetProperties_Frequency = "frequency";
		constexpr std::string_view TargetProperties_Amplitude = "amplitude";
		constexpr std::string_view TargetProperties_Distance = "distance";
	}

	namespace AppIDs
	{
		constexpr std::string_view LastSessionWindowState = "last_session_window_state";
		constexpr std::string_view LastSessionWindowState_RestoreRegionX = "restore_region_x";
		constexpr std::string_view LastSessionWindowState_RestoreRegionY = "restore_region_y";
		constexpr std::string_view LastSessionWindowState_RestoreRegionW = "restore_region_w";
		constexpr std::string_view LastSessionWindowState_RestoreRegionH = "restore_region_h";
		constexpr std::string_view LastSessionWindowState_PositionX = "position_x";
		constexpr std::string_view LastSessionWindowState_PositionY = "position_y";
		constexpr std::string_view LastSessionWindowState_SizeX = "size_x";
		constexpr std::string_view LastSessionWindowState_SizeY = "size_y";
		constexpr std::string_view LastSessionWindowState_IsFullscreen = "is_fullscreen";
		constexpr std::string_view LastSessionWindowState_IsMaximized = "is_maximized";
		constexpr std::string_view LastSessionWindowState_SwapInterval = "swap_interval";
		constexpr std::string_view LastSessionWindowState_ActiveEditorComponent = "active_editor_component";

		constexpr std::string_view LastPVScriptExportOptions = "last_pv_script_export_options";
		constexpr std::string_view LastPVScriptExportOptions_ExportFormatIndex = "export_format";
		constexpr std::string_view LastPVScriptExportOptions_PVID = "pv_id";
		constexpr std::string_view LastPVScriptExportOptions_RootDirectory = "root_directory";
		constexpr std::string_view LastPVScriptExportOptions_MDataID = "mdata_id";
		constexpr std::string_view LastPVScriptExportOptions_BackgroundDim = "background_dim";
		constexpr std::string_view LastPVScriptExportOptions_MergeWithExistingMData = "merge_with_existing_mdata";
		constexpr std::string_view LastPVScriptExportOptions_CreateSprSelPV = "create_spr_sel_pv";
		constexpr std::string_view LastPVScriptExportOptions_AddDummyMovieReference = "add_dummy_movie_reference";
		constexpr std::string_view LastPVScriptExportOptions_VorbisVBRQuality = "vorbis_vbr_quality";

		constexpr std::string_view RecentFiles = "recent_files";
		constexpr std::string_view RecentFiles_ChartFiles = "chart_files";
	}

	namespace
	{
		std::optional<SemanticVersion> TryGetJsonSettingsFileVersionFromRoot(const Json::Document& rootJson)
		{
			if (auto v = Json::TryGetStrView(Json::Find(rootJson, IDs::FileVersion)); v.has_value())
				return SemanticVersion::FromString(std::string(v.value()));
			else
				return std::nullopt;
		}
	}

	bool ComfyStudioAppSettings::LoadFromFile(std::string_view filePath)
	{
		std::string insituFileContent = IO::File::ReadAllText(filePath);
		if (insituFileContent.empty())
			return false;

		using namespace Json;
		Document rootJson;
		rootJson.ParseInsitu(insituFileContent.data());

		if (rootJson.HasParseError())
		{
			// TODO: Proper error handling
			const auto parseError = rootJson.GetParseError();
			assert(false);
			return false;
		}

		if (!rootJson.IsObject())
			return false;

		const auto fileVersion = TryGetJsonSettingsFileVersionFromRoot(rootJson).value_or(SemanticVersion {});
		if (fileVersion.Major > CurrentVersion.Major)
		{
			Logger::LogErrorLine(__FUNCTION__"(): Unsupported AppSettings version detected: \"%s\". Current version: \"%s\"", fileVersion.ToString().c_str(), CurrentVersion.ToString().c_str());
			return false;
		}

		if (const Value* windowStateJson = Find(rootJson, AppIDs::LastSessionWindowState))
		{
			const auto restoreRegionX = TryGetI32(Find(*windowStateJson, AppIDs::LastSessionWindowState_RestoreRegionX));
			const auto restoreRegionY = TryGetI32(Find(*windowStateJson, AppIDs::LastSessionWindowState_RestoreRegionY));
			const auto restoreRegionW = TryGetI32(Find(*windowStateJson, AppIDs::LastSessionWindowState_RestoreRegionW));
			const auto restoreRegionH = TryGetI32(Find(*windowStateJson, AppIDs::LastSessionWindowState_RestoreRegionH));
			if (restoreRegionX.has_value() && restoreRegionY.has_value() && restoreRegionW.has_value() && restoreRegionH.has_value())
				LastSessionWindowState.RestoreRegion = ivec4(restoreRegionX.value(), restoreRegionY.value(), restoreRegionW.value(), restoreRegionH.value());

			const auto positionX = TryGetI32(Find(*windowStateJson, AppIDs::LastSessionWindowState_PositionX));
			const auto positionY = TryGetI32(Find(*windowStateJson, AppIDs::LastSessionWindowState_PositionY));
			if (positionX.has_value() && positionY.has_value())
				LastSessionWindowState.Position = ivec2(positionX.value(), positionY.value());

			const auto sizeX = TryGetI32(Find(*windowStateJson, AppIDs::LastSessionWindowState_SizeX));
			const auto sizeY = TryGetI32(Find(*windowStateJson, AppIDs::LastSessionWindowState_SizeY));
			if (sizeX.has_value() && sizeY.has_value())
				LastSessionWindowState.Size = ivec2(sizeX.value(), sizeY.value());

			LastSessionWindowState.IsFullscreen = TryGetBool(Find(*windowStateJson, AppIDs::LastSessionWindowState_IsFullscreen));
			LastSessionWindowState.IsMaximized = TryGetBool(Find(*windowStateJson, AppIDs::LastSessionWindowState_IsMaximized));
			LastSessionWindowState.SwapInterval = TryGetI32(Find(*windowStateJson, AppIDs::LastSessionWindowState_SwapInterval));
			LastSessionWindowState.ActiveEditorComponent = TryGetStrView(Find(*windowStateJson, AppIDs::LastSessionWindowState_ActiveEditorComponent));
		}

		if (const Value* exportOptionsJson = Find(rootJson, AppIDs::LastPVScriptExportOptions))
		{
			LastPVScriptExportOptions.ExportFormatIndex = TryGetI32(Find(*exportOptionsJson, AppIDs::LastPVScriptExportOptions_ExportFormatIndex));
			LastPVScriptExportOptions.PVID = TryGetI32(Find(*exportOptionsJson, AppIDs::LastPVScriptExportOptions_PVID));
			LastPVScriptExportOptions.RootDirectory = TryGetStrView(Find(*exportOptionsJson, AppIDs::LastPVScriptExportOptions_RootDirectory));
			LastPVScriptExportOptions.MDataID = TryGetStrView(Find(*exportOptionsJson, AppIDs::LastPVScriptExportOptions_MDataID));
			LastPVScriptExportOptions.BackgroundDim = TryGetF32(Find(*exportOptionsJson, AppIDs::LastPVScriptExportOptions_BackgroundDim));
			LastPVScriptExportOptions.MergeWithExistingMData = TryGetBool(Find(*exportOptionsJson, AppIDs::LastPVScriptExportOptions_MergeWithExistingMData));
			LastPVScriptExportOptions.CreateSprSelPV = TryGetBool(Find(*exportOptionsJson, AppIDs::LastPVScriptExportOptions_CreateSprSelPV));
			LastPVScriptExportOptions.AddDummyMovieReference = TryGetBool(Find(*exportOptionsJson, AppIDs::LastPVScriptExportOptions_AddDummyMovieReference));
			LastPVScriptExportOptions.VorbisVBRQuality = TryGetF32(Find(*exportOptionsJson, AppIDs::LastPVScriptExportOptions_VorbisVBRQuality));
		}

		if (const Value* recentFilesJson = Find(rootJson, AppIDs::RecentFiles))
		{
			if (const Value* chartFilesJson = Find(*recentFilesJson, AppIDs::RecentFiles_ChartFiles); chartFilesJson && chartFilesJson->IsArray())
			{
				if (chartFilesJson->Size() > 0)
				{
					for (i32 i = static_cast<i32>(chartFilesJson->Size()) - 1; i >= 0; i--)
					{
						if (auto v = TryGetStrView(&chartFilesJson->GetArray()[i]); v.has_value())
							RecentFiles.ChartFiles.Add(v.value());
					}
				}
			}
		}

		return true;
	}

	void ComfyStudioAppSettings::SaveToFile(std::string_view filePath) const
	{
		auto memberTryIVec2 = [](Json::WriterEx& writer, std::string_view keyX, std::string_view keyY, std::optional<ivec2> value)
		{
			if (value.has_value()) { writer.MemberI32(keyX, value->x); writer.MemberI32(keyY, value->y); }
			else { writer.MemberNull(keyX); writer.MemberNull(keyY); }
		};

		auto memberTryIVec4 = [](Json::WriterEx& writer, std::string_view keyX, std::string_view keyY, std::string_view keyW, std::string_view keyH, std::optional<ivec4> value)
		{
			if (value.has_value()) { writer.MemberI32(keyX, value->x); writer.MemberI32(keyY, value->y); writer.MemberI32(keyW, value->z); writer.MemberI32(keyH, value->w); }
			else { writer.MemberNull(keyX); writer.MemberNull(keyY); writer.MemberNull(keyW); writer.MemberNull(keyH); }
		};

		Json::WriteBuffer writeBuffer {};
		Json::WriterEx writer { writeBuffer };
		writeBuffer.Reserve(0x1000);

		writer.ObjectBegin();
		{
			writer.MemberStr(IDs::FileVersion, CurrentVersion.ToString());

			writer.MemberObjectBegin(AppIDs::LastSessionWindowState);
			{
				memberTryIVec4(writer,
					AppIDs::LastSessionWindowState_RestoreRegionX, AppIDs::LastSessionWindowState_RestoreRegionY,
					AppIDs::LastSessionWindowState_RestoreRegionW, AppIDs::LastSessionWindowState_RestoreRegionH,
					LastSessionWindowState.RestoreRegion);

				memberTryIVec2(writer,
					AppIDs::LastSessionWindowState_PositionX, AppIDs::LastSessionWindowState_PositionY,
					LastSessionWindowState.Position);

				memberTryIVec2(writer,
					AppIDs::LastSessionWindowState_SizeX, AppIDs::LastSessionWindowState_SizeY,
					LastSessionWindowState.Size);

				writer.MemberTryBool(AppIDs::LastSessionWindowState_IsFullscreen, LastSessionWindowState.IsFullscreen);
				writer.MemberTryBool(AppIDs::LastSessionWindowState_IsMaximized, LastSessionWindowState.IsMaximized);
				writer.MemberTryI32(AppIDs::LastSessionWindowState_SwapInterval, LastSessionWindowState.SwapInterval);
				writer.MemberTryStr(AppIDs::LastSessionWindowState_ActiveEditorComponent, LastSessionWindowState.ActiveEditorComponent);
			}
			writer.MemberObjectEnd();

			writer.MemberObjectBegin(AppIDs::LastPVScriptExportOptions);
			{
				writer.MemberTryI32(AppIDs::LastPVScriptExportOptions_ExportFormatIndex, LastPVScriptExportOptions.ExportFormatIndex);
				writer.MemberTryI32(AppIDs::LastPVScriptExportOptions_PVID, LastPVScriptExportOptions.PVID);
				writer.MemberTryStr(AppIDs::LastPVScriptExportOptions_RootDirectory, LastPVScriptExportOptions.RootDirectory);
				writer.MemberTryStr(AppIDs::LastPVScriptExportOptions_MDataID, LastPVScriptExportOptions.MDataID);
				writer.MemberTryF32(AppIDs::LastPVScriptExportOptions_BackgroundDim, LastPVScriptExportOptions.BackgroundDim);
				writer.MemberTryBool(AppIDs::LastPVScriptExportOptions_MergeWithExistingMData, LastPVScriptExportOptions.MergeWithExistingMData);
				writer.MemberTryBool(AppIDs::LastPVScriptExportOptions_CreateSprSelPV, LastPVScriptExportOptions.CreateSprSelPV);
				writer.MemberTryBool(AppIDs::LastPVScriptExportOptions_AddDummyMovieReference, LastPVScriptExportOptions.AddDummyMovieReference);
				writer.MemberTryF32(AppIDs::LastPVScriptExportOptions_VorbisVBRQuality, LastPVScriptExportOptions.VorbisVBRQuality);
			}
			writer.MemberObjectEnd();

			writer.MemberObjectBegin(AppIDs::RecentFiles);
			{
				writer.MemberArrayBegin(AppIDs::RecentFiles_ChartFiles);
				{
					std::for_each(RecentFiles.ChartFiles.View().rbegin(), RecentFiles.ChartFiles.View().rend(), [&](auto& path) { writer.Str(path); });
				}
				writer.MemberArrayEnd();
			}
			writer.MemberObjectEnd();
		}
		writer.ObjectEnd();

		const auto formattedOutputJson = std::string_view(writeBuffer.GetString(), writeBuffer.GetLength());
		IO::File::WriteAllText(filePath, formattedOutputJson);
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

		TargetProperties JsonTryGetTargetProperties(const Json::Value& j)
		{
			TargetProperties result;
			result.Position.x = Json::TryGetF32(Json::Find(j, IDs::TargetProperties_PositionX)).value_or(0.0f);
			result.Position.y = Json::TryGetF32(Json::Find(j, IDs::TargetProperties_PositionY)).value_or(0.0f);
			result.Angle = Json::TryGetF32(Json::Find(j, IDs::TargetProperties_Angle)).value_or(0.0f);
			result.Frequency = Json::TryGetF32(Json::Find(j, IDs::TargetProperties_Frequency)).value_or(0.0f);
			result.Amplitude = Json::TryGetF32(Json::Find(j, IDs::TargetProperties_Amplitude)).value_or(0.0f);
			result.Distance = Json::TryGetF32(Json::Find(j, IDs::TargetProperties_Distance)).value_or(0.0f);
			return result;
		}
	}

	namespace UserIDs
	{
		constexpr std::string_view System = "system";

		constexpr std::string_view System_Audio = "audio";
		constexpr std::string_view System_Audio_SongVolume = "song_volume";
		constexpr std::string_view System_Audio_ButtonSoundVolume = "button_sound_volume";
		constexpr std::string_view System_Audio_SoundEffectVolume = "sound_effect_volume";
		constexpr std::string_view System_Audio_MetronomeVolume = "metronome_volume";
		constexpr std::string_view System_Audio_OpenDeviceOnStartup = "open_device_on_startup";
		constexpr std::string_view System_Audio_CloseDeviceOnIdleFocusLoss = "close_device_on_idle_focus_loss";
		constexpr std::string_view System_Audio_RequestExclusiveDeviceAccess = "request_exclusive_device_access";

		constexpr std::string_view System_Video = "video";
		constexpr std::string_view System_Video_EnableColorCorrectionEditor = "enable_color_correction_editor";
		constexpr std::string_view System_Video_EnableColorCorrectionPlaytest = "enable_color_correction_playtest";
		constexpr std::string_view System_Video_ColorCorrectionParam = "color_correction_param";
		constexpr std::string_view System_Video_ColorCorrectionParam_Gamma = "gamma";
		constexpr std::string_view System_Video_ColorCorrectionParam_Contrast = "contrast";
		constexpr std::string_view System_Video_ColorCorrectionParam_ColorCoefficients = "color_coefficients";

		constexpr std::string_view System_Gui = "gui";
		constexpr std::string_view System_Gui_ShowTestMenu = "show_test_menu";
		constexpr std::string_view System_Gui_AntiAliasedLines = "anti_aliased_lines";
		constexpr std::string_view System_Gui_AntiAliasedFill = "anti_aliased_fill";
		constexpr std::string_view System_Gui_TargetDistanceGuideCircleSegments = "target_distance_guide_circle_segments";
		constexpr std::string_view System_Gui_TargetDistanceGuideMaxCount = "target_distance_guide_max_count";
		constexpr std::string_view System_Gui_TargetButtonPathCurveSegments = "target_button_path_curve_segments";
		constexpr std::string_view System_Gui_TargetButtonPathMaxCount = "target_button_path_max_count";

		constexpr std::string_view System_Discord = "discord";
		constexpr std::string_view System_Discord_EnableRichPresence = "enable_rich_presence";
		constexpr std::string_view System_Discord_ShareElapsedTime = "share_elapsed_time";

		constexpr std::string_view Input = "input";
		constexpr std::string_view Input_ControllerLayoutMappings = "controller_layout_mappings";
		constexpr std::string_view Input_Bindings = "bindings";
		constexpr std::string_view Input_PlaytestBindings = "playtest_bindings";

		constexpr std::string_view TargetTimeline = "target_timeline";
		constexpr std::string_view TargetTimeline_MouseWheelScrollDirection = "mouse_wheel_scroll_direction";
		constexpr std::string_view TargetTimeline_MouseWheelScrollSpeed = "mouse_wheel_scroll_speed";
		constexpr std::string_view TargetTimeline_MouseWheelScrollSpeedShift = "mouse_wheel_scroll_speed_shift";
		constexpr std::string_view TargetTimeline_PlaybackMouseWheelScrollFactor = "playback_mouse_wheel_scroll_factor";
		constexpr std::string_view TargetTimeline_PlaybackMouseWheelScrollFactorShift = "playback_mouse_wheel_scroll_factor_shift";
		constexpr std::string_view TargetTimeline_PlaybackAutoScrollCursorPositionFactor = "playback_auto_scroll_cursor_position_factor";
		constexpr std::string_view TargetTimeline_PlaybackCursorPlacementOffsetSecWasapiShared = "playback_cursor_placement_offset_sec_wasapi_shared";
		constexpr std::string_view TargetTimeline_PlaybackCursorPlacementOffsetSecWasapiExclusive = "playback_cursor_placement_offset_sec_wasapi_exclusive";
		constexpr std::string_view TargetTimeline_SmoothScrollSpeedSec = "smooth_scroll_speed_sec";
		constexpr std::string_view TargetTimeline_ShowStartEndMarkersSong = "show_start_end_markers_song";
		constexpr std::string_view TargetTimeline_ShowStartEndMarkersMovie = "show_start_end_markers_movie";
		constexpr std::string_view TargetTimeline_ScalingBehaviorAutoFit = "scaling_behavior_auto_fit";
		constexpr std::string_view TargetTimeline_ScalingBehaviorAutoFit_MinRowHeight = "min_row_height";
		constexpr std::string_view TargetTimeline_ScalingBehaviorAutoFit_MaxRowHeight = "max_row_height";
		constexpr std::string_view TargetTimeline_ScalingBehaviorFixedSize = "scaling_behavior_fixed_size";
		constexpr std::string_view TargetTimeline_ScalingBehaviorFixedSize_IconScale = "icon_scale";
		constexpr std::string_view TargetTimeline_ScalingBehaviorFixedSize_RowHeight = "row_height";
		constexpr std::string_view TargetTimeline_CursorScrubbingEdgeAutoScrollThresholdFixedSize = "cursor_scrubbing_edge_auto_scroll_threshold_fixed_size";
		constexpr std::string_view TargetTimeline_CursorScrubbingEdgeAutoScrollThresholdFixedSize_Pixels = "pixels";
		constexpr std::string_view TargetTimeline_CursorScrubbingEdgeAutoScrollThresholdProportional = "cursor_scrubbing_edge_auto_scroll_threshold_proportional";
		constexpr std::string_view TargetTimeline_CursorScrubbingEdgeAutoScrollThresholdProportional_Factor = "factor";
		constexpr std::string_view TargetTimeline_CursorScrubbingEdgeAutoScrollSmoothScrollSpeedSec = "cursor_scrubbing_edge_auto_scroll_smooth_scroll_speed_sec";
		constexpr std::string_view TargetTimeline_CursorScrubbingEdgeAutoScrollSmoothScrollSpeedSecShift = "cursor_scrubbing_edge_auto_scroll_smooth_scroll_speed_sec_shift";

		constexpr std::string_view TargetPreview = "target_preview";
		constexpr std::string_view TargetPreview_ShowButtons = "show_buttons";
		constexpr std::string_view TargetPreview_ShowHoldInfo = "show_hold_info";
		constexpr std::string_view TargetPreview_PostHitLingerDurationTicks = "post_hit_linger_duration_ticks";

		constexpr std::string_view PositionTool = "position_tool";
		constexpr std::string_view PositionTool_ShowDistanceGuides = "show_distance_guides";
		constexpr std::string_view PositionTool_ShowTargetGrabTooltip = "show_target_grab_tooltip";
		constexpr std::string_view PositionTool_UseAxisSnapGuides = "use_axis_snap_guides";
		constexpr std::string_view PositionTool_AxisSnapGuideDistanceThreshold = "axis_snap_guide_distance_threshold";
		constexpr std::string_view PositionTool_PositionMouseSnap = "position_mouse_snap";
		constexpr std::string_view PositionTool_PositionMouseSnapRough = "position_mouse_snap_rough";
		constexpr std::string_view PositionTool_PositionMouseSnapPrecise = "position_mouse_snap_precise";
		constexpr std::string_view PositionTool_PositionKeyMoveStep = "position_key_move_step";
		constexpr std::string_view PositionTool_PositionKeyMoveStepRough = "position_key_move_step_rough";
		constexpr std::string_view PositionTool_PositionKeyMoveStepPrecise = "position_key_move_step_precise";
		constexpr std::string_view PositionTool_MouseRowCenterDistanceThreshold = "mouse_row_center_distance_threshold";
		constexpr std::string_view PositionTool_PositionInterpolationCommandSnap = "position_interpolation_command_snap";
		constexpr std::string_view PositionTool_DiagonalMouseRowLayouts = "diagonal_mouse_row_layouts";
		constexpr std::string_view PositionTool_DiagonalMouseRowLayouts_PerBeatDiagonalSpacingX = "per_beat_diagonal_spacing_x";
		constexpr std::string_view PositionTool_DiagonalMouseRowLayouts_PerBeatDiagonalSpacingY = "per_beat_diagonal_spacing_y";
		constexpr std::string_view PositionTool_DiagonalMouseRowLayouts_DisplayName = "display_name";

		constexpr std::string_view PathTool = "path_tool";
		constexpr std::string_view PathTool_AngleMouseSnap = "angle_mouse_snap";
		constexpr std::string_view PathTool_AngleMouseSnapRough = "angle_mouse_snap_rough";
		constexpr std::string_view PathTool_AngleMouseSnapPrecise = "angle_mouse_snap_precise";
		constexpr std::string_view PathTool_AngleMouseScrollDirection = "angle_mouse_scroll_direction";
		constexpr std::string_view PathTool_AngleMouseScrollStep = "angle_mouse_scroll_step";
		constexpr std::string_view PathTool_AngleMouseScrollRough = "angle_mouse_scroll_step_rough";
		constexpr std::string_view PathTool_AngleMouseScrollPrecise = "angle_mouse_scroll_step_precise";
		constexpr std::string_view PathTool_AngleMouseMovementDistanceThreshold = "angle_mouse_movement_distance_threshold";
		constexpr std::string_view PathTool_AngleMouseTargetCenterDistanceThreshold = "angle_mouse_target_center_distance_threshold";

		constexpr std::string_view TargetPreset = "target_preset";
		constexpr std::string_view TargetPreset_StaticSyncPresets = "static_sync_presets";
		constexpr std::string_view TargetPreset_StaticSyncPresets_Name = "name";
		constexpr std::string_view TargetPreset_StaticSyncPresets_Targets = "targets";
		constexpr std::string_view TargetPreset_StaticSyncPresets_Targets_ButtonType = "button_type";
		constexpr std::string_view TargetPreset_StaticSyncPresets_Targets_Properties = "properties";

		constexpr std::string_view TargetPreset_SequencePresets = "sequence_presets";
		constexpr std::string_view TargetPreset_SequencePresets_GuiButtonType = "gui_button_type";
		constexpr std::string_view TargetPreset_SequencePresets_Name = "name";
		constexpr std::string_view TargetPreset_SequencePresets_Circle = "circle";
		constexpr std::string_view TargetPreset_SequencePresets_Circle_DurationTicks = "duration_ticks";
		constexpr std::string_view TargetPreset_SequencePresets_Circle_Radius = "radius";
		constexpr std::string_view TargetPreset_SequencePresets_Circle_Direction = "direction";
		constexpr std::string_view TargetPreset_SequencePresets_Circle_CenterX = "center_x";
		constexpr std::string_view TargetPreset_SequencePresets_Circle_CenterY = "center_y";
		constexpr std::string_view TargetPreset_SequencePresets_BezierPath = "bezier_path";

		constexpr std::string_view TargetPreset_InspectorDropdown = "inspector_dropdown";
		constexpr std::string_view TargetPreset_InspectorDropdown_PositionsX = "positions_x";
		constexpr std::string_view TargetPreset_InspectorDropdown_PositionsY = "positions_y";
		constexpr std::string_view TargetPreset_InspectorDropdown_Angles = "angles";
		constexpr std::string_view TargetPreset_InspectorDropdown_Frequencies = "frequencies";
		constexpr std::string_view TargetPreset_InspectorDropdown_Amplitudes = "amplitudes";
		constexpr std::string_view TargetPreset_InspectorDropdown_Distances = "distances";

		constexpr std::string_view ChartProperties = "chart_properties";
		constexpr std::string_view ChartProperties_ChartCreatorDefaultName = "chart_creator_default_name";

		constexpr std::string_view BPMCalculator = "bpm_calculator";
		constexpr std::string_view BPMCalculator_AutoResetEnabled = "auto_reset_enabled";
		constexpr std::string_view BPMCalculator_ApplyToTempoMap = "apply_to_tempo_map";
		constexpr std::string_view BPMCalculator_TapSoundType = "tap_sound_type";

		constexpr std::string_view Playtest = "playtest";
		constexpr std::string_view Playtest_EnterFullscreenOnMaximizedStart = "enter_fullscreen_on_maximized_start";
		constexpr std::string_view Playtest_AutoHideCursor = "auto_hide_cursor";
		constexpr std::string_view Playtest_SongOffsetSecWasapiShared = "song_offset_sec_wasapi_shared";
		constexpr std::string_view Playtest_SongOffsetSecWasapiExclusive = "song_offset_sec_wasapi_exclusive";

		constexpr std::string_view Interface = "interface";
		constexpr std::string_view Interface_Theme = "theme";
		constexpr std::string_view Interface_BackgroundDisplayType = "background_display_type";
		constexpr std::string_view Interface_BackgroundDisplayType_Editor = "editor";
		constexpr std::string_view Interface_BackgroundDisplayType_EditorWithMovie = "editor_with_movie";
		constexpr std::string_view Interface_BackgroundDisplayType_Playtest = "playtest";
		constexpr std::string_view Interface_BackgroundDisplayType_PlaytestWithMovie = "playtest_with_movie";
		constexpr std::string_view Interface_PlacementGrid = "placement_grid";
		constexpr std::string_view Interface_PlacementGrid_ShowHorizontalSyncMarkers = "show_horizontal_sync_markers";
		constexpr std::string_view Interface_CheckerboardBackground = "checkerboard_background";
		constexpr std::string_view Interface_CheckerboardBackground_PrimaryColor = "primary_color";
		constexpr std::string_view Interface_CheckerboardBackground_SecondaryColor = "secondary_color";
		constexpr std::string_view Interface_CheckerboardBackground_ScreenPixelSize = "screen_pixel_size";
		constexpr std::string_view Interface_PracticeBackground = "practice_background";
		constexpr std::string_view Interface_PracticeBackground_DisableBackgroundGrid = "disable_background_grid";
		constexpr std::string_view Interface_PracticeBackground_DisableBackgroundDim = "disable_background_dim";
		constexpr std::string_view Interface_PracticeBackground_HideNowPrintingPlaceholderImages = "hide_now_printing_placeholder_images";
		constexpr std::string_view Interface_PracticeBackground_HideCoverImage = "hide_cover_image";
		constexpr std::string_view Interface_PracticeBackground_HideLogoImage = "hide_logo_image";
		constexpr std::string_view Interface_MovieBackground = "movie_background";
		constexpr std::string_view Interface_MovieBackground_OverlayColor = "overlay_color";
		constexpr std::string_view Interface_MovieBackground_OverlayColorGridAndPractice = "overlay_color_grid_and_practice";
		constexpr std::string_view Interface_MovieBackground_PreStartPostEndColor = "pre_start_post_end_color";

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
			func(userData.Input.TargetTimeline_SetGridDivision_4, "target_timeline_set_grid_division_1_4");
			func(userData.Input.TargetTimeline_SetGridDivision_8, "target_timeline_set_grid_division_1_8");
			func(userData.Input.TargetTimeline_SetGridDivision_12, "target_timeline_set_grid_division_1_12");
			func(userData.Input.TargetTimeline_SetGridDivision_16, "target_timeline_set_grid_division_1_16");
			func(userData.Input.TargetTimeline_SetGridDivision_24, "target_timeline_set_grid_division_1_24");
			func(userData.Input.TargetTimeline_SetGridDivision_32, "target_timeline_set_grid_division_1_32");
			func(userData.Input.TargetTimeline_SetGridDivision_48, "target_timeline_set_grid_division_1_48");
			func(userData.Input.TargetTimeline_SetGridDivision_64, "target_timeline_set_grid_division_1_64");
			func(userData.Input.TargetTimeline_SetGridDivision_96, "target_timeline_set_grid_division_1_96");
			func(userData.Input.TargetTimeline_SetGridDivision_192, "target_timeline_set_grid_division_1_192");
			func(userData.Input.TargetTimeline_SetChainSlideGridDivision_12, "target_timeline_set_chain_slide_grid_division_1_12");
			func(userData.Input.TargetTimeline_SetChainSlideGridDivision_16, "target_timeline_set_chain_slide_grid_division_1_16");
			func(userData.Input.TargetTimeline_SetChainSlideGridDivision_24, "target_timeline_set_chain_slide_grid_division_1_24");
			func(userData.Input.TargetTimeline_SetChainSlideGridDivision_32, "target_timeline_set_chain_slide_grid_division_1_32");
			func(userData.Input.TargetTimeline_SetChainSlideGridDivision_48, "target_timeline_set_chain_slide_grid_division_1_48");
			func(userData.Input.TargetTimeline_SetChainSlideGridDivision_64, "target_timeline_set_chain_slide_grid_division_1_64");
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
		std::string insituFileContent = IO::File::ReadAllText(filePath);
		if (insituFileContent.empty())
			return false;

		using namespace Json;
		Document rootJson;
		rootJson.ParseInsitu(insituFileContent.data());

		if (rootJson.HasParseError())
		{
			// TODO: Proper error handling
			const auto parseError = rootJson.GetParseError();
			assert(false);
			return false;
		}

		if (!rootJson.IsObject())
			return false;

		const auto fileVersion = TryGetJsonSettingsFileVersionFromRoot(rootJson).value_or(SemanticVersion {});
		if (fileVersion.Major > CurrentVersion.Major)
		{
			Logger::LogErrorLine(__FUNCTION__"(): Unsupported UserSettings version detected: \"%s\". Current version: \"%s\"", fileVersion.ToString().c_str(), CurrentVersion.ToString().c_str());
			return false;
		}

		// NOTE: Restore default so that unspecified objects still start off with reasonable values, thereby improving forward compatibility in case the json is from an older version.
		//		 To compensate all parser code needs to clear out all vectors to avoid duplicate entries and only assign to trivial types if the corresponding json entry was found
		RestoreDefault();

		if (const Value* systemJson = Find(rootJson, UserIDs::System))
		{
			if (const Value* audioJson = Find(*systemJson, UserIDs::System_Audio))
			{
				TryAssign(System.Audio.SongVolume, TryGetF32(Find(*audioJson, UserIDs::System_Audio_SongVolume)));
				TryAssign(System.Audio.ButtonSoundVolume, TryGetF32(Find(*audioJson, UserIDs::System_Audio_ButtonSoundVolume)));
				TryAssign(System.Audio.SoundEffectVolume, TryGetF32(Find(*audioJson, UserIDs::System_Audio_SoundEffectVolume)));
				TryAssign(System.Audio.MetronomeVolume, TryGetF32(Find(*audioJson, UserIDs::System_Audio_MetronomeVolume)));
				TryAssign(System.Audio.OpenDeviceOnStartup, TryGetBool(Find(*audioJson, UserIDs::System_Audio_OpenDeviceOnStartup)));
				TryAssign(System.Audio.CloseDeviceOnIdleFocusLoss, TryGetBool(Find(*audioJson, UserIDs::System_Audio_CloseDeviceOnIdleFocusLoss)));
				TryAssign(System.Audio.RequestExclusiveDeviceAccess, TryGetBool(Find(*audioJson, UserIDs::System_Audio_RequestExclusiveDeviceAccess)));
			}

			if (const Value* videoJson = Find(*systemJson, UserIDs::System_Video))
			{
				TryAssign(System.Video.EnableColorCorrectionEditor, TryGetBool(Find(*videoJson, UserIDs::System_Video_EnableColorCorrectionEditor)));
				TryAssign(System.Video.EnableColorCorrectionPlaytest, TryGetBool(Find(*videoJson, UserIDs::System_Video_EnableColorCorrectionPlaytest)));
				if (const Value* colorCorrectionJson = Find(*videoJson, UserIDs::System_Video_ColorCorrectionParam))
				{
					TryAssign(System.Video.ColorCorrectionParam.Gamma, TryGetF32(Find(*colorCorrectionJson, UserIDs::System_Video_ColorCorrectionParam_Gamma)));
					TryAssign(System.Video.ColorCorrectionParam.Contrast, TryGetF32(Find(*colorCorrectionJson, UserIDs::System_Video_ColorCorrectionParam_Contrast)));
					if (const Value* colorCoefficientsJson = Find(*colorCorrectionJson, UserIDs::System_Video_ColorCorrectionParam_ColorCoefficients); colorCoefficientsJson && colorCoefficientsJson->IsArray())
					{
						constexpr SizeType userDataFloatCount = (3 * 3);
						static_assert(sizeof(System.Video.ColorCorrectionParam.ColorCoefficientsRGB) == (userDataFloatCount * sizeof(f32)));
						f32* userDataFloats = &System.Video.ColorCorrectionParam.ColorCoefficientsRGB[0][0];

						for (SizeType i = 0; i < Min(userDataFloatCount, colorCoefficientsJson->GetArray().Size()); i++)
						{
							if (auto v = TryGetF32(&colorCoefficientsJson->GetArray()[static_cast<SizeType>(i)]); v.has_value())
								userDataFloats[i] = v.value();
						}
					}
				}
			}

			if (const Value* guiJson = Find(*systemJson, UserIDs::System_Gui))
			{
				TryAssign(System.Gui.ShowTestMenu, TryGetBool(Find(*guiJson, UserIDs::System_Gui_ShowTestMenu)));
				TryAssign(System.Gui.AntiAliasedLines, TryGetBool(Find(*guiJson, UserIDs::System_Gui_AntiAliasedLines)));
				TryAssign(System.Gui.AntiAliasedFill, TryGetBool(Find(*guiJson, UserIDs::System_Gui_AntiAliasedFill)));
				TryAssign(System.Gui.TargetDistanceGuideCircleSegments, TryGetI32(Find(*guiJson, UserIDs::System_Gui_TargetDistanceGuideCircleSegments)));
				TryAssign(System.Gui.TargetDistanceGuideMaxCount, TryGetI32(Find(*guiJson, UserIDs::System_Gui_TargetDistanceGuideMaxCount)));
				TryAssign(System.Gui.TargetButtonPathCurveSegments, TryGetI32(Find(*guiJson, UserIDs::System_Gui_TargetButtonPathCurveSegments)));
				TryAssign(System.Gui.TargetButtonPathMaxCount, TryGetI32(Find(*guiJson, UserIDs::System_Gui_TargetButtonPathMaxCount)));
			}

			if (const Value* discordJson = Find(*systemJson, UserIDs::System_Discord))
			{
				TryAssign(System.Discord.EnableRichPresence, TryGetBool(Find(*discordJson, UserIDs::System_Discord_EnableRichPresence)));
				TryAssign(System.Discord.ShareElapsedTime, TryGetBool(Find(*discordJson, UserIDs::System_Discord_ShareElapsedTime)));
			}
		}

		if (const Value* inputJson = Find(rootJson, UserIDs::Input))
		{
			if (const Value* layoutMappingsJson = Find(*inputJson, UserIDs::Input_ControllerLayoutMappings); layoutMappingsJson && layoutMappingsJson->IsArray())
			{
				Input.ControllerLayoutMappings.clear();
				Input.ControllerLayoutMappings.reserve(layoutMappingsJson->Size());
				for (const Value& layoutMappingJson : layoutMappingsJson->GetArray())
				{
					auto layoutMapping = Input::ControllerLayoutMappingFromStorageString(TryGetStrView(&layoutMappingJson).value_or(""));
					if (Input::IsValidControllerID(layoutMapping.ProductID))
						Input.ControllerLayoutMappings.push_back(std::move(layoutMapping));
				}
			}

			if (const Value* bindingsJson = Find(*inputJson, UserIDs::Input_Bindings))
			{
				UserIDs::ForEachMultiBindingWithID(*this, [&](Input::MultiBinding& multiBinding, std::string_view multiBindingID)
				{
					if (const Value* multiBindingJson = Find(*bindingsJson, multiBindingID); multiBindingJson && multiBindingJson->IsArray())
					{
						multiBinding.BindingCount = 0;
						for (const Value& bindingJson : multiBindingJson->GetArray())
						{
							if (multiBinding.BindingCount < multiBinding.Bindings.size() && bindingJson.IsString())
							{
								auto& binding = multiBinding.Bindings[multiBinding.BindingCount++];
								binding = Input::BindingFromStorageString(TryGetStrView(&bindingJson).value_or(""));

								if (binding.IsEmpty())
									multiBinding.BindingCount--;
							}
						}
					}
				});
			}

			if (const Value* playtestBindingsJson = Find(*inputJson, UserIDs::Input_PlaytestBindings); playtestBindingsJson && playtestBindingsJson->IsArray())
			{
				Input.PlaytestBindings.clear();
				Input.PlaytestBindings.reserve(playtestBindingsJson->Size());
				for (const Value& playtestBindingJson : playtestBindingsJson->GetArray())
				{
					if (auto playtestBinding = PlayTestBindingFromStorageString(TryGetStrView(&playtestBindingJson).value_or("")); !playtestBinding.IsEmpty())
						Input.PlaytestBindings.push_back(std::move(playtestBinding));
				}
			}
		}

		if (const Value* targetTimelineJson = Find(rootJson, UserIDs::TargetTimeline))
		{
			TryAssign(TargetTimeline.MouseWheelScrollDirection, TryGetF32(Find(*targetTimelineJson, UserIDs::TargetTimeline_MouseWheelScrollDirection)));
			TryAssign(TargetTimeline.MouseWheelScrollSpeed, TryGetF32(Find(*targetTimelineJson, UserIDs::TargetTimeline_MouseWheelScrollSpeed)));
			TryAssign(TargetTimeline.MouseWheelScrollSpeedShift, TryGetF32(Find(*targetTimelineJson, UserIDs::TargetTimeline_MouseWheelScrollSpeedShift)));
			TryAssign(TargetTimeline.PlaybackMouseWheelScrollFactor, TryGetF32(Find(*targetTimelineJson, UserIDs::TargetTimeline_PlaybackMouseWheelScrollFactor)));
			TryAssign(TargetTimeline.PlaybackMouseWheelScrollFactorShift, TryGetF32(Find(*targetTimelineJson, UserIDs::TargetTimeline_PlaybackMouseWheelScrollFactorShift)));

			TryAssign(TargetTimeline.PlaybackAutoScrollCursorPositionFactor, TryGetF32(Find(*targetTimelineJson, UserIDs::TargetTimeline_PlaybackAutoScrollCursorPositionFactor)));

			if (auto v = TryGetF64(Find(*targetTimelineJson, UserIDs::TargetTimeline_PlaybackCursorPlacementOffsetSecWasapiShared)); v.has_value())
				TargetTimeline.PlaybackCursorPlacementOffsetWasapiShared = TimeSpan::FromSeconds(v.value());
			if (auto v = TryGetF64(Find(*targetTimelineJson, UserIDs::TargetTimeline_PlaybackCursorPlacementOffsetSecWasapiExclusive)); v.has_value())
				TargetTimeline.PlaybackCursorPlacementOffsetWasapiExclusive = TimeSpan::FromSeconds(v.value());

			TryAssign(TargetTimeline.SmoothScrollSpeedSec, TryGetF32(Find(*targetTimelineJson, UserIDs::TargetTimeline_SmoothScrollSpeedSec)));

			TryAssign(TargetTimeline.ShowStartEndMarkersSong, TryGetBool(Find(*targetTimelineJson, UserIDs::TargetTimeline_ShowStartEndMarkersSong)));
			TryAssign(TargetTimeline.ShowStartEndMarkersMovie, TryGetBool(Find(*targetTimelineJson, UserIDs::TargetTimeline_ShowStartEndMarkersMovie)));

			if (const Value* scalingAutoFitJson = Find(*targetTimelineJson, UserIDs::TargetTimeline_ScalingBehaviorAutoFit))
			{
				TargetTimeline.ScalingBehavior = TargetTimelineScalingBehavior::AutoFit;
				TryAssign(TargetTimeline.ScalingBehaviorAutoFit.MinRowHeight, TryGetF32(Find(*scalingAutoFitJson, UserIDs::TargetTimeline_ScalingBehaviorAutoFit_MinRowHeight)));
				TryAssign(TargetTimeline.ScalingBehaviorAutoFit.MaxRowHeight, TryGetF32(Find(*scalingAutoFitJson, UserIDs::TargetTimeline_ScalingBehaviorAutoFit_MaxRowHeight)));
			}
			if (const Value* scalingFixedSizeJson = Find(*targetTimelineJson, UserIDs::TargetTimeline_ScalingBehaviorFixedSize))
			{
				TargetTimeline.ScalingBehavior = TargetTimelineScalingBehavior::FixedSize;
				TryAssign(TargetTimeline.ScalingBehaviorFixedSize.IconScale, TryGetF32(Find(*scalingFixedSizeJson, UserIDs::TargetTimeline_ScalingBehaviorFixedSize_IconScale)));
				TryAssign(TargetTimeline.ScalingBehaviorFixedSize.RowHeight, TryGetF32(Find(*scalingFixedSizeJson, UserIDs::TargetTimeline_ScalingBehaviorFixedSize_RowHeight)));
			}

			if (const Value* thresholdFixedSizeJson = Find(*targetTimelineJson, UserIDs::TargetTimeline_CursorScrubbingEdgeAutoScrollThresholdFixedSize))
			{
				TargetTimeline.CursorScrubbingEdgeAutoScrollThreshold = TargetTimelineCursorScrubbingEdgeAutoScrollThreshold::FixedSize;
				TryAssign(TargetTimeline.CursorScrubbingEdgeAutoScrollThresholdFixedSize.Pixels, TryGetF32(Find(*thresholdFixedSizeJson, UserIDs::TargetTimeline_CursorScrubbingEdgeAutoScrollThresholdFixedSize_Pixels)));
			}
			if (const Value* thresholdProportionalJson = Find(*targetTimelineJson, UserIDs::TargetTimeline_CursorScrubbingEdgeAutoScrollThresholdProportional))
			{
				TargetTimeline.CursorScrubbingEdgeAutoScrollThreshold = TargetTimelineCursorScrubbingEdgeAutoScrollThreshold::Proportional;
				TryAssign(TargetTimeline.CursorScrubbingEdgeAutoScrollThresholdProportional.Factor, TryGetF32(Find(*thresholdProportionalJson, UserIDs::TargetTimeline_CursorScrubbingEdgeAutoScrollThresholdProportional_Factor)));
			}

			TryAssign(TargetTimeline.CursorScrubbingEdgeAutoScrollSmoothScrollSpeedSec, TryGetF32(Find(*targetTimelineJson, UserIDs::TargetTimeline_CursorScrubbingEdgeAutoScrollSmoothScrollSpeedSec)));
			TryAssign(TargetTimeline.CursorScrubbingEdgeAutoScrollSmoothScrollSpeedSecShift, TryGetF32(Find(*targetTimelineJson, UserIDs::TargetTimeline_CursorScrubbingEdgeAutoScrollSmoothScrollSpeedSecShift)));
		}

		if (const Value* targetPreviewJson = Find(rootJson, UserIDs::TargetPreview))
		{
			TryAssign(TargetPreview.ShowButtons, TryGetBool(Find(*targetPreviewJson, UserIDs::TargetPreview_ShowButtons)));
			TryAssign(TargetPreview.ShowHoldInfo, TryGetBool(Find(*targetPreviewJson, UserIDs::TargetPreview_ShowHoldInfo)));
			if (auto v = TryGetI32(Find(*targetPreviewJson, UserIDs::TargetPreview_PostHitLingerDurationTicks)); v.has_value())
				TargetPreview.PostHitLingerDuration = BeatTick::FromTicks(v.value());
		}

		if (const Value* positionToolJson = Find(rootJson, UserIDs::PositionTool))
		{
			TryAssign(PositionTool.ShowDistanceGuides, TryGetBool(Find(*positionToolJson, UserIDs::PositionTool_ShowDistanceGuides)));
			TryAssign(PositionTool.ShowTargetGrabTooltip, TryGetBool(Find(*positionToolJson, UserIDs::PositionTool_ShowTargetGrabTooltip)));
			TryAssign(PositionTool.UseAxisSnapGuides, TryGetBool(Find(*positionToolJson, UserIDs::PositionTool_UseAxisSnapGuides)));
			TryAssign(PositionTool.AxisSnapGuideDistanceThreshold, TryGetF32(Find(*positionToolJson, UserIDs::PositionTool_AxisSnapGuideDistanceThreshold)));
			TryAssign(PositionTool.PositionMouseSnap, TryGetF32(Find(*positionToolJson, UserIDs::PositionTool_PositionMouseSnap)));
			TryAssign(PositionTool.PositionMouseSnapRough, TryGetF32(Find(*positionToolJson, UserIDs::PositionTool_PositionMouseSnapRough)));
			TryAssign(PositionTool.PositionMouseSnapPrecise, TryGetF32(Find(*positionToolJson, UserIDs::PositionTool_PositionMouseSnapPrecise)));
			TryAssign(PositionTool.PositionKeyMoveStep, TryGetF32(Find(*positionToolJson, UserIDs::PositionTool_PositionKeyMoveStep)));
			TryAssign(PositionTool.PositionKeyMoveStepRough, TryGetF32(Find(*positionToolJson, UserIDs::PositionTool_PositionKeyMoveStepRough)));
			TryAssign(PositionTool.PositionKeyMoveStepPrecise, TryGetF32(Find(*positionToolJson, UserIDs::PositionTool_PositionKeyMoveStepPrecise)));
			TryAssign(PositionTool.MouseRowCenterDistanceThreshold, TryGetF32(Find(*positionToolJson, UserIDs::PositionTool_MouseRowCenterDistanceThreshold)));
			TryAssign(PositionTool.PositionInterpolationCommandSnap, TryGetF32(Find(*positionToolJson, UserIDs::PositionTool_PositionInterpolationCommandSnap)));
			if (const Value* diagonalRowLayoutsJson = Find(*positionToolJson, UserIDs::PositionTool_DiagonalMouseRowLayouts); diagonalRowLayoutsJson && diagonalRowLayoutsJson->IsArray())
			{
				PositionTool.DiagonalMouseRowLayouts.clear();
				PositionTool.DiagonalMouseRowLayouts.reserve(diagonalRowLayoutsJson->Size());
				for (const Value& rowLayoutJson : diagonalRowLayoutsJson->GetArray())
				{
					auto& rowLayout = PositionTool.DiagonalMouseRowLayouts.emplace_back();
					rowLayout.PerBeatDiagonalSpacing.x = TryGetF32(Find(rowLayoutJson, UserIDs::PositionTool_DiagonalMouseRowLayouts_PerBeatDiagonalSpacingX)).value_or(0.0f);
					rowLayout.PerBeatDiagonalSpacing.y = TryGetF32(Find(rowLayoutJson, UserIDs::PositionTool_DiagonalMouseRowLayouts_PerBeatDiagonalSpacingY)).value_or(0.0f);
					rowLayout.DisplayName = TryGetStrView(Find(rowLayoutJson, UserIDs::PositionTool_DiagonalMouseRowLayouts_DisplayName)).value_or("");
				}
			}
		}

		if (const Value* pathToolJson = Find(rootJson, UserIDs::PathTool))
		{
			TryAssign(PathTool.AngleMouseSnap, TryGetF32(Find(*pathToolJson, UserIDs::PathTool_AngleMouseSnap)));
			TryAssign(PathTool.AngleMouseSnapRough, TryGetF32(Find(*pathToolJson, UserIDs::PathTool_AngleMouseSnapRough)));
			TryAssign(PathTool.AngleMouseSnapPrecise, TryGetF32(Find(*pathToolJson, UserIDs::PathTool_AngleMouseSnapPrecise)));
			TryAssign(PathTool.AngleMouseScrollDirection, TryGetF32(Find(*pathToolJson, UserIDs::PathTool_AngleMouseScrollDirection)));
			TryAssign(PathTool.AngleMouseScrollStep, TryGetF32(Find(*pathToolJson, UserIDs::PathTool_AngleMouseScrollStep)));
			TryAssign(PathTool.AngleMouseScrollRough, TryGetF32(Find(*pathToolJson, UserIDs::PathTool_AngleMouseScrollRough)));
			TryAssign(PathTool.AngleMouseScrollPrecise, TryGetF32(Find(*pathToolJson, UserIDs::PathTool_AngleMouseScrollPrecise)));
			TryAssign(PathTool.AngleMouseMovementDistanceThreshold, TryGetF32(Find(*pathToolJson, UserIDs::PathTool_AngleMouseMovementDistanceThreshold)));
			TryAssign(PathTool.AngleMouseTargetCenterDistanceThreshold, TryGetF32(Find(*pathToolJson, UserIDs::PathTool_AngleMouseTargetCenterDistanceThreshold)));
		}

		if (const Value* targetPresetJson = Find(rootJson, UserIDs::TargetPreset))
		{
			if (const Value* syncPresetsJson = Find(*targetPresetJson, UserIDs::TargetPreset_StaticSyncPresets); syncPresetsJson && syncPresetsJson->IsArray())
			{
				TargetPreset.StaticSyncPresets.clear();
				TargetPreset.StaticSyncPresets.reserve(syncPresetsJson->Size());
				for (const Value& syncPresetJson : syncPresetsJson->GetArray())
				{
					auto& syncPreset = TargetPreset.StaticSyncPresets.emplace_back();
					syncPreset.Name = TryGetStrView(Find(syncPresetJson, UserIDs::TargetPreset_StaticSyncPresets_Name)).value_or("");

					if (const Value* targetDataArrayJson = Find(syncPresetJson, UserIDs::TargetPreset_StaticSyncPresets_Targets); targetDataArrayJson && targetDataArrayJson->IsArray())
					{
						for (const Value& targetDataJson : targetDataArrayJson->GetArray())
						{
							if (syncPreset.TargetCount < syncPreset.Targets.size())
							{
								auto& targetData = syncPreset.Targets[syncPreset.TargetCount++];
								targetData.Type = static_cast<ButtonType>(TryGetI32(Find(targetDataJson, UserIDs::TargetPreset_StaticSyncPresets_Targets_ButtonType)).value_or(0));

								if (const Value* propertiesJson = Find(targetDataJson, UserIDs::TargetPreset_StaticSyncPresets_Targets_Properties))
									targetData.Properties = JsonTryGetTargetProperties(*propertiesJson);
							}
						}
					}
				}
			}

			if (const Value* sequencePresetsJson = Find(*targetPresetJson, UserIDs::TargetPreset_SequencePresets); sequencePresetsJson && sequencePresetsJson->IsArray())
			{
				TargetPreset.SequencePresets.clear();
				TargetPreset.SequencePresets.reserve(sequencePresetsJson->Size());
				for (const Value& sequencePresetJson : sequencePresetsJson->GetArray())
				{
					auto& sequencePreset = TargetPreset.SequencePresets.emplace_back();
					sequencePreset.ButtonType = static_cast<SequencePresetButtonType>(TryGetI32(Find(sequencePresetJson, UserIDs::TargetPreset_SequencePresets_GuiButtonType)).value_or(0));
					sequencePreset.Name = TryGetStrView(Find(sequencePresetJson, UserIDs::TargetPreset_SequencePresets_Name)).value_or("");

					if (const Value* circleJson = Find(sequencePresetJson, UserIDs::TargetPreset_SequencePresets_Circle))
					{
						sequencePreset.Type = SequencePresetType::Circle;
						sequencePreset.Circle.Duration = BeatTick::FromTicks(TryGetI32(Find(*circleJson, UserIDs::TargetPreset_SequencePresets_Circle_DurationTicks)).value_or(0));
						sequencePreset.Circle.Radius = TryGetF32(Find(*circleJson, UserIDs::TargetPreset_SequencePresets_Circle_Radius)).value_or(0.0f);
						sequencePreset.Circle.Direction = TryGetF32(Find(*circleJson, UserIDs::TargetPreset_SequencePresets_Circle_Direction)).value_or(0.0f);
						sequencePreset.Circle.Center.x = TryGetF32(Find(*circleJson, UserIDs::TargetPreset_SequencePresets_Circle_CenterX)).value_or(0.0f);
						sequencePreset.Circle.Center.y = TryGetF32(Find(*circleJson, UserIDs::TargetPreset_SequencePresets_Circle_CenterY)).value_or(0.0f);
					}
					else if (const Value* bezierPathJson = Find(sequencePresetJson, UserIDs::TargetPreset_SequencePresets_BezierPath))
					{
						// TODO: ...
						sequencePreset.Type = SequencePresetType::BezierPath;
					}
				}
			}

			if (const Value* inspectorDropdownJson = Find(*targetPresetJson, UserIDs::TargetPreset_InspectorDropdown))
			{
				auto parseVector = [inspectorDropdownJson](auto&& id, auto& outVector)
				{
					if (const Value* arrayJson = Find(*inspectorDropdownJson, id); arrayJson && arrayJson->IsArray())
					{
						outVector.clear();
						outVector.reserve(arrayJson->Size());
						for (const Value& itemJson : arrayJson->GetArray())
						{
							if constexpr (std::is_floating_point_v<typename std::remove_reference_t<decltype(outVector)>::value_type>)
								outVector.push_back(TryGetF32(&itemJson).value_or(0.0f));
							else
								outVector.push_back(TryGetI32(&itemJson).value_or(0));
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

		if (const Value* chartPropertiesJson = Find(rootJson, UserIDs::ChartProperties))
		{
			TryAssign(ChartProperties.ChartCreatorDefaultName, TryGetStrView(Find(*chartPropertiesJson, UserIDs::ChartProperties_ChartCreatorDefaultName)));
		}

		if (const Value* bpmCalculatorJson = Find(rootJson, UserIDs::BPMCalculator))
		{
			TryAssign(BPMCalculator.AutoResetEnabled, TryGetBool(Find(*bpmCalculatorJson, UserIDs::BPMCalculator_AutoResetEnabled)));
			TryAssign(BPMCalculator.ApplyToTempoMap, TryGetBool(Find(*bpmCalculatorJson, UserIDs::BPMCalculator_ApplyToTempoMap)));
			if (auto v = TryGetI32(Find(*bpmCalculatorJson, UserIDs::BPMCalculator_TapSoundType)); v.has_value())
				BPMCalculator.TapSoundType = static_cast<BPMTapSoundType>(v.value());
		}

		if (const Value* playtestJson = Find(rootJson, UserIDs::Playtest))
		{
			TryAssign(Playtest.EnterFullscreenOnMaximizedStart, TryGetBool(Find(*playtestJson, UserIDs::Playtest_EnterFullscreenOnMaximizedStart)));
			TryAssign(Playtest.AutoHideCursor, TryGetBool(Find(*playtestJson, UserIDs::Playtest_AutoHideCursor)));
			if (auto v = TryGetF64(Find(*playtestJson, UserIDs::Playtest_SongOffsetSecWasapiShared)); v.has_value())
				Playtest.SongOffsetWasapiShared = TimeSpan::FromSeconds(v.value());
			if (auto v = TryGetF64(Find(*playtestJson, UserIDs::Playtest_SongOffsetSecWasapiExclusive)); v.has_value())
				Playtest.SongOffsetWasapiExclusive = TimeSpan::FromSeconds(v.value());
		}

		if (const Value* interfaceJson = Find(rootJson, UserIDs::Interface))
		{
			// TODO: Json enum string helper functions
			if (auto v = TryGetI32(Find(*interfaceJson, UserIDs::Interface_Theme)); v.has_value())
				Interface.Theme = static_cast<GameTheme>(v.value());

			if (const Value* displayTypeJson = Find(*interfaceJson, UserIDs::Interface_BackgroundDisplayType))
			{
				auto tryAsignEnum = [interfaceJson](ChartBackgroundDisplayType& outEnum, std::string_view key)
				{
					// TODO: Json enum string helper functions
					if (auto v = TryGetI32(Find(*interfaceJson, key)); v.has_value())
						outEnum = static_cast<ChartBackgroundDisplayType>(v.value());
				};

				tryAsignEnum(Interface.BackgroundDisplayType.Editor, UserIDs::Interface_BackgroundDisplayType_Editor);
				tryAsignEnum(Interface.BackgroundDisplayType.EditorWithMovie, UserIDs::Interface_BackgroundDisplayType_EditorWithMovie);
				tryAsignEnum(Interface.BackgroundDisplayType.Playtest, UserIDs::Interface_BackgroundDisplayType_Playtest);
				tryAsignEnum(Interface.BackgroundDisplayType.PlaytestWithMovie, UserIDs::Interface_BackgroundDisplayType_PlaytestWithMovie);
			}

			if (const Value* gridJson = Find(*interfaceJson, UserIDs::Interface_PlacementGrid))
			{
				TryAssign(Interface.PlacementGrid.ShowHorizontalSyncMarkers, TryGetBool(Find(*gridJson, UserIDs::Interface_PlacementGrid_ShowHorizontalSyncMarkers)));
			}

			if (const Value* checkerboardJson = Find(*interfaceJson, UserIDs::Interface_CheckerboardBackground))
			{
				TryAssign(Interface.CheckerboardBackground.PrimaryColor, TryGetVec4HexRGBAStr(Find(*checkerboardJson, UserIDs::Interface_CheckerboardBackground_PrimaryColor)));
				TryAssign(Interface.CheckerboardBackground.SecondaryColor, TryGetVec4HexRGBAStr(Find(*checkerboardJson, UserIDs::Interface_CheckerboardBackground_SecondaryColor)));
				TryAssign(Interface.CheckerboardBackground.ScreenPixelSize, TryGetI32(Find(*checkerboardJson, UserIDs::Interface_CheckerboardBackground_ScreenPixelSize)));
			}

			if (const Value* practiceJson = Find(*interfaceJson, UserIDs::Interface_PracticeBackground))
			{
				TryAssign(Interface.PracticeBackground.DisableBackgroundGrid, TryGetBool(Find(*practiceJson, UserIDs::Interface_PracticeBackground_DisableBackgroundGrid)));
				TryAssign(Interface.PracticeBackground.DisableBackgroundDim, TryGetBool(Find(*practiceJson, UserIDs::Interface_PracticeBackground_DisableBackgroundDim)));
				TryAssign(Interface.PracticeBackground.HideNowPrintingPlaceholderImages, TryGetBool(Find(*practiceJson, UserIDs::Interface_PracticeBackground_HideNowPrintingPlaceholderImages)));
				TryAssign(Interface.PracticeBackground.HideCoverImage, TryGetBool(Find(*practiceJson, UserIDs::Interface_PracticeBackground_HideCoverImage)));
				TryAssign(Interface.PracticeBackground.HideLogoImage, TryGetBool(Find(*practiceJson, UserIDs::Interface_PracticeBackground_HideLogoImage)));
			}

			if (const Value* movieJson = Find(*interfaceJson, UserIDs::Interface_MovieBackground))
			{
				TryAssign(Interface.MovieBackground.OverlayColor, TryGetVec4HexRGBAStr(Find(*movieJson, UserIDs::Interface_MovieBackground_OverlayColor)));
				TryAssign(Interface.MovieBackground.OverlayColorGridAndPractice, TryGetVec4HexRGBAStr(Find(*movieJson, UserIDs::Interface_MovieBackground_OverlayColorGridAndPractice)));
				TryAssign(Interface.MovieBackground.PreStartPostEndColor, TryGetVec4HexRGBAStr(Find(*movieJson, UserIDs::Interface_MovieBackground_PreStartPostEndColor)));
			}
		}

		return true;
	}

	void ComfyStudioUserSettings::SaveToFile(std::string_view filePath) const
	{
		auto memberF32Vector = [](Json::WriterEx& writer, std::string_view key, const std::vector<f32>& values)
		{
			writer.MemberArrayBegin(key);
			for (const f32 value : values)
				writer.F32(value);
			writer.MemberArrayEnd();
		};

		auto memberI32Vector = [](Json::WriterEx& writer, std::string_view key, const std::vector<i32>& values)
		{
			writer.MemberArrayBegin(key);
			for (const i32 value : values)
				writer.I32(value);
			writer.MemberArrayEnd();
		};

		Json::WriteBuffer writeBuffer {};
		Json::WriterEx writer { writeBuffer };
		writeBuffer.Reserve(0x8000);

		writer.ObjectBegin();
		{
			writer.MemberStr(IDs::FileVersion, CurrentVersion.ToString());

			writer.MemberObjectBegin(UserIDs::System);
			{
				writer.MemberObjectBegin(UserIDs::System_Audio);
				{
					writer.MemberF32(UserIDs::System_Audio_SongVolume, System.Audio.SongVolume);
					writer.MemberF32(UserIDs::System_Audio_ButtonSoundVolume, System.Audio.ButtonSoundVolume);
					writer.MemberF32(UserIDs::System_Audio_SoundEffectVolume, System.Audio.SoundEffectVolume);
					writer.MemberF32(UserIDs::System_Audio_MetronomeVolume, System.Audio.MetronomeVolume);
					writer.MemberBool(UserIDs::System_Audio_OpenDeviceOnStartup, System.Audio.OpenDeviceOnStartup);
					writer.MemberBool(UserIDs::System_Audio_CloseDeviceOnIdleFocusLoss, System.Audio.CloseDeviceOnIdleFocusLoss);
					writer.MemberBool(UserIDs::System_Audio_RequestExclusiveDeviceAccess, System.Audio.RequestExclusiveDeviceAccess);
				}
				writer.MemberObjectEnd();

				writer.MemberObjectBegin(UserIDs::System_Video);
				{
					writer.MemberBool(UserIDs::System_Video_EnableColorCorrectionEditor, System.Video.EnableColorCorrectionEditor);
					writer.MemberBool(UserIDs::System_Video_EnableColorCorrectionPlaytest, System.Video.EnableColorCorrectionPlaytest);
					writer.MemberObjectBegin(UserIDs::System_Video_ColorCorrectionParam);
					{
						writer.MemberF32(UserIDs::System_Video_ColorCorrectionParam_Gamma, System.Video.ColorCorrectionParam.Gamma);
						writer.MemberF32(UserIDs::System_Video_ColorCorrectionParam_Contrast, System.Video.ColorCorrectionParam.Contrast);
						writer.MemberArrayBegin(UserIDs::System_Video_ColorCorrectionParam_ColorCoefficients);
						{
							for (auto& rgbVec3s : System.Video.ColorCorrectionParam.ColorCoefficientsRGB)
							{
								writer.F32(rgbVec3s[0]);
								writer.F32(rgbVec3s[1]);
								writer.F32(rgbVec3s[2]);
							}
						}
						writer.MemberArrayEnd();
					}
					writer.MemberObjectEnd();
				}
				writer.MemberObjectEnd();

				writer.MemberObjectBegin(UserIDs::System_Gui);
				{
					writer.MemberBool(UserIDs::System_Gui_ShowTestMenu, System.Gui.ShowTestMenu);
					writer.MemberBool(UserIDs::System_Gui_AntiAliasedLines, System.Gui.AntiAliasedLines);
					writer.MemberBool(UserIDs::System_Gui_AntiAliasedFill, System.Gui.AntiAliasedFill);
					writer.MemberI32(UserIDs::System_Gui_TargetDistanceGuideCircleSegments, System.Gui.TargetDistanceGuideCircleSegments);
					writer.MemberI32(UserIDs::System_Gui_TargetDistanceGuideMaxCount, System.Gui.TargetDistanceGuideMaxCount);
					writer.MemberI32(UserIDs::System_Gui_TargetButtonPathCurveSegments, System.Gui.TargetButtonPathCurveSegments);
					writer.MemberI32(UserIDs::System_Gui_TargetButtonPathMaxCount, System.Gui.TargetButtonPathMaxCount);
				}
				writer.MemberObjectEnd();

				writer.MemberObjectBegin(UserIDs::System_Discord);
				{
					writer.MemberBool(UserIDs::System_Discord_EnableRichPresence, System.Discord.EnableRichPresence);
					writer.MemberBool(UserIDs::System_Discord_ShareElapsedTime, System.Discord.ShareElapsedTime);
				}
				writer.MemberObjectEnd();
			}
			writer.MemberObjectEnd();

			writer.MemberObjectBegin(UserIDs::Input);
			{
				writer.MemberArrayBegin(UserIDs::Input_ControllerLayoutMappings);
				{
					for (const auto& layoutMapping : Input.ControllerLayoutMappings)
						writer.Str(Input::ControllerLayoutMappingToStorageString(layoutMapping));
				}
				writer.MemberArrayEnd();

				writer.MemberObjectBegin(UserIDs::Input_Bindings);
				{
					UserIDs::ForEachMultiBindingWithID(*const_cast<ComfyStudioUserSettings*>(this), [&](const Input::MultiBinding& multiBinding, std::string_view multiBindingID)
					{
						writer.MemberArrayBegin(multiBindingID);
						{
							for (const auto& binding : multiBinding)
								writer.Str(Input::BindingToStorageString(binding).data());
						}
						writer.MemberArrayEnd();
					});
				}
				writer.MemberObjectEnd();

				writer.MemberArrayBegin(UserIDs::Input_PlaytestBindings);
				{
					for (const auto& playtestBinding : Input.PlaytestBindings)
						writer.Str(PlayTestBindingToStorageString(playtestBinding).data());
				}
				writer.MemberArrayEnd();


			}
			writer.MemberObjectEnd();

			writer.MemberObjectBegin(UserIDs::TargetTimeline);
			{
				writer.MemberF32(UserIDs::TargetTimeline_MouseWheelScrollDirection, TargetTimeline.MouseWheelScrollDirection);
				writer.MemberF32(UserIDs::TargetTimeline_MouseWheelScrollSpeed, TargetTimeline.MouseWheelScrollSpeed);
				writer.MemberF32(UserIDs::TargetTimeline_MouseWheelScrollSpeedShift, TargetTimeline.MouseWheelScrollSpeedShift);
				writer.MemberF32(UserIDs::TargetTimeline_PlaybackMouseWheelScrollFactor, TargetTimeline.PlaybackMouseWheelScrollFactor);
				writer.MemberF32(UserIDs::TargetTimeline_PlaybackMouseWheelScrollFactorShift, TargetTimeline.PlaybackMouseWheelScrollFactorShift);

				writer.MemberF32(UserIDs::TargetTimeline_PlaybackAutoScrollCursorPositionFactor, TargetTimeline.PlaybackAutoScrollCursorPositionFactor);

				writer.MemberF64(UserIDs::TargetTimeline_PlaybackCursorPlacementOffsetSecWasapiShared, TargetTimeline.PlaybackCursorPlacementOffsetWasapiShared.TotalSeconds());
				writer.MemberF64(UserIDs::TargetTimeline_PlaybackCursorPlacementOffsetSecWasapiExclusive, TargetTimeline.PlaybackCursorPlacementOffsetWasapiExclusive.TotalSeconds());

				writer.MemberF32(UserIDs::TargetTimeline_SmoothScrollSpeedSec, TargetTimeline.SmoothScrollSpeedSec);

				writer.MemberBool(UserIDs::TargetTimeline_ShowStartEndMarkersSong, TargetTimeline.ShowStartEndMarkersSong);
				writer.MemberBool(UserIDs::TargetTimeline_ShowStartEndMarkersMovie, TargetTimeline.ShowStartEndMarkersMovie);

				if (TargetTimeline.ScalingBehavior == TargetTimelineScalingBehavior::AutoFit)
				{
					writer.MemberObjectBegin(UserIDs::TargetTimeline_ScalingBehaviorAutoFit);
					{
						writer.MemberF32(UserIDs::TargetTimeline_ScalingBehaviorAutoFit_MinRowHeight, TargetTimeline.ScalingBehaviorAutoFit.MinRowHeight);
						writer.MemberF32(UserIDs::TargetTimeline_ScalingBehaviorAutoFit_MaxRowHeight, TargetTimeline.ScalingBehaviorAutoFit.MaxRowHeight);
					}
					writer.ObjectEnd();
				}
				else if (TargetTimeline.ScalingBehavior == TargetTimelineScalingBehavior::FixedSize)
				{
					writer.MemberObjectBegin(UserIDs::TargetTimeline_ScalingBehaviorFixedSize);
					{
						writer.MemberF32(UserIDs::TargetTimeline_ScalingBehaviorFixedSize_IconScale, TargetTimeline.ScalingBehaviorFixedSize.IconScale);
						writer.MemberF32(UserIDs::TargetTimeline_ScalingBehaviorFixedSize_RowHeight, TargetTimeline.ScalingBehaviorFixedSize.RowHeight);
					}
					writer.ObjectEnd();
				}

				if (TargetTimeline.CursorScrubbingEdgeAutoScrollThreshold == TargetTimelineCursorScrubbingEdgeAutoScrollThreshold::FixedSize)
				{
					writer.MemberObjectBegin(UserIDs::TargetTimeline_CursorScrubbingEdgeAutoScrollThresholdFixedSize);
					{
						writer.MemberF32(UserIDs::TargetTimeline_CursorScrubbingEdgeAutoScrollThresholdFixedSize_Pixels, TargetTimeline.CursorScrubbingEdgeAutoScrollThresholdFixedSize.Pixels);
					}
					writer.ObjectEnd();
				}
				else if (TargetTimeline.CursorScrubbingEdgeAutoScrollThreshold == TargetTimelineCursorScrubbingEdgeAutoScrollThreshold::Proportional)
				{
					writer.MemberObjectBegin(UserIDs::TargetTimeline_CursorScrubbingEdgeAutoScrollThresholdProportional);
					{
						writer.MemberF32(UserIDs::TargetTimeline_CursorScrubbingEdgeAutoScrollThresholdProportional_Factor, TargetTimeline.CursorScrubbingEdgeAutoScrollThresholdProportional.Factor);
					}
					writer.ObjectEnd();
				}

				writer.MemberF32(UserIDs::TargetTimeline_CursorScrubbingEdgeAutoScrollSmoothScrollSpeedSec, TargetTimeline.CursorScrubbingEdgeAutoScrollSmoothScrollSpeedSec);
				writer.MemberF32(UserIDs::TargetTimeline_CursorScrubbingEdgeAutoScrollSmoothScrollSpeedSecShift, TargetTimeline.CursorScrubbingEdgeAutoScrollSmoothScrollSpeedSecShift);
			}
			writer.MemberObjectEnd();

			writer.MemberObjectBegin(UserIDs::TargetPreview);
			{
				writer.MemberBool(UserIDs::TargetPreview_ShowButtons, TargetPreview.ShowButtons);
				writer.MemberBool(UserIDs::TargetPreview_ShowHoldInfo, TargetPreview.ShowHoldInfo);
				writer.MemberI32(UserIDs::TargetPreview_PostHitLingerDurationTicks, TargetPreview.PostHitLingerDuration.Ticks());
			}
			writer.MemberObjectEnd();

			writer.MemberObjectBegin(UserIDs::PositionTool);
			{
				writer.MemberBool(UserIDs::PositionTool_ShowDistanceGuides, PositionTool.ShowDistanceGuides);
				writer.MemberBool(UserIDs::PositionTool_ShowTargetGrabTooltip, PositionTool.ShowTargetGrabTooltip);
				writer.MemberBool(UserIDs::PositionTool_UseAxisSnapGuides, PositionTool.UseAxisSnapGuides);
				writer.MemberF32(UserIDs::PositionTool_AxisSnapGuideDistanceThreshold, PositionTool.AxisSnapGuideDistanceThreshold);
				writer.MemberF32(UserIDs::PositionTool_PositionMouseSnap, PositionTool.PositionMouseSnap);
				writer.MemberF32(UserIDs::PositionTool_PositionMouseSnapRough, PositionTool.PositionMouseSnapRough);
				writer.MemberF32(UserIDs::PositionTool_PositionMouseSnapPrecise, PositionTool.PositionMouseSnapPrecise);
				writer.MemberF32(UserIDs::PositionTool_PositionKeyMoveStep, PositionTool.PositionKeyMoveStep);
				writer.MemberF32(UserIDs::PositionTool_PositionKeyMoveStepRough, PositionTool.PositionKeyMoveStepRough);
				writer.MemberF32(UserIDs::PositionTool_PositionKeyMoveStepPrecise, PositionTool.PositionKeyMoveStepPrecise);
				writer.MemberF32(UserIDs::PositionTool_MouseRowCenterDistanceThreshold, PositionTool.MouseRowCenterDistanceThreshold);
				writer.MemberF32(UserIDs::PositionTool_PositionInterpolationCommandSnap, PositionTool.PositionInterpolationCommandSnap);

				writer.MemberArrayBegin(UserIDs::PositionTool_DiagonalMouseRowLayouts);
				{
					for (const auto& rowLayout : PositionTool.DiagonalMouseRowLayouts)
					{
						writer.ObjectBegin();
						writer.MemberF32(UserIDs::PositionTool_DiagonalMouseRowLayouts_PerBeatDiagonalSpacingX, rowLayout.PerBeatDiagonalSpacing.x);
						writer.MemberF32(UserIDs::PositionTool_DiagonalMouseRowLayouts_PerBeatDiagonalSpacingY, rowLayout.PerBeatDiagonalSpacing.y);
						writer.MemberStr(UserIDs::PositionTool_DiagonalMouseRowLayouts_DisplayName, rowLayout.DisplayName);
						writer.ObjectEnd();
					}
				}
				writer.MemberArrayEnd();
			}
			writer.MemberObjectEnd();

			writer.MemberObjectBegin(UserIDs::PathTool);
			{
				writer.MemberF32(UserIDs::PathTool_AngleMouseSnap, PathTool.AngleMouseSnap);
				writer.MemberF32(UserIDs::PathTool_AngleMouseSnapRough, PathTool.AngleMouseSnapRough);
				writer.MemberF32(UserIDs::PathTool_AngleMouseSnapPrecise, PathTool.AngleMouseSnapPrecise);
				writer.MemberF32(UserIDs::PathTool_AngleMouseScrollDirection, PathTool.AngleMouseScrollDirection);
				writer.MemberF32(UserIDs::PathTool_AngleMouseScrollStep, PathTool.AngleMouseScrollStep);
				writer.MemberF32(UserIDs::PathTool_AngleMouseScrollRough, PathTool.AngleMouseScrollRough);
				writer.MemberF32(UserIDs::PathTool_AngleMouseScrollPrecise, PathTool.AngleMouseScrollPrecise);
				writer.MemberF32(UserIDs::PathTool_AngleMouseMovementDistanceThreshold, PathTool.AngleMouseMovementDistanceThreshold);
				writer.MemberF32(UserIDs::PathTool_AngleMouseTargetCenterDistanceThreshold, PathTool.AngleMouseTargetCenterDistanceThreshold);
			}
			writer.MemberObjectEnd();

			writer.MemberObjectBegin(UserIDs::TargetPreset);
			{
				writer.MemberArrayBegin(UserIDs::TargetPreset_StaticSyncPresets);
				{
					for (const auto& syncPreset : TargetPreset.StaticSyncPresets)
					{
						writer.ObjectBegin();
						writer.MemberStr(UserIDs::TargetPreset_StaticSyncPresets_Name, syncPreset.Name);
						writer.MemberArrayBegin(UserIDs::TargetPreset_StaticSyncPresets_Targets);
						{
							for (size_t i = 0; i < syncPreset.TargetCount; i++)
							{
								const auto& targetData = syncPreset.Targets[i];

								writer.ObjectBegin();
								writer.MemberI32(UserIDs::TargetPreset_StaticSyncPresets_Targets_ButtonType, static_cast<i32>(targetData.Type));

								writer.MemberObjectBegin(UserIDs::TargetPreset_StaticSyncPresets_Targets_Properties);
								writer.MemberF32(IDs::TargetProperties_PositionX, targetData.Properties.Position.x);
								writer.MemberF32(IDs::TargetProperties_PositionY, targetData.Properties.Position.y);
								writer.MemberF32(IDs::TargetProperties_Angle, targetData.Properties.Angle);
								writer.MemberF32(IDs::TargetProperties_Frequency, targetData.Properties.Frequency);
								writer.MemberF32(IDs::TargetProperties_Amplitude, targetData.Properties.Amplitude);
								writer.MemberF32(IDs::TargetProperties_Distance, targetData.Properties.Distance);
								writer.MemberObjectEnd();

								writer.ObjectEnd();
							}
						}
						writer.MemberArrayEnd();
						writer.ObjectEnd();
					}
				}
				writer.MemberArrayEnd();

				writer.MemberArrayBegin(UserIDs::TargetPreset_SequencePresets);
				{
					for (const auto& sequencePreset : TargetPreset.SequencePresets)
					{
						writer.ObjectBegin();

						writer.MemberI32(UserIDs::TargetPreset_SequencePresets_GuiButtonType, static_cast<i32>(sequencePreset.ButtonType));
						writer.MemberStr(UserIDs::TargetPreset_SequencePresets_Name, sequencePreset.Name);

						if (sequencePreset.Type == SequencePresetType::Circle)
						{
							writer.MemberObjectBegin(UserIDs::TargetPreset_SequencePresets_Circle);
							writer.MemberI32(UserIDs::TargetPreset_SequencePresets_Circle_DurationTicks, sequencePreset.Circle.Duration.Ticks());
							writer.MemberF32(UserIDs::TargetPreset_SequencePresets_Circle_Radius, sequencePreset.Circle.Radius);
							writer.MemberF32(UserIDs::TargetPreset_SequencePresets_Circle_Direction, sequencePreset.Circle.Direction);
							writer.MemberF32(UserIDs::TargetPreset_SequencePresets_Circle_CenterX, sequencePreset.Circle.Center.x);
							writer.MemberF32(UserIDs::TargetPreset_SequencePresets_Circle_CenterY, sequencePreset.Circle.Center.y);
							writer.MemberObjectEnd();
						}
						else if (sequencePreset.Type == SequencePresetType::BezierPath)
						{
							writer.MemberObjectBegin(UserIDs::TargetPreset_SequencePresets_BezierPath);
							// TODO: ...
							writer.MemberObjectEnd();
						}
						writer.ObjectEnd();
					}
				}
				writer.MemberArrayEnd();

				writer.MemberObjectBegin(UserIDs::TargetPreset_InspectorDropdown);
				{
					memberF32Vector(writer, UserIDs::TargetPreset_InspectorDropdown_PositionsX, TargetPreset.InspectorDropdown.PositionsX);
					memberF32Vector(writer, UserIDs::TargetPreset_InspectorDropdown_PositionsY, TargetPreset.InspectorDropdown.PositionsY);
					memberF32Vector(writer, UserIDs::TargetPreset_InspectorDropdown_Angles, TargetPreset.InspectorDropdown.Angles);
					memberI32Vector(writer, UserIDs::TargetPreset_InspectorDropdown_Frequencies, TargetPreset.InspectorDropdown.Frequencies);
					memberF32Vector(writer, UserIDs::TargetPreset_InspectorDropdown_Amplitudes, TargetPreset.InspectorDropdown.Amplitudes);
					memberF32Vector(writer, UserIDs::TargetPreset_InspectorDropdown_Distances, TargetPreset.InspectorDropdown.Distances);
				}
				writer.MemberObjectEnd();
			}
			writer.MemberObjectEnd();

			writer.MemberObjectBegin(UserIDs::ChartProperties);
			{
				writer.MemberStr(UserIDs::ChartProperties_ChartCreatorDefaultName, ChartProperties.ChartCreatorDefaultName);
			}
			writer.MemberObjectEnd();

			writer.MemberObjectBegin(UserIDs::BPMCalculator);
			{
				writer.MemberBool(UserIDs::BPMCalculator_AutoResetEnabled, BPMCalculator.AutoResetEnabled);
				writer.MemberBool(UserIDs::BPMCalculator_ApplyToTempoMap, BPMCalculator.ApplyToTempoMap);
				writer.MemberI32(UserIDs::BPMCalculator_TapSoundType, static_cast<i32>(BPMCalculator.TapSoundType));
			}
			writer.MemberObjectEnd();

			writer.MemberObjectBegin(UserIDs::Playtest);
			{
				writer.MemberBool(UserIDs::Playtest_EnterFullscreenOnMaximizedStart, Playtest.EnterFullscreenOnMaximizedStart);
				writer.MemberBool(UserIDs::Playtest_AutoHideCursor, Playtest.AutoHideCursor);
				writer.MemberF64(UserIDs::Playtest_SongOffsetSecWasapiShared, Playtest.SongOffsetWasapiShared.TotalSeconds());
				writer.MemberF64(UserIDs::Playtest_SongOffsetSecWasapiExclusive, Playtest.SongOffsetWasapiExclusive.TotalSeconds());
			}
			writer.MemberObjectEnd();

			writer.MemberObjectBegin(UserIDs::Interface);
			{
				// TODO: Json enum string helper functions
				writer.MemberI32(UserIDs::Interface_Theme, static_cast<i32>(Interface.Theme));

				writer.MemberObjectBegin(UserIDs::Interface_BackgroundDisplayType);
				{
					// TODO: Json enum string helper functions
					writer.MemberI32(UserIDs::Interface_BackgroundDisplayType_Editor, static_cast<i32>(Interface.BackgroundDisplayType.Editor));
					writer.MemberI32(UserIDs::Interface_BackgroundDisplayType_EditorWithMovie, static_cast<i32>(Interface.BackgroundDisplayType.EditorWithMovie));
					writer.MemberI32(UserIDs::Interface_BackgroundDisplayType_Playtest, static_cast<i32>(Interface.BackgroundDisplayType.Playtest));
					writer.MemberI32(UserIDs::Interface_BackgroundDisplayType_PlaytestWithMovie, static_cast<i32>(Interface.BackgroundDisplayType.PlaytestWithMovie));
				}
				writer.MemberObjectEnd();

				writer.MemberObjectBegin(UserIDs::Interface_PlacementGrid);
				{
					writer.MemberBool(UserIDs::Interface_PlacementGrid_ShowHorizontalSyncMarkers, Interface.PlacementGrid.ShowHorizontalSyncMarkers);
				}
				writer.MemberObjectEnd();

				writer.MemberObjectBegin(UserIDs::Interface_CheckerboardBackground);
				{
					writer.MemberHexRGBAStr(UserIDs::Interface_CheckerboardBackground_PrimaryColor, Interface.CheckerboardBackground.PrimaryColor);
					writer.MemberHexRGBAStr(UserIDs::Interface_CheckerboardBackground_SecondaryColor, Interface.CheckerboardBackground.SecondaryColor);
					writer.MemberI32(UserIDs::Interface_CheckerboardBackground_ScreenPixelSize, Interface.CheckerboardBackground.ScreenPixelSize);
				}
				writer.MemberObjectEnd();

				writer.MemberObjectBegin(UserIDs::Interface_PracticeBackground);
				{
					writer.MemberBool(UserIDs::Interface_PracticeBackground_DisableBackgroundGrid, Interface.PracticeBackground.DisableBackgroundGrid);
					writer.MemberBool(UserIDs::Interface_PracticeBackground_DisableBackgroundDim, Interface.PracticeBackground.DisableBackgroundDim);
					writer.MemberBool(UserIDs::Interface_PracticeBackground_HideNowPrintingPlaceholderImages, Interface.PracticeBackground.HideNowPrintingPlaceholderImages);
					writer.MemberBool(UserIDs::Interface_PracticeBackground_HideCoverImage, Interface.PracticeBackground.HideCoverImage);
					writer.MemberBool(UserIDs::Interface_PracticeBackground_HideLogoImage, Interface.PracticeBackground.HideLogoImage);
				}
				writer.MemberObjectEnd();

				writer.MemberObjectBegin(UserIDs::Interface_MovieBackground);
				{
					writer.MemberHexRGBAStr(UserIDs::Interface_MovieBackground_OverlayColor, Interface.MovieBackground.OverlayColor);
					writer.MemberHexRGBAStr(UserIDs::Interface_MovieBackground_OverlayColorGridAndPractice, Interface.MovieBackground.OverlayColorGridAndPractice);
					writer.MemberHexRGBAStr(UserIDs::Interface_MovieBackground_PreStartPostEndColor, Interface.MovieBackground.PreStartPostEndColor);
				}
				writer.MemberObjectEnd();
			}
			writer.MemberObjectEnd();
		}
		writer.ObjectEnd();

		const auto formattedOutputJson = std::string_view(writeBuffer.GetString(), writeBuffer.GetLength());
		IO::File::WriteAllText(filePath, formattedOutputJson);
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

		System.Video.EnableColorCorrectionEditor = false;
		System.Video.EnableColorCorrectionPlaytest = true;
		System.Video.ColorCorrectionParam.Gamma = 2.8f;
		System.Video.ColorCorrectionParam.Contrast = 0.455f;
		System.Video.ColorCorrectionParam.ColorCoefficientsRGB[0] = vec3(1.0f, 0.0f, 0.0f);
		System.Video.ColorCorrectionParam.ColorCoefficientsRGB[1] = vec3(0.0f, 1.0f, 0.0f);
		System.Video.ColorCorrectionParam.ColorCoefficientsRGB[2] = vec3(0.0f, 0.0f, 1.0f);

		System.Gui.ShowTestMenu = false;
		System.Gui.AntiAliasedLines = true;
		System.Gui.AntiAliasedFill = true;
		// NOTE: ... auto tessellation (?)
		System.Gui.TargetDistanceGuideCircleSegments = 0; // 64;
		System.Gui.TargetDistanceGuideMaxCount = 64;
		System.Gui.TargetButtonPathCurveSegments = 32;
		System.Gui.TargetButtonPathMaxCount = 64;

		// NOTE: This one is debatable...
		System.Discord.EnableRichPresence = true;
		System.Discord.ShareElapsedTime = true;

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
			Input.TargetTimeline_SetGridDivision_4 = MultiBinding();
			Input.TargetTimeline_SetGridDivision_8 = MultiBinding();
			Input.TargetTimeline_SetGridDivision_12 = MultiBinding();
			Input.TargetTimeline_SetGridDivision_16 = MultiBinding();
			Input.TargetTimeline_SetGridDivision_24 = MultiBinding();
			Input.TargetTimeline_SetGridDivision_32 = MultiBinding();
			Input.TargetTimeline_SetGridDivision_48 = MultiBinding();
			Input.TargetTimeline_SetGridDivision_64 = MultiBinding();
			Input.TargetTimeline_SetGridDivision_96 = MultiBinding();
			Input.TargetTimeline_SetGridDivision_192 = MultiBinding();
			Input.TargetTimeline_SetChainSlideGridDivision_12 = MultiBinding();
			Input.TargetTimeline_SetChainSlideGridDivision_16 = MultiBinding();
			Input.TargetTimeline_SetChainSlideGridDivision_24 = MultiBinding();
			Input.TargetTimeline_SetChainSlideGridDivision_32 = MultiBinding();
			Input.TargetTimeline_SetChainSlideGridDivision_48 = MultiBinding();
			Input.TargetTimeline_SetChainSlideGridDivision_64 = MultiBinding();
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

		TargetTimeline.MouseWheelScrollDirection = +1.0f;
		TargetTimeline.MouseWheelScrollSpeed = TargetTimelineDefaultMouseWheelScrollSpeed;
		TargetTimeline.MouseWheelScrollSpeedShift = TargetTimelineDefaultMouseWheelScrollSpeedShift;
		TargetTimeline.PlaybackMouseWheelScrollFactor = 1.0f;
		TargetTimeline.PlaybackMouseWheelScrollFactorShift = 1.0f;
		TargetTimeline.PlaybackAutoScrollCursorPositionFactor = TargetTimelineDefaultPlaybackAutoScrollCursorPositionFactor;
		TargetTimeline.PlaybackCursorPlacementOffsetWasapiShared = TimeSpan::Zero();
		TargetTimeline.PlaybackCursorPlacementOffsetWasapiExclusive = TimeSpan::Zero();
		TargetTimeline.SmoothScrollSpeedSec = TimelineDefaultSmoothScrollSpeedSec.x;
		TargetTimeline.ShowStartEndMarkersSong = true;
		TargetTimeline.ShowStartEndMarkersMovie = true;
		TargetTimeline.ScalingBehavior = TargetTimelineScalingBehavior::AutoFit;
		TargetTimeline.ScalingBehaviorAutoFit.MinRowHeight = TargetTimelineMinRowHeight;
		TargetTimeline.ScalingBehaviorAutoFit.MaxRowHeight = TargetTimelineDefaultRowHeight;
		TargetTimeline.ScalingBehaviorFixedSize.IconScale = TargetTimelineDefaultIconScale;
		TargetTimeline.ScalingBehaviorFixedSize.RowHeight = TargetTimelineDefaultRowHeight;
		TargetTimeline.CursorScrubbingEdgeAutoScrollThreshold = TargetTimelineCursorScrubbingEdgeAutoScrollThreshold::FixedSize;
		TargetTimeline.CursorScrubbingEdgeAutoScrollThresholdFixedSize.Pixels = TargetTimelineDefaultCursorScrubbingEdgeAutoScrollProportionalFactorFixedSizePixels;
		TargetTimeline.CursorScrubbingEdgeAutoScrollThresholdProportional.Factor = TargetTimelineDefaultCursorScrubbingEdgeAutoScrollProportionalFactor;
		TargetTimeline.CursorScrubbingEdgeAutoScrollSmoothScrollSpeedSec = TargetTimelineDefaultCursorScrubbingEdgeAutoScrollSmoothScrollSpeedSec;
		TargetTimeline.CursorScrubbingEdgeAutoScrollSmoothScrollSpeedSecShift = TargetTimelineDefaultCursorScrubbingEdgeAutoScrollSmoothScrollSpeedSecShift;

		TargetPreview.ShowButtons = true;
		TargetPreview.ShowHoldInfo = true;
		TargetPreview.PostHitLingerDuration = BeatTick::FromBeats(1);

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
		Playtest.SongOffsetWasapiShared = TimeSpan::Zero();
		Playtest.SongOffsetWasapiExclusive = TimeSpan::Zero();

		Interface.Theme = GameTheme::PS4ColorfulTone;
		Interface.BackgroundDisplayType.Editor = ChartBackgroundDisplayType::Checkerboard_Grid;
		Interface.BackgroundDisplayType.EditorWithMovie = ChartBackgroundDisplayType::Movie_Grid;
		Interface.BackgroundDisplayType.Playtest = ChartBackgroundDisplayType::Practice;
		Interface.BackgroundDisplayType.PlaytestWithMovie = ChartBackgroundDisplayType::Movie;
		Interface.PlacementGrid.ShowHorizontalSyncMarkers = true;
		Interface.CheckerboardBackground.PrimaryColor = { 0.13f, 0.13f, 0.13f, 1.00f };
		Interface.CheckerboardBackground.SecondaryColor = { 0.17f, 0.17f, 0.17f, 1.00f };
		Interface.CheckerboardBackground.ScreenPixelSize = 5;
		Interface.PracticeBackground.DisableBackgroundGrid = false;
		Interface.PracticeBackground.DisableBackgroundDim = false;
		Interface.PracticeBackground.HideNowPrintingPlaceholderImages = false;
		Interface.PracticeBackground.HideCoverImage = false;
		Interface.PracticeBackground.HideLogoImage = false;
		Interface.MovieBackground.OverlayColor = { 0.0f, 0.0f, 0.0f, 0.0f };
		Interface.MovieBackground.OverlayColorGridAndPractice = { 0.0f, 0.0f, 0.0f, 0.6f };
		Interface.MovieBackground.PreStartPostEndColor = { 0.13f, 0.13f, 0.13f, 1.00f };
	}
}
