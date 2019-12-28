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
		ShaderFlags_AlphaTest = 1 << 5,
		ShaderFlags_CubeMapReflection = 1 << 6,
		ShaderFlags_LinearFog = 1 << 7,
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
		} CharacterLight, StageLight;

		vec4 LightColor;
		vec4 RenderResolution;

		struct LinearFog
		{
			vec4 Parameters;
			vec4 Color;
		} DepthFog;
	};

	struct ObjectConstantData
	{
		mat4 Model;
		mat4 ModelView;
		mat4 ModelViewProjection;
		struct Material
		{
			mat4 DiffuseTextureTransform;
			mat4 AmbientTextureTransform;
			vec4 FresnelCoefficient;
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
		std::array<vec4, 8> Coefficient;
	};

	struct ToneMapConstantData
	{
		float Exposure;
		float Gamma;
		int SaturatePower;
		float SaturateCoefficient;
		float Padding[12];
	};

	struct RenderCommand
	{
	public:
		const Obj* SourceObj;
		vec3 Position;
		struct Flags
		{
			uint32_t IsReflection : 1;

			// TODO: (?)
			// uint32_t CastsShadow : 1;
			// uint32_t ReceivesShadow : 1;
			// uint32_t SubsurfaceScattering : 1;
		} Flags;

	public:
		// NOTE: Static factory methods
		static inline RenderCommand ObjPos(const Obj& obj, vec3 pos)
		{
			RenderCommand command = {};
			command.SourceObj = &obj;
			command.Position = pos;
			return command;
		}

		static inline RenderCommand ObjPosReflect(const Obj& obj, vec3 pos)
		{
			RenderCommand command = ObjPos(obj, pos);
			command.Flags.IsReflection = true;
			return command;
		}
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
		void Draw(const RenderCommand& command);
		void End();

	public:
		void ClearTextureIDs();
		void RegisterTextureIDs(const TxpSet& txpSet);
		void UnRegisterTextureIDs(const TxpSet& txpSet);

	public:
		const SceneContext* GetSceneContext() const;
		std::unordered_map<uint32_t, const Txp*>& GetTextureIDTxpMap();

	protected:
		const Txp* GetTxpFromTextureID(uint32_t textureID) const;

	private:
		struct ObjRenderCommand
		{
			const Obj* Obj;
			mat4 Transform;
			vec3 Position;
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

		void InternalFlush();

		void InternalPrepareRenderCommands();
		void InternalRenderItems();
		void InternalRenderOpaqueObjCommand(ObjRenderCommand& command);
		void InternalRenderTransparentSubMeshCommand(SubMeshRenderCommand& command);
		void InternalRenderWireframeOverlay();
		void InternalRenderPostProcessing();
		void InternalRenderBloom();

		void BindMeshVertexBuffers(const Mesh& mesh);
		void PrepareAndRenderSubMesh(const ObjRenderCommand& command, const Mesh& mesh, const SubMesh& subMesh, const Material& material, const mat4& model);
		D3D_ShaderPair& GetMaterialShader(const Material& material);
		void SubmitSubMeshDrawCall(const SubMesh& subMesh);

		bool IntersectsCameraFrustum(const Sphere& boundingSphere, const vec3& position) const;
		bool IsDebugRenderFlagSet(int bitIndex) const;

	private:
		struct ShaderPairs
		{
			D3D_ShaderPair Debug = { Debug_VS(), Debug_PS(), "Renderer3D::Debug" };

			D3D_ShaderPair BlinnPerFrag = { BlinnPerFrag_VS(), BlinnPerFrag_PS(), "Renderer3D::BlinnPerFrag" };
			D3D_ShaderPair BlinnPerVert = { BlinnPerVert_VS(), BlinnPerVert_PS(), "Renderer3D::BlinnPerVert" };
			D3D_ShaderPair ClothAniso = { ClothDefault_VS(), ClothAniso_PS(), "Renderer3D::ClothAniso" };
			D3D_ShaderPair ClothDefault = { ClothDefault_VS(), ClothDefault_PS(), "Renderer3D::ClothDefault" };
			D3D_ShaderPair Constant = { Constant_VS(), Constant_PS(), "Renderer3D::Constant" };
			D3D_ShaderPair EyeBall = { EyeBall_VS(), EyeBall_PS(), "Renderer3D::EyeBall" };
			D3D_ShaderPair EyeLens = { EyeLens_VS(), EyeLens_PS(), "Renderer3D::EyeLens" };
			D3D_ShaderPair Floor = { Floor_VS(), Floor_PS(), "Renderer3D::Floor" };
			D3D_ShaderPair GlassEye = { GlassEye_VS(), GlassEye_PS(), "Renderer3D::GlassEye" };
			D3D_ShaderPair HairAniso = { HairDefault_VS(), HairAniso_PS(), "Renderer3D::HairAniso" };
			D3D_ShaderPair HairDefault = { HairDefault_VS(), HairDefault_PS(), "Renderer3D::HairDefault" };
			D3D_ShaderPair ItemBlinn = { ItemBlinn_VS(), ItemBlinn_PS(), "Renderer3D::ItemBlinn" };
			D3D_ShaderPair Lambert = { Lambert_VS(), Lambert_PS(), "Renderer3D::Lambert" };
			D3D_ShaderPair PPGauss = { PPGauss_VS(), PPGauss_PS(), "Renderer3D::PPGauss" };
			D3D_ShaderPair ReduceTex = { ReduceTex_VS(), ReduceTex_PS(), "Renderer3D::ReduceTex" };
			D3D_ShaderPair SkinDefault = { SkinDefault_VS(), SkinDefault_PS(), "Renderer3D::SkinDefault" };
			D3D_ShaderPair SkyDefault = { SkyDefault_VS(), SkyDefault_PS(), "Renderer3D::SkyDefault" };
			D3D_ShaderPair StageBlinn = { StageBlinn_VS(), StageBlinn_PS(), "Renderer3D::StageBlinn" };
			D3D_ShaderPair Tights = { Tights_VS(), Tights_PS(), "Renderer3D::Tights" };
			D3D_ShaderPair ToneMap = { ToneMap_VS(), ToneMap_PS(), "Renderer3D::ToneMap" };
			D3D_ShaderPair Water = { Water_VS(), Water_PS(), "Renderer3D::Water" };
		} shaders;

		D3D_DefaultConstantBufferTemplate<SceneConstantData> sceneCB = { 0, "Renderer3D::SceneCB" };
		D3D_DynamicConstantBufferTemplate<ObjectConstantData> objectCB = { 1, "Renderer3D::ObjectCB" };
		D3D_DynamicConstantBufferTemplate<ReduceTexConstantData> reduceTexCB = { 0, "Renderer3D::ReduceTexCB" };
		D3D_DynamicConstantBufferTemplate<PPGaussTexConstantData> ppGaussTexCB = { 0, "Renderer3D::PPGaussTexCB" };
		D3D_DefaultConstantBufferTemplate<PPGaussCoefConstantData> ppGaussCoefCB = { 1, "Renderer3D::PPGaussCoefCB" };
		D3D_DefaultConstantBufferTemplate<ToneMapConstantData> toneMapCB = { 0, "Renderer3D::ToneMapCB" };

		UniquePtr<D3D_InputLayout> genericInputLayout = nullptr;

		D3D_RasterizerState solidBackfaceCullingRasterizerState = { D3D11_FILL_SOLID, D3D11_CULL_BACK, "Renderer3D::SolidBackfaceCulling" };
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

		std::vector<ObjRenderCommand> renderCommandList, reflectionCommandList;
		std::vector<SubMeshRenderCommand> transparentSubMeshCommands;

		size_t verticesRenderedThisFrame = 0, verticesRenderedLastFrame = 0;
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
		std::unordered_map<uint32_t, const Txp*> textureIDTxpMap = {};
	};
}
