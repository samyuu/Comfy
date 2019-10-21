#include "AetDialogs.h"
#include "Editor/Aet/AetIcons.h"
#include "ImGui/Gui.h"

namespace Editor
{
	using namespace ImGui;

	bool AddAetObjDialog::DrawGui(Aet* aet, AetLayer* layer)
	{
		BeginChild("BaseChild::AddAetObjDialog");

		if (BeginTabBar("AetObjPopupTypeTapBar::AddAetObjDialog", ImGuiTabBarFlags_None))
		{
			if (BeginTabItem("   " ICON_AETOBJPIC "  Pic   "))
			{
				newTypeIndex = static_cast<int>(AetObjType::Pic);
				Text("Select Aet Region:");

				// TODO:
				//printf("GetMousePos().y: %d\n", (int)GetMousePos().y);
				float height = (GetWindowHeight() - GetCursorPosY() - 200) * 0.5f;
				if (height <= 0.0f) height = 0.0001f;

				BeginChild("AetObjPopupAetRegionChild::AddAetObjDialog", ImVec2(0.0f, height), true);
				{
					Columns(2);

					Text("Region");
					NextColumn();
					Text("Sprites");
					NextColumn();
					Separator();

					for (int32_t i = 0; i < aet->Regions.size(); i++)
					{
						RefPtr<AetRegion>& region = aet->Regions[i];

						PushID(region.get());
						if (Selectable("##AetRegion", i == newRegionIndex, ImGuiSelectableFlags_SpanAllColumns))
							newRegionIndex = i;

						SameLine();
						Text("Region: %dx%d", region->Size.x, region->Size.y);

						NextColumn();
						if (region->SpriteCount() > 0)
						{
							Text(ICON_AETREGION "  %s", region->GetFrontSprite()->Name.c_str());

							if (region->SpriteCount() > 1)
							{
								SameLine();
								Text("...");
								SameLine();
								Text(region->GetBackSprite()->Name.c_str());
							}
						}
						else
						{
							Text("<none>");
						}
						NextColumn();

						PopID();
					}

					Columns(1);
				}
				EndChild();

				EndTabItem();
			}

			if (BeginTabItem("   " ICON_AETOBJAIF "  Aif   "))
			{
				newTypeIndex = static_cast<int>(AetObjType::Aif);
				Text("Select Sound Effect:");
				EndTabItem();
			}

			if (BeginTabItem("   " ICON_AETOBJEFF "  Eff   "))
			{
				newTypeIndex = static_cast<int>(AetObjType::Eff);
				Text("Select Aet Layer:");
				EndTabItem();
			}
			EndTabBar();
		}

		InputText("AetObj Name", newObjNameBuffer, sizeof(newObjNameBuffer));

		if (Button(ICON_FA_PLUS "  Add", ImVec2(GetWindowWidth(), 0)))
		{
			// TODO:
			// aetLayer.Objects.emplace(aetLayer.begin());
			// AetObj* newObj = &aetLayer.Objects.front();
			// 
			// newObj->Name = std::string(newObjNameBuffer);
			// newObj->Type = (AetObjType)newObjTypeIndex;
			// newObj->PlaybackSpeed = 1.0f;

			CloseCurrentPopup();
		}
		SameLine();

		EndChild();

		return false;
	}

	bool* AddAetObjDialog::GetIsGuiOpenPtr()
	{
		return &isGuiOpen;
	}

	const char* AddAetObjDialog::GetGuiName()
	{
		return "Add new AetObj##AddAetObjDialog";
	}
}