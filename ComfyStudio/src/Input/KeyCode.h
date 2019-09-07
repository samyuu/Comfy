#pragma once
#include "Types.h"

typedef int32_t KeyState;
enum KeyState_Enum : KeyState
{
	KeyState_Release = 0,
	KeyState_Press = 1,
	KeyState_Repeat = 2,
};

typedef int32_t KeyCode;
enum KeyCode_Enum : KeyCode
{
	/* The unknown key */
	KeyCode_Unknown            = -1,

	/* Printable keys */
	KeyCode_Space              = 32,
	KeyCode_Apostrophe         = 39,  /* ' */
	KeyCode_Comma              = 44,  /* , */
	KeyCode_Minus              = 45,  /* - */
	KeyCode_Period             = 46,  /* . */
	KeyCode_Slash              = 47,  /* / */
	KeyCode_0                  = 48,
	KeyCode_1                  = 49,
	KeyCode_2                  = 50,
	KeyCode_3                  = 51,
	KeyCode_4                  = 52,
	KeyCode_5                  = 53,
	KeyCode_6                  = 54,
	KeyCode_7                  = 55,
	KeyCode_8                  = 56,
	KeyCode_9                  = 57,
	KeyCode_Semicolon          = 59,  /* ; */
	KeyCode_Equal              = 61,  /* = */
	KeyCode_A                  = 65,
	KeyCode_B                  = 66,
	KeyCode_C                  = 67,
	KeyCode_D                  = 68,
	KeyCode_E                  = 69,
	KeyCode_F                  = 70,
	KeyCode_G                  = 71,
	KeyCode_H                  = 72,
	KeyCode_I                  = 73,
	KeyCode_J                  = 74,
	KeyCode_K                  = 75,
	KeyCode_L                  = 76,
	KeyCode_M                  = 77,
	KeyCode_N                  = 78,
	KeyCode_O                  = 79,
	KeyCode_P                  = 80,
	KeyCode_Q                  = 81,
	KeyCode_R                  = 82,
	KeyCode_S                  = 83,
	KeyCode_T                  = 84,
	KeyCode_U                  = 85,
	KeyCode_V                  = 86,
	KeyCode_W                  = 87,
	KeyCode_X                  = 88,
	KeyCode_Y                  = 89,
	KeyCode_Z                  = 90,
	KeyCode_Left_Bracket       = 91,  /* [ */
	KeyCode_Backslash          = 92,  /* \ */
	KeyCode_Right_Bracket      = 93,  /* ] */
	KeyCode_Grave_Accent       = 96,  /* ` */
	KeyCode_World_1            = 161, /* non-US #1 */
	KeyCode_World_2            = 162, /* non-US #2 */

	/* Function keys */
	KeyCode_Escape             = 256,
	KeyCode_Enter              = 257,
	KeyCode_Tab                = 258,
	KeyCode_Backspace          = 259,
	KeyCode_Insert             = 260,
	KeyCode_Delete             = 261,
	KeyCode_Right              = 262,
	KeyCode_Left               = 263,
	KeyCode_Down               = 264,
	KeyCode_Up                 = 265,
	KeyCode_Page_Up            = 266,
	KeyCode_Page_Down          = 267,
	KeyCode_Home               = 268,
	KeyCode_End                = 269,
	KeyCode_Caps_Lock          = 280,
	KeyCode_Scroll_Lock        = 281,
	KeyCode_Num_Lock           = 282,
	KeyCode_Print_Screen       = 283,
	KeyCode_Pause              = 284,
	KeyCode_F1                 = 290,
	KeyCode_F2                 = 291,
	KeyCode_F3                 = 292,
	KeyCode_F4                 = 293,
	KeyCode_F5                 = 294,
	KeyCode_F6                 = 295,
	KeyCode_F7                 = 296,
	KeyCode_F8                 = 297,
	KeyCode_F9                 = 298,
	KeyCode_F10                = 299,
	KeyCode_F11                = 300,
	KeyCode_F12                = 301,
	KeyCode_F13                = 302,
	KeyCode_F14                = 303,
	KeyCode_F15                = 304,
	KeyCode_F16                = 305,
	KeyCode_F17                = 306,
	KeyCode_F18                = 307,
	KeyCode_F19                = 308,
	KeyCode_F20                = 309,
	KeyCode_F21                = 310,
	KeyCode_F22                = 311,
	KeyCode_F23                = 312,
	KeyCode_F24                = 313,
	KeyCode_F25                = 314,
	KeyCode_KP_0               = 320,
	KeyCode_KP_1               = 321,
	KeyCode_KP_2               = 322,
	KeyCode_KP_3               = 323,
	KeyCode_KP_4               = 324,
	KeyCode_KP_5               = 325,
	KeyCode_KP_6               = 326,
	KeyCode_KP_7               = 327,
	KeyCode_KP_8               = 328,
	KeyCode_KP_9               = 329,
	KeyCode_KP_Decimal         = 330,
	KeyCode_KP_Divide          = 331,
	KeyCode_KP_Multiply        = 332,
	KeyCode_KP_Subtract        = 333,
	KeyCode_KP_Add             = 334,
	KeyCode_KP_Enter           = 335,
	KeyCode_KP_Equal           = 336,
	KeyCode_Left_Shift         = 340,
	KeyCode_Left_Control       = 341,
	KeyCode_Left_Alt           = 342,
	KeyCode_Left_Super         = 343,
	KeyCode_Right_Shift        = 344,
	KeyCode_Right_Control      = 345,
	KeyCode_Right_Alt          = 346,
	KeyCode_Right_Super        = 347,
	KeyCode_Menu               = 348,
	KeyCode_Count,
};

const char* GetKeyCodeName(KeyCode keyCode);