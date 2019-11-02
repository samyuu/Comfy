#pragma once
#include "Types.h"

namespace Graphics
{
	enum class PrimitiveType : uint32_t
	{
		// NOTE: GL_POINTS
		Points = 0,
		// NOTE: GL_LINES
		Lines = 1,
		// NOTE: GL_LINE_STRIP
		LineStrip = 2,
		// NOTE: GL_LINE_LOOP
		LineLoop = 3,
		// NOTE: GL_TRIANGLES
		Triangles = 4,
		// NOTE: GL_TRIANGLE_STRIP
		TriangleStrip = 5,
		// NOTE: GL_TRIANGLE_FAN
		TriangleFan = 6,
		// NOTE: GL_QUADS
		Quads = 7,
		// NOTE: GL_QUAD_STRIP
		QuadStrip = 8,
		// NOTE: GL_POLYGON
		Polygon = 9,

		Count
	};

	enum class AetBlendMode : uint8_t
	{
		Unknown = 0,

		// NOTE: Unsupported (COPY)
		Copy = 1,
		// NOTE: Unsupported (BEHIND)
		Behind = 2,

		// NOTE: Supported (IN_FRONT)
		Normal = 3,

		// NOTE: Unsupported (DISSOLVE)
		Dissolve = 4,

		// NOTE: Supported (ADD)
		Add = 5,
		// NOTE: Supported (MULTIPLY)
		Multiply = 6,
		// NOTE: Supported (SCREEN)
		Screen = 7,

		// NOTE: Unsupported (OVERLAY)
		Overlay = 8,
		// NOTE: Unsupported (SOFT_LIGHT)
		SoftLight = 9,
		// NOTE: Unsupported (HARD_LIGHT)
		HardLight = 10,
		// NOTE: Unsupported (DARKEN)
		Darken = 11,
		// NOTE: Unsupported (LIGHTEN)
		Lighten = 12,
		// NOTE: Unsupported (DIFFERENCE)
		ClassicDifference = 13,
		// NOTE: Unsupported (HUE)
		Hue = 14,
		// NOTE: Unsupported (SATURATION)
		Saturation = 15,
		// NOTE: Unsupported (COLOR)
		Color = 16,
		// NOTE: Unsupported (LUMINOSITY)
		Luminosity = 17,
		// NOTE: Unsupported (MULTIPLY_ALPHA)
		StenciilAlpha = 18,
		// NOTE: Unsupported (MULTIPLY_ALPHA_LUMA)
		StencilLuma = 19,
		// NOTE: Unsupported (MULTIPLY_NOT_ALPHA)
		SilhouetteAlpha = 20,
		// NOTE: Unsupported (MULTIPLY_NOT_ALPHA_LUMA)
		SilhouetteLuma = 21,
		// NOTE: Unsupported (ADDITIVE_PREMUL)
		LuminescentPremul = 22,
		// NOTE: Unsupported (ALPHA_ADD)
		AlphaAdd = 23,
		// NOTE: Unsupported (COLOR_DODGE)
		ClassicColorDodge = 24,
		// NOTE: Unsupported (COLOR_BURN)
		ClassicColorBurn = 25,
		// NOTE: Unsupported (EXCLUSION)
		Exclusion = 26,
		// NOTE: Unsupported (DIFFERENCE2)
		Difference = 27,
		// NOTE: Unsupported (COLOR_DODGE2)
		ColorDodge = 28,
		// NOTE: Unsupported (COLOR_BURN2)
		ColorBurn = 29,
		// NOTE: Unsupported (LINEAR_DODGE)
		LinearDodge = 30,
		// NOTE: Unsupported (LINEAR_BURN)
		LinearBurn = 31,
		// NOTE: Unsupported (LINEAR_LIGHT)
		LinearLight = 32,
		// NOTE: Unsupported (VIVID_LIGHT)
		VividLight = 33,
		// NOTE: Unsupported (PIN_LIGHT)
		PinLight = 34,
		// NOTE: Unsupported (HARD_MIX)
		HardMix = 35,
		// NOTE: Unsupported (LIGHTER_COLOR)
		LighterColor = 36,
		// NOTE: Unsupported (DARKER_COLOR)
		DarkerColor = 37,
		// NOTE: Unsupported (SUBTRACT)
		Subtract = 38,
		// NOTE: Unsupported (DIVIDE)
		Divide = 39,

		Count
	};

	enum class TextureFormat : int32_t
	{
		Unknown = -1,
		// NOTE: GL_ALPHA8
		A8 = 0,
		// NOTE: GL_RGB8
		RGB8 = 1,
		// NOTE: GL_RGBA8
		RGBA8 = 2,
		// NOTE: GL_RGB5
		RGB5 = 3,
		// NOTE: GL_RGB5_A1
		RGB5_A1 = 4,
		// NOTE: GL_RGBA4
		RGBA4 = 5,
		// NOTE: GL_COMPRESSED_RGB_S3TC_DXT1_EXT
		DXT1 = 6,
		// NOTE: GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
		DXT1a = 7,
		// NOTE: GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
		DXT3 = 8,
		// NOTE: GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
		DXT5 = 9,
		// NOTE: GL_COMPRESSED_RED_RGTC1
		RGTC1 = 10,
		// NOTE: GL_COMPRESSED_RG_RGTC2
		RGTC2 = 11,
		// NOTE: GL_LUMINANCE8
		L8 = 12,
		// NOTE: GL_LUMINANCE8_ALPHA8
		L8A8 = 13,
		
		Count
	};

	enum class DisplayMode : uint32_t
	{
		// NOTE:  320 x  240
		QVGA = 0,
		// NOTE:  640 x  480
		VGA = 1,
		// NOTE:  800 x  600
		SVGA = 2,
		// NOTE: 1024 x  768
		XGA = 3,
		// NOTE: 1280 x 1024
		SXGA = 4,
		// NOTE: 1400 x 1050
		SXGA_PLUS = 5,
		// NOTE: 1600 x 1200
		UXGA = 6,
		// NOTE:  800 x  480
		WVGA = 7,
		// NOTE: 1024 x  600
		WSVGA = 8,
		// NOTE: 1280 x  768
		WXGA = 9,
		// NOTE: 1360 x  768
		WXGA_ = 10,
		// NOTE: 1920 x 1200
		WUXGA = 11,
		// NOTE: 2560 x 1536
		WQXGA = 12,
		// NOTE: 1280 x  720
		HDTV720 = 13,
		// NOTE: 1920 x 1080
		HDTV1080 = 14,
		// NOTE: 2560 x 1440
		WQHD = 15,
		// NOTE:  480 x  272
		HVGA = 16,
		// NOTE:  960 x  544
		qHD = 17,
		// NOTE: ____ x ____
		Custom = 18,
	};
}