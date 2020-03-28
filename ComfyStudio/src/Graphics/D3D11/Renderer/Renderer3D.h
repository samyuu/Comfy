#pragma once
#include "../Direct3D.h"
#include "../Buffer/ConstantBuffer.h"
#include "../Buffer/IndexBuffer.h"
#include "../Buffer/VertexBuffer.h"
#include "../Shader/Shader.h"
#include "../Shader/Bytecode/ShaderBytecode.h"
#include "../State/BlendState.h"
#include "../State/DepthStencilState.h"
#include "../State/InputLayout.h"
#include "../State/OcclusionQuery.h"
#include "../State/RasterizerState.h"
#include "../Texture/Texture.h"
#include "Detail/ConstantData.h"
#include "Detail/ShaderFlags.h"
#include "Detail/BlendStateCache.h"
#include "Detail/TextureSamplerCache.h"
#include "Detail/ToneMapData.h"
#include "Graphics/Auth3D/Transform.h"
#include "Graphics/Auth3D/ObjSet.h"
#include "Graphics/Auth3D/ObjAnimationData.h"
#include "Graphics/Auth3D/SceneContext.h"
#include <functional>

namespace Comfy::Graphics
{
	struct RenderCommand
	{
		const Obj* SourceObj = nullptr;
		const Obj* SourceMorphObj = nullptr;

		Transform Transform = Graphics::Transform(vec3(0.0f));
		const ObjAnimationData* Animation = nullptr;

		struct CommandFlags
		{
			bool IsReflection = false;
			bool SilhouetteOutline = false;

			bool CastsShadow = false;
			bool ReceivesShadow = true;

			// TODO:
			// int ShadowMapIndex = -1;
			// bool SubsurfaceScattering = false;
			// bool SubsurfaceScatteringFocusPoint = false;
			// bool Skeleton = false;

			// NOTE: Optionally render the specified subset of the SourceObj instead
			int MeshIndex = -1;
			int SubMeshIndex = -1;

		} Flags;

	public:
		RenderCommand() = default;
		RenderCommand(const Obj& obj) : SourceObj(&obj) {}
		RenderCommand(const Obj& obj, const vec3& position) : SourceObj(&obj), Transform(position) {}
	};
}

namespace Comfy::Graphics::D3D11
{
	using TxpGetterFunction = std::function<const Txp*(const Cached_TxpID* txpID)>;

	class Renderer3D : NonCopyable
	{
	public:
		Renderer3D(TxpGetterFunction txpGetter);
		~Renderer3D() = default;

	public:
		void Begin(SceneViewport& viewport, const SceneParameters& scene);
		void Draw(const RenderCommand& command);
		void End();

	private:
		void UpdateIsAnyCommandFlags(const RenderCommand& command);

	public:
		const Txp* GetTxpFromTextureID(const Cached_TxpID* textureID) const;

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
			RenderFlags_NoFrustumCulling = (1 << 6),
		};

		void InternalFlush();

		void InternalPrepareRenderCommands(RenderPassCommandLists& commandList);
		void InternalRenderScene();
		void InternalSetUploadSceneCB();
		void InternalBindSceneTextures();

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

		void InternalQueryRenderLensFlare();
		void InternalRenderLensFlare();

		void InternalRenderPostProcessing();
		void InternalRenderBloom();

		void InternalRenderExposurePreBloom();
		void InternalRenderExposurePostBloom();

		void BindMeshVertexBuffers(const Mesh& primaryMesh, const Mesh* morphMesh);
		void PrepareAndRenderSubMesh(const ObjRenderCommand& command, const Mesh& mesh, const SubMesh& subMesh, const Material& material, RenderFlags flags = RenderFlags_None);

		uint32_t MaterialTextureTypeToTextureSlot(MaterialTextureType textureType, bool secondColorMap);
		uint32_t BindMaterialTextures(const ObjRenderCommand& command, const Material& material, RenderFlags flags);
		bool GetIsTextureSlotUsed(Material::ShaderTypeIdentifier shaderType, Material::MaterialUsedTextureFlags usedTextureFlags, uint32_t textureSlot);
		void SetSubMeshRasterizerState(const Material& material);
		void SetObjectCBMaterialData(const Material& material, ObjectConstantData::MaterialData& outMaterialData) const;
		void SetObjectCBTransforms(const ObjRenderCommand& command, const Mesh& mesh, const SubMesh& subMesh, ObjectConstantData& outData) const;
		vec4 GetObjectCBMorphWeight(const ObjRenderCommand& command) const;
		uint32_t GetObjectCBShaderFlags(const ObjRenderCommand& command, const Mesh& mesh, const SubMesh& subMesh, const Material& material, uint32_t boundMaterialTexturesFlags) const;

		ShaderPair& GetMaterialShader(const ObjRenderCommand& command, const Mesh& mesh, const SubMesh& subMesh, const Material& material);
		ShaderPair& GetSSSMaterialShader(const Material& material);

		void SubmitSubMeshDrawCall(const SubMesh& subMesh);

		Sphere CalculateShadowViewFrustumSphere() const;

		bool IntersectsCameraFrustum(const ObjRenderCommand& command) const;
		bool IntersectsCameraFrustum(const Sphere& boundingSphere, const ObjRenderCommand& command) const;
		bool IntersectsCameraFrustum(const Sphere& boundingSphere, const SubMeshRenderCommand& command) const;

		bool IsDebugRenderFlagSet(int bitIndex) const;

	private:
		struct ShaderPairs
		{
			ShaderPair DebugMaterial = { DebugMaterial_VS(), DebugMaterial_PS(), "Renderer3D::DebugMaterial" };
			ShaderPair SilhouetteOutline = { FullscreenQuad_VS(), SilhouetteOutline_PS(), "Renderer3D::SilhouetteOutline" };

			ShaderPair BlinnPerFrag = { BlinnPerFrag_VS(), BlinnPerFrag_PS(), "Renderer3D::BlinnPerFrag" };
			ShaderPair BlinnPerVert = { BlinnPerVert_VS(), BlinnPerVert_PS(), "Renderer3D::BlinnPerVert" };
			ShaderPair ClothAniso = { ClothDefault_VS(), ClothAniso_PS(), "Renderer3D::ClothAniso" };
			ShaderPair ClothDefault = { ClothDefault_VS(), ClothDefault_PS(), "Renderer3D::ClothDefault" };
			ShaderPair Constant = { Constant_VS(), Constant_PS(), "Renderer3D::Constant" };
			ShaderPair DepthThreshold = { FullscreenQuad_VS(), DepthThreshold_PS(), "Renderer3D::DepthThreshold" };
			ShaderPair EyeBall = { EyeBall_VS(), EyeBall_PS(), "Renderer3D::EyeBall" };
			ShaderPair EyeLens = { EyeLens_VS(), EyeLens_PS(), "Renderer3D::EyeLens" };
			ShaderPair Floor = { Floor_VS(), Floor_PS(), "Renderer3D::Floor" };
			ShaderPair GlassEye = { GlassEye_VS(), GlassEye_PS(), "Renderer3D::GlassEye" };
			ShaderPair HairAniso = { HairDefault_VS(), HairAniso_PS(), "Renderer3D::HairAniso" };
			ShaderPair HairDefault = { HairDefault_VS(), HairDefault_PS(), "Renderer3D::HairDefault" };
			ShaderPair ESMFilterMin = { FullscreenQuad_VS(), ESMFilterMin_PS(), "Renderer3D::ESMFilterMin" };
			ShaderPair ESMFilterErosion = { FullscreenQuad_VS(), ESMFilterErosion_PS(), "Renderer3D::ESMFilterErosion" };
			ShaderPair ESMGauss = { FullscreenQuad_VS(), ESMGauss_PS(), "Renderer3D::ESMGauss" };
			ShaderPair ExposureMinify = { FullscreenQuad_VS(), ExposureMinify_PS(), "Renderer3D::ExposureMinify" };
			ShaderPair ExposureMeasure = { FullscreenQuad_VS(), ExposureMeasure_PS(), "Renderer3D::ExposureMeasure" };
			ShaderPair ExposureAverage = { FullscreenQuad_VS(), ExposureAverage_PS(), "Renderer3D::ExposureAverage" };
			ShaderPair ImgFilter = { FullscreenQuad_VS(), ImgFilter_PS(), "Renderer3D::ImgFilter" };
			ShaderPair ImgFilterBlur = { FullscreenQuad_VS(), ImgFilterBlur_PS(), "Renderer3D::ImgFilterBlur" };
			ShaderPair ItemBlinn = { ItemBlinn_VS(), ItemBlinn_PS(), "Renderer3D::ItemBlinn" };
			ShaderPair Lambert = { Lambert_VS(), Lambert_PS(), "Renderer3D::Lambert" };
			ShaderPair LensFlare = { LensFlare_VS(), LensFlare_PS(), "Renderer3D::LensFlare" };
			ShaderPair Silhouette = { Silhouette_VS(), Silhouette_PS(), "Renderer3D::Silhouette" };
			ShaderPair PPGauss = { FullscreenQuad_VS(), PPGauss_PS(), "Renderer3D::PPGauss" };
			ShaderPair ReduceTex = { FullscreenQuad_VS(), ReduceTex_PS(), "Renderer3D::ReduceTex" };
			ShaderPair SkinDefault = { SkinDefault_VS(), SkinDefault_PS(), "Renderer3D::SkinDefault" };
			ShaderPair SkyDefault = { SkyDefault_VS(), SkyDefault_PS(), "Renderer3D::SkyDefault" };
			ShaderPair SolidBlack = { PositionTransform_VS(), SolidBlack_PS(), "Renderer3D::SolidBlack" };
			ShaderPair SolidWhite = { PositionTransform_VS(), SolidWhite_PS(), "Renderer3D::SolidWhite" };
			ShaderPair SSSFilterCopy = { FullscreenQuad_VS(), SSSFilterCopy_PS(), "Renderer3D::SSSFilterCopy" };
			ShaderPair SSSFilterMin = { FullscreenQuad_VS(), SSSFilterMin_PS(), "Renderer3D::SSSFilterMin" };
			ShaderPair SSSFilterGauss2D = { FullscreenQuad_VS(), SSSFilterGauss2D_PS(), "Renderer3D::SSSFilterGauss2D" };
			ShaderPair SSSSkin = { SSSSkin_VS(), SSSSkin_PS(), "Renderer3D::SSSSkin" };
			ShaderPair SSSSkinConst = { PositionTransform_VS(), SSSSkinConst_PS(), "Renderer3D::SSSSkinConst" };
			ShaderPair StageBlinn = { StageBlinn_VS(), StageBlinn_PS(), "Renderer3D::StageBlinn" };
			ShaderPair Sun = { LensFlare_VS(), Sun_PS(), "Renderer3D::Sun" };
			ShaderPair Tights = { Tights_VS(), Tights_PS(), "Renderer3D::Tights" };
			ShaderPair ToneMap = { FullscreenQuad_VS(), ToneMap_PS(), "Renderer3D::ToneMap" };
			ShaderPair Water = { Water_VS(), Water_PS(), "Renderer3D::Water" };
		} shaders;

		// TODO: Separate scene CB from ~~viewport~~ camera CB (camera view/projection/eye, SSS param & render resolution)
		DefaultConstantBufferTemplate<SceneConstantData> sceneCB = { 0, "Renderer3D::SceneCB" };
		DynamicConstantBufferTemplate<ObjectConstantData> objectCB = { 1, "Renderer3D::ObjectCB" };
		DynamicConstantBufferTemplate<SkeletonConstantData> skeletonCB = { 2, "Renderer3D::SkeletonCB" };

		DynamicConstantBufferTemplate<ESMFilterConstantData> esmFilterCB = { 4, "Renderer3D::ESMFilterCB" };
		DynamicConstantBufferTemplate<SSSFilterConstantData> sssFilterCB = { 5, "Renderer3D::SSSFilterCB" };
		DynamicConstantBufferTemplate<ReduceTexConstantData> reduceTexCB = { 6, "Renderer3D::ReduceTexCB" };
		DynamicConstantBufferTemplate<PPGaussTexConstantData> ppGaussTexCB = { 7, "Renderer3D::PPGaussTexCB" };
		DefaultConstantBufferTemplate<PPGaussCoefConstantData> ppGaussCoefCB = { 8, "Renderer3D::PPGaussCoefCB" };
		DefaultConstantBufferTemplate<ExposureConstantData> exposureCB = { 9, "Renderer3D::ExposureCB" };
		DefaultConstantBufferTemplate<ToneMapConstantData> toneMapCB = { 9, "Renderer3D::ToneMapCB" };

		UniquePtr<InputLayout> genericInputLayout = nullptr;
		UniquePtr<InputLayout> shadowSilhouetteInputLayout = nullptr;

		// TODO: Viewport renderdata instance data
		OcclusionQuery sunOcclusionQuery = { "Renderer3D::SunOcclusionQuery" };

		RasterizerState solidBackfaceCullingRasterizerState = { D3D11_FILL_SOLID, D3D11_CULL_BACK, "Renderer3D::SolidBackfaceCulling" };
		RasterizerState solidFrontfaceCullingRasterizerState = { D3D11_FILL_SOLID, D3D11_CULL_FRONT, "Renderer3D::SolidFrontfaceCulling" };
		RasterizerState solidNoCullingRasterizerState = { D3D11_FILL_SOLID, D3D11_CULL_NONE, "Renderer3D::SolidNoCulling" };
		RasterizerState wireframeRasterizerState = { D3D11_FILL_WIREFRAME, D3D11_CULL_NONE, "Renderer3D::Wireframe" };

		DepthStencilState transparencyPassDepthStencilState = { true, D3D11_DEPTH_WRITE_MASK_ZERO, "Renderer3D::Transparency" };

		BlendState lensFlareSunQueryBlendState = { D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE { } };

		// TODO: Viewport renderdata instance data
		TextureSamplerCache cachedTextureSamplers;
		// TODO: Viewport renderdata instance data
		BlendStateCache cachedBlendStates;

		// NOTE: To avoid having to bind and clear render targets that won't be used this frame
		struct IsAnyCommandFlags
		{
			bool ScreenReflection;
			bool SubsurfaceScattering;
			bool CastShadow;
			bool ReceiveShadow;
			bool SilhouetteOutline;
		} isAnyCommand = {};

		// TODO:
		/*
		struct CachedD3DState
		{
			struct LastSetData
			{
				const VertexBuffer* VertexBuffer;
				const IndexBuffer* IndexBuffer;
				const ShaderPair* ShaderPair;
			} LastSet;
		} cachedD3DState = {};
		*/

		RenderPassCommandLists defaultCommandList, reflectionCommandList;

		struct Statistics
		{
			size_t VerticesRendered = 0;

			// TODO: Implement different statistics
			// size_t ObjectsRendered = 0, MeshesRendered = 0, SubMeshesRendered = 0;
			// size_t ObjectsCulled = 0, MeshesCulled = 0, SubMeshesCulled = 0;
		} statistics, lastFrameStatistics;

		// TODO: Viewport renderdata instance data
		ToneMapData toneMapData;

		struct BeginEndData
		{
			SceneViewport* Viewport;
			const SceneParameters* Scene;
		} current = {};

		TxpGetterFunction txpGetter = nullptr;
	};
}
