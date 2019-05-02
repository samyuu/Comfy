#include "TestTimeline.h"
#include "../Application.h"

static void ShowExampleAppCustomNodeGraph(bool* opened)
{
	ImGui::SetNextWindowSize(ImVec2(700, 600), ImGuiSetCond_FirstUseEver);
	if (!ImGui::Begin("Example: Custom Node Graph", opened))
	{
		ImGui::End();
		return;
	}

	// Dummy
	struct Node
	{
		int     ID;
		char    Name[32];
		ImVec2  Pos, Size;
		float   Value;
		ImVec4  Color;
		int     InputsCount, OutputsCount;

		Node(int id, const char* name, const ImVec2& pos, float value, const ImVec4& color, int inputs_count, int outputs_count) { ID = id; strncpy_s(Name, name, 31); Name[31] = 0; Pos = pos; Value = value; Color = color; InputsCount = inputs_count; OutputsCount = outputs_count; }

		ImVec2 GetInputSlotPos(int slot_no) const { return ImVec2(Pos.x, Pos.y + Size.y * ((float)slot_no + 1) / ((float)InputsCount + 1)); }
		ImVec2 GetOutputSlotPos(int slot_no) const { return ImVec2(Pos.x + Size.x, Pos.y + Size.y * ((float)slot_no + 1) / ((float)OutputsCount + 1)); }
	};
	struct NodeLink
	{
		int     InputIdx, InputSlot, OutputIdx, OutputSlot;

		NodeLink(int input_idx, int input_slot, int output_idx, int output_slot) { InputIdx = input_idx; InputSlot = input_slot; OutputIdx = output_idx; OutputSlot = output_slot; }
	};

	static ImVector<Node> nodes;
	static ImVector<NodeLink> links;
	static bool inited = false;
	static ImVec2 scrolling = ImVec2(0.0f, 0.0f);
	static bool show_grid = true;
	static int node_selected = -1;
	if (!inited)
	{
		nodes.push_back(Node(0, "MainTex", ImVec2(40, 50), 0.5f, ImColor(255, 100, 100), 1, 1));
		nodes.push_back(Node(1, "BumpMap", ImVec2(40, 150), 0.42f, ImColor(200, 100, 200), 1, 1));
		nodes.push_back(Node(2, "Combine", ImVec2(270, 80), 1.0f, ImColor(0, 200, 100), 2, 2));
		links.push_back(NodeLink(0, 0, 2, 0));
		links.push_back(NodeLink(1, 0, 2, 1));
		inited = true;
	}

	// Draw a list of nodes on the left side
	bool open_context_menu = false;
	int node_hovered_in_list = -1;
	int node_hovered_in_scene = -1;
	ImGui::BeginChild("node_list", ImVec2(100, 0));
	ImGui::Text("Nodes");
	ImGui::Separator();
	for (int node_idx = 0; node_idx < nodes.Size; node_idx++)
	{
		Node* node = &nodes[node_idx];
		ImGui::PushID(node->ID);
		if (ImGui::Selectable(node->Name, node->ID == node_selected))
			node_selected = node->ID;
		if (ImGui::IsItemHovered())
		{
			node_hovered_in_list = node->ID;
			open_context_menu |= ImGui::IsMouseClicked(1);
		}
		ImGui::PopID();
	}
	ImGui::EndChild();

	ImGui::SameLine();
	ImGui::BeginGroup();

	const float NODE_SLOT_RADIUS = 4.0f;
	const ImVec2 NODE_WINDOW_PADDING(8.0f, 8.0f);

	// Create our child canvas
	ImGui::Text("Hold middle mouse button to scroll (%.2f,%.2f)", scrolling.x, scrolling.y);
	ImGui::SameLine(ImGui::GetWindowWidth() - 100);
	ImGui::Checkbox("Show grid", &show_grid);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1, 1));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, IM_COL32(60, 60, 70, 200));
	ImGui::BeginChild("scrolling_region", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
	ImGui::PushItemWidth(120.0f);

	ImVec2 offset = ImGui::GetCursorScreenPos() + scrolling;
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	// Display grid
	if (show_grid)
	{
		ImU32 GRID_COLOR = IM_COL32(200, 200, 200, 40);
		float GRID_SZ = 64.0f;
		ImVec2 win_pos = ImGui::GetCursorScreenPos();
		ImVec2 canvas_sz = ImGui::GetWindowSize();
		for (float x = fmodf(scrolling.x, GRID_SZ); x < canvas_sz.x; x += GRID_SZ)
			draw_list->AddLine(ImVec2(x, 0.0f) + win_pos, ImVec2(x, canvas_sz.y) + win_pos, GRID_COLOR);
		for (float y = fmodf(scrolling.y, GRID_SZ); y < canvas_sz.y; y += GRID_SZ)
			draw_list->AddLine(ImVec2(0.0f, y) + win_pos, ImVec2(canvas_sz.x, y) + win_pos, GRID_COLOR);
	}

	// Display links
	draw_list->ChannelsSplit(2);
	draw_list->ChannelsSetCurrent(0); // Background
	for (int link_idx = 0; link_idx < links.Size; link_idx++)
	{
		NodeLink* link = &links[link_idx];
		Node* node_inp = &nodes[link->InputIdx];
		Node* node_out = &nodes[link->OutputIdx];
		ImVec2 p1 = offset + node_inp->GetOutputSlotPos(link->InputSlot);
		ImVec2 p2 = offset + node_out->GetInputSlotPos(link->OutputSlot);
		draw_list->AddBezierCurve(p1, p1 + ImVec2(+50, 0), p2 + ImVec2(-50, 0), p2, IM_COL32(200, 200, 100, 255), 3.0f);
	}

	// Display nodes
	for (int node_idx = 0; node_idx < nodes.Size; node_idx++)
	{
		Node* node = &nodes[node_idx];
		ImGui::PushID(node->ID);
		ImVec2 node_rect_min = offset + node->Pos;

		// Display node contents first
		draw_list->ChannelsSetCurrent(1); // Foreground
		bool old_any_active = ImGui::IsAnyItemActive();
		ImGui::SetCursorScreenPos(node_rect_min + NODE_WINDOW_PADDING);
		ImGui::BeginGroup(); // Lock horizontal position
		ImGui::Text("%s", node->Name);
		ImGui::SliderFloat("##value", &node->Value, 0.0f, 1.0f, "Alpha %.2f");
		ImGui::ColorEdit3("##color", &node->Color.x);
		ImGui::EndGroup();

		// Save the size of what we have emitted and whether any of the widgets are being used
		bool node_widgets_active = (!old_any_active && ImGui::IsAnyItemActive());
		node->Size = ImGui::GetItemRectSize() + NODE_WINDOW_PADDING + NODE_WINDOW_PADDING;
		ImVec2 node_rect_max = node_rect_min + node->Size;

		// Display node box
		draw_list->ChannelsSetCurrent(0); // Background
		ImGui::SetCursorScreenPos(node_rect_min);
		ImGui::InvisibleButton("node", node->Size);
		if (ImGui::IsItemHovered())
		{
			node_hovered_in_scene = node->ID;
			open_context_menu |= ImGui::IsMouseClicked(1);
		}
		bool node_moving_active = ImGui::IsItemActive();
		if (node_widgets_active || node_moving_active)
			node_selected = node->ID;
		if (node_moving_active && ImGui::IsMouseDragging(0))
			node->Pos = node->Pos + ImGui::GetIO().MouseDelta;

		ImU32 node_bg_color = (node_hovered_in_list == node->ID || node_hovered_in_scene == node->ID || (node_hovered_in_list == -1 && node_selected == node->ID)) ? IM_COL32(75, 75, 75, 255) : IM_COL32(60, 60, 60, 255);
		draw_list->AddRectFilled(node_rect_min, node_rect_max, node_bg_color, 4.0f);
		draw_list->AddRect(node_rect_min, node_rect_max, IM_COL32(100, 100, 100, 255), 4.0f);
		for (int slot_idx = 0; slot_idx < node->InputsCount; slot_idx++)
			draw_list->AddCircleFilled(offset + node->GetInputSlotPos(slot_idx), NODE_SLOT_RADIUS, IM_COL32(150, 150, 150, 150));
		for (int slot_idx = 0; slot_idx < node->OutputsCount; slot_idx++)
			draw_list->AddCircleFilled(offset + node->GetOutputSlotPos(slot_idx), NODE_SLOT_RADIUS, IM_COL32(150, 150, 150, 150));

		ImGui::PopID();
	}
	draw_list->ChannelsMerge();

	// Open context menu
	if (!ImGui::IsAnyItemHovered() && ImGui::IsMouseHoveringWindow() && ImGui::IsMouseClicked(1))
	{
		node_selected = node_hovered_in_list = node_hovered_in_scene = -1;
		open_context_menu = true;
	}
	if (open_context_menu)
	{
		ImGui::OpenPopup("context_menu");
		if (node_hovered_in_list != -1)
			node_selected = node_hovered_in_list;
		if (node_hovered_in_scene != -1)
			node_selected = node_hovered_in_scene;
	}

	// Draw context menu
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
	if (ImGui::BeginPopup("context_menu"))
	{
		Node* node = node_selected != -1 ? &nodes[node_selected] : NULL;
		ImVec2 scene_pos = ImGui::GetMousePosOnOpeningCurrentPopup() - offset;
		if (node)
		{
			ImGui::Text("Node '%s'", node->Name);
			ImGui::Separator();
			if (ImGui::MenuItem("Rename..", NULL, false, false)) {}
			if (ImGui::MenuItem("Delete", NULL, false, false)) {}
			if (ImGui::MenuItem("Copy", NULL, false, false)) {}
		}
		else
		{
			if (ImGui::MenuItem("Add")) { nodes.push_back(Node(nodes.Size, "New node", scene_pos, 0.5f, ImColor(100, 100, 200), 2, 2)); }
			if (ImGui::MenuItem("Paste", NULL, false, false)) {}
		}
		ImGui::EndPopup();
	}
	ImGui::PopStyleVar();

	// Scrolling
	if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() && ImGui::IsMouseDragging(2, 0.0f))
		scrolling = scrolling + ImGui::GetIO().MouseDelta;

	ImGui::PopItemWidth();
	ImGui::EndChild();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar(2);
	ImGui::EndGroup();

	ImGui::End();
}

TestTimeline::TestTimeline(Application* parent) : BaseWindow(parent)
{
}

TestTimeline::~TestTimeline()
{
}

const char* TestTimeline::GetGuiName()
{
	return u8"Test Timeline";
}

void TestTimeline::DrawGui()
{
	using namespace ImGui;
	ImGuiWindow* window = GetCurrentWindow();
	ImDrawList* drawList = window->DrawList;

	static float cursorPos;

	if (true)
	{
		auto draw_list = ImGui::GetWindowDrawList();

		static ImVec2 scrolling = ImVec2(0.0f, 0.0f);

		//if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() && ImGui::IsMouseDragging(2, 0.0f))
		//	scrolling = scrolling + ImGui::GetIO().MouseDelta;

		float mouseWheel = GetIO().MouseWheel;

		if (IsWindowHovered() && !IsAnyItemActive() && mouseWheel != 0.0f)
		{
			constexpr float scrollAmount = 200;
			scrolling.x -= mouseWheel * scrollAmount;
		}

		ImU32 GRID_COLOR = IM_COL32(200, 200, 200, 40);
		ImVec2 win_pos = ImGui::GetCursorScreenPos();
		ImVec2 canvas_sz = ImGui::GetWindowSize();

		constexpr float GRID_SZ = 64.0f;

		draw_list->AddLine(ImVec2(0.0f, 0) + win_pos, ImVec2(canvas_sz.x, 0) + win_pos, GRID_COLOR);

		for (float x = fmodf(scrolling.x, GRID_SZ); x < canvas_sz.x; x += GRID_SZ)
			draw_list->AddLine(ImVec2(x, 0.0f) + win_pos, ImVec2(x, canvas_sz.y) + win_pos, GRID_COLOR);

		//for (float y = fmodf(scrolling.y, GRID_SZ); y < canvas_sz.y; y += GRID_SZ)
		//	draw_list->AddLine(ImVec2(0.0f, y) + win_pos, ImVec2(canvas_sz.x, y) + win_pos, GRID_COLOR);
	}

	return;

	//float startY = ImGui::GetCursorPosY();

	//for (size_t i = 0; i < 100; i++)
	//{
	//	//drawList->AddRectFilled();
	//	const float width = 20;
	//	ImGui::SetCursorPosX(i * width);
	//	ImGui::SetCursorPosY(startY);

	//	ImGui::Button("O");
	//}

	//drawList->AddRectFilled(ImGui::GetCursorPos(), { ImGui::GetCursorPosX() + 20000, ImGui::GetCursorPosY() + 200 }, IM_COL32_WHITE);
	//drawList->AddRectFilled({ 200, 200 }, { ImGui::GetCursorPosX() + 20000, ImGui::GetCursorPosY() + 20000 }, IM_COL32_WHITE);

	if (ImGui::IsWindowFocused() && ImGui::IsMouseDown(1))
	{
		cursorPos = ImGui::GetCursorPosX() - ImGui::GetWindowPos().x;
	}

	//ImVec2 cursorPos;
	//ImGui::Image
	//drawList->AddRectFilled();

	ImVec2 size = { 4000 * 200, 4000  * 200};
	ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);

	//printf("cursorPos: %dx%d\n", (int)GetCursorPosX(), (int)GetCursorPosY());
	//printf("DC.CursorPos: %dx%d\n", (int)window->DC.CursorPos.x, (int)window->DC.CursorPos.y);

	// advance cursor
	ItemSize(bb);
	// declare item bounding box
	if (!ItemAdd(bb, 0))
		return;

	// doing something like this would be extremely inefficient, only the lines inside the visible scroll are need to be drawn
	// imgui might not be best suited for this afterall
	for (int i = 0; i < 4000; i++)
	{
		drawList->AddRect(bb.Min + ImVec2 { i * 200.0f, 0 }, bb.Min + ImVec2 { i * 200.0f, 8 }, IM_COL32(255, 0, 0, 255));
	}

	//drawList->AddRectFilledMultiColor(bb.Min, bb.Max, IM_COL32(255, 0, 0, 255), IM_COL32(0, 255, 0, 255), IM_COL32(0, 0, 255, 255), IM_COL32(255, 255, 0, 255));

	//Image

	//static bool craphOpened;
	//ShowExampleAppCustomNodeGraph(&craphOpened);

	return;

	size_t itemCount = 4;

	ImGui::BeginColumns("timeline_column", 2);
	static char nameBuffer[512] = "test";

	for (size_t i = 0; i < itemCount; i++)
	{
		ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer));
		ImGui::Text("test");
	}

	ImGui::NextColumn();

	for (size_t i = 0; i < itemCount; i++)
	{
		ImGui::Text("test");
	}

	ImGui::EndColumns();


	ImGui::Separator();

	if (ImGui::CollapsingHeader("Column Demo"))
	{
		static bool h_borders = true;
		static bool v_borders = true;
		ImGui::Checkbox("horizontal", &h_borders);
		ImGui::SameLine();
		ImGui::Checkbox("vertical", &v_borders);
		ImGui::Columns(4, NULL, v_borders);
		for (int i = 0; i < 4 * 3; i++)
		{
			if (h_borders && ImGui::GetColumnIndex() == 0)
				ImGui::Separator();
			ImGui::Text("%c%c%c", 'a' + i, 'a' + i, 'a' + i);
			ImGui::Text("Width %.2f\nOffset %.2f", ImGui::GetColumnWidth(), ImGui::GetColumnOffset());
			ImGui::NextColumn();
		}
		ImGui::Columns(1);
		if (h_borders)
			ImGui::Separator();
	}
}

ImGuiWindowFlags TestTimeline::GetWindowFlags()
{
	return ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar;
}