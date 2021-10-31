#pragma once
#include "Types.h"

namespace Comfy::Studio::BuildConfiguration
{
	static constexpr bool Debug = COMFY_DEBUG_RELEASE_SWITCH(true, false);
	static constexpr bool Release = COMFY_DEBUG_RELEASE_SWITCH(false, true);
};
