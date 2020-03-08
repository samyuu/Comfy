#pragma once
#include "Types.h"
#include "Graphics/Auth3D/ObjSet.h"

namespace Comfy::Graphics
{
	struct SceneConstantData
	{
		mat4 IBLIrradianceRed;
		mat4 IBLIrradianceGreen;
		mat4 IBLIrradianceBlue;

		struct SceneData
		{
			mat4 View;
			mat4 ViewProjection;
			mat4 LightSpace;
			vec4 EyePosition;
		} Scene;

		struct ParallelLight
		{
			vec4 Ambient;
			vec4 Diffuse;
			vec4 Specular;
			vec4 Direction;
		} CharaLight, StageLight;

		vec4 IBLStageColor;
		vec4 IBLCharaColor;
		vec4 IBLSunColor;

		vec4 RenderResolution;

		struct RenderTime
		{
			static constexpr vec4 Scales = vec4(0.5f, 1.0f, 2.0f, 4.0f);
			vec4 Time;
			vec4 TimeSin;
			vec4 TimeCos;
		} RenderTime;

		struct LinearFog
		{
			vec4 Parameters;
			vec4 Color;
		} DepthFog;

		vec4 ShadowAmbient;
		vec4 OneMinusShadowAmbient;
		float ShadowExponent;
		float SubsurfaceScatteringParameter;

		uint32_t DebugFlags;
		uint32_t Padding[1];
		vec4 DebugValue;
	};

	struct ObjectConstantData
	{
		mat4 Model;
		mat4 ModelView;
		mat4 ModelViewProjection;
		struct MaterialData
		{
			mat4 DiffuseTextureTransform;
			mat4 AmbientTextureTransform;
			vec4 FresnelCoefficients;
			vec3 Diffuse;
			float Transparency;
			vec4 Ambient;
			vec3 Specular;
			float Reflectivity;
			vec4 Emission;
			vec2 Shininess;
			float Intensity;
			float BumpDepth;
		} Material;
		vec4 MorphWeight;
		uint32_t ShaderFlags;
		uint32_t DiffuseRGTC1;
		uint32_t DiffuseScreenTexture;
		uint32_t AmbientTextureType;
	};

	struct SkeletonConstantData
	{
		// NOTE: Model space final bone transforms
		std::array<mat4, Skeleton::MaxBoneCount> FinalBoneTransforms;
	};

	struct ESMFilterConstantData
	{
		std::array<vec4, 2> Coefficients;
		vec2 TextureStep;
		vec2 FarTexelOffset;
	};

	struct SSSFilterConstantData
	{
		vec4 TextureSize;
		std::array<vec4, 36> Coefficients;
	};

	struct ReduceTexConstantData
	{
		vec4 TextureSize;
		int ExtractBrightness;
		int CombineBlurred;
		float Padding[2];
	};

	struct PPGaussTexConstantData
	{
		vec4 TextureSize;
		vec4 TextureOffsets;
		int FinalPass;
		int Padding[3];
	};

	struct PPGaussCoefConstantData
	{
		std::array<vec4, 8> Coefficients;
	};

	struct ExposureConstantData
	{
		vec4 SpotWeight;
		std::array<vec4, 32> SpotCoefficients;
	};

	struct ToneMapConstantData
	{
		float Exposure;
		float Gamma;
		float SaturatePower;
		float SaturateCoefficient;
		float AlphaLerp;
		float AlphaValue;
		int AutoExposure;
		int Padding[1];
	};
}
