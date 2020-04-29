#pragma once
#include "Types.h"
#include "Task.h"
#include "Graphics/Auth2D/Aet/AetMgr.h"
#include "Graphics/Auth2D/Aet/AetSet.h"
#include "Graphics/Auth2D/SprSet.h"
#include "IO/FileLoader.h"

namespace Comfy::App
{
	using namespace Graphics;

	struct AetLayerSourceData
	{
		const Aet::Layer* Layer;
		const char* Name;

		inline const Aet::Layer* operator->() const { return Layer; };
		inline operator const Aet::Layer*() { return Layer; };
	};

	struct Ps4MenuAetData
	{
#define SourceDataField(fieldName, layerName) AetLayerSourceData fieldName = { nullptr, layerName }
		SourceDataField(CommonBackground, "cmn_bg01_f__f");
		SourceDataField(MenuDeco, "menu_deco_f__f");
		SourceDataField(MenuListIn02, "menu_list_in02");

		SourceDataField(MenuHeader, "menu_header_ft");
		SourceDataField(MenuFooter, "menu_footer");

		SourceDataField(MenuTextGame, "menu_txt_game");
		SourceDataField(MenuTextCustom, "menu_txt_custom");
		SourceDataField(MenuTextPv, "menu_txt_pv");
		SourceDataField(MenuTextMyRoom, "menu_txt_myroom");
		SourceDataField(MenuTextStore, "menu_txt_store");
		SourceDataField(MenuTextOption, "menu_txt_option");

		SourceDataField(MenuCharaGame, "menu_chara_game");
		SourceDataField(MenuCharaCustom, "menu_chara_custom");
		SourceDataField(MenuCharaPv, "menu_chara_pv");
		SourceDataField(MenuCharaMyRoom, "menu_chara_myroom");
		SourceDataField(MenuCharaStore, "menu_chara_store");
		SourceDataField(MenuCharaOption, "menu_chara_option");

		SourceDataField(MenuPlateGame, "menu_plate_game");
		SourceDataField(MenuPlateCustom, "menu_plate_custom");
		SourceDataField(MenuPlatePv, "menu_plate_pv");
		SourceDataField(MenuPlateMyRoom, "menu_plate_myroom");
		SourceDataField(MenuPlateStore, "menu_plate_store");
		SourceDataField(MenuPlateOption, "menu_plate_option");

		SourceDataField(MenuPlateGameSel, "menu_plate_game_sel_f");
		SourceDataField(MenuPlateCustomSel, "menu_plate_custom_sel_f");
		SourceDataField(MenuPlatePvSel, "menu_plate_pv_sel_f");
		SourceDataField(MenuPlateMyRoomSel, "menu_plate_myroom_sel_f");
		SourceDataField(MenuPlateStoreSel, "menu_plate_store_sel_f");
		SourceDataField(MenuPlateOptionSel, "menu_plate_option_sel_f");

		SourceDataField(p_MenuList01_c, "p_menu_list01_c");
		SourceDataField(p_MenuList02_c, "p_menu_list02_c");
		SourceDataField(p_MenuList03_c, "p_menu_list03_c");
		SourceDataField(p_MenuList04_c, "p_menu_list04_c");
		SourceDataField(p_MenuList05_c, "p_menu_list05_c");
		SourceDataField(p_MenuList06_c, "p_menu_list06_c");

		// SourceDataField(Dummy, "");
#undef SourceDataField

		void Initialize(const Aet::AetSet* aetSet);
	};

	enum class TaskPs4MenuStateType
	{
		MainMenuInitialize,
		MainMenuIn,
		MainMenuLoop,
	};

	using MainMenuItem = i32;
	enum MainMenuItem_Enum : MainMenuItem
	{
		MainMenuItem_Game,
		MainMenuItem_Custom,
		MainMenuItem_Pv,
		MainMenuItem_MyRoom,
		MainMenuItem_Store,
		MainMenuItem_Option,
		MainMenuItem_Count,
	};

	struct MainMenuState
	{
		float Frames;
		float FramesSinceItemSwitch;
		MainMenuItem selectedItem;
	};

	class TaskPs4Menu : public Task
	{
	public:
		bool Initialize() override;
		bool Update() override;
		bool Render(GPU_Renderer2D* renderer, Aet::AetRenderer* aetRenderer) override;
		bool PostDrawGui() override;

	protected:
		Aet::SpriteGetterFunction spriteGetterFunction;

		IO::FileLoader aetSetLoader = { "dev_rom/2d/aet_ps4_menu.bin" };
		IO::FileLoader sprSetLoader = { "dev_rom/2d/spr_ps4_menu.bin" };

		UniquePtr<Aet::AetSet> aetSet;
		UniquePtr<SprSet> sprSet;

		float elapsedFrames;
		Ps4MenuAetData aetData;
		TaskPs4MenuStateType stateType;
		MainMenuState mainMenuState;

	private:
		bool isLoading = true;

		void RenderMenuBackground(Aet::AetRenderer* aetRenderer, float frame);
		void RenderMainMenuChara(Aet::AetRenderer* aetRenderer, float frame);
		void RenderMainMenuList(Aet::AetRenderer* aetRenderer, bool selectedComp, float menuListFrame, float menuPlateFrame);
	};
}
