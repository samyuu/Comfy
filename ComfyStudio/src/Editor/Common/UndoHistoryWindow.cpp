#include "UndoHistoryWindow.h"
#include "ImGui/Gui.h"

namespace Comfy::Studio::Editor
{
	UndoHistoryWindow::UndoHistoryWindow(Undo::UndoManager& undoManager) : undoManager(undoManager)
	{
	}

	bool UndoHistoryWindow::Gui()
	{
		// TODO: Implement single column, clickable, grayed out if undone, PS-like history window
		Gui::Columns(2, nullptr, false);

		if (Gui::MenuItem("UndoManager::Undo()", nullptr, nullptr, undoManager.CanUndo()))
			undoManager.Undo();
		Gui::NextColumn();

		if (Gui::MenuItem("UndoManager::Redo()", nullptr, nullptr, undoManager.CanRedo()))
			undoManager.Redo();
		Gui::NextColumn();

		auto commandStackListBoxGui = [](const char* headerName, const std::vector<std::unique_ptr<Undo::ICommand>>& stackView)
		{
			if (Gui::ListBoxHeader(headerName, Gui::GetContentRegionAvail()))
			{
				if (stackView.empty())
				{
					Gui::Selectable("Empty", false, ImGuiSelectableFlags_Disabled);
				}
				else
				{
					// TODO: String view start / end although since commands only ever return views to string literals this works fine for now
					for (const auto& command : stackView)
						Gui::Selectable(command->GetName().data());
				}
				Gui::ListBoxFooter();
			}
			Gui::NextColumn();
		};

		commandStackListBoxGui("##HistoryWindow::UndoListBox", undoManager.GetUndoStackView());
		commandStackListBoxGui("##HistoryWindow::RedoListBox", undoManager.GetRedoStackView());

		Gui::Columns(1);
		return false;
	}
}
