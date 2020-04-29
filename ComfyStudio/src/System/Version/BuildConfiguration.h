#pragma once
#include "CoreMacros.h"

namespace Comfy::BuildConfiguration
{
	static constexpr bool Debug = COMFY_DEBUG_RELEASE_SWITCH(true, false);
	static constexpr bool Release = COMFY_DEBUG_RELEASE_SWITCH(false, true);
};
