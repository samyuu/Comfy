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
#include "Graphics/Auth3D/ObjSet.h"
#include "Graphics/Camera.h"

namespace Graphics
{
	class D3D_Renderer3D
	{
	public:
		D3D_Renderer3D();
		D3D_Renderer3D(const D3D_Renderer3D&) = default;
		~D3D_Renderer3D() = default;

		D3D_Renderer3D& operator=(const D3D_Renderer3D&) = delete;

	public:
		void Begin(const PerspectiveCamera& camera);
		void Draw(ObjSet* objSet, Obj* obj, vec3 position);
		void End();

		// DEBUG:
		bool DEBUG_RenderWireframe = false;
		bool DEBUG_AlphaSort = true;
		bool DEBUG_RenderOpaque = true;
		bool DEBUG_RenderTransparent = true;

	public:
		const PerspectiveCamera* GetCamera() const;

	private:
		struct CameraConstantData
		{
			mat4 ViewProjection;
			vec3 EyePosition;
			float Padding[13];
		};

		struct DynamicConstantData
		{
			VertexAttributeFlags AttributeFlags;
			uint32_t TextureFlags;
			uint32_t Padding[2];
		};

		struct MaterialConstantData
		{
			vec3 DiffuseColor;
			float Transparency;
			vec4 AmbientColor;
			vec3 SpecularColor;
			float Reflectivity;
			vec4 EmissionColor;
			float Shininess;
			float Intensity;
			float BumpDepth;
			float AlphaTestThreshold;
			mat4 TextureTransform;
		};

		struct ViewPositionData
		{
			vec4 CB_EyePosition;
			float Padding[12];
		};

		struct ObjRenderCommand
		{
			ObjSet* ObjSet;
			Obj* Obj;
			mat4 Transform;
			vec3 Position;
		};

		struct SubMeshRenderCommand
		{
			ObjRenderCommand* ObjCommand;
			
			Mesh* ParentMesh;
			SubMesh* SubMesh;
			
			float CameraDistance;
		};

		void InternalFlush();

		void InternalRenderItems();
		void InternalRenderOpaqueObjCommand(ObjRenderCommand& command);
		void InternalRenderTransparentSubMeshCommand(SubMeshRenderCommand& command);

		void BindMeshVertexBuffers(Mesh& mesh);
		void UpdateMeshVertexAttributeConstantBuffer(Mesh& mesh);
		void UpdateMaterialConstantBuffer(Material& material);
		D3D_BlendState CreateMaterialBlendState(Material& material);
		void BindSubMeshTextures(SubMesh& subMesh, Material& material, ObjSet* objSet);
		void BindIndexBufferSubmitSubMeshDrawCall(SubMesh& subMesh);

	private:
		D3D_ShaderPair testShader;

		D3D_DefaultConstantBufferTemplate<CameraConstantData> cameraConstantBuffer = { 0 };
		D3D_DynamicConstantBufferTemplate<DynamicConstantData> dynamicConstantBuffer = { 0 };
		D3D_DynamicConstantBufferTemplate<MaterialConstantData> materialConstantBuffer = { 1 };

		UniquePtr<D3D_InputLayout> inputLayout = nullptr;

		D3D_RasterizerState solidBackfaceCullingRasterizerState = { D3D11_FILL_SOLID, D3D11_CULL_BACK };
		D3D_RasterizerState solidNoCullingRasterizerState = { D3D11_FILL_SOLID, D3D11_CULL_NONE };
		D3D_RasterizerState wireframeRasterizerState = { D3D11_FILL_WIREFRAME, D3D11_CULL_NONE };

		D3D_DepthStencilState transparencyPassDepthStencilState = { true, D3D11_DEPTH_WRITE_MASK_ZERO };

		std::vector<ObjRenderCommand> renderCommandList;
		std::vector<SubMeshRenderCommand> transparentSubMeshCommands;

		const PerspectiveCamera* perspectiveCamera = nullptr;
	};
}
