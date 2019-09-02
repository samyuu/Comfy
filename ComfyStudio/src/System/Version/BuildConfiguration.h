#pragma once

#if defined(COMFY_DEBUG)
#define DEBUG_ONLY(expression) expression
#define RELEASE_ONLY(expression)
#define DEBUG_RELEASE_SWITCH(debug, release) debug
#elif defined(COMFY_RELEASE)
#define DEBUG_ONLY(expression)
#define RELEASE_ONLY(expression) expression
#define DEBUG_RELEASE_SWITCH(debug, release) release
#endif /* COMFY_DEBUG / COMFY_RELEASE */

class BuildConfiguration
{
public:
	static constexpr bool Debug = DEBUG_RELEASE_SWITCH(true, false);
	static constexpr bool Release = DEBUG_RELEASE_SWITCH(false, true);
};
