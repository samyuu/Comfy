#pragma once
#include "Editor/Command/ICommand.h"

namespace Editor
{
	class AetCommand : public ICommand
	{
	};

	class TestCommand : public AetCommand
	{
	public:
		virtual void Do() override;
		virtual void Undo() override;
		virtual void Redo() override;
		virtual const char* GetName() const override;
	};

	class NameTestCommand : public AetCommand
	{
	public:
		virtual void Do() override;
		virtual void Undo() override;
		virtual void Redo() override;
		virtual const char* GetName() const override;
	};

	class NumberTestCommand : public AetCommand
	{
		int number = 0;

	public:
		NumberTestCommand(int n);

		virtual void Do() override;
		virtual void Undo() override;
		virtual void Redo() override;
		virtual const char* GetName() const override;
	};
}