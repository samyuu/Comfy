#pragma once

#include <stdint.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include "Core/SmartPointers.h"

typedef uint8_t unk8_t;
typedef uint16_t unk16_t;
typedef uint32_t unk32_t;
typedef float frame_t;

using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat3 = glm::mat3;
using mat4 = glm::mat4;

#if defined(COMFY_DEBUG)
#define DEBUG_ONLY(expression) expression
#define RELEASE_ONLY(expression)
#define DEBUG_RELEASE(debug, release) debug
#elif defined(COMFY_RELEASE)
#define DEBUG_ONLY(expression)
#define RELEASE_ONLY(expression) expression
#define DEBUG_RELEASE(debug, release) release
#endif /* COMFY_DEBUG / COMFY_RELEASE */
