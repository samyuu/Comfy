#pragma once
#include "CoreTypes.h"

namespace Comfy::Studio::Editor
{
	class IFileDropReceiver
	{
	public:
		virtual ~IFileDropReceiver() = default;

		virtual bool OnFileDropped(const std::string& filePath) { return false; }
	};
}
