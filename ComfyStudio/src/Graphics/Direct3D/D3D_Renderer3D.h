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
#include "Graphics/Auth3D/ObjSet.h"
#include "Graphics/Auth3D/SceneContext.h"

namespace Graphics
{
	enum ShaderFlags : uint32_t
	{
		ShaderFlags_VertexColor = 1 << 0,
		ShaderFlags_DiffuseTexture = 1 << 1,
		ShaderFlags_AmbientTexture = 1 << 2,
		ShaderFlags_NormalTexture = 1 << 3,
		ShaderFlags_SpecularTexture = 1 << 4,
		ShaderFlags_AlphaTest = 1 << 5,
		ShaderFlags_CubeMapReflection = 1 << 6,
	};

	struct SceneConstantData
	{
		mat4 IrradianceRed;
		mat4 IrradianceGreen;
		mat4 IrradianceBlue;

		struct SceneData
		{
			mat4 View;
			mat4 ViewProjection;
			vec4 EyePosition;
		} Scene;

		struct ParallelLight
		{
			vec4 Ambient;
			vec4 Diffuse;
			vec4 Specular;
			vec4 Direction;
		} StageLight;

		vec4 LightColor;
		float Padding[8];
	};

	struct ObjectConstantData
	{
		mat4 Model;
		struct Material
		{
			mat4 DiffuseTextureTransform;
			mat4 AmbientTextureTransform;
			vec3 Diffuse;
			float Transparency;
			vec4 Ambient;
			vec3 Specular;
			float Reflectivity;
			vec4 Emission;
			float Shininess;
			float Intensity;
			float BumpDepth;
		} Material;
		uint32_t ShaderFlags;
		struct TextureFormats
		{
			TextureFormat Diffuse;
			TextureFormat Ambient;
			TextureFormat Normal;
			TextureFormat Specular;
			TextureFormat ToonCurve;
			TextureFormat Reflection;
			TextureFormat Tangent;
			TextureFormat Reserved;
		} TextureFormats;
		float Padding[4];
	};

	struct PostProcessConstantData
	{
		float Exposure;
		float Gamma;
		int SaturatePower;
		float SaturateCoefficient;
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
		void Begin(SceneContext& scene);
		void Draw(ObjSet* objSet, Obj* obj, vec3 position);
		void End();

	public:
		const SceneContext* GetSceneContext() const;

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

		void InternalPrepareRenderCommands();
		void InternalRenderItems();
		void InternalRenderOpaqueObjCommand(ObjRenderCommand& command);
		void InternalRenderTransparentSubMeshCommand(SubMeshRenderCommand& command);
		void InternalRenderWireframeOverlay();
		void InternalRenderPostProcessing();

		void BindMeshVertexBuffers(Mesh& mesh);
		void PrepareAndRenderSubMesh(ObjRenderCommand& command, Mesh& mesh, SubMesh& subMesh, Material& material, const mat4& model);
		D3D_BlendState CreateMaterialBlendState(Material& material);
		D3D_ShaderPair& GetMaterialShader(Material& material);
		D3D_TextureSampler CreateTextureSampler(MaterialTexture& materialTexture, TextureFormat format);
		void CheckBindMaterialTexture(ObjSet* objSet, MaterialTexture& materialTexture, int slot, TextureFormat& constantBufferTextureFormat);
		void SubmitSubMeshDrawCall(SubMesh& subMesh);

	private:
		struct ShaderPairs
		{
			D3D_ShaderPair Debug = { Debug_VS(), Debug_PS() };
			D3D_ShaderPair Constant = { Constant_VS(), Constant_PS() };
			D3D_ShaderPair Lambert = { Lambert_VS(), Lambert_PS() };
			D3D_ShaderPair BlinnPerVert = { BlinnPerVert_VS(), BlinnPerVert_PS() };
			D3D_ShaderPair BlinnPerFrag = { BlinnPerFrag_VS(), BlinnPerFrag_PS() };
			D3D_ShaderPair StageBlinn = { StageBlinn_VS(), StageBlinn_PS() };
			D3D_ShaderPair ItemBlinn = { ItemBlinn_VS(), ItemBlinn_PS() };
			D3D_ShaderPair SkyDefault = { SkyDefault_VS(), SkyDefault_PS() };
			D3D_ShaderPair Water = { Water_VS(), Water_PS() };

			D3D_ShaderPair ToneMap = { ToneMap_VS(), ToneMap_PS() };
		} shaders;

		D3D_DynamicConstantBufferTemplate<SceneConstantData> sceneConstantBuffer = { 0 };
		D3D_DynamicConstantBufferTemplate<ObjectConstantData> objectConstantBuffer = { 1 };
		D3D_DynamicConstantBufferTemplate<PostProcessConstantData> postProcessConstantBuffer = { 0 };

		UniquePtr<D3D_InputLayout> genericInputLayout = nullptr;
		D3D_InputLayout postProcessInputLayout = { nullptr, 0, shaders.ToneMap.VS };

		D3D_RasterizerState solidBackfaceCullingRasterizerState = { D3D11_FILL_SOLID, D3D11_CULL_BACK };
		D3D_RasterizerState solidNoCullingRasterizerState = { D3D11_FILL_SOLID, D3D11_CULL_NONE };
		D3D_RasterizerState wireframeRasterizerState = { D3D11_FILL_WIREFRAME, D3D11_CULL_NONE };

		D3D_DepthStencilState transparencyPassDepthStencilState = { true, D3D11_DEPTH_WRITE_MASK_ZERO };

		std::vector<ObjRenderCommand> renderCommandList;
		std::vector<SubMeshRenderCommand> transparentSubMeshCommands;

		bool currentlyRenderingWireframeOverlay = false;

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
	};
}
