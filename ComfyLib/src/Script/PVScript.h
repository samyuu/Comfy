#pragma once
#include "Types.h"
#include "IO/Stream/FileInterfaces.h"
#include "Time/TimeSpan.h"
#include <memory_resource>
#include <map>

namespace Comfy
{
	enum class PVCommandType : u32
	{
		End,
		Time,
		MikuMove,
		MikuRot,
		MikuDisp,
		MikuShadow,
		Target,
		SetMotion,
		SetPlaydata,
		Effect,
		FadeinField,
		EffectOff,
		SetCamera,
		DataCamera,
		ChangeField,
		HideField,
		MoveField,
		FadeoutField,
		EyeAnim,
		MouthAnim,
		HandAnim,
		LookAnim,
		Expression,
		LookCamera,
		Lyric,
		MusicPlay,
		ModeSelect,
		EditMotion,
		BarTimeSet,
		ShadowHeight,
		EditFace,
		MoveCamera,
		PVEnd,
		ShadowPos,
		EditLyric,
		EditTarget,
		EditMouth,
		SetChara,
		EditMove,
		EditShadow,
		EditEyelid,
		EditEye,
		EditItem,
		EditEffect,
		EditDisp,
		EditAnim,
		Aim,
		HandItem,
		EditBlush,
		NearClip,
		ClothWet,
		LightRot,
		SceneFade,
		ToneTrans,
		Saturate,
		FadeMode,
		AutoBlink,
		PartsDisp,
		TargetFlyingTime,
		CharaSize,
		CharaHeightAdjust,
		ItemAnim,
		CharaPosAdjust,
		SceneRot,
		MotSmooth,
		PVBranchMode,
		DataCameraStart,
		MoviePlay,
		MovieDisp,
		Wind,
		OsageStep,
		OsageMVCLL,
		CharaColor,
		SEEffect,
		EditMoveXYZ,
		EditEyelidAnim,
		EditInstrumentItem,
		EditMotionLoop,
		EditExpression,
		EditEyeAnim,
		EditMouthAnim,
		EditEditCamera,
		EditModeSelect,
		PVEndFadeout,
		TargetFlag,
		ItemAnimAttach,
		ShadowRange,
		HandScale,
		LightPos,
		FaceType,
		ShadowCast,
		EditMotionF,
		Fog,
		Bloom,
		ColorColle,
		DOG,
		CharaAlpha,
		AutoCap,
		ManCap,
		Toon,
		Shimmer,
		ItemAlpha,
		MovieCutChg,
		CharaLight,
		StageLight,
		AgeAgeCtrl,
		PSE,
		Count,
	};

	struct PVCommandInfo
	{
		PVCommandType Type;
		u32 ParamCount;
		std::string_view Name;
	};

	constexpr std::array<PVCommandInfo, EnumCount<PVCommandType>()> PVCommandInfoTable =
	{
		PVCommandInfo
		{ PVCommandType::End, 0, "END", },
		{ PVCommandType::Time, 1, "TIME", },
		{ PVCommandType::MikuMove, 4, "MIKU_MOVE", },
		{ PVCommandType::MikuRot, 2, "MIKU_ROT", },
		{ PVCommandType::MikuDisp, 2, "MIKU_DISP", },
		{ PVCommandType::MikuShadow, 2, "MIKU_SHADOW", },
		{ PVCommandType::Target, 7, "TARGET", },
		{ PVCommandType::SetMotion, 4, "SET_MOTION", },
		{ PVCommandType::SetPlaydata, 2, "SET_PLAYDATA", },
		{ PVCommandType::Effect, 6, "EFFECT", },
		{ PVCommandType::FadeinField, 2, "FADEIN_FIELD", },
		{ PVCommandType::EffectOff, 1, "EFFECT_OFF", },
		{ PVCommandType::SetCamera, 6, "SET_CAMERA", },
		{ PVCommandType::DataCamera, 2, "DATA_CAMERA", },
		{ PVCommandType::ChangeField, 1, "CHANGE_FIELD", },
		{ PVCommandType::HideField, 1, "HIDE_FIELD", },
		{ PVCommandType::MoveField, 3, "MOVE_FIELD", },
		{ PVCommandType::FadeoutField, 2, "FADEOUT_FIELD", },
		{ PVCommandType::EyeAnim, 3, "EYE_ANIM", },
		{ PVCommandType::MouthAnim, 5, "MOUTH_ANIM", },
		{ PVCommandType::HandAnim, 5, "HAND_ANIM", },
		{ PVCommandType::LookAnim, 4, "LOOK_ANIM", },
		{ PVCommandType::Expression, 4, "EXPRESSION", },
		{ PVCommandType::LookCamera, 5, "LOOK_CAMERA", },
		{ PVCommandType::Lyric, 2, "LYRIC", },
		{ PVCommandType::MusicPlay, 0, "MUSIC_PLAY", },
		{ PVCommandType::ModeSelect, 2, "MODE_SELECT", },
		{ PVCommandType::EditMotion, 4, "EDIT_MOTION", },
		{ PVCommandType::BarTimeSet, 2, "BAR_TIME_SET", },
		{ PVCommandType::ShadowHeight, 2, "SHADOWHEIGHT", },
		{ PVCommandType::EditFace, 1, "EDIT_FACE", },
		{ PVCommandType::MoveCamera, 21, "MOVE_CAMERA", },
		{ PVCommandType::PVEnd, 0, "PV_END", },
		{ PVCommandType::ShadowPos, 3, "SHADOWPOS", },
		{ PVCommandType::EditLyric, 2, "EDIT_LYRIC", },
		{ PVCommandType::EditTarget, 5, "EDIT_TARGET", },
		{ PVCommandType::EditMouth, 1, "EDIT_MOUTH", },
		{ PVCommandType::SetChara, 1, "SET_CHARA", },
		{ PVCommandType::EditMove, 7, "EDIT_MOVE", },
		{ PVCommandType::EditShadow, 1, "EDIT_SHADOW", },
		{ PVCommandType::EditEyelid, 1, "EDIT_EYELID", },
		{ PVCommandType::EditEye, 2, "EDIT_EYE", },
		{ PVCommandType::EditItem, 1, "EDIT_ITEM", },
		{ PVCommandType::EditEffect, 2, "EDIT_EFFECT", },
		{ PVCommandType::EditDisp, 1, "EDIT_DISP", },
		{ PVCommandType::EditAnim, 2, "EDIT_HAND_ANIM", },
		{ PVCommandType::Aim, 3, "AIM", },
		{ PVCommandType::HandItem, 3, "HAND_ITEM", },
		{ PVCommandType::EditBlush, 1, "EDIT_BLUSH", },
		{ PVCommandType::NearClip, 2, "NEAR_CLIP", },
		{ PVCommandType::ClothWet, 2, "CLOTH_WET", },
		{ PVCommandType::LightRot, 3, "LIGHT_ROT", },
		{ PVCommandType::SceneFade, 6, "SCENE_FADE", },
		{ PVCommandType::ToneTrans, 6, "TONE_TRANS", },
		{ PVCommandType::Saturate, 1, "SATURATE", },
		{ PVCommandType::FadeMode, 1, "FADE_MODE", },
		{ PVCommandType::AutoBlink, 2, "AUTO_BLINK", },
		{ PVCommandType::PartsDisp, 3, "PARTS_DISP", },
		{ PVCommandType::TargetFlyingTime, 1, "TARGET_FLYING_TIME", },
		{ PVCommandType::CharaSize, 2, "CHARA_SIZE", },
		{ PVCommandType::CharaHeightAdjust, 2, "CHARA_HEIGHT_ADJUST", },
		{ PVCommandType::ItemAnim, 4, "ITEM_ANIM", },
		{ PVCommandType::CharaPosAdjust, 4, "CHARA_POS_ADJUST", },
		{ PVCommandType::SceneRot, 1, "SCENE_ROT", },
		{ PVCommandType::MotSmooth, 2, "MOT_SMOOTH", },
		{ PVCommandType::PVBranchMode, 1, "PV_BRANCH_MODE", },
		{ PVCommandType::DataCameraStart, 2, "DATA_CAMERA_START", },
		{ PVCommandType::MoviePlay, 1, "MOVIE_PLAY", },
		{ PVCommandType::MovieDisp, 1, "MOVIE_DISP", },
		{ PVCommandType::Wind, 3, "WIND", },
		{ PVCommandType::OsageStep, 3, "OSAGE_STEP", },
		{ PVCommandType::OsageMVCLL, 3, "OSAGE_MV_CCL", },
		{ PVCommandType::CharaColor, 2, "CHARA_COLOR", },
		{ PVCommandType::SEEffect, 1, "SE_EFFECT", },
		{ PVCommandType::EditMoveXYZ, 9, "EDIT_MOVE_XYZ", },
		{ PVCommandType::EditEyelidAnim, 3, "EDIT_EYELID_ANIM", },
		{ PVCommandType::EditInstrumentItem, 2, "EDIT_INSTRUMENT_ITEM", },
		{ PVCommandType::EditMotionLoop, 4, "EDIT_MOTION_LOOP", },
		{ PVCommandType::EditExpression, 2, "EDIT_EXPRESSION", },
		{ PVCommandType::EditEyeAnim, 3, "EDIT_EYE_ANIM", },
		{ PVCommandType::EditMouthAnim, 2, "EDIT_MOUTH_ANIM", },
		{ PVCommandType::EditEditCamera, 24, "EDIT_CAMERA", },
		{ PVCommandType::EditModeSelect, 1, "EDIT_MODE_SELECT", },
		{ PVCommandType::PVEndFadeout, 2, "PV_END_FADEOUT", },
		{ PVCommandType::TargetFlag, 1, "TARGET_FLAG", },
		{ PVCommandType::ItemAnimAttach, 3, "ITEM_ANIM_ATTACH", },
		{ PVCommandType::ShadowRange, 1, "SHADOW_RANGE", },
		{ PVCommandType::HandScale, 3, "HAND_SCALE", },
		{ PVCommandType::LightPos, 4, "LIGHT_POS", },
		{ PVCommandType::FaceType, 1, "FACE_TYPE", },
		{ PVCommandType::ShadowCast, 2, "SHADOW_CAST", },
		{ PVCommandType::EditMotionF, 6, "EDIT_MOTION_F", },
		{ PVCommandType::Fog, 3, "FOG", },
		{ PVCommandType::Bloom, 2, "BLOOM", },
		{ PVCommandType::ColorColle, 3, "COLOR_COLLE", },
		{ PVCommandType::DOG, 3, "DOF", },
		{ PVCommandType::CharaAlpha, 4, "CHARA_ALPHA", },
		{ PVCommandType::AutoCap, 1, "AOTO_CAP", },
		{ PVCommandType::ManCap, 1, "MAN_CAP", },
		{ PVCommandType::Toon, 3, "TOON", },
		{ PVCommandType::Shimmer, 3, "SHIMMER", },
		{ PVCommandType::ItemAlpha, 4, "ITEM_ALPHA", },
		{ PVCommandType::MovieCutChg, 2, "MOVIE_CUT_CHG", },
		{ PVCommandType::CharaLight, 3, "CHARA_LIGHT", },
		{ PVCommandType::StageLight, 3, "STAGE_LIGHT", },
		{ PVCommandType::AgeAgeCtrl, 8, "AGEAGE_CTRL", },
		{ PVCommandType::PSE, 2, "PSE", },
	};

	constexpr u32 GetPVCommandParamCount(PVCommandType type)
	{
		return (type >= PVCommandType::Count) ? 0 : PVCommandInfoTable[static_cast<u32>(type)].ParamCount;
	}

	constexpr std::string_view GetPVCommandName(PVCommandType type)
	{
		return (type >= PVCommandType::Count) ? "UNKNOWN" : PVCommandInfoTable[static_cast<u32>(type)].Name;
	}

	constexpr u32 MaxPVCommandParamCount = 24;

	struct PVCommand
	{
		PVCommandType Type;
		std::array<u32, MaxPVCommandParamCount> Param;

		PVCommand() = default;
		constexpr PVCommand(PVCommandType type) : Type(type), Param() {}

		constexpr u32 ParamCount() const { return GetPVCommandParamCount(Type); }
		constexpr std::string_view Name() const { return GetPVCommandName(Type); }

		template <typename Layout>
		Layout& View() { static_assert(sizeof(Layout) < sizeof(Param)); assert(Type == Layout::CommandType); return *reinterpret_cast<Layout*>(Param.data()); }
		template <typename Layout>
		const Layout& View() const { static_assert(sizeof(Layout) < sizeof(Param)); assert(Type == Layout::CommandType); return *reinterpret_cast<const Layout*>(Param.data()); }

		template <typename Layout>
		Layout* TryView() { return (Type == Layout::CommandType) ? &View<Layout>() : nullptr; }
		template <typename Layout>
		const Layout* TryView() const { return (Type == Layout::CommandType) ? &View<const Layout>() : nullptr; }
	};

	namespace PVCommandLayout
	{
		template <typename Dervied, PVCommandType T>
		struct LayoutBase
		{
			static constexpr PVCommandType CommandType = T;

			constexpr operator PVCommand() const
			{
				static_assert(std::is_standard_layout_v<Dervied>);
				if constexpr (std::is_empty_v<Dervied>)
					static_assert(GetPVCommandParamCount(T) == 0, "Unexpected Layout Size");
				else
					static_assert(sizeof(Dervied) == (sizeof(u32) * GetPVCommandParamCount(T)), "Unexpected Layout Size");

				PVCommand out = { CommandType };
				std::memcpy(out.Param.data(), this, sizeof(Dervied));
				return out;
			}
		};

		struct End : LayoutBase<End, PVCommandType::End> {};

		struct Time : LayoutBase<Time, PVCommandType::Time>
		{
			i32 TimePoint;

			Time() = default;
			constexpr Time(TimeSpan time) : TimePoint(static_cast<i32>(time.TotalSeconds() * 100000.0)) {}
			constexpr operator TimeSpan() const { return TimeSpan::FromSeconds(static_cast<f64>(TimePoint) / 100000.0); }
			constexpr bool operator<(PVCommandLayout::Time other) const { return TimePoint < other.TimePoint; }
		};

		struct MikuMove : LayoutBase<MikuMove, PVCommandType::MikuMove>
		{
			i32 CharaIndex;
			i32 X, Y, Z;

			MikuMove() = default;
			constexpr MikuMove(i32 index, vec3 position) : CharaIndex(index),
				X(static_cast<i32>(position.x * 1000.0f)), Y(static_cast<i32>(position.y * 1000.0f)), Z(static_cast<i32>(position.z * 1000.0f))
			{
			}
		};

		struct MikuDisp : LayoutBase<MikuDisp, PVCommandType::MikuDisp>
		{
			i32 CharaIndex;
			i32 Visible;

			MikuDisp() = default;
			constexpr MikuDisp(i32 index, bool visible) : CharaIndex(index), Visible(visible) {}
		};

		enum class TargetType : i32
		{
			Triangle = 0,
			Circle = 1,
			Cross = 2,
			Square = 3,

			TriangleHold = 4,
			CircleHold = 5,
			CrossHold = 6,
			SquareHold = 7,

			Random = 8,
			RandomHold = 9,
			RandomRepeat = 10,

			SlideBoth = 11,
			SlideL = 12,
			SlideR = 13,
			// SlideCrashL = 14,

			SlideChainL = 15,
			SlideChainR = 16,
			// GreenSquareL = 17,

			TriangleChance = 18,
			CircleChance = 19,
			CrossChance = 20,
			SquareChance = 21,

			SlideBothChance = 22,
			SlideLChance = 23,
			SlideRChance = 24,
		};

		struct Target : LayoutBase<Target, PVCommandType::Target>
		{
			TargetType Type;
			i32 PositionX;
			i32 PositionY;
			i32 Angle;
			i32 Distance;
			i32 Amplitude;
			i32 Frequency;
		};

		struct ChangeField : LayoutBase<ChangeField, PVCommandType::ChangeField>
		{
			i32 FieldID;

			ChangeField() = default;
			constexpr ChangeField(i32 id) : FieldID(id) {}
		};

		struct MusicPlay : LayoutBase<MusicPlay, PVCommandType::MusicPlay> {};

		struct ModeSelect : LayoutBase<ModeSelect, PVCommandType::ModeSelect>
		{
			u32 DifficultyFlags;
			i32 ModeType;
		};

		struct BarTimeSet : LayoutBase<BarTimeSet, PVCommandType::BarTimeSet>
		{
			i32 BeatsPerMinute;
			i32 TimeSignature;

			BarTimeSet() = default;
			constexpr operator TimeSpan() const { return TimeSpan::FromSeconds(static_cast<f64>((60 * (TimeSignature + 1))) / static_cast<f64>(BeatsPerMinute)); }
		};

		struct PVEnd : LayoutBase<PVEnd, PVCommandType::PVEnd> {};

		struct SceneFade : LayoutBase<SceneFade, PVCommandType::SceneFade>
		{
			i32 Transition;
			i32 StartAlpha;
			i32 EndAlpha;
			i32 Red;
			i32 Green;
			i32 Blue;

			SceneFade() = default;
			constexpr SceneFade(TimeSpan transition, f32 startAlpha, f32 endAlpha, vec3 color) :
				Transition(static_cast<i32>(transition.TotalMilliseconds())),
				StartAlpha(static_cast<i32>(startAlpha * 1000.0f)),
				EndAlpha(static_cast<i32>(endAlpha * 1000.0f)),
				Red(static_cast<i32>(color.r * 1000.0f)),
				Green(static_cast<i32>(color.g * 1000.0f)),
				Blue(static_cast<i32>(color.b * 1000.0f))
			{
			}
		};

		struct TargetFlyingTime : LayoutBase<TargetFlyingTime, PVCommandType::TargetFlyingTime>
		{
			i32 DurationMS;

			TargetFlyingTime() = default;
			TargetFlyingTime(i32 timeMS) : DurationMS(timeMS) {}
			TargetFlyingTime(TimeSpan time) : DurationMS(static_cast<i32>(glm::round(time.TotalMilliseconds()))) {}
			constexpr operator TimeSpan() const { return TimeSpan::FromMilliseconds(static_cast<f64>(DurationMS)); }
		};

		enum class PVBranchModeType : i32
		{
			Default = 0,
			Failure = 1,
			Success = 2,
		};

		struct PVBranchMode : LayoutBase<PVBranchMode, PVCommandType::PVBranchMode>
		{
			PVBranchModeType Mode;
		};

		struct MoviePlay : LayoutBase<MoviePlay, PVCommandType::MoviePlay>
		{
			i32 ID;

			MoviePlay() = default;
			MoviePlay(i32 id) : ID(id) {}
		};

		struct MovieDisp : LayoutBase<MovieDisp, PVCommandType::MovieDisp>
		{
			i32 Visible;

			MovieDisp() = default;
			MovieDisp(bool visible) : Visible(visible) {}
		};
	}

	enum class PVScriptVersion : u32
	{
		Current = 0x14050921,
	};

	class PVScript : public IO::IBufferParsable, public IO::IStreamWritable
	{
	public:
		static constexpr std::string_view Extension = ".dsc";
		static constexpr std::string_view FilterName = "Project DIVA Future Tone PV Script";
		static constexpr std::string_view FilterSpec = "*.dsc";

	public:
		PVScriptVersion Version = PVScriptVersion::Current;
		std::vector<PVCommand> Commands;

	public:
		void Parse(const u8* buffer, size_t bufferSize) override;
		IO::StreamResult Write(IO::StreamWriter& writer) override;

	public:
		std::string ToString() const;
		void AppendToString(std::string& outString) const;
	};

	class PVScriptBuilder
	{
	public:
		void Add(TimeSpan time, const PVCommand& command, i32 branch = 0);
		std::vector<PVCommand> Create();

	private:
		std::pmr::monotonic_buffer_resource bufferResource {};
		std::pmr::map<PVCommandLayout::Time, std::pmr::vector<PVCommand>> timedCommandGroups { &bufferResource };
	};
}
