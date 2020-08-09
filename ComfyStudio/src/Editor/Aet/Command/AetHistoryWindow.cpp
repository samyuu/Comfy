#include "AetHistoryWindow.h"
#include "ImGui/Gui.h"

namespace Comfy::Studio::Editor
{
	AetHistoryWindow::AetHistoryWindow(Undo::UndoManager& undoManager) : undoManager(undoManager)
	{
	}

	bool AetHistoryWindow::Gui()
	{
		// TODO: Implement single column, clickable, grayed out if undone, PS-like history window
		Gui::Columns(2, nullptr, false);

		if (Gui::MenuItem("UndoManager::Undo()", nullptr, nullptr, undoManager.CanUndo()))
			undoManager.Undo();
		Gui::NextColumn();

		if (Gui::MenuItem("UndoManager::Redo()", nullptr, nullptr, undoManager.CanRedo()))
			undoManager.Redo();
		Gui::NextColumn();

		if (Gui::ListBoxHeader("##HistoryWindow::UndoListBox", Gui::GetContentRegionAvail()))
		{
			if (undoManager.GetUndoStackView().empty())
			{
				Gui::Selectable("Empty", false, ImGuiSelectableFlags_Disabled);
			}
			else
			{
				// TODO: String view start / end
				for (const auto& undoCommand : undoManager.GetUndoStackView())
					Gui::Selectable(undoCommand->GetName().data());
			}
			Gui::ListBoxFooter();
		}
		Gui::NextColumn();

		if (Gui::ListBoxHeader("##HistoryWindow::RedoListBox", Gui::GetContentRegionAvail()))
		{
			if (undoManager.GetRedoStackView().empty())
			{
				Gui::Selectable("Empty", false, ImGuiSelectableFlags_Disabled);
			}
			else
			{
				for (const auto& redoCommand : undoManager.GetRedoStackView())
					Gui::Selectable(redoCommand->GetName().data());
			}
			Gui::ListBoxFooter();
		}
		Gui::NextColumn();

		Gui::Columns(1);
		return false;
	}
}
