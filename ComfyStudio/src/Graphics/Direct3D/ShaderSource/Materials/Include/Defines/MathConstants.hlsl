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

#endif /* MATHCONSTANTS_HLSL */
