#pragma once

#ifdef COMFY_DEBUG  
#define DEBUG_ONLY(expression) expression
#define RELEASE_ONLY(expression)
#define DEBUG_RELEASE(debug, release) debug
#endif
#ifdef COMFY_RELEASE
#define DEBUG_ONLY(expression)
#define RELEASE_ONLY(expression) expression
#define DEBUG_RELEASE(debug, release) release
#endif

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

#define IMGUI_DEFINE_MATH_OPERATORS
#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_opengl3.h"
#include "ImGui/imgui_extensions.h"
#undef IMGUI_DEFINE_MATH_OPERATORS

#include "Types.h"
