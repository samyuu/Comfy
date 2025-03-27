#pragma once
#include "Database/Database.h"
#include "Script/PVScript.h"
#include "Time/TimeSpan.h"

namespace Comfy::Database
{
	enum class PVDifficultyType : i32
	{
		Easy,
		Normal,
		Hard,
		Extreme,
		Encore,
		Count
	};

	enum class PVDifficultyLevel
	{
		Level_00_0, Level_00_5,
		Level_01_0, Level_01_5,
		Level_02_0, Level_02_5,
		Level_03_0, Level_03_5,
		Level_04_0, Level_04_5,
		Level_05_0, Level_05_5,
		Level_06_0, Level_06_5,
		Level_07_0, Level_07_5,
		Level_08_0, Level_08_5,
		Level_09_0, Level_09_5,
		Level_10_0,
		Count,
	};

	constexpr std::array<const char*, EnumCount<PVDifficultyLevel>()> PVDifficultyLevelNames =
	{
		"PV_LV_00_0", "PV_LV_00_5",
		"PV_LV_01_0", "PV_LV_01_5",
		"PV_LV_02_0", "PV_LV_02_5",
		"PV_LV_03_0", "PV_LV_03_5",
		"PV_LV_04_0", "PV_LV_04_5",
		"PV_LV_05_0", "PV_LV_05_5",
		"PV_LV_06_0", "PV_LV_06_5",
		"PV_LV_07_0", "PV_LV_07_5",
		"PV_LV_08_0", "PV_LV_08_5",
		"PV_LV_09_0", "PV_LV_09_5",
		"PV_LV_10_0",
	};

	enum class PVDifficultyEdition : i32
	{
		Normal,
		Extra,
		Count
	};

	struct PvDBAetSceneLayerEntry
	{
		std::string Layer;
		std::string Scene;
	};

	struct PvDBFieldData
	{
		std::string Stage;
		std::string ExStage;

		std::string Auth3D;
		std::vector<std::string> Auth3DList;
		std::string ExAuth3D;
		std::vector<std::string> ExAuth3DList;
		std::vector<i32> Auth3DFrameList;
		std::string Auth3DLight;
		i32 Auth3DLightFrame;

		PvDBAetSceneLayerEntry AetFront, AetFrontLow, AetFront3DSurf, AetBack;
		std::vector<PvDBAetSceneLayerEntry> AetFrontList, AetFrontLowList, AetFront3DSurfList, AetBackList;

		std::string SprSetBack;

		i32 StageFlag;
		i32 NprType;
		i32 CamBlur;
		i32 ShadowOff;

		std::vector<std::string> ParticlePlayEffectList;
		std::vector<std::string> ParticleStopEffectList;
		std::vector<std::string> ParticleRestartEffectList;
		std::vector<TimeSpan> ParticleEmissionRestartEffectList;
	};

	struct PvDBTitleImageEntry
	{
		TimeSpan StartTime;
		TimeSpan PlayTime;
		std::string AetLayerName;
	};

	struct PvDBSongInfoEntry
	{
		std::string Music;
		std::string Lyrics;
		std::string Arranger;
		std::string Manipulator;
		std::string PVEditor;
		std::string GuitarPlayer;
		struct ExtraInfoData { std::string Key, Value; };
		std::array<ExtraInfoData, 4> ExtraInfo;

		std::string MovieFileName;
		std::vector<std::string> MovieFileNameList;

		i32 MovieSurfaceType;

		std::string SfxArchiveFileName;
		std::vector<std::string> SfxNames;

		f32 HighSpeedRate;
		f32 HiddenTiming;
		f32 SuddenTiming;

		i32 EditCharaScale;

		// ... performer ... pv_costume
	};

	// NOTE: Either either globally per slot or refined per difficulty
	struct PvDBCommonEntry
	{
		std::string ButtonSfx;
		std::string SlideSfx;
		std::string ChainSlideSfxFirst;
		std::string ChainSlideSfxSub;
		std::string ChainSlideSfxSuccess;
		std::string ChainSlideSfxFailure;
		std::string SliderTouchSfx;

		// std::array<std::array<std::string, 100>, 6> PerPlayerMotions;
		std::array<std::vector<std::string>, 6> PerPlayerMotions;

		// std::array<std::string, 10> PVItems;
		std::vector<std::string> PVItems;
		// std::array<std::string, 20> HandItems;
		std::vector<std::string> HandItems;
		// std::array<i32, 512> EditEffects;
		std::vector<i32> EditEffects;
		// std::array<i32, 512> EditEffectLowFields;
		std::vector<i32> EditEffectLowFields;

		PvDBTitleImageEntry TitleImage;
		PvDBSongInfoEntry SongInfo;

		std::vector<PvDBFieldData> Fields;
	};

	struct PvDBDifficultyAttributes
	{
		bool Extra;
		bool Original;
		bool Slide;
	};

	struct PvDBDifficultyEntry
	{
		std::string ScriptFileName;
		PVScriptVersion ScriptFormat;

		PVDifficultyEdition Edition;
		PvDBDifficultyAttributes Attribute;
		PVDifficultyLevel Level;
		i32 LevelSortIndex;

		PvDBCommonEntry Common;
	};

	struct PvDBMDataEntry
	{
		u32 Flag;
		std::string Directory;
	};

	struct PvDBDifficultyEditionsEntry
	{
		std::array<PvDBDifficultyEntry, EnumCount<PVDifficultyEdition>()> Editions;
		i32 Count;
		PvDBDifficultyAttributes Attribute;
	};

	struct PvDBSongHighPointEntry
	{
		TimeSpan StartTime;
		TimeSpan PlayTime;
	};

	struct PvDBPerformerEntry
	{
		i32 Type;
		i32 Chara;
		i32 Costume;
		i32 PVCostume;
		i32 Size;
		i32 BackItem, FaceItem, NeckItem, OverheadItem;
		bool Fixed;
		bool PseudoSameID;
		bool Exclude;
	};

	struct PvDBCharaAuthReplacementEntry
	{
		i32 Chara;
		i32 ID;
		std::string Name;
		std::string OriginalName;
	};

	struct PvDBCharaEffectReplacementEntry
	{
		struct ReplacementData
		{
			std::string Name;
			i32 Type;
		};

		i32 ID;
		std::string Name;
		std::vector<ReplacementData> Data;
	};

	struct PvDBCharaSongReplacementEntry
	{
		// i32 Chara;
		std::array<i32, 6> PerPlayerChara;
		std::string SongName;
		std::string SongFileName;

		struct ExAuthData
		{
			std::string Name;
			std::string OriginalName;
		};

		std::vector<ExAuthData> ExAuth;
	};

	struct PvDBOsageInitEntry
	{
		i32 Frame;
		std::string Motion;
		std::string Stage;
	};

	struct PvDBStageParamEntry
	{
		i32 MotionHeadID;
		std::string Stage;
		std::string CollisionFileName;
		std::string WindFileName;
	};

	struct PvDBDisp2DEntry
	{
		i32 SetName;
		i32 TargetShadowType;
		i32 TitleStart2DField, TitleEnd2DField;
		i32 TitleStart2DLowField, TitleEnd2DLowField;
		i32 TitleStart3DField, TitleEnd3DField;
		std::string Title2DLayer;
	};

	struct PvDBSongVocalReplacementEntry
	{
		std::string SongName;
		std::string SongFileName;
	};

	struct PvDBAuthPerModuleReplacementEntry
	{
		i32 ID;
		i32 ModuleID;
		std::string Name;
		std::string OriginalName;
	};

	struct PvDBEntry
	{
		i32 ID;
		i32 BPM;

		std::string SongName;
		std::string SongNameReading;
		std::string SongFileName;
		DateEntry Date;

		PvDBMDataEntry MData;
		std::array<PvDBDifficultyEditionsEntry, EnumCount<PVDifficultyType>()> Difficulties;

		// std::array<std::string, 1000> Lyrics;
		std::vector<std::string> Lyrics;

		PvDBSongHighPointEntry HighPoint;

		i32 EditType;
		bool IsOldPV;
		bool DisableCalcMotFrameLimit;
		bool EyesXRotationAdjust;
		// bool UseOsagePlayData;
		i32 EyesBaseAdjustType;
		// std::vector<...> EyesRotationRates;

		PvDBCommonEntry Common;

		std::vector<PvDBPerformerEntry> Performers;
		std::vector<PvDBCharaAuthReplacementEntry> CharaCameraReplacements;
		std::vector<PvDBCharaAuthReplacementEntry> CharaMotionReplacements;
		std::vector<PvDBCharaEffectReplacementEntry> CharaEffectReplacements;
		std::vector<PvDBCharaSongReplacementEntry> ExtraSong;
		std::vector<PvDBOsageInitEntry> OsageInit;
		std::vector<PvDBStageParamEntry> StageParam;
		PvDBDisp2DEntry Disp2D;
		std::string PVExpressionFileName;
		// std::vector<FrameTexture> ... frame_texture, frame_texture_a ... frame_texture_e, frame_texture_type (?)
		std::vector<PvDBSongVocalReplacementEntry> AnotherSong;
		bool PrePlayScript;
		std::vector<PvDBAuthPerModuleReplacementEntry> AuthReplaceByModule;
	};

	class PvDB final : public TextDatabase, public IO::IStreamWritable
	{
	public:
		std::vector<std::unique_ptr<PvDBEntry>> Entries;

	public:
		void Parse(const u8* buffer, size_t bufferSize) override;
		IO::StreamResult Write(IO::StreamWriter& writer) override;

	private:
	};
}
