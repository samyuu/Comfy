#ifndef BLENDMODES_HLSL
#define BLENDMODES_HLSL

// NOTE: BlendModes
static const int BlendMode_None = 0;
static const int BlendMode_Copy = 1;
static const int BlendMode_Behind = 2;
static const int BlendMode_Normal = 3;
static const int BlendMode_Dissolve = 4;
static const int BlendMode_Add = 5;
static const int BlendMode_Multiply = 6;
static const int BlendMode_Screen = 7;
static const int BlendMode_Overlay = 8;
static const int BlendMode_SoftLight = 9;
static const int BlendMode_HardLight = 10;
static const int BlendMode_Darken = 11;
static const int BlendMode_Lighten = 12;
static const int BlendMode_ClassicDifference = 13;
static const int BlendMode_Hue = 14;
static const int BlendMode_Saturation = 15;
static const int BlendMode_Color = 16;
static const int BlendMode_Luminosity = 17;
static const int BlendMode_StenciilAlpha = 18;
static const int BlendMode_StencilLuma = 19;
static const int BlendMode_SilhouetteAlpha = 20;
static const int BlendMode_SilhouetteLuma = 21;
static const int BlendMode_LuminescentPremul = 22;
static const int BlendMode_AlphaAdd = 23;
static const int BlendMode_ClassicColorDodge = 24;
static const int BlendMode_ClassicColorBurn = 25;
static const int BlendMode_Exclusion = 26;
static const int BlendMode_Difference = 27;
static const int BlendMode_ColorDodge = 28;
static const int BlendMode_ColorBurn = 29;
static const int BlendMode_LinearDodge = 30;
static const int BlendMode_LinearBurn = 31;
static const int BlendMode_LinearLight = 32;
static const int BlendMode_VividLight = 33;
static const int BlendMode_PinLight = 34;
static const int BlendMode_HardMix = 35;
static const int BlendMode_LighterColor = 36;
static const int BlendMode_DarkerColor = 37;
static const int BlendMode_Subtract = 38;
static const int BlendMode_Divide = 39;

static const float4 COLOR_MASK = { 1.0, 1.0, 1.0, 0.0 };

float4 AdjustMultiplyBlending(float4 inputColor)
{
    return ((inputColor - COLOR_MASK) * inputColor.aaaa) + COLOR_MASK;
}

#endif /* BLENDMODES_HLSL */
