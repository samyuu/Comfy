#ifndef MATHCONSTANTS_HLSL
#define MATHCONSTANTS_HLSL

static const float TWO_PI = 6.28318530717958647693;
static const float PI = 3.14159265358979323846;
static const float PI_HALF = 1.57079632679489661923;

static const float3x3 FLOAT3x3_IDENTITY =
{
    { 1.0, 0.0, 0.0 },
    { 0.0, 1.0, 0.0 },
    { 0.0, 0.0, 1.0 },
};

static const float4x4 FLOAT4x4_IDENTITY =
{
    { 1.0, 0.0, 0.0, 0.0 },
    { 0.0, 1.0, 0.0, 0.0 },
    { 0.0, 0.0, 1.0, 0.0 },
    { 0.0, 0.0, 0.0, 1.0 },
};

static const float4x4 ScreenDoorThresholdMatrix =
{
     1.0 / 17.0,  9.0 / 17.0,  3.0 / 17.0, 11.0 / 17.0,
    13.0 / 17.0,  5.0 / 17.0, 15.0 / 17.0,  7.0 / 17.0,
     4.0 / 17.0, 12.0 / 17.0,  2.0 / 17.0, 10.0 / 17.0,
    16.0 / 17.0,  8.0 / 17.0, 14.0 / 17.0,  6.0 / 17.0,
};

#endif /* MATHCONSTANTS_HLSL */
