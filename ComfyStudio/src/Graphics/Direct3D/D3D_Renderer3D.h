#pragma once
#include "Direct3D.h"
#include "D3D_BlendState.h"
#include "D3D_ConstantBuffer.h"
#include "D3D_DepthStencilState.h"
#include "D3D_IndexBuffer.h"
#include "D3D_InputLayout.h"
#include "D3D_RasterizerState.h"
#include "D3D_Shader.h"
#include "D3D_Texture.h"
#include "D3D_VertexBuffer.h"
#include "ShaderBytecode/ShaderBytecode.h"
#include "Graphics/Auth3D/Transform.h"
#include "Graphics/Auth3D/ObjSet.h"
#include "Graphics/Auth3D/ObjAnimationData.h"
#include "Graphics/Auth3D/SceneContext.h"
#include <unordered_map>

namespace Graphics
{
	enum ShaderFlags : uint32_t
	{
		ShaderFlags_VertexColor = 1 << 0,

		ShaderFlags_DiffuseTexture = 1 << 1,
		ShaderFlags_AmbientTexture = 1 << 2,
		ShaderFlags_NormalTexture = 1 << 3,
		ShaderFlags_SpecularTexture = 1 << 4,
		ShaderFlags_TransparencyTexture = 1 << 5,
		ShaderFlags_EnvironmentTexture = 1 << 6,
		ShaderFlags_TranslucencyTexture = 1 << 7,
		ShaderFlags_ScreenTexture = 1 << 8,

		ShaderFlags_PunchThrough = 1 << 9,
		ShaderFlags_ClipPlane = 1 << 10,
		ShaderFlags_LinearFog = 1 << 11,

		ShaderFlags_Morph = 1 << 12,
		ShaderFlags_MorphColor = 1 << 13,
		ShaderFlags_Skinning = 1 << 14,

		ShaderFlags_Shadow = 1 << 15,
		ShaderFlags_ShadowSecondary = 1 << 16,
		ShaderFlags_SelfShadow = 1 << 17,
	};

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
		float Padding[2];
	};

	struct RenderCommand
	{
		const Obj* SourceObj = nullptr;
		const Obj* SourceMorphObj = nullptr;

		Transform Transform = Graphics::Transform(vec3(0.0f));
		const ObjAnimationData* Animation = nullptr;

		struct Flags
		{
			bool IsReflection = false;
			bool SilhouetteOutline = false;

			// TODO:
			bool CastsShadow = false;
			bool ReceivesShadow = true;
			// int ShadowMapIndex = -1;
			// bool SubsurfaceScattering = false;
			// bool SubsurfaceScatteringFocusPoint = false;
			// bool Skeleton = false;

		} Flags;

	public:
		RenderCommand() = default;
		RenderCommand(const Obj& obj) : SourceObj(&obj) {}
		RenderCommand(const Obj& obj, const vec3& position) : SourceObj(&obj), Transform(position) {}
	};

	class D3D_Renderer3D : NonCopyable
	{
	public:
		D3D_Renderer3D();
		~D3D_Renderer3D() = default;

	public:
		void Begin(SceneContext& scene);
		void Draw(const RenderCommand& command);
		void End();

	private:
		void UpdateIsAnyCommandFlags(const RenderCommand& command);

	public:
		void ClearTextureIDs();
		void RegisterTextureIDs(const TxpSet& txpSet);
		void UnRegisterTextureIDs(const TxpSet& txpSet);

	public:
		const SceneContext* GetSceneContext() const;
		std::unordered_map<TxpID, const Txp*>& GetTextureIDTxpMap();

		const Txp* GetTxpFromTextureID(TxpID textureID) const;

	private:
		struct ObjRenderCommand
		{
			RenderCommand SourceCommand;

			// NOTE: To avoid needlessly calculating them multiple times
			mat4 ModelMatrix;
			Sphere TransformedBoundingSphere;

			// NOTE: To avoid needlessly binding buffers during the opaque render pass
			bool AreAllMeshesTransparent;
		};

		struct SubMeshRenderCommand
		{
			ObjRenderCommand* ObjCommand;
			const Mesh* ParentMesh;
			const SubMesh* SubMesh;
			float CameraDistance;
		};

		struct RenderPassCommandLists
		{
			std::vector<ObjRenderCommand> OpaqueAndTransparent;
			std::vector<SubMeshRenderCommand> Transparent;
		};

		using RenderFlags = uint32_t;
		enum RenderFlagsEnum : RenderFlags
		{
			RenderFlags_None = 0,
			RenderFlags_SSSPass = (1 << 0),
			RenderFlags_SilhouetteOutlinePass = (1 << 1),
			RenderFlags_NoMaterialShader = (1 << 2),
			RenderFlags_NoMaterialTextures = (1 << 3),
			RenderFlags_DiffuseTextureOnly = (1 << 4),
			RenderFlags_NoRasterizerState = (1 << 5),
			RenderFlags_NoDoFrustumCulling = (1 << 6),
		};

		void InternalFlush();

		void InternalPrepareRenderCommands(RenderPassCommandLists& commandList);
		void InternalRenderScene();
		void InternalPreRenderShadowMap();
		void InternalPreRenderReduceFilterShadowMap();
		void InternalPreRenderScreenReflection();
		void InternalPreRenderSubsurfaceScattering();
		void InternalPreRenderReduceFilterSubsurfaceScattering();

		// TODO: Add wrapper function to loop over commnad list and add opaque and transparent to RenderFlags
		void InternalRenderOpaqueObjCommand(ObjRenderCommand& command, RenderFlags flags = RenderFlags_None);
		void InternalRenderTransparentSubMeshCommand(SubMeshRenderCommand& command);
		void InternalRenderSilhouette();
		void InternalRenderSilhouetteOutlineOverlay();
		void InternalRenderPostProcessing();
		void InternalRenderBloom();

		void BindMeshVertexBuffers(const Mesh& primaryMesh, const Mesh* morphMesh);
		void PrepareAndRenderSubMesh(const ObjRenderCommand& command, const Mesh& mesh, const SubMesh& subMesh, const Material& material, RenderFlags flags = RenderFlags_None);
		D3D_ShaderPair& GetMaterialShader(const Material& material);
		D3D_ShaderPair& GetSSSMaterialShader(const Material& material);
		void SubmitSubMeshDrawCall(const SubMesh& subMesh);

		Sphere CalculateShadowViewFrustumSphere() const;

		bool IntersectsCameraFrustum(const ObjRenderCommand& command) const;
		bool IntersectsCameraFrustum(const Sphere& boundingSphere, const ObjRenderCommand& command) const;
		bool IntersectsCameraFrustum(const Sphere& boundingSphere, const SubMeshRenderCommand& command) const;

		bool IsDebugRenderFlagSet(int bitIndex) const;

	private:
		struct ShaderPairs
		{
			D3D_ShaderPair Debug = { Debug_VS(), Debug_PS(), "Renderer3D::Debug" };
			D3D_ShaderPair SilhouetteOutline = { FullscreenQuad_VS(), SilhouetteOutline_PS(), "Renderer3D::SilhouetteOutline" };

			D3D_ShaderPair BlinnPerFrag = { BlinnPerFrag_VS(), BlinnPerFrag_PS(), "Renderer3D::BlinnPerFrag" };
			D3D_ShaderPair BlinnPerVert = { BlinnPerVert_VS(), BlinnPerVert_PS(), "Renderer3D::BlinnPerVert" };
			D3D_ShaderPair ClothAniso = { ClothDefault_VS(), ClothAniso_PS(), "Renderer3D::ClothAniso" };
			D3D_ShaderPair ClothDefault = { ClothDefault_VS(), ClothDefault_PS(), "Renderer3D::ClothDefault" };
			D3D_ShaderPair Constant = { Constant_VS(), Constant_PS(), "Renderer3D::Constant" };
			D3D_ShaderPair DepthThreshold = { FullscreenQuad_VS(), DepthThreshold_PS(), "Renderer3D::DepthThreshold" };
			D3D_ShaderPair EyeBall = { EyeBall_VS(), EyeBall_PS(), "Renderer3D::EyeBall" };
			D3D_ShaderPair EyeLens = { EyeLens_VS(), EyeLens_PS(), "Renderer3D::EyeLens" };
			D3D_ShaderPair Floor = { Floor_VS(), Floor_PS(), "Renderer3D::Floor" };
			D3D_ShaderPair GlassEye = { GlassEye_VS(), GlassEye_PS(), "Renderer3D::GlassEye" };
			D3D_ShaderPair HairAniso = { HairDefault_VS(), HairAniso_PS(), "Renderer3D::HairAniso" };
			D3D_ShaderPair HairDefault = { HairDefault_VS(), HairDefault_PS(), "Renderer3D::HairDefault" };
			D3D_ShaderPair ESMFilterMin = { FullscreenQuad_VS(), ESMFilterMin_PS(), "Renderer3D::ESMFilterMin" };
			D3D_ShaderPair ESMFilterErosion = { FullscreenQuad_VS(), ESMFilterErosion_PS(), "Renderer3D::ESMFilterErosion" };
			D3D_ShaderPair ESMGauss = { FullscreenQuad_VS(), ESMGauss_PS(), "Renderer3D::ESMGauss" };
			D3D_ShaderPair ExposureMinify = { FullscreenQuad_VS(), ExposureMinify_PS(), "Renderer3D::ExposureMinify" };
			D3D_ShaderPair ExposureMeasure = { FullscreenQuad_VS(), ExposureMeasure_PS(), "Renderer3D::ExposureMeasure" };
			D3D_ShaderPair ExposureAverage = { FullscreenQuad_VS(), ExposureAverage_PS(), "Renderer3D::ExposureAverage" };
			D3D_ShaderPair ImgFilter = { FullscreenQuad_VS(), ImgFilter_PS(), "Renderer3D::ImgFilter" };
			D3D_ShaderPair ImgFilterBlur = { FullscreenQuad_VS(), ImgFilterBlur_PS(), "Renderer3D::ImgFilterBlur" };
			D3D_ShaderPair ItemBlinn = { ItemBlinn_VS(), ItemBlinn_PS(), "Renderer3D::ItemBlinn" };
			D3D_ShaderPair Lambert = { Lambert_VS(), Lambert_PS(), "Renderer3D::Lambert" };
			D3D_ShaderPair Silhouette = { Silhouette_VS(), Silhouette_PS(), "Renderer3D::Silhouette" };
			D3D_ShaderPair PPGauss = { FullscreenQuad_VS(), PPGauss_PS(), "Renderer3D::PPGauss" };
			D3D_ShaderPair ReduceTex = { FullscreenQuad_VS(), ReduceTex_PS(), "Renderer3D::ReduceTex" };
			D3D_ShaderPair SkinDefault = { SkinDefault_VS(), SkinDefault_PS(), "Renderer3D::SkinDefault" };
			D3D_ShaderPair SkyDefault = { SkyDefault_VS(), SkyDefault_PS(), "Renderer3D::SkyDefault" };
			D3D_ShaderPair SolidBlack = { PositionTransform_VS(), SolidBlack_PS(), "Renderer3D::SolidBlack" };
			D3D_ShaderPair SolidWhite = { PositionTransform_VS(), SolidWhite_PS(), "Renderer3D::SolidWhite" };
			D3D_ShaderPair SSSFilterCopy = { FullscreenQuad_VS(), SSSFilterCopy_PS(), "Renderer3D::SSSFilterCopy" };
			D3D_ShaderPair SSSFilterMin = { FullscreenQuad_VS(), SSSFilterMin_PS(), "Renderer3D::SSSFilterMin" };
			D3D_ShaderPair SSSFilterGauss2D = { FullscreenQuad_VS(), SSSFilterGauss2D_PS(), "Renderer3D::SSSFilterGauss2D" };
			D3D_ShaderPair SSSSkin = { SSSSkin_VS(), SSSSkin_PS(), "Renderer3D::SSSSkin" };
			D3D_ShaderPair SSSSkinConst = { PositionTransform_VS(), SSSSkinConst_PS(), "Renderer3D::SSSSkinConst" };
			D3D_ShaderPair StageBlinn = { StageBlinn_VS(), StageBlinn_PS(), "Renderer3D::StageBlinn" };
			D3D_ShaderPair Tights = { Tights_VS(), Tights_PS(), "Renderer3D::Tights" };
			D3D_ShaderPair ToneMap = { FullscreenQuad_VS(), ToneMap_PS(), "Renderer3D::ToneMap" };
			D3D_ShaderPair Water = { Water_VS(), Water_PS(), "Renderer3D::Water" };
		} shaders;

		// TODO: Separate scene CB from ~~viewport~~ camera CB (camera view/projection/eye, SSS param & render resolution)
		D3D_DefaultConstantBufferTemplate<SceneConstantData> sceneCB = { 0, "Renderer3D::SceneCB" };
		D3D_DynamicConstantBufferTemplate<ObjectConstantData> objectCB = { 1, "Renderer3D::ObjectCB" };
		D3D_DynamicConstantBufferTemplate<ESMFilterConstantData> esmFilterCB = { 3, "Renderer3D::ESMFilterCB" };
		D3D_DynamicConstantBufferTemplate<SSSFilterConstantData> sssFilterCB = { 5, "Renderer3D::SSSFilterCB" };
		D3D_DynamicConstantBufferTemplate<ReduceTexConstantData> reduceTexCB = { 6, "Renderer3D::ReduceTexCB" };
		D3D_DynamicConstantBufferTemplate<PPGaussTexConstantData> ppGaussTexCB = { 7, "Renderer3D::PPGaussTexCB" };
		D3D_DefaultConstantBufferTemplate<PPGaussCoefConstantData> ppGaussCoefCB = { 8, "Renderer3D::PPGaussCoefCB" };
		D3D_DefaultConstantBufferTemplate<ToneMapConstantData> toneMapCB = { 9, "Renderer3D::ToneMapCB" };

		UniquePtr<D3D_InputLayout> genericInputLayout = nullptr;
		UniquePtr<D3D_InputLayout> shadowSilhouetteInputLayout = nullptr;

		D3D_RasterizerState solidBackfaceCullingRasterizerState = { D3D11_FILL_SOLID, D3D11_CULL_BACK, "Renderer3D::SolidBackfaceCulling" };
		D3D_RasterizerState solidFrontfaceCullingRasterizerState = { D3D11_FILL_SOLID, D3D11_CULL_FRONT, "Renderer3D::SolidFrontfaceCulling" };
		D3D_RasterizerState solidNoCullingRasterizerState = { D3D11_FILL_SOLID, D3D11_CULL_NONE, "Renderer3D::SolidNoCulling" };
		D3D_RasterizerState wireframeRasterizerState = { D3D11_FILL_WIREFRAME, D3D11_CULL_NONE, "Renderer3D::Wireframe" };

		D3D_DepthStencilState transparencyPassDepthStencilState = { true, D3D11_DEPTH_WRITE_MASK_ZERO, "Renderer3D::Transparency" };

		struct TextureSamplerCache
		{
		public:
			void CreateIfNeeded(const RenderParameters& renderParameters);
			D3D_TextureSampler& GetSampler(MaterialTextureFlags flags);

		private:
			enum AddressMode { Mirror, Repeat, Clamp, AddressMode_Count };

			int32_t lastAnistropicFiltering = -1;
			std::array<std::array<UniquePtr<D3D_TextureSampler>, AddressMode_Count>, AddressMode_Count> samplers;

		} cachedTextureSamplers;

		struct BlendStateCache
		{
		public:
			BlendStateCache();
			D3D_BlendState& GetState(BlendFactor source, BlendFactor destination);

		private:
			std::array<std::array<UniquePtr<D3D_BlendState>, BlendFactor_Count>, BlendFactor_Count> states;

		} cachedBlendStates;

		// NOTE: To avoid having to bind and clear render targets that won't be used this frame
		struct IsAnyCommandFlags
		{
			bool ScreenReflection;
			bool SubsurfaceScattering;
			bool CastShadow;
			bool ReceiveShadow;
			bool SilhouetteOutline;
		} isAnyCommand = {};

		RenderPassCommandLists defaultCommandList, reflectionCommandList;

		struct Statistics
		{
			size_t VerticesRendered = 0;

			// TODO: Implement different statistics
			// size_t ObjectsRendered = 0, MeshesRendered = 0, SubMeshesRendered = 0;
			// size_t ObjectsCulled = 0, MeshesCulled = 0, SubMeshesCulled = 0;
		} statistics, lastFrameStatistics;

		struct ToneMapData
		{
			GlowParameter Glow;

			std::array<vec2, 512> TextureData;
			UniquePtr<D3D_Texture1D> LookupTexture = nullptr;

		public:
			bool NeedsUpdating(const SceneContext* sceneContext);
			void Update();

		private:
			void GenerateLookupData();
			void UpdateTexture();

		} toneMapData;

		SceneContext* sceneContext = nullptr;
		RenderParameters* renderParameters = nullptr;
		RenderData* renderData = nullptr;

		std::unordered_map<TxpID, const Txp*> textureIDTxpMap = {};
	};
}
