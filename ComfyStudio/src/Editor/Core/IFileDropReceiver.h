#pragma once
#include "Core/CoreTypes.h"

namespace Editor
{
	class IFileDropReceiver
	{
	public:
		virtual bool OnFileDropped(const String& filePath) { return false; };
	};
}