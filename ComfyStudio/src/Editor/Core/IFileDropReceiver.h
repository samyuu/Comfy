#pragma once
#include "CoreTypes.h"

namespace Editor
{
	class IFileDropReceiver
	{
	public:
		virtual bool OnFileDropped(const std::string& filePath) { return false; };
	};
}