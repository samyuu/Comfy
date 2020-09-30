#include "TargetPositionTool.h"

namespace Comfy::Studio::Editor
{
	void TargetPositionTool::OnSelected()
	{
	}

	void TargetPositionTool::OnDeselected()
	{
	}

	void TargetPositionTool::PreRender(Chart& chart, Render::Renderer2D& renderer)
	{
	}

	void TargetPositionTool::PostRender(Chart& chart, Render::Renderer2D& renderer)
	{
	}

	void TargetPositionTool::OnContextMenuGUI(Chart& chart)
	{
		// TODO:
		Gui::MenuItem("Maybe Position Tool sub modes...", nullptr, nullptr, false);
		Gui::Separator();
	}

	void TargetPositionTool::PreRenderGUI(Chart& chart, ImDrawList& drawList)
	{
	}

	void TargetPositionTool::PostRenderGUI(Chart& chart, ImDrawList& drawList)
	{
		// TODO:
	}

	void TargetPositionTool::UpdateInput(Chart& chart)
	{
		// TODO:
	}

	const char* TargetPositionTool::GetName() const
	{
		return "Position Tool";
	}
}
