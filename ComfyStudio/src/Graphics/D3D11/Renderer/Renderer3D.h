#pragma once
#include "Types.h"
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

	using TexGetterFunction = std::function<const Tex*(const Cached_TexID* texID)>;
}

namespace Comfy::Graphics::D3D11
{
	class Renderer3D : NonCopyable
	{
	public:
		Renderer3D(TexGetterFunction texGetter);
		~Renderer3D() = default;

	public:
		void Begin(SceneViewport& viewport, const SceneParameters& scene);
		void Draw(const RenderCommand& command);
		void End();

		const Tex* GetTexFromTextureID(const Cached_TexID* textureID) const;

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

		using RenderFlags = u32;
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

		void UpdateIsAnyCommandFlags(const RenderCommand& command);
		void Flush();

		void PrepareRenderCommands(RenderPassCommandLists& commandList);
		void RenderScene();
		void ResolveMSAAIfNeeded();
		void SetSceneCBData(SceneConstantData& outData);
		void BindUploadSceneCBs();
		void BindSceneTextures();

		void PreRenderShadowMap();
		void PreRenderReduceFilterShadowMap();

		void PreRenderScreenReflection();

		void PreRenderSubsurfaceScattering();
		void PreRenderReduceFilterSubsurfaceScattering();

		// TODO: Add wrapper function to loop over commnad list and add opaque and transparent to RenderFlags
		void RenderOpaqueObjCommand(ObjRenderCommand& command, RenderFlags flags = RenderFlags_None);
		void RenderTransparentSubMeshCommand(SubMeshRenderCommand& command);

		void RenderSilhouette();
		void RenderSilhouetteOutlineOverlay();

		void QueryRenderLensFlare();
		void RenderLensFlare();

		void RenderPostProcessing();
		void RenderBloom();

		void RenderExposurePreBloom();
		void RenderExposurePostBloom();

		void BindMeshVertexBuffers(const Mesh& primaryMesh, const Mesh* morphMesh);
		void PrepareAndRenderSubMesh(const ObjRenderCommand& command, const Mesh& mesh, const SubMesh& subMesh, const Material& material, RenderFlags flags = RenderFlags_None);

		u32 MaterialTextureTypeToTextureSlot(MaterialTextureType textureType, bool secondColorMap);
		u32 BindMaterialTextures(const ObjRenderCommand& command, const Material& material, RenderFlags flags);
		bool GetIsTextureSlotUsed(Material::ShaderTypeIdentifier shaderType, Material::MaterialUsedTextureFlags usedTextureFlags, u32 textureSlot);
		void SetSubMeshRasterizerState(const Material& material);
		void SetObjectCBMaterialData(const Material& material, ObjectConstantData::MaterialData& outMaterialData) const;
		void SetObjectCBTransforms(const ObjRenderCommand& command, const Mesh& mesh, const SubMesh& subMesh, ObjectConstantData& outData) const;
		vec4 GetObjectCBMorphWeight(const ObjRenderCommand& command) const;
		u32 GetObjectCBShaderFlags(const ObjRenderCommand& command, const Mesh& mesh, const SubMesh& subMesh, const Material& material, u32 boundMaterialTexturesFlags) const;

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

		std::unique_ptr<InputLayout> genericInputLayout = nullptr;
		std::unique_ptr<InputLayout> shadowSilhouetteInputLayout = nullptr;

		RasterizerState solidBackfaceCullingRasterizerState = { D3D11_FILL_SOLID, D3D11_CULL_BACK, "Renderer3D::SolidBackfaceCulling" };
		RasterizerState solidFrontfaceCullingRasterizerState = { D3D11_FILL_SOLID, D3D11_CULL_FRONT, "Renderer3D::SolidFrontfaceCulling" };
		RasterizerState solidNoCullingRasterizerState = { D3D11_FILL_SOLID, D3D11_CULL_NONE, "Renderer3D::SolidNoCulling" };
		RasterizerState wireframeRasterizerState = { D3D11_FILL_WIREFRAME, D3D11_CULL_NONE, "Renderer3D::Wireframe" };

		DepthStencilState transparencyPassDepthStencilState = { true, D3D11_DEPTH_WRITE_MASK_ZERO, "Renderer3D::Transparency" };

		BlendState lensFlareSunQueryBlendState = { D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE { } };

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

		struct BeginEndData
		{
			SceneViewport* Viewport;
			const SceneParameters* Scene;
		} current = {};

		TexGetterFunction texGetter = nullptr;
	};
}
