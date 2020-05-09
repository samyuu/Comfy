#pragma once
#include "Core/ScopeExitFunc.h"
#include <assert.h>

#define COMFY_NODISCARD [[nodiscard]]

#define COMFY_CONCAT_DETAIL(x,y) x##y
#define COMFY_CONCAT(x,y) COMFY_CONCAT_DETAIL(x,y)

#define COMFY_STRINGIFY_DETAIL(value) #value
#define COMFY_STRINGIFY(value) COMFY_STRINGIFY_DETAIL(value)

#define COMFY_UNIQUENAME(prefix) COMFY_CONCAT(prefix, __COUNTER__)

// NOTE: Example: COMFY_SCOPE_EXIT([&] { DoCleanup(); });
#define COMFY_SCOPE_EXIT(lambda) auto COMFY_UNIQUENAME(__SCOPE_EXIT) = ::Comfy::ScopeExitFunc(lambda)

#if defined(COMFY_DEBUG)
#define COMFY_DEBUG_ONLY(expression) expression
#define COMFY_RELEASE_ONLY(expression)
#define COMFY_DEBUG_RELEASE_SWITCH(debug, release) (debug)
#elif defined(COMFY_RELEASE)
#define COMFY_DEBUG_ONLY(expression)
#define COMFY_RELEASE_ONLY(expression) expression
#define COMFY_DEBUG_RELEASE_SWITCH(debug, release) (release)
#endif /* COMFY_DEBUG / COMFY_RELEASE */
