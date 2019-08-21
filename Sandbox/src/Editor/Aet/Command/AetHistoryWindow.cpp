#include "AetHistoryWindow.h"
#include "ImGui/Gui.h"

namespace Editor
{
	AetHistoryWindow::AetHistoryWindow(AetCommandManager* commandManager) : IMutableAetEditorComponent(commandManager)
	{
	}

	bool AetHistoryWindow::DrawGui()
	{
		AetCommandManager* commandManager = GetCommandManager();

		Gui::Columns(2, nullptr, false);

		if (Gui::MenuItem("AetCommandManager::Undo()", nullptr, nullptr, commandManager->GetCanUndo()))
			commandManager->Undo();
		Gui::NextColumn();

		if (Gui::MenuItem("AetCommandManager::Redo()", nullptr, nullptr, commandManager->GetCanRedo()))
			commandManager->Redo();
		Gui::NextColumn();

		if (Gui::ListBoxHeader("##AetHistoryWindow::UndoListBox", Gui::GetContentRegionAvail()))
		{
			if (commandManager->GetUndoStack().empty())
			{
				Gui::Selectable("Empty", false, ImGuiSelectableFlags_Disabled);
			}
			else
			{
				for (auto& undoCommand : commandManager->GetUndoStack())
					Gui::Selectable(undoCommand->GetName());
			}
			Gui::ListBoxFooter();
		}
		Gui::NextColumn();

		if (Gui::ListBoxHeader("##AetHistoryWindow::RedoListBox", Gui::GetContentRegionAvail()))
		{
			if (commandManager->GetRedoStack().empty())
			{
				Gui::Selectable("Empty", false, ImGuiSelectableFlags_Disabled);
			}
			else
			{
				for (auto& redoCommand : commandManager->GetRedoStack())
					Gui::Selectable(redoCommand->GetName());
			}
			Gui::ListBoxFooter();
		}
		Gui::NextColumn();

		Gui::Columns(1);
		return false;
	}
}