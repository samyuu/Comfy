#pragma once
#include "Types.h"
#include "Undo/Undo.h"
#include "ImGui/Gui.h"

namespace Comfy::Studio::Editor
{
	class UndoHistoryWindow
	{
	public:
		enum class DisplayType
		{
			SingleColumn,
			SideBySideColumn,
			Count
		};

		UndoHistoryWindow(Undo::UndoManager& undoManager);
		~UndoHistoryWindow() = default;

	public:
		bool Gui();

	private:
		bool SingleColumnGui();
		void SingleColumnListBoxGui(const char* headerName);

	private:
		bool SideBySideColumnGui();
		void CommandStackListBoxGui(const char* headerName, const std::vector<std::unique_ptr<Undo::Command>>& stackView);

	private:
		bool CommandSelectableGui(const Undo::Command& command, bool selected = false) const;
		bool CommandSelectableGui(std::string_view name, bool selected = false, ImGuiSelectableFlags flags = ImGuiSelectableFlags_None) const;

	private:
		Undo::UndoManager& undoManager;
		DisplayType activeDisplayType = DisplayType::SingleColumn;
	};
}
