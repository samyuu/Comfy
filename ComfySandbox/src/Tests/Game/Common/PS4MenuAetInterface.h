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

		SongListItem_Selected = SongListItem_05,
	};

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
