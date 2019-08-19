#pragma once

namespace Editor
{
	class INamedCommand
	{
	public:
		virtual const char* GetName() const = 0;
	};

	class ICommand : public INamedCommand
	{
	public:
		virtual void Do() = 0;
		virtual void Undo() = 0;
		virtual void Redo() = 0;
	};
}