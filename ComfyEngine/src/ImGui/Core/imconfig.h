//-----------------------------------------------------------------------------
// COMPILE-TIME OPTIONS FOR DEAR IMGUI
// Runtime options (clipboard callbacks, enabling various features, etc.) can generally be set via the ImGuiIO structure.
// You can use ImGui::SetAllocatorFunctions() before calling ImGui::CreateContext() to rewire memory allocation functions.
//-----------------------------------------------------------------------------
// A) You may edit imconfig.h (and not overwrite it when updating imgui, or maintain a patch/branch with your modifications to imconfig.h)
// B) or add configuration directives in your own file and compile with #define IMGUI_USER_CONFIG "myfilename.h"
// If you do so you need to make sure that configuration settings are defined consistently _everywhere_ dear imgui is used, which include
// the imgui*.cpp files but also _any_ of your code that uses imgui. This is because some compile-time options have an affect on data structures.
// Defining those options in imconfig.h will ensure every compilation unit gets to see the same data structure layouts.
// Call IMGUI_CHECKVERSION() from your .cpp files to verify that the data structures your files are using are matching the ones imgui.cpp is using.
//-----------------------------------------------------------------------------

#pragma once

//---- Define assertion handler. Defaults to calling assert().
#ifdef COMFY_DEBUG
#define IM_ASSERT(_EXPR)	assert(_EXPR)
#endif
#ifdef COMFY_RELEASE
#define IM_ASSERT(_EXPR)	((void)(_EXPR))     // Disable asserts
#endif

//---- Define attributes of all API symbols declarations, e.g. for DLL under Windows.
//#define IMGUI_API __declspec( dllexport )
//#define IMGUI_API __declspec( dllimport )

//---- Don't define obsolete functions/enums names. Consider enabling from time to time after updating to avoid using soon-to-be obsolete function/names.
#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS

//---- Don't implement demo windows functionality (ShowDemoWindow()/ShowStyleEditor()/ShowUserGuide() methods will be empty)
//---- It is very strongly recommended to NOT disable the demo windows during development. Please read the comments in imgui_demo.cpp.
#ifdef COMFY_RELEASE
#define IMGUI_DISABLE_DEMO_WINDOWS
#endif

//---- Don't implement some functions to reduce linkage requirements.
//#define IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS   // [Win32] Don't implement default clipboard handler. Won't use and link with OpenClipboard/GetClipboardData/CloseClipboard etc.
//#define IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS         // [Win32] Don't implement default IME handler. Won't use and link with ImmGetContext/ImmSetCompositionWindow.
//#define IMGUI_DISABLE_WIN32_FUNCTIONS                     // [Win32] Won't use and link with any Win32 function.
//#define IMGUI_DISABLE_FORMAT_STRING_FUNCTIONS             // Don't implement ImFormatString/ImFormatStringV so you can implement them yourself if you don't want to link with vsnprintf.
//#define IMGUI_DISABLE_MATH_FUNCTIONS                      // Don't implement ImFabs/ImSqrt/ImPow/ImFmod/ImCos/ImSin/ImAcos/ImAtan2 wrapper so you can implement them yourself. Declare your prototypes in imconfig.h.
//#define IMGUI_DISABLE_DEFAULT_ALLOCATORS                  // Don't implement default allocators calling malloc()/free() to avoid linking with them. You will need to call ImGui::SetAllocatorFunctions().

//---- Include imgui_user.h at the end of imgui.h as a convenience
#define IMGUI_INCLUDE_IMGUI_USER_H

//---- Pack colors to BGRA8 instead of RGBA8 (to avoid converting from one to another)
//#define IMGUI_USE_BGRA_PACKED_COLOR

//---- Avoid multiple STB libraries implementations, or redefine path/filenames to prioritize another version
// By default the embedded implementations are declared static and not available outside of imgui cpp files.
//#define IMGUI_STB_TRUETYPE_FILENAME   "my_folder/stb_truetype.h"
//#define IMGUI_STB_RECT_PACK_FILENAME  "my_folder/stb_rect_pack.h"
//#define IMGUI_DISABLE_STB_TRUETYPE_IMPLEMENTATION
//#define IMGUI_DISABLE_STB_RECT_PACK_IMPLEMENTATION

//---- Define constructor and implicit cast operators to convert back<>forth between your math types and ImVec2/ImVec4.
// This will be inlined as part of ImVec2 and ImVec4 class declarations.
/**/
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#define IM_VEC2_CLASS_EXTRA                                                 \
		ImVec2(const glm::vec2& f) { x = f.x; y = f.y; }                    \
		operator glm::vec2() const { return glm::vec2(x, y); }

#define IM_VEC4_CLASS_EXTRA                                                 \
		ImVec4(const glm::vec4& f) { x = f.x; y = f.y; z = f.z; w = f.w; }	\
		operator glm::vec4() const { return glm::vec4(x, y, z, w); }
/**/

//---- Use 32-bit vertex indices (default is 16-bit) to allow meshes with more than 64K vertices. Render function needs to support it.
//#define ImDrawIdx unsigned int

//---- Comfy specific ImGui patches, moved outside here to make version updates easier and cleaner

// HACK: Added 2019/07/30: To improve general usability from what's expected from a typical application
#define IMGUI_HACKS_SRC_CODE_PATCH_IMGUI_CPP_UPDATE_MOUSE_MOVING_WINDOW_END_FRAME_ADDITIONAL_WINDOW_FOCUS_MOUSE_CLICK_CHECK \
		|| g.IO.MouseClicked[2]

// HACK: Added 2020/01/01: To improve type safety and store additional data. Bypassing the original ImTextureID macro check for better IDE support
#include "ImGui/ComfyTextureID.h"
#define IMGUI_HACKS_SRC_CODE_PATCH_IMGUI_H_CUSTOM_IMTEXTUREID_TYPEDEF \
		using ImTextureID = ::Comfy::ComfyTextureID;

// HACK: Added 2020/05/25: To avoid creating windows that are too small to move by dragging the title bar if io.ConfigViewportsNoAutoMerge is set
#define IMGUI_HACKS_SRC_CODE_PATCH_IMGUI_CPP_CREATENEWWINDOW_ASSIGN_DEFAULT_MIN_WINDOW_SIZE \
		window->Size = window->SizeFull = ImVec2(320.0f, 240.0f);

// HACK: Added 2020/10/03: To be able to delay loading fonts with a full character set
#define IMGUI_HACKS_SRC_CODE_PATCH_IMGUI_H_IMFONT_ADDITIONAL_MEMBER_FIELDS \
		mutable bool MissingGlyphEncountered = false;
#define IMGUI_HACKS_SRC_CODE_PATCH_IMGUI_DRAW_CPP_IMFONT_FINDGLYPH_ON_NOT_FOUND \
		MissingGlyphEncountered = true;
/**/
