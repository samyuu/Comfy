#ifndef CHECKERBOARD_HLSL
#define CHECKERBOARD_HLSL

// NOTE: Multiply by color to create a checkerboard pattern
float GetCheckerboardFactor(float2 texCoord, float2 checkboardSize)
{
    // TODO: Is this necessary for D3D?
	// NOTE: Offset the coordinates so we scale towards the top left corner
    float2 result = floor((texCoord - float2(0.0, 1.0)) * checkboardSize);
    return lerp(1.0, 0.0, fmod(result.x + result.y, 2.0));
}

#endif /* CHECKERBOARD_HLSL */
