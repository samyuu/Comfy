#pragma once
#include "Types.h"
#include "Undo/Undo.h"

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
		void CommandStackListBoxGui(const char* headerName, const std::vector<std::unique_ptr<Undo::ICommand>>& stackView);

	private:
		const char* GetCommandName(const Undo::ICommand& command) const;

	private:
		Undo::UndoManager& undoManager;
		DisplayType activeDisplayType = DisplayType::SingleColumn;
	};
}
