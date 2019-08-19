#pragma once
#include <string>

namespace Editor
{
	class IFileDropReceiver
	{
	public:
		virtual bool OnFileDropped(const std::string& filePath) { return false; };
	};
}