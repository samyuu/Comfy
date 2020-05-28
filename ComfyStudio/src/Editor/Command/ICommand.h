#pragma once

namespace Comfy::Studio::Editor
{
	class ICommand
	{
	public:
		virtual ~ICommand() = default;

	public:
		virtual void Do() = 0;
		virtual void Undo() = 0;
		virtual void Redo() = 0;

		virtual const char* GetName() = 0;
	};
}
