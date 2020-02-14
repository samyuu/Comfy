#pragma once

#include <stdint.h>
#include <glm/gtc/type_ptr.hpp>

#include "Core/SmartPointers.h"

using unk8_t = uint8_t;
using unk16_t = uint16_t;
using unk32_t = uint32_t;
using frame_t = float;

using ivec2 = glm::vec<2, int, glm::defaultp>;
using ivec3 = glm::vec<3, int, glm::defaultp>;
using ivec4 = glm::vec<4, int, glm::defaultp>;

using vec2 = glm::vec<2, float, glm::defaultp>;
using vec3 = glm::vec<3, float, glm::defaultp>;
using vec4 = glm::vec<4, float, glm::defaultp>;

using mat3 = glm::mat<3, 3, float, glm::defaultp>;
using mat4 = glm::mat<4, 4, float, glm::defaultp>;
