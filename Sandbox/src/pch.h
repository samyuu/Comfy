#pragma once

#include <stdio.h>
#include <stddef.h>
#include <vector>
#include <array>
#include <string>
#include <thread>
#include <memory>
#include <functional>
#include <algorithm>

#include <stb/stb_image.h>
#include <glad/glad.h>
#include <glfw/glfw3.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_opengl3.h"
#include "ImGui/imgui_extensions.h"
#undef IMGUI_DEFINE_MATH_OPERATORS

using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat3 = glm::mat3;
using mat4 = glm::mat4;