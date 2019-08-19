#include "AetCommand.h"
#include "Core/Logger.h"
#include <string>

namespace Editor
{
	void TestCommand::Do() { Logger::LogLine(__FUNCTION__"()"); }
	void TestCommand::Redo() { Logger::LogLine(__FUNCTION__"()"); }
	void TestCommand::Undo() { Logger::LogLine(__FUNCTION__"()"); }
	const char* TestCommand::GetName() const { return "TEST"; }

	void NameTestCommand::Do() { Logger::LogLine(__FUNCTION__"()"); }
	void NameTestCommand::Redo() { Logger::LogLine(__FUNCTION__"()"); }
	void NameTestCommand::Undo() { Logger::LogLine(__FUNCTION__"()"); }
	const char* NameTestCommand::GetName() const { return "NAME"; }
	
	NumberTestCommand::NumberTestCommand(int n) : number(n) {}
	void NumberTestCommand::Do() { Logger::LogLine(__FUNCTION__"()"); }
	void NumberTestCommand::Redo() { Logger::LogLine(__FUNCTION__"()"); }
	void NumberTestCommand::Undo() { Logger::LogLine(__FUNCTION__"()"); }
	const char* NumberTestCommand::GetName() const { static std::string str(" ", 32); sprintf_s(str.data(), str.size(), "NUMBER: %d", number); return str.c_str(); }
}