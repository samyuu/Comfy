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
	enum ShaderFlags : uint32_t
	{
		ShaderFlags_VertexColor = 1 << 0,
		ShaderFlags_DiffuseTexture = 1 << 1,
		ShaderFlags_AmbientTexture = 1 << 2,
		ShaderFlags_AlphaTest = 1 << 3,
	};

	struct ParallelLight
	{
		vec4 Ambient;
		vec4 Diffuse;
		vec4 Specular;
		vec4 Position;
		vec4 Direction;
	};

	struct SceneConstantData
	{
		struct SceneData
		{
			mat4 ViewProjection;
			vec4 EyePosition;
		} Scene;
		ParallelLight StageLight;
		vec4 LightDiffuse;
		float Padding[4];
	};

	struct ObjectConstantData
	{
		mat4 Model;
		struct Material
		{
			mat4 DiffuseTextureTransform;
			mat4 AmbientTextureTransform;
			vec3 DiffuseColor;
			float Transparency;
			vec4 AmbientColor;
			vec3 SpecularColor;
			float Reflectivity;
			vec4 EmissionColor;
			float Shininess;
			float Intensity;
			float BumpDepth;
		} Material;
		uint32_t ShaderFlags;
		float Padding[12];
	};

	class D3D_Renderer3D
	{
	public:
		D3D_Renderer3D();
		D3D_Renderer3D(const D3D_Renderer3D&) = default;
		~D3D_Renderer3D() = default;

		D3D_Renderer3D& operator=(const D3D_Renderer3D&) = delete;

	public:
		void Begin(const PerspectiveCamera& camera, const vec4& diffiuse, const ParallelLight& stageLight);
		void Draw(ObjSet* objSet, Obj* obj, vec3 position);
		void End();

		// TODO: Add more debug checkboxes to for example only render meshes with an alpha threshold (add CheckSkipSubMesh(...) method which checks debug flags)
		// DEBUG:
		bool DEBUG_RenderWireframe = false;
		bool DEBUG_AlphaSort = true;
		bool DEBUG_RenderOpaque = true;
		bool DEBUG_RenderTransparent = true;

	public:
		const PerspectiveCamera* GetCamera() const;

	private:
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
		void UpdateObjectConstantBuffer(Mesh& mesh, Material& material, const mat4& model);
		D3D_BlendState CreateMaterialBlendState(Material& material);
		D3D_ShaderPair& GetMaterialShader(Material& material);
		void UpdateSubMeshShaderState(SubMesh& subMesh, Material& material, ObjSet* objSet);
		void SubmitSubMeshDrawCall(SubMesh& subMesh);

	private:
		struct
		{
			D3D_ShaderPair testShader;
			D3D_ShaderPair constantShader;
			D3D_ShaderPair lambertShader;
		};

		D3D_DynamicConstantBufferTemplate<SceneConstantData> sceneConstantBuffer = { 0 };
		D3D_DynamicConstantBufferTemplate<ObjectConstantData> objectConstantBuffer = { 1 };

		UniquePtr<D3D_InputLayout> inputLayout = nullptr;

		D3D_RasterizerState solidBackfaceCullingRasterizerState = { D3D11_FILL_SOLID, D3D11_CULL_BACK };
		D3D_RasterizerState solidNoCullingRasterizerState = { D3D11_FILL_SOLID, D3D11_CULL_NONE };
		D3D_RasterizerState wireframeRasterizerState = { D3D11_FILL_WIREFRAME, D3D11_CULL_NONE };

		D3D_DepthStencilState transparencyPassDepthStencilState = { true, D3D11_DEPTH_WRITE_MASK_ZERO };

		std::vector<ObjRenderCommand> renderCommandList;
		std::vector<SubMeshRenderCommand> transparentSubMeshCommands;

		const PerspectiveCamera* perspectiveCamera = nullptr;
		const vec4* lightDiffuse = nullptr;
		const ParallelLight* parallelStageLight = nullptr;
	};
}
