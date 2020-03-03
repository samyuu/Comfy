#pragma once
#include "CoreTypes.h"

namespace Comfy::Editor
{
	class IFileDropReceiver
	{
	public:
		virtual bool OnFileDropped(const std::string& filePath) { return false; };
	};
}
