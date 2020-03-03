#pragma once
#include "IntegralTypes.h"
#include "FileAddr.h"
#include "Core/NonCopyable.h"
#include "Core/SmartPointers.h"
#include <glm/gtc/type_ptr.hpp>

// NOTE: Measure a frame unit / frame rate
using frame_t = float;

using ivec2 = glm::vec<2, int, glm::defaultp>;
using ivec3 = glm::vec<3, int, glm::defaultp>;
using ivec4 = glm::vec<4, int, glm::defaultp>;

using vec2 = glm::vec<2, float, glm::defaultp>;
using vec3 = glm::vec<3, float, glm::defaultp>;
using vec4 = glm::vec<4, float, glm::defaultp>;

using mat3 = glm::mat<3, 3, float, glm::defaultp>;
using mat4 = glm::mat<4, 4, float, glm::defaultp>;
