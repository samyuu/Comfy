#ifndef CHECKERBOARD_HLSL
#define CHECKERBOARD_HLSL

// NOTE: Multiply by color to create a checkerboard pattern
float GetCheckerboardFactor(float2 texCoord, float2 checkboardSize)
{
    // NOTE: Offset the coordinates so we scale towards the top left corner
    texCoord.y = 1.0 - texCoord.y;
    
    float2 result = floor(texCoord * checkboardSize);
    return lerp(0.0, 1.0, fmod(result.x + result.y, 2.0));
}

#endif /* CHECKERBOARD_HLSL */
