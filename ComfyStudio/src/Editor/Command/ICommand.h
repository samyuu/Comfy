#pragma once

namespace Comfy::Editor
{
	class INamedCommand
	{
	public:
		virtual const char* GetName() = 0;

		// TODO: Implement by AetCommands to be disaplyed in the history window, if unspecified return a default gear icon (ICON_FA_COG) (?)
		// virtual const char* GetIcon() = 0;
	};

	class ICommand : public INamedCommand
	{
	public:
		virtual void Do() = 0;
		virtual void Undo() = 0;
		virtual void Redo() = 0;
	};
}
