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

		// TEMP TEST:

		if (Gui::Button("EnqueCommand<TestCommand>()")) commandManager->EnqueCommand<TestCommand>();
		if (Gui::Button("EnqueCommand<NameTestCommand>()")) commandManager->EnqueCommand<NameTestCommand>();
		if (Gui::Button("EnqueCommand<NumberTestCommand>()")) commandManager->EnqueCommand<NumberTestCommand>(rand());

		if (Gui::MenuItem("commandManager->Undo()", nullptr, nullptr, commandManager->GetCanUndo()))
			commandManager->Undo();
		if (Gui::MenuItem("commandManager->Redo()", nullptr, nullptr, commandManager->GetCanRedo()))
			commandManager->Redo();

		Gui::BeginChild("UndoStackWindow", vec2(200, 300), true);
		{
			Gui::BulletText("Undo Stack");
			for (auto& undoCommand : commandManager->GetUndoStack())
				Gui::TextUnformatted(undoCommand->GetName());
		}
		Gui::EndChild();

		Gui::SameLine();
		Gui::BeginChild("RedoStackWindow", vec2(200, 300), true);
		{
			Gui::BulletText("Redo Stack");
			for (auto& redoCommand : commandManager->GetRedoStack())
				Gui::TextUnformatted(redoCommand->GetName());
		}
		Gui::EndChild();

		return false;
	}
}