#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "SortedTargetList.h"
#include <optional>

namespace Comfy::Studio::Editor
{
	class ClipboardHelper : NonCopyable
	{
	public:
		void TimelineCopySelectedTargets(const std::vector<TimelineTarget>& targets);
		std::optional<std::vector<TimelineTarget>> TimelineTryGetPasteTargets();

	private:
		std::string_view GetClipboardText();
		void SetClipboardText(const std::string& text);

	private:
		std::string buffer;
	};
}
