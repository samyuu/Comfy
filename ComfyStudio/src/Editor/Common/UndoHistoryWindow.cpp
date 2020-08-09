#include "UndoHistoryWindow.h"
#include "ImGui/Gui.h"

namespace Comfy::Studio::Editor
{
	UndoHistoryWindow::UndoHistoryWindow(Undo::UndoManager& undoManager) : undoManager(undoManager)
	{
	}

	bool UndoHistoryWindow::Gui()
	{
#if COMFY_DEBUG // DEBUG:
		if (Gui::IsWindowFocused() && !Gui::GetIO().WantCaptureKeyboard && Gui::IsKeyPressed('M'))
			activeDisplayType = ((activeDisplayType = static_cast<DisplayType>(static_cast<int>(activeDisplayType) + 1)) == DisplayType::Count) ? static_cast<DisplayType>(0) : activeDisplayType;
#endif

		switch (activeDisplayType)
		{
		case DisplayType::SingleColumn:
			return SingleColumnGui();

		case DisplayType::SideBySideColumn:
			return SideBySideColumnGui();

		default:
			assert(false);
			return false;
		}
	}

	bool UndoHistoryWindow::SingleColumnGui()
	{
		Gui::Columns(2, nullptr, false);
		if (Gui::MenuItem("UndoManager::Undo()", nullptr, nullptr, undoManager.CanUndo()))
			undoManager.Undo();
		Gui::NextColumn();

		if (Gui::MenuItem("UndoManager::Redo()", nullptr, nullptr, undoManager.CanRedo()))
			undoManager.Redo();
		Gui::NextColumn();
		Gui::Columns(1);

		SingleColumnListBoxGui("##HistoryWindow::UndoRedoListBox");
		return false;
	}

	void UndoHistoryWindow::SingleColumnListBoxGui(const char* headerName)
	{
		if (Gui::ListBoxHeader(headerName, Gui::GetContentRegionAvail()))
		{
			const auto& undoStack = undoManager.GetUndoStackView();
			const auto& redoStack = undoManager.GetRedoStackView();

			if (undoStack.empty() && redoStack.empty())
				Gui::Selectable("Empty", false, ImGuiSelectableFlags_Disabled);

			int undoClickedIndex = -1, redoClickedIndex = -1;

			for (int i = 0; i < static_cast<int>(undoStack.size()); i++)
			{
				Gui::PushID(undoStack[i].get());
				if (Gui::Selectable(GetCommandName(*undoStack[i])))
					undoClickedIndex = i;
				Gui::PopID();
			}

			Gui::PushStyleColor(ImGuiCol_Text, Gui::GetStyleColorVec4(ImGuiCol_TextDisabled));
			for (int i = static_cast<int>(redoStack.size()) - 1; i >= 0; i--)
			{
				Gui::PushID(redoStack[i].get());
				if (Gui::Selectable(GetCommandName(*redoStack[i])))
					redoClickedIndex = i;
				Gui::PopID();
			}
			Gui::PopStyleColor();

			if (InBounds(undoClickedIndex, undoStack))
				undoManager.Undo(undoStack.size() - undoClickedIndex - 1);
			else if (InBounds(redoClickedIndex, redoStack))
				undoManager.Redo(redoStack.size() - redoClickedIndex);

			Gui::ListBoxFooter();
		}
	}

	bool UndoHistoryWindow::SideBySideColumnGui()
	{
		Gui::Columns(2, nullptr, false);

		if (Gui::MenuItem("UndoManager::Undo()", nullptr, nullptr, undoManager.CanUndo()))
			undoManager.Undo();
		Gui::NextColumn();

		if (Gui::MenuItem("UndoManager::Redo()", nullptr, nullptr, undoManager.CanRedo()))
			undoManager.Redo();
		Gui::NextColumn();

		CommandStackListBoxGui("##HistoryWindow::UndoListBox", undoManager.GetUndoStackView());
		Gui::NextColumn();

		CommandStackListBoxGui("##HistoryWindow::RedoListBox", undoManager.GetRedoStackView());
		Gui::NextColumn();

		Gui::Columns(1);
		return false;
	}

	void UndoHistoryWindow::CommandStackListBoxGui(const char* headerName, const std::vector<std::unique_ptr<Undo::ICommand>>& stackView)
	{
		if (Gui::ListBoxHeader(headerName, Gui::GetContentRegionAvail()))
		{
			if (stackView.empty())
			{
				Gui::Selectable("Empty", false, ImGuiSelectableFlags_Disabled);
			}
			else
			{
				for (const auto& command : stackView)
					Gui::Selectable(GetCommandName(*command));
			}
			Gui::ListBoxFooter();
		}
	}

	const char* UndoHistoryWindow::GetCommandName(const Undo::ICommand& command) const
	{
		// TODO: String view start / end although since commands only ever return views to string literals this works fine for now
		return command.GetName().data();
	}
}
