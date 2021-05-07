#include "UndoHistoryWindow.h"
#include "ImGui/Gui.h"

namespace Comfy::Studio::Editor
{
	UndoHistoryWindow::UndoHistoryWindow(Undo::UndoManager& undoManager) : undoManager(undoManager)
	{
	}

	bool UndoHistoryWindow::Gui()
	{
		return SingleColumnGui();
	}

	bool UndoHistoryWindow::SingleColumnGui()
	{
		if (undoRedoHeaderButtons)
		{
			Gui::Columns(2, nullptr, false);
			if (Gui::MenuItem("UndoManager::Undo()", nullptr, nullptr, undoManager.CanUndo()))
				undoManager.Undo();
			Gui::NextColumn();

			if (Gui::MenuItem("UndoManager::Redo()", nullptr, nullptr, undoManager.CanRedo()))
				undoManager.Redo();
			Gui::NextColumn();
			Gui::Columns(1);
		}

		if (Gui::BeginListBox("##HistoryWindow::UndoRedoListBox", Gui::GetContentRegionAvail()))
		{
			SingleColumnListBoxGui();
			Gui::EndListBox();
		}

		return false;
	}

	void UndoHistoryWindow::SingleColumnListBoxGui()
	{
		const auto& undoStack = undoManager.GetUndoStackView();
		const auto& redoStack = undoManager.GetRedoStackView();

		if (CommandSelectableGui("Initial State", undoStack.empty()))
			undoManager.Undo(undoStack.size());

		int undoClickedIndex = -1, redoClickedIndex = -1;

		ImGuiListClipper undoClipper; undoClipper.Begin(static_cast<int>(undoStack.size()));
		while (undoClipper.Step())
		{
			for (int i = undoClipper.DisplayStart; i < undoClipper.DisplayEnd; i++)
			{
				const auto selected = ((i + 1) == undoStack.size());
				if (CommandSelectableGui(*undoStack[i], selected))
					undoClickedIndex = i;
			}
		}

		Gui::PushStyleColor(ImGuiCol_Text, Gui::GetStyleColorVec4(ImGuiCol_TextDisabled));
		ImGuiListClipper redoClipper; redoClipper.Begin(static_cast<int>(redoStack.size()));
		while (redoClipper.Step())
		{
			for (int i = redoClipper.DisplayStart; i < redoClipper.DisplayEnd; i++)
			{
				const auto redoIndex = ((static_cast<int>(redoStack.size()) - 1) - i);
				if (CommandSelectableGui(*redoStack[redoIndex]))
					redoClickedIndex = redoIndex;
			}
		}
		Gui::PopStyleColor();

		if (InBounds(undoClickedIndex, undoStack))
			undoManager.Undo(undoStack.size() - undoClickedIndex - 1);
		else if (InBounds(redoClickedIndex, redoStack))
			undoManager.Redo(redoStack.size() - redoClickedIndex);

		UpdateAutoScroll(undoStack.size());
	}

	bool UndoHistoryWindow::CommandSelectableGui(const Undo::Command& command, bool selected) const
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

	void UndoHistoryWindow::UpdateAutoScroll(size_t undoStackSize)
	{
		constexpr auto autoScrollThreshold = 64.0f;

		lastFrameIsAtBottom = thisFrameIsAtBottom;
		thisFrameIsAtBottom = (Gui::GetScrollY() >= (Gui::GetScrollMaxY() - autoScrollThreshold));

		lastFrameUndoCount = thisFrameUndoCount;
		thisFrameUndoCount = undoStackSize;

#if 0 // NOTE: Too easy to accidentally prevent autoscrolling after having resized the window for example
		if ((thisFrameIsAtBottom && lastFrameIsAtBottom) && (thisFrameUndoCount != lastFrameUndoCount))
#else
		if (thisFrameUndoCount != lastFrameUndoCount)
#endif
		{
			// BUG: Minor issue with item spacing for the last selectable, the window scroll height seems to be a few pixels too large when using list clippers
			// Gui::SetScrollHereY(1.0f);
			Gui::SetScrollY(Gui::GetScrollMaxY() + autoScrollThreshold);
		}
	}
}
