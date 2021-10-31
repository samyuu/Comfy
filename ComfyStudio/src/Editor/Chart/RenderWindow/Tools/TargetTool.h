#pragma once
#include "Types.h"
#include "Editor/Chart/Chart.h"
#include "Undo/Undo.h"
#include "ImGui/Gui.h"
#include "Input/Input.h"
#include "Render/Render.h"

namespace Comfy::Studio::Editor
{
	class TargetRenderWindow;

	enum class TargetToolType : u8
	{
		Position,
		Path,
		Count,

		StartupType = Position,
	};

	class TargetTool : NonCopyable
	{
	public:
		TargetTool(TargetRenderWindow& renderWindow, Undo::UndoManager& undoManager);
		virtual ~TargetTool() = default;

	public:
		virtual void OnSelected() = 0;
		virtual void OnDeselected() = 0;

		virtual void PreRender(Chart& chart, Render::Renderer2D& renderer) = 0;
		virtual void PostRender(Chart& chart, Render::Renderer2D& renderer) = 0;

		virtual void OnContextMenuGUI(Chart& chart) = 0;
		virtual void OnOverlayGUI(Chart& chart) = 0;

		virtual void PreRenderGUI(Chart& chart, ImDrawList& drawList) = 0;
		virtual void PostRenderGUI(Chart& chart, ImDrawList& drawList) = 0;

		virtual void UpdateInput(Chart& chart) = 0;

		virtual const char* GetName() const = 0;

	public:
		static std::array<std::unique_ptr<TargetTool>, EnumCount<TargetToolType>()> CreateAllToolTypes(TargetRenderWindow& renderWindow, Undo::UndoManager& undoManager);

	protected:
		TargetRenderWindow& renderWindow;
		Undo::UndoManager& undoManager;
	};
}
