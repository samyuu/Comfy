#include "UndoHistoryWindow.h"
#include "ImGui/Gui.h"

namespace Comfy::Studio::Editor
{
	UndoHistoryWindow::UndoHistoryWindow(Undo::UndoManager& undoManager) : undoManager(undoManager)
	{
	}

	bool UndoHistoryWindow::Gui()
	{
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
				CommandSelectableGui("Empty", false, ImGuiSelectableFlags_Disabled);

			int undoClickedIndex = -1, redoClickedIndex = -1;

			for (int i = 0; i < static_cast<int>(undoStack.size()); i++)
			{
				const auto selected = ((i + 1) == undoStack.size());

				if (CommandSelectableGui(*undoStack[i], selected))
					undoClickedIndex = i;
			}

			Gui::PushStyleColor(ImGuiCol_Text, Gui::GetStyleColorVec4(ImGuiCol_TextDisabled));
			for (int i = static_cast<int>(redoStack.size()) - 1; i >= 0; i--)
			{
				if (CommandSelectableGui(*redoStack[i]))
					redoClickedIndex = i;
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
				CommandSelectableGui("Empty", false, ImGuiSelectableFlags_Disabled);
			}
			else
			{
				for (const auto& command : stackView)
					CommandSelectableGui(*command);
			}
			Gui::ListBoxFooter();
		}
	}

	bool UndoHistoryWindow::CommandSelectableGui(const Undo::ICommand& command, bool selected) const
	{
		Gui::PushID(&command);
		const auto clicked = CommandSelectableGui(command.GetName(), selected);
		Gui::PopID();

		return clicked;
	}

	bool UndoHistoryWindow::CommandSelectableGui(std::string_view name, bool selected, ImGuiSelectableFlags flags) const
	{
		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(0.0f, Gui::GetStyle().ItemSpacing.y));

		const auto clicked = Gui::Selectable("##CommandSelectable", selected, flags);
		Gui::SameLine();

		Gui::PopStyleVar();

		if (flags & ImGuiSelectableFlags_Disabled)
			Gui::PushStyleColor(ImGuiCol_Text, Gui::GetStyleColorVec4(ImGuiCol_TextDisabled));

		Gui::TextUnformatted(Gui::StringViewStart(name), Gui::StringViewEnd(name));

		if (flags & ImGuiSelectableFlags_Disabled)
			Gui::PopStyleColor();

		return clicked;
	}
}
