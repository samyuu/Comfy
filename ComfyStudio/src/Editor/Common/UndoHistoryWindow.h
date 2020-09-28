#pragma once
#include "Types.h"
#include "Undo/Undo.h"
#include "ImGui/Gui.h"

namespace Comfy::Studio::Editor
{
	class UndoHistoryWindow
	{
	public:
		UndoHistoryWindow(Undo::UndoManager& undoManager);
		~UndoHistoryWindow() = default;

	public:
		bool Gui();

	private:
		bool SingleColumnGui();
		void SingleColumnListBoxGui();

	private:
		bool CommandSelectableGui(const Undo::Command& command, bool selected = false) const;
		bool CommandSelectableGui(std::string_view name, bool selected = false, ImGuiSelectableFlags flags = ImGuiSelectableFlags_None) const;

		void UpdateAutoScroll(size_t undoStackSize);

	private:
		Undo::UndoManager& undoManager;

		bool undoRedoHeaderButtons = false;
		bool thisFrameIsAtBottom = false, lastFrameIsAtBottom = false;
		size_t thisFrameUndoCount = 0, lastFrameUndoCount = 0;
	};
}
