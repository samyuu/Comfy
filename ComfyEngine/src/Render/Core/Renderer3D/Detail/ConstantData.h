#pragma once
#include "Types.h"
#include "Graphics/Auth3D/ObjSet.h"
#include "Render/D3D11/Buffer/ConstantBuffer.h"

namespace Comfy::Render::Detail
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
		mat4 ModelInverse;
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
		std::array<mat4, Graphics::Skeleton::MaxBoneCount> FinalBoneTransforms;
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
		D3D11::DefaultConstantBufferTemplate<SceneConstantData> Scene = { 0, "Renderer3D::SceneCB" };
		D3D11::DynamicConstantBufferTemplate<ObjectConstantData> Object = { 1, "Renderer3D::ObjectCB" };
		D3D11::DynamicConstantBufferTemplate<SkeletonConstantData> Skeleton = { 2, "Renderer3D::SkeletonCB" };

		D3D11::DynamicConstantBufferTemplate<ESMFilterConstantData> ESMFilter = { 4, "Renderer3D::ESMFilterCB" };
		D3D11::DynamicConstantBufferTemplate<SSSFilterConstantData> SSSFilter = { 5, "Renderer3D::SSSFilterCB" };
		D3D11::DynamicConstantBufferTemplate<ReduceTexConstantData> ReduceTex = { 6, "Renderer3D::ReduceTexCB" };
		D3D11::DynamicConstantBufferTemplate<PPGaussTexConstantData> PPGaussTex = { 7, "Renderer3D::PPGaussTexCB" };
		D3D11::DefaultConstantBufferTemplate<PPGaussCoefConstantData> PPGaussCoef = { 8, "Renderer3D::PPGaussCoefCB" };
		D3D11::DefaultConstantBufferTemplate<ExposureConstantData> Exposure = { 9, "Renderer3D::ExposureCB" };
		D3D11::DefaultConstantBufferTemplate<ToneMapConstantData> ToneMap = { 9, "Renderer3D::ToneMapCB" };
	};
}
