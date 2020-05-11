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

namespace Comfy::Render
{

}

namespace Comfy::Render::D3D11
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

	};
}
