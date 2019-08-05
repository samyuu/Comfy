#include "TestTasks.h"
#include "ImGui/imgui_extensions.h"
#include "FileSystem/FileHelper.h"
#include "Misc/StringHelper.h"
#include "Input/KeyCode.h"
#include <assert.h>
#include <cmath>

namespace App
{
	static float TimespanToFrame(TimeSpan time, float frameRate = 60.0f)
	{
		return static_cast<float>(time.TotalSeconds() / (1.0f / frameRate));
	}

	void Ps4MenuAetData::Initialize(const AetSet* aetSet)
	{
		const Aet& mainAet = aetSet->front();

		AetObjSourceData* objSourcesBegin = reinterpret_cast<AetObjSourceData*>(this);
		AetObjSourceData* objSourcesEnd = reinterpret_cast<AetObjSourceData*>(this + 1);

		for (AetObjSourceData* objSource = objSourcesBegin; objSource < objSourcesEnd; objSource++)
		{
			objSource->Obj = mainAet.GetObj(objSource->Name);
			assert(objSource->Obj != nullptr);
		}

		p_MenuList01_c.Obj = MenuListIn02.Obj->GetLayer()->GetObj(p_MenuList01_c.Name);
		p_MenuList02_c.Obj = MenuListIn02.Obj->GetLayer()->GetObj(p_MenuList02_c.Name);
		p_MenuList03_c.Obj = MenuListIn02.Obj->GetLayer()->GetObj(p_MenuList03_c.Name);
		p_MenuList04_c.Obj = MenuListIn02.Obj->GetLayer()->GetObj(p_MenuList04_c.Name);
		p_MenuList05_c.Obj = MenuListIn02.Obj->GetLayer()->GetObj(p_MenuList05_c.Name);
		p_MenuList06_c.Obj = MenuListIn02.Obj->GetLayer()->GetObj(p_MenuList06_c.Name);
	}

	void TaskPs4Menu::RenderMenuBackground(AetRenderer* aetRenderer, float frame)
	{
		aetRenderer->RenderAetObjLooped(aetData.CommonBackground, frame);
		aetRenderer->RenderAetObjLooped(aetData.MenuDeco, frame);
	}

	void TaskPs4Menu::RenderMainMenuChara(AetRenderer* aetRenderer, float frame)
	{
		struct { const AetObj *MenuChara, *MenuText; } charaData[MainMenuItem_Count]
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

		aetRenderer->RenderAetObjClamped(data.MenuChara, inputFrame);
		aetRenderer->RenderAetObjClamped(data.MenuText, inputFrame);
	}

	void TaskPs4Menu::RenderMainMenuList(AetRenderer* aetRenderer, bool selectedLayer, float menuListFrame, float menuPlateFrame)
	{
		struct { const AetObj *PointObj, *MenuPlate, *MenuPlateSel; } listObjData[MainMenuItem_Count]
		{
			{ aetData.p_MenuList01_c, aetData.MenuPlateGame, aetData.MenuPlateGameSel },
			{ aetData.p_MenuList02_c, aetData.MenuPlateCustom, aetData.MenuPlateCustomSel },
			{ aetData.p_MenuList03_c, aetData.MenuPlatePv, aetData.MenuPlatePvSel },
			{ aetData.p_MenuList04_c, aetData.MenuPlateMyRoom, aetData.MenuPlateMyRoomSel },
			{ aetData.p_MenuList05_c, aetData.MenuPlateStore, aetData.MenuPlateStoreSel },
			{ aetData.p_MenuList06_c, aetData.MenuPlateOption, aetData.MenuPlateOptionSel },
		};

		static std::vector<AetMgr::ObjCache> objects; objects.clear();
		AetMgr::GetAddObjects(objects, aetData.MenuListIn02, menuListFrame);
		
		for (auto& obj : objects)
		{
			for (MainMenuItem i = 0; i < MainMenuItem_Count; i++)
			{
				auto& data = listObjData[i];
				if (obj.AetObj == data.PointObj)
				{
					if (selectedLayer)
					{
						if (mainMenuState.selectedItem == i)
						{
							aetRenderer->RenderAetObjLooped(
								data.MenuPlateSel,
								menuPlateFrame,
								obj.Properties.Position,
								obj.Properties.Opacity);
						}
					}
					else
					{
						aetRenderer->RenderAetObjLooped(
							data.MenuPlate,
							menuPlateFrame,
							obj.Properties.Position,
							obj.Properties.Opacity);
					}
				}
			}
		}
	}

	bool TaskPs4Menu::Initialize()
	{
		spriteGetterFunction = [this](AetSprite* inSprite, Texture** outTexture, Sprite** outSprite) { return AetRenderer::SpriteNameSprSetSpriteGetter(sprSet.get(), inSprite, outTexture, outSprite); };

		return true;
	}

	bool TaskPs4Menu::Update()
	{
		elapsedTime += ImGui::GetIO().DeltaTime;

		if (aetSet == nullptr || sprSet == nullptr)
		{
			if (aetSet == nullptr && aetSetLoader.GetIsLoaded())
			{
				aetSet = std::make_unique<AetSet>();

				aetSetLoader.Read(aetSet.get());
				aetSetLoader.FreeData();
				aetData.Initialize(aetSet.get());
			}
			else
			{
				aetSetLoader.CheckStartLoadAsync();
			}

			if (sprSet == nullptr && sprSetLoader.GetIsLoaded())
			{
				sprSet = std::make_unique<SprSet>();
				sprSetLoader.Parse(sprSet.get());
				sprSet->TxpSet->UploadAll();
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

	bool TaskPs4Menu::Render(Renderer2D* renderer, Auth2D::AetRenderer* aetRenderer)
	{
		if (isLoading)
			return true;

		using namespace ImGui;

		aetRenderer->SetSpriteGetterFunction(&spriteGetterFunction);

		float deltaFrame = TimespanToFrame(ImGui::GetIO().DeltaTime);
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
			aetRenderer->RenderAetObjLooped(aetData.MenuFooter, elapsedFrames);
			aetRenderer->RenderAetObjLooped(aetData.MenuHeader, elapsedFrames);

			break;
		}

		case TaskPs4MenuStateType::MainMenuIn:
		{
			mainMenuState.Frames += deltaFrame;

			if (mainMenuState.Frames >= aetData.MenuListIn02->LoopEnd)
			{
				mainMenuState.Frames = aetData.MenuListIn02->LoopEnd - 1.0f;
				mainMenuState.FramesSinceItemSwitch = mainMenuState.Frames;
				stateType = TaskPs4MenuStateType::MainMenuLoop;
			}

			RenderMenuBackground(aetRenderer, elapsedFrames);
			RenderMainMenuChara(aetRenderer, mainMenuState.Frames);
			RenderMainMenuList(aetRenderer, false, mainMenuState.Frames, mainMenuState.FramesSinceItemSwitch);
			aetRenderer->RenderAetObjLooped(aetData.MenuFooter, elapsedFrames);
			aetRenderer->RenderAetObjLooped(aetData.MenuHeader, elapsedFrames);

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
			aetRenderer->RenderAetObjLooped(aetData.MenuFooter, elapsedFrames);
			aetRenderer->RenderAetObjLooped(aetData.MenuHeader, elapsedFrames);

			break;
		}

		default:
			break;
		}

		return true;
	}

	bool TaskPs4Menu::PostDrawGui()
	{
		if (isLoading)
		{
			static int loadingFrameCount = 0;
			ImGui::Text("Loading... %d", loadingFrameCount++);
		}

		return true;
	}
}