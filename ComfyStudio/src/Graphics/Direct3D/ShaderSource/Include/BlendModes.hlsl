#ifndef BLENDMODES_HLSL
#define BLENDMODES_HLSL

typedef int BlendMode;

// NOTE: BlendModes
static const BlendMode BlendMode_None = 0;
static const BlendMode BlendMode_Copy = 1;
static const BlendMode BlendMode_Behind = 2;
static const BlendMode BlendMode_Normal = 3;
static const BlendMode BlendMode_Dissolve = 4;
static const BlendMode BlendMode_Add = 5;
static const BlendMode BlendMode_Multiply = 6;
static const BlendMode BlendMode_Screen = 7;
static const BlendMode BlendMode_Overlay = 8;
static const BlendMode BlendMode_SoftLight = 9;
static const BlendMode BlendMode_HardLight = 10;
static const BlendMode BlendMode_Darken = 11;
static const BlendMode BlendMode_Lighten = 12;
static const BlendMode BlendMode_ClassicDifference = 13;
static const BlendMode BlendMode_Hue = 14;
static const BlendMode BlendMode_Saturation = 15;
static const BlendMode BlendMode_Color = 16;
static const BlendMode BlendMode_Luminosity = 17;
static const BlendMode BlendMode_StenciilAlpha = 18;
static const BlendMode BlendMode_StencilLuma = 19;
static const BlendMode BlendMode_SilhouetteAlpha = 20;
static const BlendMode BlendMode_SilhouetteLuma = 21;
static const BlendMode BlendMode_LuminescentPremul = 22;
static const BlendMode BlendMode_AlphaAdd = 23;
static const BlendMode BlendMode_ClassicColorDodge = 24;
static const BlendMode BlendMode_ClassicColorBurn = 25;
static const BlendMode BlendMode_Exclusion = 26;
static const BlendMode BlendMode_Difference = 27;
static const BlendMode BlendMode_ColorDodge = 28;
static const BlendMode BlendMode_ColorBurn = 29;
static const BlendMode BlendMode_LinearDodge = 30;
static const BlendMode BlendMode_LinearBurn = 31;
static const BlendMode BlendMode_LinearLight = 32;
static const BlendMode BlendMode_VividLight = 33;
static const BlendMode BlendMode_PinLight = 34;
static const BlendMode BlendMode_HardMix = 35;
static const BlendMode BlendMode_LighterColor = 36;
static const BlendMode BlendMode_DarkerColor = 37;
static const BlendMode BlendMode_Subtract = 38;
static const BlendMode BlendMode_Divide = 39;

static const float4 COLOR_MASK = { 1.0, 1.0, 1.0, 0.0 };

float4 AdjustMultiplyBlending(float4 inputColor)
{
    return ((inputColor - COLOR_MASK) * inputColor.aaaa) + COLOR_MASK;
}

#endif /* BLENDMODES_HLSL */
