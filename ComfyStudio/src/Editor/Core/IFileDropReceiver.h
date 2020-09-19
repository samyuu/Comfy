#pragma once
#include "CoreTypes.h"

namespace Comfy::Studio::Editor
{
	class IFileDropReceiver
	{
	public:
		virtual ~IFileDropReceiver() = default;

	public:
		virtual bool OnFileDropped(std::string_view filePath) { return false; }
	};
}
