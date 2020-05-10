#pragma once
#include "Types.h"
#include "Graphics/Auth3D/ObjSet.h"

namespace Comfy::Graphics::D3D11
{
	struct SceneConstantData
	{
		struct IBLData
		{
			std::array<mat4, 3> IrradianceRGB;
			std::array<vec4, 4> LightColors;
		} IBL;

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

		u32 DebugFlags;
		u32 Padding[1];
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
		u32 ShaderFlags;
		u32 DiffuseRGTC1;
		u32 DiffuseScreenTexture;
		u32 AmbientTextureType;
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

	struct RendererConstantBuffers
	{
		DefaultConstantBufferTemplate<SceneConstantData> Scene = { 0, "Renderer3D::SceneCB" };
		DynamicConstantBufferTemplate<ObjectConstantData> Object = { 1, "Renderer3D::ObjectCB" };
		DynamicConstantBufferTemplate<SkeletonConstantData> Skeleton = { 2, "Renderer3D::SkeletonCB" };

		DynamicConstantBufferTemplate<ESMFilterConstantData> ESMFilter = { 4, "Renderer3D::ESMFilterCB" };
		DynamicConstantBufferTemplate<SSSFilterConstantData> SSSFilter = { 5, "Renderer3D::SSSFilterCB" };
		DynamicConstantBufferTemplate<ReduceTexConstantData> ReduceTex = { 6, "Renderer3D::ReduceTexCB" };
		DynamicConstantBufferTemplate<PPGaussTexConstantData> PPGaussTex = { 7, "Renderer3D::PPGaussTexCB" };
		DefaultConstantBufferTemplate<PPGaussCoefConstantData> PPGaussCoef = { 8, "Renderer3D::PPGaussCoefCB" };
		DefaultConstantBufferTemplate<ExposureConstantData> Exposure = { 9, "Renderer3D::ExposureCB" };
		DefaultConstantBufferTemplate<ToneMapConstantData> ToneMap = { 9, "Renderer3D::ToneMapCB" };
	};
}
