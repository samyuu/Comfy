#include "AetLayerView.h"
#include "ImGui/imgui_extensions.h"

namespace Editor
{
	bool AetLayerView::DrawGui()
	{
		ImGui::Text(__FUNCTION__ "(): Test");

		// TODO:
		// "this" AetObj / AetLayer header
		// list of reorderable children below, visiblity button to the left (use columns api (?))
		// add, delete, duplicate buttons at the bottom

		return true;
	}
}