#pragma once
#include "Core/CoreTypes.h"

namespace Editor
{
	class IFileDropReceiver
	{
	public:
		virtual bool OnFileDropped(const std::string& filePath) { return false; };
	};
}