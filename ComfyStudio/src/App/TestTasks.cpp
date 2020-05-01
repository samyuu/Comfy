#include "TestTasks.h"
#include "ImGui/Gui.h"
#include "IO/File.h"
#include "Misc/StringHelper.h"
#include "Input/KeyCode.h"

namespace Comfy::App
{
	static float TimespanToFrame(TimeSpan time, float frameRate = 60.0f)
	{
		return static_cast<float>(time.TotalSeconds() / (1.0f / frameRate));
	}

	void Ps4MenuAetData::Initialize(const Aet::AetSet* aetSet)
	{
		const Aet::Scene& mainAet = *aetSet->GetScenes().front();

		AetLayerSourceData* layerSourcesBegin = reinterpret_cast<AetLayerSourceData*>(this);
		AetLayerSourceData* layerSourcesEnd = reinterpret_cast<AetLayerSourceData*>(this + 1);

		for (AetLayerSourceData* layerSource = layerSourcesBegin; layerSource < layerSourcesEnd; layerSource++)
		{
			layerSource->Layer = mainAet.FindLayer(layerSource->Name).get();
			assert(layerSource->Layer != nullptr);
		}

		p_MenuList01_c.Layer = MenuListIn02.Layer->GetCompItem()->FindLayer(p_MenuList01_c.Name).get();
		p_MenuList02_c.Layer = MenuListIn02.Layer->GetCompItem()->FindLayer(p_MenuList02_c.Name).get();
		p_MenuList03_c.Layer = MenuListIn02.Layer->GetCompItem()->FindLayer(p_MenuList03_c.Name).get();
		p_MenuList04_c.Layer = MenuListIn02.Layer->GetCompItem()->FindLayer(p_MenuList04_c.Name).get();
		p_MenuList05_c.Layer = MenuListIn02.Layer->GetCompItem()->FindLayer(p_MenuList05_c.Name).get();
		p_MenuList06_c.Layer = MenuListIn02.Layer->GetCompItem()->FindLayer(p_MenuList06_c.Name).get();
	}

	void TaskPs4Menu::RenderMenuBackground(Aet::AetRenderer* aetRenderer, float frame)
	{
		aetRenderer->RenderLayerLooped(aetData.CommonBackground, frame);
		aetRenderer->RenderLayerLooped(aetData.MenuDeco, frame);
	}

	void TaskPs4Menu::RenderMainMenuChara(Aet::AetRenderer* aetRenderer, float frame)
	{
		struct { const Aet::Layer *MenuChara, *MenuText; } charaData[MainMenuItem_Count]
		{
			{ aetData.MenuCharaGame, aetData.MenuTextGame },
			{ aetData.MenuCharaCustom, aetData.MenuTextCustom },
			{ aetData.MenuCharaPv, aetData.MenuTextPv },
			{ aetData.MenuCharaMyRoom, aetData.MenuTextMyRoom },
			{ aetData.MenuCharaStore, aetData.MenuTextStore },
			{ aetData.MenuCharaOption, aetData.MenuTextOption },
		};

		auto& data = charaData[mainMenuState.selectedItem];
		float inputFrame = glm::clamp(frame, 0.0f, 21.0f);

		aetRenderer->RenderLayerClamped(data.MenuChara, inputFrame);
		aetRenderer->RenderLayerClamped(data.MenuText, inputFrame);
	}

	void TaskPs4Menu::RenderMainMenuList(Aet::AetRenderer* aetRenderer, bool selectedComp, float menuListFrame, float menuPlateFrame)
	{
		struct { const Aet::Layer *PointLayer, *MenuPlate, *MenuPlateSel; } listLayerData[MainMenuItem_Count]
		{
			{ aetData.p_MenuList01_c, aetData.MenuPlateGame, aetData.MenuPlateGameSel },
			{ aetData.p_MenuList02_c, aetData.MenuPlateCustom, aetData.MenuPlateCustomSel },
			{ aetData.p_MenuList03_c, aetData.MenuPlatePv, aetData.MenuPlatePvSel },
			{ aetData.p_MenuList04_c, aetData.MenuPlateMyRoom, aetData.MenuPlateMyRoomSel },
			{ aetData.p_MenuList05_c, aetData.MenuPlateStore, aetData.MenuPlateStoreSel },
			{ aetData.p_MenuList06_c, aetData.MenuPlateOption, aetData.MenuPlateOptionSel },
		};

		// TEMP:
		static std::vector<Aet::AetMgr::ObjCache> objects; objects.clear();
		Aet::AetMgr::GetAddObjects(objects, aetData.MenuListIn02, menuListFrame);

		for (auto& obj : objects)
		{
			for (MainMenuItem i = 0; i < MainMenuItem_Count; i++)
			{
				auto& data = listLayerData[i];
				if (obj.Source == data.PointLayer)
				{
					if (selectedComp)
					{
						if (mainMenuState.selectedItem == i)
						{
							aetRenderer->RenderLayerLooped(
								data.MenuPlateSel,
								menuPlateFrame,
								obj.Transform.Position,
								obj.Transform.Opacity);
						}
					}
					else
					{
						aetRenderer->RenderLayerLooped(
							data.MenuPlate,
							menuPlateFrame,
							obj.Transform.Position,
							obj.Transform.Opacity);
					}
				}
			}
		}
	}

	bool TaskPs4Menu::Initialize()
	{
		spriteGetterFunction = [this](const Aet::VideoSource* source, const Tex** outTex, const Spr** outSpr) { return Aet::AetRenderer::SpriteNameSprSetSpriteGetter(sprSet.get(), source, outTex, outSpr); };

		return true;
	}

	bool TaskPs4Menu::Update()
	{
		elapsedTime += TimeSpan::FromSeconds(Gui::GetIO().DeltaTime);

		if (aetSet == nullptr || sprSet == nullptr)
		{
			if (aetSet == nullptr && aetSetLoader.GetIsLoaded())
			{
				aetSet = MakeUnique<Aet::AetSet>();

				aetSetLoader.Read(*aetSet);
				aetSetLoader.FreeData();
				aetData.Initialize(aetSet.get());
			}
			else
			{
				aetSetLoader.CheckStartLoadAsync();
			}

			if (sprSet == nullptr && sprSetLoader.GetIsLoaded())
			{
				sprSet = MakeUnique<SprSet>();
				sprSetLoader.Parse(*sprSet);
				sprSet->TexSet->UploadAll(sprSet.get());
				sprSetLoader.FreeData();
			}
			else
			{
				sprSetLoader.CheckStartLoadAsync();
			}

			return true;
		}
		else
		{
			isLoading = false;
		}

		return true;
	}

	bool TaskPs4Menu::Render(GPU_Renderer2D* renderer, Aet::AetRenderer* aetRenderer)
	{
		if (isLoading)
			return true;

		using namespace ImGui;

		aetRenderer->SetSpriteGetterFunction(&spriteGetterFunction);

		float deltaFrame = TimespanToFrame(TimeSpan::FromSeconds(Gui::GetIO().DeltaTime));
		elapsedFrames += deltaFrame;

		if (IsWindowFocused())
		{
			if (IsKeyPressed(KeyCode_1))
				stateType = TaskPs4MenuStateType::MainMenuInitialize;
		}

		switch (stateType)
		{
		case TaskPs4MenuStateType::MainMenuInitialize:
		{
			elapsedFrames = 0.0f;
			mainMenuState.Frames = 0.0f;
			mainMenuState.FramesSinceItemSwitch = 0.0f;
			mainMenuState.selectedItem = 0;
			stateType = TaskPs4MenuStateType::MainMenuIn;

			RenderMenuBackground(aetRenderer, elapsedFrames);
			aetRenderer->RenderLayerLooped(aetData.MenuFooter, elapsedFrames);
			aetRenderer->RenderLayerLooped(aetData.MenuHeader, elapsedFrames);

			break;
		}

		case TaskPs4MenuStateType::MainMenuIn:
		{
			mainMenuState.Frames += deltaFrame;

			if (mainMenuState.Frames >= aetData.MenuListIn02->EndFrame)
			{
				mainMenuState.Frames = aetData.MenuListIn02->EndFrame - 1.0f;
				mainMenuState.FramesSinceItemSwitch = mainMenuState.Frames;
				stateType = TaskPs4MenuStateType::MainMenuLoop;
			}

			RenderMenuBackground(aetRenderer, elapsedFrames);
			RenderMainMenuChara(aetRenderer, mainMenuState.Frames);
			RenderMainMenuList(aetRenderer, false, mainMenuState.Frames, mainMenuState.FramesSinceItemSwitch);
			aetRenderer->RenderLayerLooped(aetData.MenuFooter, elapsedFrames);
			aetRenderer->RenderLayerLooped(aetData.MenuHeader, elapsedFrames);

			break;
		}

		case TaskPs4MenuStateType::MainMenuLoop:
		{
			mainMenuState.FramesSinceItemSwitch += deltaFrame;

			if (IsWindowFocused())
			{
				if (IsKeyPressed(KeyCode_Up))
				{
					mainMenuState.FramesSinceItemSwitch = 0.0f;
					mainMenuState.selectedItem--;

					if (mainMenuState.selectedItem < 0)
						mainMenuState.selectedItem = MainMenuItem_Count - 1;
				}

				if (IsKeyPressed(KeyCode_Down))
				{
					mainMenuState.FramesSinceItemSwitch = 0.0f;
					mainMenuState.selectedItem++;

					if (mainMenuState.selectedItem >= MainMenuItem_Count)
						mainMenuState.selectedItem = 0;
				}

				if (IsKeyPressed(KeyCode_Enter))
				{

				}
			}

			RenderMenuBackground(aetRenderer, elapsedFrames);
			RenderMainMenuChara(aetRenderer, mainMenuState.FramesSinceItemSwitch);
			RenderMainMenuList(aetRenderer, false, mainMenuState.Frames, mainMenuState.FramesSinceItemSwitch);
			RenderMainMenuList(aetRenderer, true, mainMenuState.Frames, mainMenuState.FramesSinceItemSwitch);
			aetRenderer->RenderLayerLooped(aetData.MenuFooter, elapsedFrames);
			aetRenderer->RenderLayerLooped(aetData.MenuHeader, elapsedFrames);

			break;
		}

		default:
			break;
		}

		return true;
	}

	static void DwGuiTest()
	{
		using namespace Gui;

		Gui::WindowContextMenu("dw_gui##ContextMenu", []()
		{
			if (BeginMenu("Data Test"))
			{
				if (MenuItem("OSAGE TEST", nullptr, false, false)) {}
				if (MenuItem("CHAR PERFORMANCE", nullptr, false, false)) {}
				if (MenuItem("CHARA TEST", nullptr, false, false)) {}
				if (MenuItem("MOTION FLAGS", nullptr, false, false)) {}
				if (MenuItem("NODE FLAGS", nullptr, false, false)) {}
				if (MenuItem("CNS/EXP NODE", nullptr, false, false)) {}
				if (MenuItem("TEXTURE TEST", nullptr, false, false)) {}
				if (MenuItem("SPRITE TEST", nullptr, false, false)) {}
				if (MenuItem("ITEM EQUIP TEST", nullptr, false, false)) {}
				if (MenuItem("SAVE & LOAD ITEM EQUIP", nullptr, false, false)) {}
				if (MenuItem("MESH", nullptr, false, false)) {}
				if (MenuItem("SOUND", nullptr, false, false)) {}
				if (MenuItem("MOVIE", nullptr, false, false)) {}
				if (MenuItem("ITEM DISP", nullptr, false, false)) {}
				if (MenuItem("ROB COLLISION", nullptr, false, false)) {}
				if (MenuItem("ROB SLEEVE", nullptr, false, false)) {}
				if (MenuItem("ROB SCALE", nullptr, false, false)) {}
				if (MenuItem("ROB EFFECTOR SCALE", nullptr, false, false)) {}
				if (MenuItem("ROB LOOK", nullptr, false, false)) {}
				if (MenuItem("HAND ITEM", nullptr, false, false)) {}
				if (MenuItem("CMN ITEM", nullptr, false, false)) {}
				Gui::EndMenu();
			}
			if (BeginMenu("Graphics"))
			{
				if (MenuItem("NPPerfKit", nullptr, false, false)) {}
				if (MenuItem("Light", nullptr, false, false)) {}
				if (MenuItem("Face Light", nullptr, false, false)) {}
				if (MenuItem("Fog", nullptr, false, false)) {}
				if (MenuItem("Post process", nullptr, false, false)) {}
				if (MenuItem("Color Change", nullptr, false, false)) {}
				if (MenuItem("Shadow", nullptr, false, false)) {}
				if (MenuItem("Rendering", nullptr, false, false)) {}
				if (MenuItem("Performance", nullptr, false, false)) {}
				if (MenuItem("Global Material", nullptr, false, false)) {}
				if (MenuItem("Material", nullptr, false, false)) {}
				if (MenuItem("Ripple Effect", nullptr, false, false)) {}
				if (MenuItem("Rain Effect", nullptr, false, false)) {}
				if (MenuItem("Snow Effect", nullptr, false, false)) {}
				if (MenuItem("Reflect/Refract", nullptr, false, false)) {}
				if (MenuItem("Wind", nullptr, false, false)) {}
				if (MenuItem("Show Vector", nullptr, false, false)) {}
				if (MenuItem("Show MIPMAP level", nullptr, false, false)) {}
				if (MenuItem("Splash", nullptr, false, false)) {}
				if (MenuItem("Fog Ring", nullptr, false, false)) {}
				if (MenuItem("Star Effect", nullptr, false, false)) {}
				if (MenuItem("Glitter", nullptr, false, false)) {}
				if (MenuItem("Capture", nullptr, false, false)) {}
				if (MenuItem("NPR Cloth", nullptr, false, false)) {}
				Gui::EndMenu();
			}
			if (BeginMenu("Information"))
			{
				if (MenuItem("System", nullptr, false, false)) {}
				if (MenuItem("Console", nullptr, false, false)) {}
				if (MenuItem("Task", nullptr, false, false)) {}
				if (MenuItem("FrameSpeed", nullptr, false, false)) {}
				if (MenuItem("a3d", nullptr, false, false)) {}
				if (MenuItem("Rob Motion Info", nullptr, false, false)) {}
				Gui::EndMenu();
			}
			if (BeginMenu("Rob"))
			{
				if (MenuItem("Motkind flag", nullptr, false, false)) {}
				if (MenuItem("Kaesare flag", nullptr, false, false)) {}
				if (MenuItem("Attack Sub", nullptr, false, false)) {}
				if (MenuItem("Offensive Move", nullptr, false, false)) {}
				if (MenuItem("Yarare Test", nullptr, false, false)) {}
				Gui::EndMenu();
			}
			if (BeginMenu("DEBUG CAMERA"))
			{
				if (MenuItem("DEBUG_CAMERA", nullptr, false, false)) {}
				if (MenuItem("DEBUG_CAMERA_PSP", nullptr, false, false)) {}
				if (MenuItem("SET_DEBUG_CAMERA", nullptr, false, false)) {}
				Gui::EndMenu();
			}
			if (MenuItem("PlayerData", nullptr, false, false)) {}
			if (MenuItem("Contest", nullptr, false, false)) {}
			if (MenuItem("Noblesse", nullptr, false, false)) {}
			if (MenuItem("Quest", nullptr, false, false)) {}
			if (MenuItem("Campaign", nullptr, false, false)) {}
			if (MenuItem("PV_SELECTOR", nullptr, false, false)) {}
			if (MenuItem("Slider", nullptr, false, false)) {}
			if (MenuItem("CommFailure", nullptr, false, false)) {}
			if (MenuItem("Closing Mode", nullptr, false, false)) {}
			Separator();
			if (BeginMenu("GuiInternal"))
			{
				if (MenuItem("InfoWindow...", nullptr, false, false)) {}
				if (MenuItem("DebugConsole...", nullptr, false, false)) {}
				if (MenuItem("FileViewer...", nullptr, false, false)) {}
				if (MenuItem("WindowDemo...", nullptr, false, false)) {}
				if (MenuItem("GraphDemo...", nullptr, false, false)) {}
				if (MenuItem("RegionDemo...", nullptr, false, false)) {}
				if (BeginMenu("MenuDemo"))
				{
					if (MenuItem("check 0", nullptr, false, false)) {}
					if (MenuItem("check 1", nullptr, false, false)) {}
					if (MenuItem("radio 0", nullptr, false, false)) {}
					if (MenuItem("radio 1", nullptr, false, false)) {}
					if (MenuItem("radio 2", nullptr, false, false)) {}
					Gui::EndMenu();
				}
				Gui::EndMenu();
			}
			if (BeginMenu("Options"))
			{
				if (MenuItem("ColorListWindow...", nullptr, false, false)) {}
				if (MenuItem("FontListWindow...", nullptr, false, false)) {}
				if (MenuItem("config save", nullptr, false, false)) {}
				if (MenuItem("config load", nullptr, false, false)) {}
				Gui::EndMenu();
			}
		});
	}

	bool TaskPs4Menu::PostDrawGui()
	{
#if defined(COMFY_DEBUG)
		DwGuiTest();
#endif

		if (isLoading)
		{
			static int loadingFrameCount = 0;
			Gui::Text("Loading... %d", loadingFrameCount++);
		}

		return true;
	}
}
