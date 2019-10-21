#include "AetLayerView.h"
#include "Editor/Aet/AetIcons.h"
#include "ImGui/Gui.h"

namespace Editor
{
	// TODO:
	// "this" AetObj / AetLayer header
	// list of reorderable children below, visiblity button to the left (use columns api (?))
	// add, delete, duplicate buttons at the bottom

	struct Data
	{
		bool Visible;
		const char* Name;

		//Data(const Data& other) { Name = other.Name; };
		//bool operator == (const Data& other) { return Name == other.Name; };
		Data(const char* name) : Visible(true), Name(name) {};
	};

	static void ShiftVector(std::vector<Data>& vector, int sourceIndex, int destinationIndex)
	{
		if (sourceIndex == destinationIndex)
			return;

		if (sourceIndex < destinationIndex)
			destinationIndex--;

		Data sourceData = vector[sourceIndex];
		vector.erase(vector.begin() + sourceIndex);
		vector.insert(vector.begin() + destinationIndex, sourceData);
	}

	static int selectedIndex = 0;
	static int dragSourceIndex = -1;
	static int dragDestinationIndex = -1;

	bool AetLayerView::DrawGui()
	{
		using namespace Gui;

		static std::vector<Data> testData = { "0. test_eff", "1. gam_cmn_block.pic", "2. gam_cmn_blimp.pic", "3. kirai" };

		BeginChild("Test Child", GetContentRegionAvail() - ImVec2(0, 26), false, ImGuiWindowFlags_None);
		{
			Text("sel: %d", selectedIndex);
			Text("src: %d", dragSourceIndex);
			Text("dst: %d", dragDestinationIndex);

			//Columns(2, nullptr, false);
			//SetColumnWidth(0, 34);

			if (IsMouseDown(0) && IsWindowFocused())
				dragSourceIndex = -1;

			for (int i = 0; i < testData.size(); i++)
			{
				Data& data = testData[i];

				//if (SmallButton(data.Visible ? ICON_VISIBLE : ICON_INVISIBLE, ImVec2(26, 0)))
				//	data.Visible ^= true;

				//NextColumn();
				{
					bool selected = i == selectedIndex;

					PushID(&data);
					Selectable(data.Name, selected, ImGuiSelectableFlags_SpanAllColumns);
					PopID();

					if (dragSourceIndex >= 0 && IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) && IsWindowFocused())
						dragDestinationIndex = selected ? -1 : i;

					if (dragDestinationIndex == i)
					{
						GetForegroundDrawList()->AddRect(GetItemRectMin(), GetItemRectMin() + ImVec2(GetItemRectSize().x, 3), GetColorU32(ImGuiCol_NavWindowingHighlight, .75f));
					}

					if (WasActiveWindowFocusedOnMouseClicked(0) && IsMouseReleased(0) && IsItemHovered(ImGuiHoveredFlags_None))
					{
						selectedIndex = i;
						selected = true;
					}

					if (selected && IsMouseDown(0) && IsWindowFocused())
						dragSourceIndex = selectedIndex;

					if (dragSourceIndex == i)
					{
						GetForegroundDrawList()->AddRect(GetItemRectMin(), GetItemRectMax(), GetColorU32(ImGuiCol_NavWindowingHighlight, .75f));
					}
				}
				//NextColumn();
			}

			if (dragSourceIndex >= 0 && testData.size() > 0 && IsWindowFocused())
			{
				if (IsMouseDown(0) && GetMousePos().y > GetCursorScreenPos().y)
				{
					ImVec2 bottomLeft = ImVec2(GetItemRectMin().x, GetItemRectMax().y);
					GetForegroundDrawList()->AddRect(bottomLeft, bottomLeft + ImVec2(GetItemRectSize().x, 3), GetColorU32(ImGuiCol_NavWindowingHighlight, .75f));
			
					dragDestinationIndex = static_cast<int32_t>(testData.size());
				}
			}


			if (IsMouseReleased(0))
			{
				if (dragSourceIndex >= 0 && dragDestinationIndex >= 0 && IsWindowFocused())
				{
					ShiftVector(testData, dragSourceIndex, dragDestinationIndex);
				}

				dragSourceIndex = dragDestinationIndex = -1;
			}


			//Columns(1);
		}
		EndChild();

		PushStyleVar(ImGuiStyleVar_FramePadding, vec2(0));
		PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(0));

		Separator();
		{
			SetCursorPosX(GetWindowWidth() - 22 * 2);
			ComfySmallButton(ICON_ADD, ImVec2(22, 22));
			SameLine();
			SetCursorPosX(GetWindowWidth() - 22 * 1);
			ComfySmallButton(ICON_DELETE, ImVec2(22, 22));
		}
		PopStyleVar(2);

		return true;
	}
}