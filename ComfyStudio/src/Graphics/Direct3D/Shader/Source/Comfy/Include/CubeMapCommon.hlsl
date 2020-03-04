#ifndef CUBEMAPCOMMON_HLSL
#define CUBEMAPCOMMON_HLSL

#include "../../Materials/Include/Defines/MathConstants.hlsl"

bool IsInRange(const float input, const float rangeMin, const float rangeMax)
{
	return input >= rangeMin && input <= rangeMax;
}

bool IsInRangeSize(const float input, const float rangeMin, const float rangeSize)
{
	return input >= rangeMin && input <= (rangeMin + rangeSize);
}

static const int CubeMapFaces = 6;

static const float2 CubeMapNetDivisions = float2(4.0, 3.0);
static const float2 CubeMapNetFaceSize = 1.0 / CubeMapNetDivisions;

// NOTE: CubeMap net layout:
// .... [+Y] .... ....
// [-X] [+Z] [+X] [-Z]
// .... [-Y] .... ....
static const float2 CubeMapNetFaceRanges[CubeMapFaces] = 
{
	// NOTE: [2][1] +X
	float2(2.0, 1.0) * CubeMapNetFaceSize,
	// NOTE: [0][1] -X
	float2(0.0, 1.0) * CubeMapNetFaceSize,

	// NOTE: [1][0] +Y
	float2(1.0, 0.0) * CubeMapNetFaceSize,
	// NOTE: [1][2] -Y
	float2(1.0, 2.0) * CubeMapNetFaceSize,

	// NOTE: [1][2] +Z
	float2(1.0, 1.0) * CubeMapNetFaceSize,
	// NOTE: [3][1] -Z
	float2(3.0, 1.0) * CubeMapNetFaceSize,
};

// NOTE: Convert the input range [TL] (0.0, 0.0) [BR] (1.0, 1.0) to a float3 CubeMap coordinate direction
float3 GetCubeMapTextureCoordinates(float2 inputTexCoord)
{
    float2 latitudeLongitude = mad(inputTexCoord, 2.0, 1.0) * float2(PI, PI_HALF);

	float4 cosSin = float4(cos(latitudeLongitude), sin(latitudeLongitude));
	float3 cartesianCoord = float3(cosSin.zx * cosSin.y, cosSin.w);
	
	return cartesianCoord;
}

float3 GetCubeMapFaceTextureCoordinates(float2 inputTexCoord, int face)
{
	inputTexCoord = mad(inputTexCoord, 2.0, -1.0);

	const float3 faceTextureCoords[CubeMapFaces] = 
	{
        float3(+1.0, -inputTexCoord.y, +inputTexCoord.x),
        float3(-1.0, -inputTexCoord.y, +inputTexCoord.x),
        
        float3(+inputTexCoord.x, +1.0, +inputTexCoord.y),
        float3(+inputTexCoord.x, -1.0, -inputTexCoord.y),
        
        float3(+inputTexCoord.x, -inputTexCoord.y, +1.0),
        float3(-inputTexCoord.x, -inputTexCoord.y, -1.0),
	};

	return faceTextureCoords[face];
}

float3 GetCubeMapNetTextureCoordinates(float2 inputTexCoord)
{
	for (int i = 0; i < CubeMapFaces; i++)
	{
		float2 range = CubeMapNetFaceRanges[i];

		if (IsInRangeSize(inputTexCoord.x, range.x, CubeMapNetFaceSize.x) && IsInRangeSize(inputTexCoord.y, range.y, CubeMapNetFaceSize.y))
		{
			float2 remappedTexCoord = (inputTexCoord - range) * CubeMapNetDivisions;
			return GetCubeMapFaceTextureCoordinates(remappedTexCoord, i);
		}
	}

	return float3(0.0, 0.0, 0.0);
}

#endif /* CUBEMAPCOMMON_HLSL */
