#pragma once
#include "Tests/TestTask.h"

namespace Comfy::Sandbox::Tests::Game
{
	enum MenuSubState
	{
		MenuSubState_Game,
		MenuSubState_Custom,
		MenuSubState_PV,
		MenuSubState_MyRoom,
		MenuSubState_Store,
		MenuSubState_Options,
		MenuSubState_Count
	};

	static constexpr std::array<std::string_view, MenuSubState_Count> MainMenuPlateLayerNames
	{
		"menu_plate_game",
		"menu_plate_custom",
		"menu_plate_pv",
		"menu_plate_myroom",
		"menu_plate_store",
		"menu_plate_option",
	};

	static constexpr std::array<std::string_view, MenuSubState_Count> MainMenuPlateSelectLayerNames
	{
		"menu_plate_game_sel_t",
		"menu_plate_custom_sel_t",
		"menu_plate_pv_sel_t",
		"menu_plate_myroom_sel_t",
		"menu_plate_store_sel_t",
		"menu_plate_option_sel_t",
	};

	static constexpr std::array<std::string_view, MenuSubState_Count> MainMenuTextLayerNames
	{
		"menu_txt_game",
		"menu_txt_custom",
		"menu_txt_pv",
		"menu_txt_myroom",
		"menu_txt_store",
		"menu_txt_option",
	};

	static constexpr std::array<std::string_view, MenuSubState_Count> MainMenuCharaLayerNames
	{
		"menu_chara_game",
		"menu_chara_custom",
		"menu_chara_pv",
		"menu_chara_myroom",
		"menu_chara_store",
		"menu_chara_option",
	};

	static constexpr std::array<std::string_view, MenuSubState_Count> MainMenuItemPointLayerNames
	{
		"p_menu_list01_c",
		"p_menu_list02_c",
		"p_menu_list03_c",
		"p_menu_list04_c",
		"p_menu_list05_c",
		"p_menu_list06_c",
	};

	enum SongListDifficulty
	{
		SongListDifficulty_Easy,
		SongListDifficulty_Normal,
		SongListDifficulty_Hard,
		SongListDifficulty_Extreme,
		SongListDifficulty_ExtraExtreme,
		SongListDifficulty_Count
	};

	enum SongListItem
	{
		SongListItem_00,
		SongListItem_01,
		SongListItem_02,
		SongListItem_03,
		SongListItem_04,
		SongListItem_05,
		SongListItem_06,
		SongListItem_07,
		SongListItem_08,
		SongListItem_09,
		SongListItem_10,
		SongListItem_11,
		SongListItem_12,
		SongListItem_Count,

		SongListItem_Center = SongListItem_05,
	};

	namespace
	{
		static constexpr std::array<std::string_view, SongListItem_Count> SongListItemIconSlideLayerNames =
		{
			"p_icon_slide00_c",
			"p_icon_slide01_c",
			"p_icon_slide02_c",
			"p_icon_slide03_c",
			"p_icon_slide04_c",
			"p_icon_slide05_c",
			"p_icon_slide06_c",
			"p_icon_slide07_c",
			"p_icon_slide08_c",
			"p_icon_slide09_c",
			"p_icon_slide10_c",
			"p_icon_slide11_c",
			"p_icon_slide12_c",
		};

		static constexpr std::array<std::string_view, SongListItem_Count> SongListItemAchieveLayerNames =
		{
			"p_achieve_00",
			"p_achieve_01",
			"p_achieve_02",
			"p_achieve_03",
			"p_achieve_04",
			"p_achieve_05",
			"p_achieve_06",
			"p_achieve_07",
			"p_achieve_08",
			"p_achieve_09",
			"p_achieve_10",
			"p_achieve_11",
			"p_achieve_12",
		};

		static constexpr std::array<std::string_view, SongListItem_Count> SongListItemIconDuelLayerNames =
		{
			"p_icon_duet00_c",
			"p_icon_duet01_c",
			"p_icon_duet02_c",
			"p_icon_duet03_c",
			"p_icon_duet04_c",
			"p_icon_duet05_c",
			"p_icon_duet06_c",
			"p_icon_duet07_c",
			"p_icon_duet08_c",
			"p_icon_duet09_c",
			"p_icon_duet10_c",
			"p_icon_duet11_c",
			"p_icon_duet12_c",
		};

		static constexpr std::array<std::string_view, SongListItem_Count> SongListItemSongNameLayerNames =
		{
			"p_song_name00_lt",
			"p_song_name01_lt",
			"p_song_name02_lt",
			"p_song_name03_lt",
			"p_song_name04_lt",
			"p_song_name05_lt",
			"p_song_name06_lt",
			"p_song_name07_lt",
			"p_song_name08_lt",
			"p_song_name09_lt",
			"p_song_name10_lt",
			"p_song_name11_lt",
			"p_song_name12_lt",
		};

		static constexpr std::array<std::string_view, SongListItem_Count> SongListItemSongClearLayerNames =
		{
			"p_song_clear00_c",
			"p_song_clear01_c",
			"p_song_clear02_c",
			"p_song_clear03_c",
			"p_song_clear04_c",
			"p_song_clear05_c",
			"p_song_clear06_c",
			"p_song_clear07_c",
			"p_song_clear08_c",
			"p_song_clear09_c",
			"p_song_clear10_c",
			"p_song_clear11_c",
			"p_song_clear12_c",
		};

		static constexpr std::array<std::string_view, SongListItem_Count> SongListItemSongLevelLayerNames =
		{
			"p_song_level00_c",
			"p_song_level01_c",
			"p_song_level02_c",
			"p_song_level03_c",
			"p_song_level04_c",
			"p_song_level05_c",
			"p_song_level06_c",
			"p_song_level07_c",
			"p_song_level08_c",
			"p_song_level09_c",
			"p_song_level10_c",
			"p_song_level11_c",
			"p_song_level12_c",
		};

		static constexpr std::array<std::string_view, SongListItem_Count> SongListItemIconNewLayerNames =
		{
			"p_icon_new00_c",
			"p_icon_new01_c",
			"p_icon_new02_c",
			"p_icon_new03_c",
			"p_icon_new04_c",
			"p_icon_new05_c",
			"p_icon_new06_c",
			"p_icon_new07_c",
			"p_icon_new08_c",
			"p_icon_new09_c",
			"p_icon_new10_c",
			"p_icon_new11_c",
			"p_icon_new12_c",
		};

		static constexpr std::array<std::string_view, SongListItem_Count> SongListItemIconFavoriteLayerNames =
		{
			"p_icon_fav00_c",
			"p_icon_fav01_c",
			"p_icon_fav02_c",
			"p_icon_fav03_c",
			"p_icon_fav04_c",
			"p_icon_fav05_c",
			"p_icon_fav06_c",
			"p_icon_fav07_c",
			"p_icon_fav08_c",
			"p_icon_fav09_c",
			"p_icon_fav10_c",
			"p_icon_fav11_c",
			"p_icon_fav12_c",
		};

		static constexpr std::array<std::string_view, SongListItem_Count> SongListItemOptionLayerNames =
		{
			"p_option_00",
			"p_option_01",
			"p_option_02",
			"p_option_03",
			"p_option_04",
			"p_option_05",
			"p_option_06",
			"p_option_07",
			"p_option_08",
			"p_option_09",
			"p_option_10",
			"p_option_11",
			"p_option_12",
		};

		static constexpr std::array<std::string_view, SongListItem_Count> SongListItemIconPackLayerNames =
		{
			"p_icon_pack00_c",
			"p_icon_pack01_c",
			"p_icon_pack02_c",
			"p_icon_pack03_c",
			"p_icon_pack04_c",
			"p_icon_pack05_c",
			"p_icon_pack06_c",
			"p_icon_pack07_c",
			"p_icon_pack08_c",
			"p_icon_pack09_c",
			"p_icon_pack10_c",
			"p_icon_pack11_c",
			"p_icon_pack12_c",
		};

		static constexpr std::array<std::string_view, SongListItem_Count> SongListItemBaseLayerNames =
		{
			"p_song_list_base00_c",
			"p_song_list_base01_c",
			"p_song_list_base02_c",
			"p_song_list_base03_c",
			"p_song_list_base04_c",
			"p_song_list_base05_c",
			"p_song_list_base06_c",
			"p_song_list_base07_c",
			"p_song_list_base08_c",
			"p_song_list_base09_c",
			"p_song_list_base10_c",
			"p_song_list_base11_c",
			"p_song_list_base12_c",
		};
	}

	enum SongListState
	{
		SongListState_Appear,
		SongListState_ScrollUp,
		SongListState_ScrollDown,
		SongListState_Still,
		SongListState_Count
	};

	static constexpr std::array<std::string_view, SongListDifficulty_Count + 1> SongListBaseLayerNames =
	{
		"song_list_base_easy",
		"song_list_base_normal",
		"song_list_base_hard",
		"song_list_base_extreme",
		"song_list_base_extra_extreme",
		"song_list_base_blank",
	};

	static constexpr std::array<std::string_view, SongListDifficulty_Count + 1> SongListBaseSelectedLayerNames =
	{
		"song_list_base_easy_sel",
		"song_list_base_normal_sel",
		"song_list_base_hard_sel",
		"song_list_base_extreme_sel",
		"song_list_base_extra_extreme_sel",
		"song_list_base_blank",
	};

	enum LevelStar
	{
		LevelStar_00,
		LevelStar_00_5,
		LevelStar_01,
		LevelStar_01_5,
		LevelStar_02,
		LevelStar_02_5,
		LevelStar_03,
		LevelStar_03_5,
		LevelStar_04,
		LevelStar_04_5,
		LevelStar_05,
		LevelStar_05_5,
		LevelStar_06,
		LevelStar_06_5,
		LevelStar_07,
		LevelStar_07_5,
		LevelStar_08,
		LevelStar_08_5,
		LevelStar_09,
		LevelStar_09_5,
		LevelStar_10,
		LevelStar_Count
	};

	static constexpr std::array<std::string_view, LevelStar_Count> LevelStarLayerName
	{
		"level_star_00",
		"level_star_00_5",
		"level_star_01",
		"level_star_01_5",
		"level_star_02",
		"level_star_02_5",
		"level_star_03",
		"level_star_03_5",
		"level_star_04",
		"level_star_04_5",
		"level_star_05",
		"level_star_05_5",
		"level_star_06",
		"level_star_06_5",
		"level_star_07",
		"level_star_07_5",
		"level_star_08",
		"level_star_08_5",
		"level_star_09",
		"level_star_09_5",
		"level_star_10",
	};
}
