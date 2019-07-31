#include "TestTasks.h"
#include "ImGui/imgui_extensions.h"
#include "FileSystem/FileHelper.h"
#include "Misc/StringHelper.h"
#include "Input/KeyCode.h"
#include <assert.h>
#include <cmath>

namespace App
{
	static std::vector<AetMgr::ObjCache>& GetObjCache(const AetObj* aetObj, float frame)
	{
		static std::vector<AetMgr::ObjCache> objects;
		objects.clear();
		AetMgr::GetAddObjects(objects, aetObj, frame);

		return objects;
	}

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

	bool TaskPs4Menu::SpriteGetter(AetSprite* inSprite, Texture** outTexture, Sprite** outSprite)
	{
		if (inSprite == nullptr)
			return false;

		if (inSprite->SpriteCache != nullptr)
		{
		from_sprite_cache:
			*outTexture = sprSet->TxpSet->Textures[inSprite->SpriteCache->TextureIndex].get();
			*outSprite = inSprite->SpriteCache;
			return true;
		}

		for (auto& sprite : sprSet->Sprites)
		{
			if (EndsWith(inSprite->Name, sprite.Name))
			{
				inSprite->SpriteCache = &sprite;
				goto from_sprite_cache;
			}
		}

		return false;
	}

	void TaskPs4Menu::RenderObjCache(Renderer2D& renderer, const AetMgr::ObjCache& obj, const vec2& position, float opacity)
	{
		if (obj.Region == nullptr)
			return;

		Texture* texture;
		Sprite* sprite;
		bool validSprite = SpriteGetter(obj.Region->GetSprite(obj.SpriteIndex), &texture, &sprite);

		if (validSprite)
		{
			renderer.Draw(
				texture->Texture2D.get(),
				sprite->PixelRegion,
				obj.Properties.Position + position,
				obj.Properties.Origin,
				obj.Properties.Rotation,
				obj.Properties.Scale,
				vec4(1.0f, 1.0f, 1.0f, obj.Properties.Opacity * opacity),
				obj.BlendMode);
		}
	}

	void TaskPs4Menu::RenderAetObj(Renderer2D& renderer, const AetObj* aetObj, float frame, const vec2& position, float opacity)
	{
		static std::vector<AetMgr::ObjCache> objects;
		objects.clear();

		AetMgr::GetAddObjects(objects, aetObj, frame);

		for (auto& objCache : objects)
			RenderObjCache(renderer, objCache, position, opacity);
	}

	void TaskPs4Menu::RenderAetObjLooped(Renderer2D& renderer, const AetObj* aetObj, float frame, const vec2& position, float opacity)
	{
		RenderAetObj(renderer, aetObj, fmod(frame, aetObj->LoopEnd - 1.0f), position, opacity);
	}

	void TaskPs4Menu::RenderAetObjClamped(Renderer2D& renderer, const AetObj* aetObj, float frame, const vec2& position, float opacity)
	{
		RenderAetObj(renderer, aetObj, (frame >= aetObj->LoopEnd ? aetObj->LoopEnd : frame), position, opacity);
	}

	void TaskPs4Menu::RenderMenuBackground(Renderer2D& renderer, float frame)
	{
		RenderAetObjLooped(renderer, aetData.CommonBackground, frame);
		RenderAetObjLooped(renderer, aetData.MenuDeco, frame);
	}

	void TaskPs4Menu::RenderMainMenuChara(Renderer2D& renderer, float frame)
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

		RenderAetObjClamped(renderer, data.MenuChara, inputFrame);
		RenderAetObjClamped(renderer, data.MenuText, inputFrame);
	}

	void TaskPs4Menu::RenderMainMenuList(Renderer2D& renderer, bool selectedLayer, float menuListFrame, float menuPlateFrame)
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

		auto& objects = GetObjCache(aetData.MenuListIn02, menuListFrame);
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
							RenderAetObjLooped(renderer,
								data.MenuPlateSel,
								menuPlateFrame,
								obj.Properties.Position,
								obj.Properties.Opacity);
						}
					}
					else
					{
						RenderAetObjLooped(renderer,
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

	bool TaskPs4Menu::Render(Auth2D::Renderer2D& renderer)
	{
		if (isLoading)
			return true;

		using namespace ImGui;

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

			RenderMenuBackground(renderer, elapsedFrames);
			RenderAetObjLooped(renderer, aetData.MenuFooter, elapsedFrames);
			RenderAetObjLooped(renderer, aetData.MenuHeader, elapsedFrames);

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

			RenderMenuBackground(renderer, elapsedFrames);
			RenderMainMenuChara(renderer, mainMenuState.Frames);
			RenderMainMenuList(renderer, false, mainMenuState.Frames, mainMenuState.FramesSinceItemSwitch);
			RenderAetObjLooped(renderer, aetData.MenuFooter, elapsedFrames);
			RenderAetObjLooped(renderer, aetData.MenuHeader, elapsedFrames);

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

			RenderMenuBackground(renderer, elapsedFrames);
			RenderMainMenuChara(renderer, mainMenuState.FramesSinceItemSwitch);
			RenderMainMenuList(renderer, false, mainMenuState.Frames, mainMenuState.FramesSinceItemSwitch);
			RenderMainMenuList(renderer, true, mainMenuState.Frames, mainMenuState.FramesSinceItemSwitch);
			RenderAetObjLooped(renderer, aetData.MenuFooter, elapsedFrames);
			RenderAetObjLooped(renderer, aetData.MenuHeader, elapsedFrames);

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
			static int frameCount = 0;
			//ImGui::Text("Loading...");

			for (size_t i = 0; i < frameCount; i++)
			{
				ImGui::Text("Loading... %d", frameCount);
			}

			frameCount++;
		}

		return true;
	}
}