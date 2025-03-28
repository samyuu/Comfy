#pragma once
#include "Types.h"
#include "SortedTargetList.h"
#include "SortedTempoMap.h"
#include "Editor/Common/AsyncLoadedImageFile.h"
#include "Time/TimeSpan.h"

namespace Comfy::Studio::Editor
{
	enum class Difficulty : u8
	{
		Easy,
		Normal,
		Hard,
		Extreme,
		ExExtreme,
		Count,
	};

	constexpr std::array<const char*, EnumCount<Difficulty>()> DifficultyNames =
	{
		"Easy",
		"Normal",
		"Hard",
		"Extreme",
		"Extra Extreme",
	};

	enum class DifficultyLevel : u8
	{
		Star_00_0, Star_00_5,
		Star_01_0, Star_01_5,
		Star_02_0, Star_02_5,
		Star_03_0, Star_03_5,
		Star_04_0, Star_04_5,
		Star_05_0, Star_05_5,
		Star_06_0, Star_06_5,
		Star_07_0, Star_07_5,
		Star_08_0, Star_08_5,
		Star_09_0, Star_09_5,
		Star_10_0,
		Count,
		StarMin = Star_00_5,
		StarMax = Star_10_0,
	};

	constexpr std::array<const char*, EnumCount<DifficultyLevel>()> DifficultyLevelNames =
	{
		"0.0 Stars", "0.5 Stars",
		"1.0 Stars", "1.5 Stars",
		"2.0 Stars", "2.5 Stars",
		"3.0 Stars", "3.5 Stars",
		"4.0 Stars", "4.5 Stars",
		"5.0 Stars", "5.5 Stars",
		"6.0 Stars", "6.5 Stars",
		"7.0 Stars", "7.5 Stars",
		"8.0 Stars", "8.5 Stars",
		"9.0 Stars", "9.5 Stars",
		"10.0 Stars",
	};

	class Chart
	{
	public:
		// NOTE: In case there is no audio file to take as a reference
		static constexpr auto FallbackDuration = TimeSpan::FromMinutes(1.5);
		static constexpr auto FallbackSongTitle = std::string_view { u8"�_�~�[" };

	public:
		// NOTE: An empty path means the chart hasn't yet been saved to nor has been loaded from disk
		std::string ChartFilePath;

		// NOTE: Should (but doesn't have to) be relative to the chart file directory
		std::string SongFileName;
		std::string MovieFileName;

		struct PropertyData
		{
			struct SongInfo
			{
				std::string Title;
				std::string TitleReading;

				std::string Artist;
				std::string Album;

				std::string Lyricist;
				std::string Arranger;

				i32 TrackNumber;
				i32 DiskNumber;

				struct KeyValue { std::string Key, Value; };
				std::array<KeyValue, 4> ExtraInfo;
			} Song;

			struct ImageData
			{
				std::string CoverFileName;
				std::string LogoFileName;
				std::string BackgroundFileName;

				AsyncLoadedImageFile Cover = { AsyncImageFileFlags_FlipY | AsyncImageFileFlags_TransparentBorder | AsyncImageFileFlags_TransparentBorderNoSprAdjust };
				AsyncLoadedImageFile Logo = { AsyncImageFileFlags_FlipY };
				AsyncLoadedImageFile Background = { AsyncImageFileFlags_FlipY };
			} Image;

			struct CreatorInfo
			{
				std::string Name;
				std::string Comment;
			} Creator;

			struct ButtonSoundData
			{
				u32 ButtonID;
				u32 SlideID;
				u32 ChainSlideID;
				u32 SliderTouchID;
			} ButtonSound;

			struct DifficultyInfo
			{
				Difficulty Type = Difficulty::Hard;
				DifficultyLevel Level = DifficultyLevel::Star_07_5;
			} Difficulty;

			struct SongPreviewData
			{
				TimeSpan StartTime;
				TimeSpan Duration;
			} SongPreview;

		} Properties = {};

		TimeSpan SongOffset = TimeSpan::Zero();
		TimeSpan MovieOffset = TimeSpan::Zero();
		TimeSpan Duration = TimeSpan::Zero();

		SortedTargetList Targets;
		SortedTempoMap TempoMap;

	public:
		inline std::string_view SongTitleOrDefault() const { return Properties.Song.Title.empty() ? FallbackSongTitle : Properties.Song.Title; }
		inline TimeSpan DurationOrDefault() const { return (Duration <= TimeSpan::Zero()) ? FallbackDuration : Duration; }
	};
}
