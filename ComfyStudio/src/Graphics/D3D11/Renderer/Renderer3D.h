#pragma once
#include "../Direct3D.h"
#include "../Buffer/ConstantBuffer.h"
#include "../Buffer/IndexBuffer.h"
#include "../Buffer/VertexBuffer.h"
#include "../Shader/Shader.h"
#include "../State/BlendState.h"
#include "../State/DepthStencilState.h"
#include "../State/InputLayout.h"
#include "../State/OcclusionQuery.h"
#include "../State/RasterizerState.h"
#include "../Texture/Texture.h"
#include "Detail/ConstantData.h"
#include "Detail/ShaderFlags.h"
#include "Detail/ShaderPairs.h"
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
		void InternalResolveMSAAIfNeeded();
		void InternalSetSceneCB(SceneConstantData& outData);
		void InternalBindUploadSceneCBs();
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
		void SubmitQuadDrawCall();

		Sphere CalculateShadowViewFrustumSphere() const;

		bool IntersectsCameraFrustum(const ObjRenderCommand& command) const;
		bool IntersectsCameraFrustum(const Sphere& boundingSphere, const ObjRenderCommand& command) const;
		bool IntersectsCameraFrustum(const Sphere& boundingSphere, const SubMeshRenderCommand& command) const;

		bool IsDebugRenderFlagSet(int bitIndex) const;

	private:
		RendererShaderPairs shaders;
		RendererConstantBuffers constantBuffers;

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
