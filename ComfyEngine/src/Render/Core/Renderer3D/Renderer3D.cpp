#include "Renderer3D.h"
#include "Detail/ConstantData.h"
#include "Detail/GaussianBlur.h"
#include "Detail/LensFlare.h"
#include "Detail/MeshTransparency.h"
#include "Detail/RenderTarget3DImpl.h"
#include "Detail/ShaderFlags.h"
#include "Detail/ShaderPairs.h"
#include "Detail/SubsurfaceScattering.h"
#include "Render/D3D11/D3D11.h"
#include "Render/D3D11/D3D11Buffer.h"
#include "Render/D3D11/D3D11GraphicsTypeHelpers.h"
#include "Render/D3D11/D3D11OpaqueResource.h"
#include "Render/D3D11/D3D11Shader.h"
#include "Render/D3D11/D3D11State.h"
#include "Render/D3D11/D3D11Texture.h"

#include "ImGui/Gui.h"

namespace Comfy::Render
{
	using namespace Graphics;

	constexpr std::array<vec4, 2> DefaultShadowCoefficients =
	{
		vec4(0.199471f, 0.176033f, 0.120985f, 0.064759f),
		vec4(0.026995f, 0.008764f, 0.002216f, 0.000436f),
	};

	constexpr vec3 DefaultShadowAmbient = vec3(0.4f);
	constexpr float DefaultShadowExpontent = 80.0f * (9.95f * 2.0f) * 1.442695f;
	constexpr float DefaultShadowTexelOffset = 0.05f / (9.95f * 2.0f);

	constexpr u32 MorphVertexAttributeOffset = VertexAttribute_Count;

	const Material& GetSubMeshMaterial(const SubMesh& subMesh, const RenderCommand3D& command)
	{
		if (command.Dynamic != nullptr)
		{
			for (auto& materialOverride : command.Dynamic->MaterialOverrides)
			{
				if (&subMesh == materialOverride.SubMeshToReplace && materialOverride.NewMaterial != nullptr)
					return *materialOverride.NewMaterial;
			}
		}

		const auto& parentObj = *command.SourceObj;

		if (InBounds(subMesh.MaterialIndex, parentObj.Materials))
			return parentObj.Materials[subMesh.MaterialIndex];

		static const Material dummyMaterial = {};
		return dummyMaterial;
	}

	constexpr bool IsMeshOrSubMeshIndexSpecified(int index)
	{
		return (index >= 0);
	}

	template <typename Func>
	void IterateCommandMeshes(const RenderCommand3D& command, Func func)
	{
		if (IsMeshOrSubMeshIndexSpecified(command.Flags.MeshIndex))
		{
			func(command.SourceObj->Meshes[command.Flags.MeshIndex]);
		}
		else
		{
			for (auto& mesh : command.SourceObj->Meshes)
				func(mesh);
		}
	}

	template <typename Func>
	void IterateCommandSubMeshes(const RenderCommand3D& command, const Mesh& mesh, Func func)
	{
		if (IsMeshOrSubMeshIndexSpecified(command.Flags.SubMeshIndex) && IsMeshOrSubMeshIndexSpecified(command.Flags.MeshIndex))
		{
			auto& specifiedSubMesh = mesh.SubMeshes[command.Flags.SubMeshIndex];
			func(specifiedSubMesh, GetSubMeshMaterial(specifiedSubMesh, command));
		}
		else
		{
			for (auto& subMesh : mesh.SubMeshes)
				func(subMesh, GetSubMeshMaterial(subMesh, command));
		}
	}

	template <typename Func>
	void IterateCommandMeshesAndSubMeshes(const RenderCommand3D& command, Func func)
	{
		IterateCommandMeshes(command, [&](auto& mesh)
		{
			IterateCommandSubMeshes(command, mesh, [&](auto& subMesh, auto& material)
			{
				func(mesh, subMesh, material);
			});
		});
	}

	constexpr bool CastsShadow(const RenderCommand3D& command, const Mesh& mesh, const SubMesh& subMesh, const Material& material)
	{
		if (!command.Flags.CastsShadow)
			return false;

		if (command.Flags.IgnoreShadowCastObjFlags)
			return true;

		if (subMesh.Flags.CastsShadows || material.ShaderFlags.CastsShadows)
			return true;

		return false;
	}

	bool AllSubMeshesCastShadows(const RenderCommand3D& command)
	{
		if (!command.Flags.CastsShadow)
			return false;

		if (command.Flags.IgnoreShadowCastObjFlags)
			return true;

		bool allCastShaows = true;
		IterateCommandMeshesAndSubMeshes(command, [&](auto& mesh, auto& subMesh, auto& material)
		{
			allCastShaows &= (subMesh.Flags.CastsShadows || material.ShaderFlags.CastsShadows);
		});
		return allCastShaows;
	}

	constexpr bool ReceivesShadows(const RenderCommand3D& command, const Mesh& mesh, const SubMesh& subMesh)
	{
		return (command.Flags.ReceivesShadow && subMesh.Flags.ReceivesShadows);
	}

	constexpr bool ReceivesSelfShadow(const RenderCommand3D& command, const Mesh& mesh, const SubMesh& subMesh)
	{
		return (command.Flags.ReceivesShadow);
	}

	const Mesh* GetMorphMesh(const Obj& obj, const Obj* morphObj, const Mesh& mesh)
	{
		if (morphObj == nullptr)
			return nullptr;

		const size_t meshIndex = static_cast<size_t>(std::distance(&obj.Meshes.front(), &mesh));
		return (meshIndex < morphObj->Meshes.size()) ? &morphObj->Meshes[meshIndex] : nullptr;
	}

	const Mesh* GetMorphMesh(const Obj& obj, const RenderCommand3D& command, const Mesh& mesh)
	{
		if (command.Dynamic == nullptr)
			return nullptr;
		else
			return GetMorphMesh(obj, command.Dynamic->MorphObj, mesh);
	}

	vec4 GetPackedTextureSize(vec2 size)
	{
		return vec4(1.0f / size, size);
	}

	vec4 GetPackedTextureSize(const D3D11RenderTargetAndView& renderTarget)
	{
		return GetPackedTextureSize(vec2(renderTarget.GetSize()));
	}

	void CalculateExposureSpotCoefficients(Detail::ExposureConstantData& outData)
	{
		// TODO: Calculate dynamically
		outData.SpotWeight = vec4(1.28, 0.0, 0.0, 0.0);
		outData.SpotCoefficients =
		{
			vec4(0.500000, 0.675559, 0.0, 4.0),
			vec4(0.500000, 0.663732, 0.0, 4.0),
			vec4(0.491685, 0.657819, 0.0, 3.0),
			vec4(0.490021, 0.669645, 0.0, 2.0),
			vec4(0.509979, 0.669645, 0.0, 2.0),
			vec4(0.508315, 0.657819, 0.0, 3.0),
			vec4(0.500000, 0.648949, 0.0, 3.0),
		};
	}

	enum Renderer3DTextureSlot : i32
	{
		// NOTE: Material Textures
		TextureSlot_Diffuse = 0,
		TextureSlot_Ambient = 1,
		TextureSlot_Normal = 2,
		TextureSlot_Specular = 3,
		TextureSlot_Transparency = 4,
		TextureSlot_Environment = 5,
		TextureSlot_Translucency = 6,
		TextureSlot_Reserved = 7,
		TextureSlot_MaterialTextureCount = 8,

		// NOTE: IBL Light Maps
		TextureSlot_IBLLightMaps_0 = 9,
		TextureSlot_IBLLightMaps_1 = 10,
		TextureSlot_IBLLightMaps_2 = 11,

		// NOTE: Screen Space Lookups
		TextureSlot_ScreenReflection = 15,
		TextureSlot_SubsurfaceScattering = 16,

		// NOTE: Shadow Maps
		TextureSlot_ShadowMap = 19,	// textures[6]
		TextureSlot_ESMFull = 20,	// textures[19]
		TextureSlot_ESMGauss = 21,	// textures[20]

		TextureSlot_Count,
	};

	Sphere CombineBoundingSpheres(const std::vector<Sphere>& spheres)
	{
		if (spheres.empty())
			return Sphere { vec3(0.0f), 1.0f };

		vec3 min = spheres.front().Center, max = spheres.front().Center;
		for (auto& sphere : spheres)
		{
			const float radius = sphere.Radius;
			min.x = std::min(min.x, sphere.Center.x - radius);
			min.y = std::min(min.y, sphere.Center.y - radius);
			min.z = std::min(min.z, sphere.Center.z - radius);

			max.x = std::max(max.x, sphere.Center.x + radius);
			max.y = std::max(max.y, sphere.Center.y + radius);
			max.z = std::max(max.z, sphere.Center.z + radius);
		}

		const vec3 halfSpan = (max - min) / 2.0f;
		return Sphere { (min + halfSpan), (std::max(halfSpan.x, std::max(halfSpan.y, halfSpan.z))) };
	}

	using RenderFlags = u32;
	enum RenderFlags_Enum : RenderFlags
	{
		RenderFlags_None = 0,
		RenderFlags_ShadowPass = (1 << 0),
		RenderFlags_SSSPass = (1 << 1),
		RenderFlags_SilhouetteOutlinePass = (1 << 2),
		RenderFlags_NoMaterialShader = (1 << 3),
		RenderFlags_NoMaterialTextures = (1 << 4),
		RenderFlags_DiffuseTextureOnly = (1 << 5),
		RenderFlags_NoRasterizerState = (1 << 6),
		RenderFlags_NoFrustumCulling = (1 << 7),
	};

	struct ObjRenderCommand
	{
		RenderCommand3D SourceCommand;

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

	struct Renderer3D::Impl
	{
	public:
		D3D11& D3D11 = GlobalD3D11;
		TexGetter TexGetter;

		Detail::RendererShaderPairs Shaders;
		Detail::RendererConstantBuffers ConstantBuffers;

		std::unique_ptr<D3D11InputLayout> GenericInputLayout = nullptr;
		std::unique_ptr<D3D11InputLayout> ShadowSilhouetteInputLayout = nullptr;

		D3D11RasterizerState SolidBackfaceCullingRasterizerState = { D3D11, D3D11_FILL_SOLID, D3D11_CULL_BACK, "Renderer3D::SolidBackfaceCulling" };
		D3D11RasterizerState SolidFrontfaceCullingRasterizerState = { D3D11, D3D11_FILL_SOLID, D3D11_CULL_FRONT, "Renderer3D::SolidFrontfaceCulling" };
		D3D11RasterizerState SolidNoCullingRasterizerState = { D3D11, D3D11_FILL_SOLID, D3D11_CULL_NONE, "Renderer3D::SolidNoCulling" };
		D3D11RasterizerState WireframeRasterizerState = { D3D11, D3D11_FILL_WIREFRAME, D3D11_CULL_NONE, "Renderer3D::Wireframe" };

		D3D11DepthStencilState TransparencyPassDepthStencilState = { D3D11, true, D3D11_DEPTH_WRITE_MASK_ZERO, "Renderer3D::Transparency" };

		//D3D11DepthStencilState DepthNoWriteDepthStencilState = { D3D11, true, D3D11_DEPTH_WRITE_MASK_ZERO, "Renderer3D::DepthNoWrite" };
		//D3D11DepthStencilState NoDepthNoWriteDepthStencilState = { D3D11, false, D3D11_DEPTH_WRITE_MASK_ZERO, "Renderer3D::NoDepthNoWrite" };
		D3D11DepthStencilState LensFlareReadDepthDepthStencilState = { D3D11, true, D3D11_DEPTH_WRITE_MASK_ZERO, "Renderer3D::LensFlareReadDepth" };
		D3D11DepthStencilState LensFlareIgnoreDepthDepthStencilState = { D3D11, false, D3D11_DEPTH_WRITE_MASK_ZERO, "Renderer3D::LensFlareIgnoreDepth" };

		// D3D11DepthStencilState LensFlareNoDepthStencilState = { D3D11, false, D3D11_DEPTH_WRITE_MASK_ZERO, "Renderer3D::LensFlareNoDepth" };
		D3D11BlendState LensFlareSunQueryNoColorWriteBlendState = { D3D11, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE { } };

		Detail::LensFlareMesh LensFlareMesh;

		// NOTE: To avoid having to bind and clear render targets that won't be used this frame
		struct IsAnyCommandFlags
		{
			bool ScreenReflection;
			bool SubsurfaceScattering;
			bool CastsShadow;
			bool ReceiveShadow;
			bool SilhouetteOutline;
		} IsAnyCommand = {};

		/* // TODO:
		struct CachedD3DState
		{
			struct LastSetData
			{
				const VertexBuffer* VertexBuffer;
				const IndexBuffer* IndexBuffer;
				const ShaderPair* ShaderPair;
			} LastSet;
		} CachedD3DState = {};
		*/

		struct RenderPassCommandLists
		{
			std::vector<ObjRenderCommand> OpaqueAndTransparent;
			std::vector<SubMeshRenderCommand> Transparent;
		} DefaultCommandList, ReflectionCommandList;

		struct Statistics
		{
			size_t VerticesRendered = 0;

			// TODO: Implement different statistics
			// size_t ObjectsRendered = 0, MeshesRendered = 0, SubMeshesRendered = 0;
			// size_t ObjectsCulled = 0, MeshesCulled = 0, SubMeshesCulled = 0;
		} Statistics, LastFrameStatistics;

		struct BeginEndData
		{
			Camera3D* Camera;
			Detail::RenderTarget3DImpl* RenderTarget;
			const SceneParam3D* SceneParam;
		} Current = {};

	public:
		Impl()
		{
			static constexpr D3D11InputElement genericElements[] =
			{
				{ "POSITION",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, VertexAttribute_Position },
				{ "NORMAL",			0, DXGI_FORMAT_R32G32B32_FLOAT,		0, VertexAttribute_Normal },
				{ "TANGENT",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, VertexAttribute_Tangent },
				{ "TEXCOORD",		0, DXGI_FORMAT_R32G32_FLOAT,		0, VertexAttribute_TextureCoordinate0 },
				{ "TEXCOORD",		1, DXGI_FORMAT_R32G32_FLOAT,		0, VertexAttribute_TextureCoordinate1 },
				{ "TEXCOORD",		2, DXGI_FORMAT_R32G32_FLOAT,		0, VertexAttribute_TextureCoordinate2 },
				{ "TEXCOORD",		3, DXGI_FORMAT_R32G32_FLOAT,		0, VertexAttribute_TextureCoordinate3 },
				{ "COLOR",			0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, VertexAttribute_Color0 },
				{ "COLOR",			1, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, VertexAttribute_Color1 },
				{ "BLENDWEIGHT",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, VertexAttribute_BoneWeight },
				{ "BLENDINDICES",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, VertexAttribute_BoneIndex },
				{ "POSITION",		1, DXGI_FORMAT_R32G32B32_FLOAT,		0, MorphVertexAttributeOffset + VertexAttribute_Position },
				{ "NORMAL",			1, DXGI_FORMAT_R32G32B32_FLOAT,		0, MorphVertexAttributeOffset + VertexAttribute_Normal },
				{ "TANGENT",		1, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, MorphVertexAttributeOffset + VertexAttribute_Tangent },
				{ "TEXCOORD",		4, DXGI_FORMAT_R32G32_FLOAT,		0, MorphVertexAttributeOffset + VertexAttribute_TextureCoordinate0 },
				{ "TEXCOORD",		5, DXGI_FORMAT_R32G32_FLOAT,		0, MorphVertexAttributeOffset + VertexAttribute_TextureCoordinate1 },
				{ "TEXCOORD",		6, DXGI_FORMAT_R32G32_FLOAT,		0, MorphVertexAttributeOffset + VertexAttribute_TextureCoordinate2 },
				{ "TEXCOORD",		7, DXGI_FORMAT_R32G32_FLOAT,		0, MorphVertexAttributeOffset + VertexAttribute_TextureCoordinate3 },
				{ "COLOR",			2, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, MorphVertexAttributeOffset + VertexAttribute_Color0 },
				{ "COLOR",			3, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, MorphVertexAttributeOffset + VertexAttribute_Color1 },
			};

			GenericInputLayout = std::make_unique<D3D11InputLayout>(D3D11, genericElements, std::size(genericElements), Shaders.DebugMaterial.VS);
			D3D11_SetObjectDebugName(GenericInputLayout->InputLayout.Get(), "Renderer3D::GenericInputLayout");

			static constexpr D3D11InputElement silhouetteElements[] =
			{
				{ "POSITION",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, VertexAttribute_Position },
				{ "TEXCOORD",		0, DXGI_FORMAT_R32G32_FLOAT,		0, VertexAttribute_TextureCoordinate0 },
				{ "BONE_WEIGHT",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, VertexAttribute_BoneWeight },
				{ "BONE_INDEX",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, VertexAttribute_BoneIndex },
				{ "POSITION",		1, DXGI_FORMAT_R32G32B32_FLOAT,		0, MorphVertexAttributeOffset + VertexAttribute_Position },
				{ "TEXCOORD",		4, DXGI_FORMAT_R32G32_FLOAT,		0, MorphVertexAttributeOffset + VertexAttribute_TextureCoordinate0 },
			};

			ShadowSilhouetteInputLayout = std::make_unique<D3D11InputLayout>(D3D11, silhouetteElements, std::size(silhouetteElements), Shaders.Silhouette.VS);
			D3D11_SetObjectDebugName(ShadowSilhouetteInputLayout->InputLayout.Get(), "Renderer3D::ShadowSilhouetteInputLayout");

			constexpr size_t reasonableInitialCapacity = 64;
			DefaultCommandList.OpaqueAndTransparent.reserve(reasonableInitialCapacity);
			DefaultCommandList.Transparent.reserve(reasonableInitialCapacity);

			ReflectionCommandList.OpaqueAndTransparent.reserve(reasonableInitialCapacity);
			ReflectionCommandList.Transparent.reserve(reasonableInitialCapacity);
		}

		void UpdateIsAnyCommandFlags(const RenderCommand3D& command)
		{
			if (command.Flags.IsReflection)
				IsAnyCommand.ScreenReflection = true;

			if (command.Flags.SilhouetteOutline)
				IsAnyCommand.SilhouetteOutline = true;

			if (!IsAnyCommand.CastsShadow)
			{
				IterateCommandMeshesAndSubMeshes(command, [&](auto& mesh, auto& subMesh, auto& material)
				{
					if (CastsShadow(command, mesh, subMesh, material))
						IsAnyCommand.CastsShadow = true;
				});
			}

			if (!IsAnyCommand.ReceiveShadow)
			{
				IterateCommandMeshesAndSubMeshes(command, [&](auto& mesh, auto& subMesh, auto& material)
				{
					if (ReceivesShadows(command, mesh, subMesh) || ReceivesSelfShadow(command, mesh, subMesh))
						IsAnyCommand.ReceiveShadow = true;
				});
			}

			if (!IsAnyCommand.SubsurfaceScattering)
			{
				IterateCommandMeshesAndSubMeshes(command, [&](auto& mesh, auto& subMesh, auto& material)
				{
					if (Detail::UsesSSSSkin(GetSubMeshMaterial(subMesh, command)))
						IsAnyCommand.SubsurfaceScattering = true;
				});
			}
		}

		void Flush()
		{
			PrepareRenderCommands(DefaultCommandList);
			PrepareRenderCommands(ReflectionCommandList);

			RenderScene();
			RenderPostProcessing();

			if (IsAnyCommand.SilhouetteOutline)
				RenderSilhouetteOutlineOverlay();

			DefaultCommandList.OpaqueAndTransparent.clear();
			DefaultCommandList.Transparent.clear();

			ReflectionCommandList.OpaqueAndTransparent.clear();
			ReflectionCommandList.Transparent.clear();
		}

		void PrepareRenderCommands(RenderPassCommandLists& commandList)
		{
			if (commandList.OpaqueAndTransparent.empty())
				return;

			for (auto& command : commandList.OpaqueAndTransparent)
			{
				command.AreAllMeshesTransparent = true;

				IterateCommandMeshesAndSubMeshes(command.SourceCommand, [&](auto& mesh, auto& subMesh, auto& material)
				{
					if (Detail::IsMeshTransparent(mesh, subMesh, material))
					{
						const auto boundingSphere = (subMesh.BoundingSphere * command.SourceCommand.Transform);
						const float cameraDistance = glm::distance(boundingSphere.Center, Current.Camera->ViewPoint);

						commandList.Transparent.push_back({ &command, &mesh, &subMesh, cameraDistance });
					}
					else
					{
						command.AreAllMeshesTransparent = false;
					}
				});
			}

			if (Current.RenderTarget->Param.AlphaSort)
			{
				std::sort(commandList.Transparent.begin(), commandList.Transparent.end(), [](SubMeshRenderCommand& a, SubMeshRenderCommand& b)
				{
					constexpr float comparisonThreshold = 0.001f;
					const bool sameDistance = std::abs(a.CameraDistance - b.CameraDistance) < comparisonThreshold;

					if (sameDistance)
						return a.SubMesh->BoundingSphere.Radius > b.SubMesh->BoundingSphere.Radius;

					return (a.CameraDistance > b.CameraDistance);
				});
			}
		}

		void RenderScene()
		{
			D3D11_BeginDebugEvent("Scene");

			SetSceneCBData(ConstantBuffers.Scene.Data);
			BindUploadSceneCBs();

			BindSceneTextures();

			if (Current.RenderTarget->Param.ShadowMapping && IsAnyCommand.CastsShadow && IsAnyCommand.ReceiveShadow)
			{
				PreRenderShadowMap();
				PreRenderReduceFilterShadowMap();
			}

			GenericInputLayout->Bind(D3D11);

			if (Current.RenderTarget->Param.RenderReflection && IsAnyCommand.ScreenReflection)
			{
				PreRenderScreenReflection();
				Current.RenderTarget->Reflection.RenderTarget.BindColorTexturePS(D3D11, TextureSlot_ScreenReflection);
			}

			if (Current.RenderTarget->Param.RenderSubsurfaceScattering && IsAnyCommand.SubsurfaceScattering)
			{
				PreRenderSubsurfaceScattering();
				PreRenderReduceFilterSubsurfaceScattering();
				Current.RenderTarget->SubsurfaceScattering.FilterRenderTargets.back().BindColorTexturePS(D3D11, TextureSlot_SubsurfaceScattering);
			}

			D3D11_BeginDebugEvent("Opaque Geometry");
			// TODO: If the MultiSampleCount is unsupported then this currently tries to recreate the unsupported format every frame.
			//		 After attempting to set the MultiSampleCount the source Param->X variable should be set to the actual MultiSampleCount (?) which would be 1 in case of failure
			Current.RenderTarget->Main.Current().RecreateWithNewMultiSampleCountIfDifferent(D3D11, Current.RenderTarget->Param.MultiSampleCount);
			Current.RenderTarget->Main.Current().RecreateWithNewSizeIfDifferent(D3D11, Current.RenderTarget->Param.RenderResolution);
			Current.RenderTarget->Main.Current().BindAndSetViewport(D3D11);

			if (Current.RenderTarget->Param.Clear)
				Current.RenderTarget->Main.Current().ClearColor(D3D11, Current.RenderTarget->Param.ClearColor);
			Current.RenderTarget->Main.Current().ClearDepth(D3D11);

			if (Current.RenderTarget->Param.Wireframe)
				WireframeRasterizerState.Bind(D3D11);

			if (Current.RenderTarget->Param.RenderOpaque && !DefaultCommandList.OpaqueAndTransparent.empty())
			{
				D3D11.ImmediateContext->OMSetBlendState(nullptr, nullptr, D3D11_DEFAULT_SAMPLE_MASK);

				for (auto& command : DefaultCommandList.OpaqueAndTransparent)
					RenderOpaqueObjCommand(command);

				if (Current.RenderTarget->Param.RenderLensFlare && Current.SceneParam->Light.Sun.Type == LightSourceType::Parallel)
					QueryRenderLensFlareSun();
			}
			D3D11_EndDebugEvent();

			D3D11_BeginDebugEvent("Transparent Geometry");
			if (Current.RenderTarget->Param.RenderTransparent && !DefaultCommandList.Transparent.empty())
			{
				TransparencyPassDepthStencilState.Bind(D3D11);

				for (auto& command : DefaultCommandList.Transparent)
					RenderTransparentSubMeshCommand(command);

				TransparencyPassDepthStencilState.UnBind(D3D11);
			}
			D3D11_EndDebugEvent();

			if (Current.RenderTarget->Param.RenderLensFlare && Current.SceneParam->Light.Sun.Type == LightSourceType::Parallel)
				RenderLensFlareGhosts();

			if (IsAnyCommand.SilhouetteOutline)
				RenderSilhouette();

			Current.RenderTarget->Main.Current().UnBind(D3D11);
			GenericInputLayout->UnBind(D3D11);

			ResolveMSAAIfNeeded();
			D3D11_EndDebugEvent();
		}

		void ResolveMSAAIfNeeded()
		{
			if (!Current.RenderTarget->Main.MSAAEnabled())
				return;

			D3D11_BeginDebugEvent("Resolve MSAA");
			auto& currentMain = Current.RenderTarget->Main.Current();
			auto& currentMainResolved = Current.RenderTarget->Main.CurrentResolved();

			currentMainResolved.RecreateWithNewSizeIfDifferent(D3D11, currentMain.GetSize());
			D3D11.ImmediateContext->ResolveSubresource(currentMainResolved.ColorTexture.Get(), 0, currentMain.ColorTexture.Get(), 0, currentMain.ColorTextureDesc.Format);
			D3D11_EndDebugEvent();
		}

		void SetSceneCBData(Detail::SceneConstantData& outData)
		{
			outData.RenderResolution = GetPackedTextureSize(Current.RenderTarget->Param.RenderResolution);

			const vec4 renderTimeNow = static_cast<float>(TimeSpan::GetTimeNow().TotalSeconds()) * Detail::SceneConstantData::RenderTime::Scales;
			outData.RenderTime.Time = renderTimeNow;
			outData.RenderTime.TimeSin = (glm::sin(renderTimeNow) + 1.0f) * 0.5f;
			outData.RenderTime.TimeCos = (glm::cos(renderTimeNow) + 1.0f) * 0.5f;

			if (Current.SceneParam->IBL != nullptr)
			{
				const auto& ibl = *Current.SceneParam->IBL;
				for (size_t component = 0; component < ibl.Lights[1].IrradianceRGB.size(); component++)
					outData.IBL.IrradianceRGB[component] = glm::transpose(ibl.Lights[1].IrradianceRGB[component]);

				for (size_t i = 0; i < ibl.Lights.size(); i++)
					outData.IBL.LightColors[i] = vec4(ibl.Lights[i].LightColor, 1.0f);
			}

			auto& camera = *Current.Camera;
			camera.UpdateMatrices();

			outData.Scene.View = glm::transpose(camera.GetView());
			outData.Scene.ViewProjection = glm::transpose(camera.GetViewProjection());
			outData.Scene.EyePosition = vec4(camera.ViewPoint, 1.0f);

			const auto& light = Current.SceneParam->Light;
			outData.CharaLight.Ambient = vec4(light.Character.Ambient, 1.0f);
			outData.CharaLight.Diffuse = vec4(light.Character.Diffuse, 1.0f);
			outData.CharaLight.Specular = light.Character.Specular;
			outData.CharaLight.Direction = vec4(glm::normalize(light.Character.Position), 1.0f);

			outData.StageLight.Ambient = vec4(light.Stage.Ambient, 1.0f);
			outData.StageLight.Diffuse = vec4(light.Stage.Diffuse, 1.0f);
			outData.StageLight.Specular = light.Stage.Specular;
			outData.StageLight.Direction = vec4(glm::normalize(light.Stage.Position), 1.0f);

			const auto& depthFog = Current.SceneParam->Fog.Depth;
			outData.DepthFog.Parameters = vec4(Current.RenderTarget->Param.RenderFog ? depthFog.Density : 0.0f, depthFog.Start, depthFog.End, 1.0f / (depthFog.End - depthFog.Start));
			outData.DepthFog.Color = vec4(depthFog.Color, 1.0f);

			outData.ShadowAmbient = (Current.SceneParam->Light.Shadow.Type == LightSourceType::Parallel) ?
				vec4(Current.SceneParam->Light.Shadow.Ambient, 1.0f) :
				vec4(DefaultShadowAmbient, 1.0);
			outData.OneMinusShadowAmbient = vec4(1.0f) - outData.ShadowAmbient;
			outData.ShadowExponent = DefaultShadowExpontent;

			outData.SubsurfaceScatteringParameter = Current.RenderTarget->Param.RenderSubsurfaceScattering ? Detail::DefaultSSSParameter : 0.0f;

			outData.DebugFlags = Current.RenderTarget->Param.ShaderDebugFlags;
			outData.DebugValue = Current.RenderTarget->Param.ShaderDebugValue;
		}

		void BindUploadSceneCBs()
		{
			ConstantBuffers.Scene.UploadData(D3D11);
			ConstantBuffers.Scene.BindShaders(D3D11);

			ConstantBuffers.Object.BindShaders(D3D11);
			ConstantBuffers.Skeleton.BindVertexShader(D3D11);
		}

		void BindSceneTextures()
		{
			ID3D11ShaderResourceView* sceneTextureViews[TextureSlot_Count] =
			{
				// NOTE: Diffuse = 0
				nullptr,
				// NOTE: Ambient = 1
				nullptr,
				// NOTE: Normal = 2
				nullptr,
				// NOTE: Specular = 3
				nullptr,
				// NOTE: Tranparency = 4
				nullptr,
				// NOTE: Environment = 5
				nullptr,
				// NOTE: Translucency = 6
				nullptr,
				// NOTE: Reserved = 7
				nullptr,

				// NOTE: ---
				nullptr,

				// NOTE: IBLLightMaps_0 = 9
				(Current.SceneParam->IBL == nullptr) ? nullptr : GetD3D11Texture2DView(D3D11, Current.SceneParam->IBL->LightMaps[0]),
				// NOTE: IBLLightMaps_1 = 10
				(Current.SceneParam->IBL == nullptr) ? nullptr : GetD3D11Texture2DView(D3D11, Current.SceneParam->IBL->LightMaps[1]),
				// NOTE: IBLLightMaps_2 = 11
				(Current.SceneParam->IBL == nullptr) ? nullptr : GetD3D11Texture2DView(D3D11, Current.SceneParam->IBL->LightMaps[2]),

				// NOTE: ---
				nullptr,
				// NOTE: ---
				nullptr,
				// NOTE: ---
				nullptr,

				// NOTE: ScreenReflection = 15
				nullptr,

				// NOTE: SubsurfaceScattering = 16
				nullptr,

				// NOTE: ---
				nullptr,

				// NOTE: ---
				nullptr,

				// NOTE: StageShadowMap = 19
				nullptr,
			};

			D3D11.ImmediateContext->PSSetShaderResources(TextureSlot_Diffuse, TextureSlot_Count, sceneTextureViews);
		}

		void PreRenderShadowMap()
		{
			D3D11_BeginDebugEvent("Shadow Map");
			const auto& light = Current.SceneParam->Light.Character;

			ShadowSilhouetteInputLayout->Bind(D3D11);

			SolidNoCullingRasterizerState.Bind(D3D11);

			const Sphere frustumSphere = CalculateShadowViewFrustumSphere();
			constexpr float nearFarTargetSpan = 20.0f - 0.1f;

			const float lightDistance = frustumSphere.Radius;
			const float nearFarPadding = (nearFarTargetSpan / 2.0f) - (frustumSphere.Radius);
			const vec2 nearFarPlane = { -nearFarPadding, (frustumSphere.Radius * 2.0f) + nearFarPadding };

			const vec3 lightViewPoint = glm::normalize(light.Position) * lightDistance;
			const vec3 lightInterest = vec3(0.0f, 0.0f, 0.0f);

			const mat4 lightView = glm::lookAt(
				lightViewPoint + frustumSphere.Center,
				lightInterest + frustumSphere.Center,
				Current.Camera->UpDirection);

			const mat4 lightProjection = glm::ortho(
				-frustumSphere.Radius, +frustumSphere.Radius,
				-frustumSphere.Radius, +frustumSphere.Radius,
				nearFarPlane.x, nearFarPlane.y);

			ConstantBuffers.Scene.Data.Scene.View = glm::transpose(lightView);
			ConstantBuffers.Scene.Data.Scene.ViewProjection = glm::transpose(lightProjection * lightView);
			ConstantBuffers.Scene.UploadData(D3D11);

			Current.RenderTarget->Shadow.RenderTarget.RecreateWithNewSizeIfDifferent(D3D11, Current.RenderTarget->Param.ShadowMapResolution);
			Current.RenderTarget->Shadow.RenderTarget.BindAndSetViewport(D3D11);
			Current.RenderTarget->Shadow.RenderTarget.ClearDepth(D3D11);
			D3D11.ImmediateContext->OMSetBlendState(nullptr, nullptr, D3D11_DEFAULT_SAMPLE_MASK);

			Shaders.Silhouette.Bind(D3D11);
			for (auto& command : DefaultCommandList.OpaqueAndTransparent)
			{
				RenderOpaqueObjCommand(command, RenderFlags_ShadowPass | RenderFlags_NoMaterialShader | RenderFlags_NoRasterizerState | RenderFlags_NoFrustumCulling | RenderFlags_DiffuseTextureOnly);
			}

			Current.RenderTarget->Shadow.RenderTarget.UnBind(D3D11);

			ConstantBuffers.Scene.Data.Scene.View = glm::transpose(Current.Camera->GetView());
			ConstantBuffers.Scene.Data.Scene.ViewProjection = glm::transpose(Current.Camera->GetViewProjection());
			ConstantBuffers.Scene.Data.Scene.LightSpace = glm::transpose(lightProjection * lightView);
			ConstantBuffers.Scene.UploadData(D3D11);
			D3D11_EndDebugEvent();
		}

		void PreRenderReduceFilterShadowMap()
		{
			D3D11_BeginDebugEvent("Shadow Map Filter");
			SolidNoCullingRasterizerState.Bind(D3D11);

			D3D11.ImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			D3D11.ImmediateContext->OMSetBlendState(nullptr, nullptr, D3D11_DEFAULT_SAMPLE_MASK);

			D3D11.ImmediateContext->PSSetShaderResources(0, 1, PtrArg<ID3D11ShaderResourceView*>(nullptr));

			const ivec2 fullResolution = Current.RenderTarget->Shadow.RenderTarget.GetSize();
			const ivec2 halfResolution = fullResolution / 2;
			const ivec2 blurResolution = fullResolution / 4;

			// NOTE: ESM
			{
				ConstantBuffers.ESMFilter.BindPixelShader(D3D11);

				for (auto& renderTarget : Current.RenderTarget->Shadow.ExponentialRenderTargets)
					renderTarget.RecreateWithNewSizeIfDifferent(D3D11, fullResolution);

				Shaders.ESMGauss.Bind(D3D11);
				Current.RenderTarget->Shadow.ExponentialRenderTargets[0].BindAndSetViewport(D3D11);
				Current.RenderTarget->Shadow.RenderTarget.BindDepthTexturePS(D3D11, 0);
				ConstantBuffers.ESMFilter.Data.Coefficients = DefaultShadowCoefficients;
				ConstantBuffers.ESMFilter.Data.TextureStep = vec2(1.0f / fullResolution.x, 0.0f);
				ConstantBuffers.ESMFilter.Data.FarTexelOffset = vec2(DefaultShadowTexelOffset, DefaultShadowTexelOffset);
				ConstantBuffers.ESMFilter.UploadData(D3D11);
				SubmitQuadDrawCall();
				Current.RenderTarget->Shadow.ExponentialRenderTargets[0].UnBind(D3D11);

				Current.RenderTarget->Shadow.ExponentialRenderTargets[1].Bind(D3D11);
				Current.RenderTarget->Shadow.ExponentialRenderTargets[0].BindColorTexturePS(D3D11, 0);
				ConstantBuffers.ESMFilter.Data.TextureStep = vec2(0.0f, 1.0f / fullResolution.y);
				ConstantBuffers.ESMFilter.UploadData(D3D11);
				SubmitQuadDrawCall();
				Current.RenderTarget->Shadow.ExponentialRenderTargets[1].UnBind(D3D11);
				Current.RenderTarget->Shadow.ExponentialRenderTargets[1].BindColorTexturePS(D3D11, TextureSlot_ESMFull);

				for (auto& renderTarget : Current.RenderTarget->Shadow.ExponentialBlurRenderTargets)
					renderTarget.RecreateWithNewSizeIfDifferent(D3D11, blurResolution);

				Shaders.ESMFilterMin.Bind(D3D11);
				Current.RenderTarget->Shadow.ExponentialBlurRenderTargets[0].BindAndSetViewport(D3D11);
				Current.RenderTarget->Shadow.ExponentialRenderTargets[1].BindColorTexturePS(D3D11, 0);
				ConstantBuffers.ESMFilter.Data.TextureStep = vec2(1.0f) / vec2(fullResolution);
				ConstantBuffers.ESMFilter.UploadData(D3D11);
				SubmitQuadDrawCall();
				Current.RenderTarget->Shadow.ExponentialBlurRenderTargets[0].UnBind(D3D11);

				Shaders.ESMFilterErosion.Bind(D3D11);
				Current.RenderTarget->Shadow.ExponentialBlurRenderTargets[1].Bind(D3D11);
				Current.RenderTarget->Shadow.ExponentialBlurRenderTargets[0].BindColorTexturePS(D3D11, 0);
				ConstantBuffers.ESMFilter.Data.TextureStep = vec2(0.75f) / vec2(blurResolution);
				ConstantBuffers.ESMFilter.UploadData(D3D11);
				SubmitQuadDrawCall();
				Current.RenderTarget->Shadow.ExponentialBlurRenderTargets[1].UnBind(D3D11);
				Current.RenderTarget->Shadow.ExponentialBlurRenderTargets[1].BindColorTexturePS(D3D11, TextureSlot_ESMGauss);
			}

			{
				// NOTE: This is the alternative to rendering to a depth and color buffer, is this more performant though, I'm not sure
				Shaders.DepthThreshold.Bind(D3D11);

				Current.RenderTarget->Shadow.ThresholdRenderTarget.RecreateWithNewSizeIfDifferent(D3D11, halfResolution);
				Current.RenderTarget->Shadow.ThresholdRenderTarget.BindAndSetViewport(D3D11);
				Current.RenderTarget->Shadow.RenderTarget.BindDepthTexturePS(D3D11, 0);
				SubmitQuadDrawCall();
				Current.RenderTarget->Shadow.ThresholdRenderTarget.UnBind(D3D11);

				for (auto& renderTarget : Current.RenderTarget->Shadow.BlurRenderTargets)
					renderTarget.RecreateWithNewSizeIfDifferent(D3D11, blurResolution);

				const int blurTargets = static_cast<int>(Current.RenderTarget->Shadow.BlurRenderTargets.size());
				const int blurPasses = Current.RenderTarget->Param.ShadowBlurPasses + 1;

				D3D11.SetViewport(blurResolution);
				Shaders.ImgFilter.Bind(D3D11);

				for (int passIndex = 0; passIndex < blurPasses; passIndex++)
				{
					const int blurIndex = (passIndex % blurTargets);
					const int previousBlurIndex = ((passIndex - 1) + blurTargets) % blurTargets;

					auto& sourceTarget = (passIndex == 0) ? Current.RenderTarget->Shadow.ThresholdRenderTarget : Current.RenderTarget->Shadow.BlurRenderTargets[previousBlurIndex];
					auto& destinationTarget = Current.RenderTarget->Shadow.BlurRenderTargets[blurIndex];

					if (passIndex == 1)
						Shaders.ImgFilterBlur.Bind(D3D11);

					sourceTarget.BindColorTexturePS(D3D11, 0);
					destinationTarget.Bind(D3D11);
					SubmitQuadDrawCall();
					destinationTarget.UnBind(D3D11);

					if (passIndex == (blurPasses - 1))
						destinationTarget.BindColorTexturePS(D3D11, TextureSlot_ShadowMap);
				}
			}
			D3D11_EndDebugEvent();
		}

		void PreRenderScreenReflection()
		{
			D3D11_BeginDebugEvent("Screen Reflection");
			Current.RenderTarget->Reflection.RenderTarget.RecreateWithNewSizeIfDifferent(D3D11, Current.RenderTarget->Param.ReflectionRenderResolution);
			Current.RenderTarget->Reflection.RenderTarget.BindAndSetViewport(D3D11);

			if (Current.RenderTarget->Param.ClearReflection)
				Current.RenderTarget->Reflection.RenderTarget.ClearColor(D3D11, Current.RenderTarget->Param.ClearColor);
			Current.RenderTarget->Reflection.RenderTarget.ClearDepth(D3D11);

			if (!ReflectionCommandList.OpaqueAndTransparent.empty())
			{
				// TODO: Render using cheaper reflection shaders
				if (Current.RenderTarget->Param.RenderOpaque)
				{
					D3D11.ImmediateContext->OMSetBlendState(nullptr, nullptr, D3D11_DEFAULT_SAMPLE_MASK);

					for (auto& command : ReflectionCommandList.OpaqueAndTransparent)
						RenderOpaqueObjCommand(command);
				}

				if (Current.RenderTarget->Param.RenderTransparent && !ReflectionCommandList.Transparent.empty())
				{
					TransparencyPassDepthStencilState.Bind(D3D11);

					for (auto& command : ReflectionCommandList.Transparent)
						RenderTransparentSubMeshCommand(command);

					TransparencyPassDepthStencilState.UnBind(D3D11);
				}
			}

			Current.RenderTarget->Reflection.RenderTarget.UnBind(D3D11);
			D3D11_EndDebugEvent();
		}

		void PreRenderSubsurfaceScattering()
		{
			D3D11_BeginDebugEvent("Subsurface Scattering");
			Current.RenderTarget->SubsurfaceScattering.RenderTarget.RecreateWithNewSizeIfDifferent(D3D11, Current.RenderTarget->Param.RenderResolution);

			Current.RenderTarget->SubsurfaceScattering.RenderTarget.BindAndSetViewport(D3D11);
			Current.RenderTarget->SubsurfaceScattering.RenderTarget.ClearColorAndDepth(D3D11, vec4(0.0f));

			if (!DefaultCommandList.OpaqueAndTransparent.empty())
			{
				D3D11.ImmediateContext->OMSetBlendState(nullptr, nullptr, D3D11_DEFAULT_SAMPLE_MASK);

				for (auto& command : DefaultCommandList.OpaqueAndTransparent)
					RenderOpaqueObjCommand(command, RenderFlags_SSSPass);
			}

			Current.RenderTarget->SubsurfaceScattering.RenderTarget.UnBind(D3D11);
			D3D11_EndDebugEvent();
		}

		void PreRenderReduceFilterSubsurfaceScattering()
		{
			D3D11_BeginDebugEvent("Subsurface Scattering Filter");
			SolidNoCullingRasterizerState.Bind(D3D11);

			D3D11.ImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			D3D11.ImmediateContext->OMSetBlendState(nullptr, nullptr, D3D11_DEFAULT_SAMPLE_MASK);

			D3D11.ImmediateContext->PSSetSamplers(0, 1, PtrArg<ID3D11SamplerState*>(nullptr));

			Shaders.SSSFilterCopy.Bind(D3D11);
			Current.RenderTarget->SubsurfaceScattering.RenderTarget.BindColorTexturePS(D3D11, 0);
			Current.RenderTarget->SubsurfaceScattering.FilterRenderTargets[0].BindAndSetViewport(D3D11);
			SubmitQuadDrawCall();
			Current.RenderTarget->SubsurfaceScattering.FilterRenderTargets[0].UnBind(D3D11);

			Shaders.SSSFilterMin.Bind(D3D11);
			Current.RenderTarget->SubsurfaceScattering.FilterRenderTargets[0].BindColorTexturePS(D3D11, 0);
			Current.RenderTarget->SubsurfaceScattering.FilterRenderTargets[1].BindAndSetViewport(D3D11);
			SubmitQuadDrawCall();
			Current.RenderTarget->SubsurfaceScattering.FilterRenderTargets[1].UnBind(D3D11);

			ConstantBuffers.SSSFilter.Data.TextureSize = GetPackedTextureSize(Current.RenderTarget->SubsurfaceScattering.FilterRenderTargets[1]);
			Detail::CalculateSSSCoefficients(*Current.Camera, ConstantBuffers.SSSFilter.Data);
			ConstantBuffers.SSSFilter.BindPixelShader(D3D11);
			ConstantBuffers.SSSFilter.UploadData(D3D11);
			Shaders.SSSFilterGauss2D.Bind(D3D11);
			Current.RenderTarget->SubsurfaceScattering.FilterRenderTargets[1].BindColorTexturePS(D3D11, 0);
			Current.RenderTarget->SubsurfaceScattering.FilterRenderTargets[2].BindAndSetViewport(D3D11);
			SubmitQuadDrawCall();
			Current.RenderTarget->SubsurfaceScattering.FilterRenderTargets[2].UnBind(D3D11);
			D3D11_EndDebugEvent();
		}

		void RenderOpaqueObjCommand(ObjRenderCommand& command, RenderFlags flags = RenderFlags_None)
		{
			if (command.AreAllMeshesTransparent && !(flags & RenderFlags_ShadowPass) && !(flags & RenderFlags_SSSPass))
				return;

			if (!(flags & RenderFlags_NoFrustumCulling) && !IntersectsCameraFrustum(command.SourceCommand.SourceObj->BoundingSphere, command))
				return;

			IterateCommandMeshes(command.SourceCommand, [&](auto& mesh)
			{
				if (!(flags & RenderFlags_NoFrustumCulling) && !IntersectsCameraFrustum(mesh.BoundingSphere, command))
					return;

				D3D11_BeginDebugEvent("Draw Mesh");
				BindMeshVertexBuffers(mesh, GetMorphMesh(*command.SourceCommand.SourceObj, command.SourceCommand, mesh));

				IterateCommandSubMeshes(command.SourceCommand, mesh, [&](auto& subMesh, auto& material)
				{
					if (!(flags & RenderFlags_ShadowPass) && !(flags & RenderFlags_SSSPass))
					{
						if (Detail::IsMeshTransparent(mesh, subMesh, material))
							return;
					}

					if ((flags & RenderFlags_ShadowPass) && !CastsShadow(command.SourceCommand, mesh, subMesh, material))
						return;

					if ((flags & RenderFlags_SSSPass) && !(Detail::UsesSSSSkin(material) || Detail::UsesSSSSkinConst(material)))
						return;

					if (!(flags & RenderFlags_NoFrustumCulling) && !IntersectsCameraFrustum(subMesh.BoundingSphere, command))
						return;

					PrepareAndRenderSubMesh(command, mesh, subMesh, material, flags);
				});
				D3D11_EndDebugEvent();
			});
		}

		void RenderTransparentSubMeshCommand(SubMeshRenderCommand& command)
		{
			auto& objCommand = command.ObjCommand;
			auto& transform = objCommand->SourceCommand.Transform;
			auto& obj = *objCommand->SourceCommand.SourceObj;
			auto& mesh = *command.ParentMesh;
			auto& subMesh = *command.SubMesh;

			if (!IntersectsCameraFrustum(obj.BoundingSphere, command)
				|| !IntersectsCameraFrustum(mesh.BoundingSphere, command)
				|| !IntersectsCameraFrustum(subMesh.BoundingSphere, command))
				return;

			D3D11_BeginDebugEvent("Draw Sub Mesh");
			BindMeshVertexBuffers(mesh, GetMorphMesh(obj, objCommand->SourceCommand, mesh));

			auto& material = GetSubMeshMaterial(subMesh, command.ObjCommand->SourceCommand);
			Current.RenderTarget->BlendStates.GetState(material.BlendFlags.SrcBlendFactor, material.BlendFlags.DstBlendFactor).Bind(D3D11);

			PrepareAndRenderSubMesh(*command.ObjCommand, mesh, subMesh, material);
			D3D11_EndDebugEvent();
		}

		void RenderSilhouette()
		{
			D3D11_BeginDebugEvent("Silhouette");
			Current.RenderTarget->Silhouette.RenderTarget.RecreateWithNewSizeIfDifferent(D3D11, Current.RenderTarget->Param.RenderResolution);
			Current.RenderTarget->Silhouette.RenderTarget.BindAndSetViewport(D3D11);
			Current.RenderTarget->Silhouette.RenderTarget.ClearColorAndDepth(D3D11, vec4(0.0f));

			if (!DefaultCommandList.OpaqueAndTransparent.empty())
			{
				D3D11.ImmediateContext->OMSetBlendState(nullptr, nullptr, D3D11_DEFAULT_SAMPLE_MASK);

				for (auto& command : DefaultCommandList.OpaqueAndTransparent)
					RenderOpaqueObjCommand(command, RenderFlags_SilhouetteOutlinePass | RenderFlags_NoMaterialShader | RenderFlags_NoMaterialTextures);
			}

			Current.RenderTarget->Silhouette.RenderTarget.UnBind(D3D11);
			D3D11_EndDebugEvent();
		}

		void RenderSilhouetteOutlineOverlay()
		{
			D3D11_BeginDebugEvent("Silhouette Outline");
			Current.RenderTarget->Silhouette.RenderTarget.BindColorTexturePS(D3D11, 0);

			Shaders.SilhouetteOutline.Bind(D3D11);
			SubmitQuadDrawCall();
			D3D11_EndDebugEvent();
		}

		void QueryRenderLensFlareSun()
		{
			D3D11_BeginDebugEvent("Query Lens Flare");
			const vec3 sunWorldPosition = Current.SceneParam->Light.Sun.Position;

			constexpr float sunScreenScaleFactor = 0.045f;
			const float sunCameraeWorldDistance = glm::distance(sunWorldPosition, Current.Camera->ViewPoint);
			const float sunDistanceScaleFactor = (sunCameraeWorldDistance * sunScreenScaleFactor);

			const auto& viewMatrix = Current.Camera->GetView();
			auto viewAlignedTranslation = glm::translate(mat4(1.0f), sunWorldPosition);

			viewAlignedTranslation[0][0] = viewMatrix[0][0];
			viewAlignedTranslation[0][1] = viewMatrix[1][0];
			viewAlignedTranslation[0][2] = viewMatrix[2][0];

			viewAlignedTranslation[1][0] = viewMatrix[0][1];
			viewAlignedTranslation[1][1] = viewMatrix[1][1];
			viewAlignedTranslation[1][2] = viewMatrix[2][1];

			viewAlignedTranslation[2][0] = viewMatrix[0][2];
			viewAlignedTranslation[2][1] = viewMatrix[1][2];
			viewAlignedTranslation[2][2] = viewMatrix[2][2];

			auto renderSun = [&](float scale)
			{
				// NOTE: Unfortunately no constexpr cosine; = 1.0 / (1.0 - glm::cos(glm::radians(3.0)));
				constexpr float diffuseFactor = 729.67921174026924f;

				const auto scaleMatrix = glm::scale(mat4(1.0f), vec3(scale * sunDistanceScaleFactor));
				ConstantBuffers.Object.Data.ModelViewProjection = glm::transpose(Current.Camera->GetViewProjection() * viewAlignedTranslation * scaleMatrix);
				ConstantBuffers.Object.Data.Material.Diffuse = vec4(Current.SceneParam->Light.Sun.Diffuse * diffuseFactor, 1.0f);
				ConstantBuffers.Object.UploadData(D3D11);

				SubmitSubMeshDrawCall(LensFlareMesh.GetSunSubMesh());
			};

			auto queryRenderSun = [&](auto& occlusionQuery, float scale = 1.0f)
			{
				occlusionQuery.QueryData(D3D11);
				occlusionQuery.BeginQuery(D3D11);
				renderSun(scale);
				occlusionQuery.EndQuery(D3D11);
			};

			SolidNoCullingRasterizerState.Bind(D3D11);
			Shaders.Sun.Bind(D3D11);

			D3D11.ImmediateContext->PSSetShaderResources(TextureSlot_Diffuse, 1, PtrArg<ID3D11ShaderResourceView*>(GetD3D11Texture2DView(D3D11, TexGetter(&Current.SceneParam->LensFlare.Textures.Sun))));
			D3D11.ImmediateContext->PSSetSamplers(TextureSlot_Diffuse, 1, PtrArg<ID3D11SamplerState*>(nullptr));

			BindMeshVertexBuffers(LensFlareMesh.GetVertexBufferMesh(), nullptr);

			LensFlareReadDepthDepthStencilState.Bind(D3D11);

			queryRenderSun(Current.RenderTarget->Sun.OcclusionQuery, 1.0f);
			// queryRenderSun(Current.RenderTarget->Sun.OffScreenOcclusionQuery, 1.5f);

			LensFlareSunQueryNoColorWriteBlendState.Bind(D3D11);
			LensFlareIgnoreDepthDepthStencilState.Bind(D3D11);

			queryRenderSun(Current.RenderTarget->Sun.NoDepthOcclusionQuery, 1.0f);

			if (Current.RenderTarget->Param.DebugVisualizeSunOcclusionQuery)
			{
				LensFlareSunQueryNoColorWriteBlendState.UnBind(D3D11);
				LensFlareIgnoreDepthDepthStencilState.Bind(D3D11);

				Shaders.DebugMaterial.Bind(D3D11);
				renderSun(1.0f);
			}
			D3D11_EndDebugEvent();
		}

		void RenderLensFlareGhosts()
		{
			const auto* ghostTexture = GetD3D11Texture2D(D3D11, TexGetter(&Current.SceneParam->LensFlare.Textures.Ghost));
			if (ghostTexture == nullptr)
				return;

			D3D11_BeginDebugEvent("Lens Flare Ghosts");
			TransparencyPassDepthStencilState.Bind(D3D11);

			Current.RenderTarget->BlendStates.GetState(BlendFactor::One, BlendFactor::One).Bind(D3D11);
			Shaders.LensFlare.Bind(D3D11);

			D3D11.ImmediateContext->PSSetShaderResources(TextureSlot_Diffuse, 1, PtrArg<ID3D11ShaderResourceView*>(ghostTexture->TextureView.Get()));
			D3D11.ImmediateContext->PSSetSamplers(TextureSlot_Diffuse, 1, PtrArg<ID3D11SamplerState*>(nullptr));

			BindMeshVertexBuffers(LensFlareMesh.GetVertexBufferMesh(), nullptr);

			auto renderGhost = [&](Detail::LensFlareGhostType type, vec2 normalizedScreenPos, float scale, float rotation, float opacity)
			{
				const auto translation = vec2((normalizedScreenPos.x * 2.0f - 1.0f), (1.0f - normalizedScreenPos.y * 2.0f));

				ConstantBuffers.Object.Data.Material.Transparency = opacity;
				ConstantBuffers.Object.Data.ModelViewProjection = glm::transpose(
					glm::scale(mat4(1.0f), vec3(1.0f, Current.Camera->AspectRatio, 1.0f)) *
					glm::translate(mat4(1.0f), vec3(translation.x, translation.y / Current.Camera->AspectRatio, 0.0f)) *
					glm::rotate(mat4(1.0f), rotation, vec3(0.0f, 0.0f, 1.0f)) *
					glm::scale(mat4(1.0f), vec3(scale))
				);

				ConstantBuffers.Object.UploadData(D3D11);
				SubmitSubMeshDrawCall(LensFlareMesh.GetGhostSubMesh(type));
			};

			constexpr vec2 normalizedCenter = vec2(0.5f, 0.5f);
			const vec2 normalizedSunScreenPosition = Current.Camera->ProjectPointNormalizedScreen(Current.SceneParam->Light.Sun.Position);
			const vec2 sunDirection = glm::normalize(normalizedCenter - normalizedSunScreenPosition);

			const auto& sunOcclusionData = Current.RenderTarget->Sun;
			const float coveredPercentage = (static_cast<float>(sunOcclusionData.OcclusionQuery.GetCoveredPixels()) / static_cast<float>(sunOcclusionData.NoDepthOcclusionQuery.GetCoveredPixels()));

			constexpr float baseScale = 1.7f;
			const float ghostRotation = (glm::atan(sunDirection.x, sunDirection.y));

			for (const auto& info : Detail::LensFlareGhostLayout)
			{
				const vec2 ghostPosition = Detail::GetLensFlareGhostPosition(info, normalizedCenter, normalizedSunScreenPosition);
				const float distancePercentage = (1.1f - glm::distance(ghostPosition, normalizedCenter)) * coveredPercentage;
				const float ghostScale = (((distancePercentage * distancePercentage) * 0.03f) + 0.02f) * info.Scale * baseScale;
				const float ghostOpacity = distancePercentage * info.Opacity * Current.SceneParam->Glow.GhostA;

				if (!glm::isnan(ghostOpacity))
					renderGhost(info.Type, ghostPosition, ghostScale, ghostRotation, ghostOpacity);
			}

			TransparencyPassDepthStencilState.UnBind(D3D11);

			// TODO: Render lens flare flare and shaft textures at sun screen position
			D3D11_EndDebugEvent();
		}

		void RenderPostProcessing()
		{
			D3D11_BeginDebugEvent("Post Processing");
			D3D11.ImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

			constexpr u32 morphAttrbutesFactor = 2;
			constexpr u32 attributesToReset = (VertexAttribute_Count * morphAttrbutesFactor);

			std::array<ID3D11Buffer*, attributesToReset> buffers = {};
			std::array<UINT, attributesToReset> strides = {}, offsets = {};
			D3D11.ImmediateContext->IASetVertexBuffers(0, attributesToReset, buffers.data(), strides.data(), offsets.data());

			SolidNoCullingRasterizerState.Bind(D3D11);
			D3D11.ImmediateContext->OMSetBlendState(nullptr, nullptr, D3D11_DEFAULT_SAMPLE_MASK);

			D3D11.ImmediateContext->PSSetShaderResources(0, 1, PtrArg<ID3D11ShaderResourceView*>(nullptr));

			if (Current.RenderTarget->Param.RenderBloom)
				RenderBloom();

			D3D11_BeginDebugEvent("Tone Mapping");
			Current.RenderTarget->Output.RenderTarget.RecreateWithNewSizeIfDifferent(D3D11, Current.RenderTarget->Param.RenderResolution);
			Current.RenderTarget->Output.RenderTarget.BindAndSetViewport(D3D11);

			Current.RenderTarget->ToneMap.UpdateIfNeeded(Current.SceneParam->Glow);

			const bool autoExposureEnabled = (Current.RenderTarget->Param.AutoExposure && Current.RenderTarget->Param.RenderBloom && Current.SceneParam->Glow.AutoExposure);

			ID3D11ShaderResourceView* shaderResourceViews[4] =
			{
				Current.RenderTarget->Main.CurrentOrResolved().ColorTextureView.Get(),
				Current.RenderTarget->Param.RenderBloom ? Current.RenderTarget->Bloom.CombinedBlurRenderTarget.ColorTextureView.Get() : nullptr,
				Current.RenderTarget->ToneMap.GetLookupTexture()->TextureView.Get(),
				autoExposureEnabled ? Current.RenderTarget->Bloom.ExposureRenderTargets.back().ColorTextureView.Get() : nullptr,
			};
			D3D11.ImmediateContext->PSSetShaderResources(0, static_cast<UINT>(std::size(shaderResourceViews)), shaderResourceViews);

			ConstantBuffers.ToneMap.Data.Exposure = Current.SceneParam->Glow.Exposure;
			ConstantBuffers.ToneMap.Data.Gamma = Current.SceneParam->Glow.Gamma;
			ConstantBuffers.ToneMap.Data.SaturatePower = static_cast<float>(Current.SceneParam->Glow.SaturatePower);
			ConstantBuffers.ToneMap.Data.SaturateCoefficient = Current.SceneParam->Glow.SaturateCoefficient;
			ConstantBuffers.ToneMap.Data.AlphaLerp = Current.RenderTarget->Param.ToneMapPreserveAlpha ? 0.0f : 1.0f;
			ConstantBuffers.ToneMap.Data.AlphaValue = 1.0f;
			ConstantBuffers.ToneMap.Data.AutoExposure = autoExposureEnabled;
			ConstantBuffers.ToneMap.UploadData(D3D11);
			ConstantBuffers.ToneMap.BindPixelShader(D3D11);

			Shaders.ToneMap.Bind(D3D11);
			SubmitQuadDrawCall();
			D3D11_EndDebugEvent();

			ID3D11ShaderResourceView* nullShaderResourceViews[3] = {};
			D3D11.ImmediateContext->PSSetShaderResources(0, static_cast<UINT>(std::size(nullShaderResourceViews)), nullShaderResourceViews);

			Current.RenderTarget->Main.AdvanceRenderTarget();
			D3D11_EndDebugEvent();
		}

		void RenderBloom()
		{
			D3D11_BeginDebugEvent("Bloom");
			auto& bloom = Current.RenderTarget->Bloom;

			bloom.BaseRenderTarget.RecreateWithNewSizeIfDifferent(D3D11, Current.RenderTarget->Param.RenderResolution / 2);

			ConstantBuffers.ReduceTex.Data.CombineBlurred = false;
			ConstantBuffers.ReduceTex.BindPixelShader(D3D11);
			Shaders.ReduceTex.Bind(D3D11);

			for (int i = -1; i < static_cast<int>(bloom.ReduceRenderTargets.size()); i++)
			{
				auto& renderTarget = (i < 0) ? bloom.BaseRenderTarget : bloom.ReduceRenderTargets[i];
				auto& lastRenderTarget = (i < 0) ? Current.RenderTarget->Main.CurrentOrResolved() : (i == 0) ? bloom.BaseRenderTarget : bloom.ReduceRenderTargets[i - 1];

				ConstantBuffers.ReduceTex.Data.TextureSize = GetPackedTextureSize(lastRenderTarget);
				ConstantBuffers.ReduceTex.Data.ExtractBrightness = (i == 0);
				ConstantBuffers.ReduceTex.UploadData(D3D11);

				renderTarget.BindAndSetViewport(D3D11);
				lastRenderTarget.BindColorTexturePS(D3D11, 0);

				SubmitQuadDrawCall();
			}

			if (Current.RenderTarget->Param.AutoExposure && Current.SceneParam->Glow.AutoExposure)
				RenderExposurePreBloom();

			CalculateGaussianBlurKernel(Current.SceneParam->Glow, ConstantBuffers.PPGaussCoef.Data);
			ConstantBuffers.PPGaussCoef.UploadData(D3D11);
			ConstantBuffers.PPGaussCoef.BindPixelShader(D3D11);

			ConstantBuffers.PPGaussTex.Data.FinalPass = false;
			ConstantBuffers.PPGaussTex.BindPixelShader(D3D11);

			Shaders.PPGauss.Bind(D3D11);

			for (int i = 1; i < 4; i++)
			{
				auto* sourceTarget = &bloom.ReduceRenderTargets[i];
				auto* destinationTarget = &bloom.BlurRenderTargets[i];

				ConstantBuffers.PPGaussTex.Data.TextureSize = GetPackedTextureSize(*sourceTarget);

				for (int j = 0; j < 2; j++)
				{
					constexpr vec4 horizontalOffsets = vec4(1.0f, 0.0f, 1.0f, 0.0f);
					constexpr vec4 verticalOffsets = vec4(0.0f, 1.0f, 0.0f, 1.0f);

					ConstantBuffers.PPGaussTex.Data.TextureOffsets = (j % 2 == 0) ? horizontalOffsets : verticalOffsets;
					ConstantBuffers.PPGaussTex.UploadData(D3D11);

					sourceTarget->BindColorTexturePS(D3D11, 0);
					destinationTarget->BindAndSetViewport(D3D11);
					SubmitQuadDrawCall();
					destinationTarget->UnBind(D3D11);

					// NOTE: Ping pong between them to avoid having to use additional render targets
					std::swap(sourceTarget, destinationTarget);
				}
			}

			ConstantBuffers.PPGaussTex.Data.TextureSize = GetPackedTextureSize(bloom.ReduceRenderTargets[0]);
			ConstantBuffers.PPGaussTex.Data.FinalPass = true;
			ConstantBuffers.PPGaussTex.UploadData(D3D11);

			bloom.BlurRenderTargets[0].BindAndSetViewport(D3D11);
			bloom.ReduceRenderTargets[0].BindColorTexturePS(D3D11, 0);
			SubmitQuadDrawCall();

			bloom.CombinedBlurRenderTarget.BindAndSetViewport(D3D11);

			ID3D11ShaderResourceView* combinedBlurInputTargets[4] =
			{
				bloom.BlurRenderTargets[0].ColorTextureView.Get(),
				// NOTE: Use the reduce targets because of the ping pong blur rendering
				bloom.ReduceRenderTargets[1].ColorTextureView.Get(),
				bloom.ReduceRenderTargets[2].ColorTextureView.Get(),
				bloom.ReduceRenderTargets[3].ColorTextureView.Get(),
			};
			D3D11.ImmediateContext->PSSetShaderResources(0, static_cast<UINT>(std::size(combinedBlurInputTargets)), combinedBlurInputTargets);

			ConstantBuffers.ReduceTex.Data.TextureSize = GetPackedTextureSize(bloom.ReduceRenderTargets[3]);
			ConstantBuffers.ReduceTex.Data.ExtractBrightness = false;
			ConstantBuffers.ReduceTex.Data.CombineBlurred = true;
			ConstantBuffers.ReduceTex.UploadData(D3D11);
			ConstantBuffers.ReduceTex.BindPixelShader(D3D11);
			Shaders.ReduceTex.Bind(D3D11);

			SubmitQuadDrawCall();

			if (Current.RenderTarget->Param.AutoExposure && Current.SceneParam->Glow.AutoExposure)
				RenderExposurePostBloom();
			D3D11_EndDebugEvent();
		}

		void RenderExposurePreBloom()
		{
			D3D11_BeginDebugEvent("Exposure Pre Bloom");
			Shaders.ExposureMinify.Bind(D3D11);
			Current.RenderTarget->Bloom.ExposureRenderTargets[0].BindAndSetViewport(D3D11);
			Current.RenderTarget->Bloom.ReduceRenderTargets.back().BindColorTexturePS(D3D11, 0);
			SubmitQuadDrawCall();
			Current.RenderTarget->Bloom.ExposureRenderTargets[0].UnBind(D3D11);
			D3D11_EndDebugEvent();
		}

		void RenderExposurePostBloom()
		{
			D3D11_BeginDebugEvent("Exposure Post Bloom");
			CalculateExposureSpotCoefficients(ConstantBuffers.Exposure.Data);
			ConstantBuffers.Exposure.UploadData(D3D11);
			ConstantBuffers.Exposure.BindPixelShader(D3D11);

			Shaders.ExposureMeasure.Bind(D3D11);
			Current.RenderTarget->Bloom.ExposureRenderTargets[1].BindAndSetViewport(D3D11);
			Current.RenderTarget->Bloom.ExposureRenderTargets[0].BindColorTexturePS(D3D11, 0);
			SubmitQuadDrawCall();
			Current.RenderTarget->Bloom.ExposureRenderTargets[1].UnBind(D3D11);

			Shaders.ExposureAverage.Bind(D3D11);
			Current.RenderTarget->Bloom.ExposureRenderTargets[2].BindAndSetViewport(D3D11);
			Current.RenderTarget->Bloom.ExposureRenderTargets[1].BindColorTexturePS(D3D11, 0);
			SubmitQuadDrawCall();
			Current.RenderTarget->Bloom.ExposureRenderTargets[2].UnBind(D3D11);
			D3D11_EndDebugEvent();
		}

		void BindMeshVertexBuffers(const Mesh& primaryMesh, const Mesh* morphMesh)
		{
			std::array<ID3D11Buffer*, VertexAttribute_Count> buffers;
			std::array<UINT, VertexAttribute_Count> strides;
			std::array<UINT, VertexAttribute_Count> offsets;

			const int meshCount = (morphMesh == nullptr) ? 1 : 2;
			for (int meshIndex = 0; meshIndex < meshCount; meshIndex++)
			{
				const Mesh& mesh = (meshIndex == 0) ? primaryMesh : *morphMesh;

				for (VertexAttribute i = 0; i < VertexAttribute_Count; i++)
				{
					if (auto* vertexBuffer = GetD3D11VertexBuffer(D3D11, mesh, i); vertexBuffer != nullptr)
					{
						buffers[i] = vertexBuffer->Buffer.Get();
						strides[i] = vertexBuffer->BufferDesc.StructureByteStride;
						offsets[i] = 0;
					}
					else
					{
						buffers[i] = nullptr;
						strides[i] = 0;
						offsets[i] = 0;
					}
				}

				const UINT startSlot = MorphVertexAttributeOffset * meshIndex;
				D3D11.ImmediateContext->IASetVertexBuffers(startSlot, VertexAttribute_Count, buffers.data(), strides.data(), offsets.data());
			}
		}

		void PrepareAndRenderSubMesh(const ObjRenderCommand& command, const Mesh& mesh, const SubMesh& subMesh, const Material& material, RenderFlags flags = RenderFlags_None)
		{
			// TODO: Call BindMeshVertexBuffers() here and prevent redundant rebinds by storing last set state

			if (flags & RenderFlags_SilhouetteOutlinePass)
			{
				if (command.SourceCommand.Flags.SilhouetteOutline)
					Shaders.SolidWhite.Bind(D3D11);
				else
					Shaders.SolidBlack.Bind(D3D11);
			}
			else if (!(flags & RenderFlags_NoMaterialShader))
			{
				((flags & RenderFlags_SSSPass) ? GetSSSMaterialShader(material) : GetMaterialShader(command, mesh, subMesh, material)).Bind(D3D11);
			}

			const u32 boundMaterialTexturesFlags = (flags & RenderFlags_NoMaterialTextures) ? 0 : BindMaterialTextures(command, material, flags);

			if (!Current.RenderTarget->Param.Wireframe && !(flags & RenderFlags_NoRasterizerState))
				SetSubMeshRasterizerState(material);

			SetObjectCBMaterialData(material, ConstantBuffers.Object.Data.Material);
			SetObjectCBTransforms(command, mesh, subMesh, material, ConstantBuffers.Object.Data);

			ConstantBuffers.Object.Data.MorphWeight = GetObjectCBMorphWeight(command);
			ConstantBuffers.Object.Data.ShaderFlags = GetObjectCBShaderFlags(command, mesh, subMesh, material, boundMaterialTexturesFlags, flags);

			ConstantBuffers.Object.UploadData(D3D11);

			// DEBUG:
			{
				// subMesh.BoneIndices;
				// ConstantBuffers.Skeleton.UploadData();
			}

			SubmitSubMeshDrawCall(subMesh);
		}

		u32 MaterialTextureTypeToTextureSlot(MaterialTextureType textureType, bool secondColorMap)
		{
			switch (textureType)
			{
			case MaterialTextureType::ColorMap:
				return (secondColorMap) ? TextureSlot_Ambient : TextureSlot_Diffuse;
			case MaterialTextureType::NormalMap:
				return TextureSlot_Normal;
			case MaterialTextureType::SpecularMap:
				return TextureSlot_Specular;
			case MaterialTextureType::TranslucencyMap:
				return TextureSlot_Translucency;
			case MaterialTextureType::TransparencyMap:
				return TextureSlot_Transparency;
			case MaterialTextureType::EnvironmentMapCube:
				return TextureSlot_Environment;

			case MaterialTextureType::None:
			case MaterialTextureType::HeightMap:
			case MaterialTextureType::ReflectionMap:
			case MaterialTextureType::EnvironmentMapSphere:
				return TextureSlot_MaterialTextureCount;

			default:
				// assert(false);
				return TextureSlot_MaterialTextureCount;
			}
		}

		u32 BindMaterialTextures(const ObjRenderCommand& command, const Material& material, RenderFlags flags)
		{
			auto applyTextureTransform = [](mat4& outTransform, const RenderCommand3D::DynamicData::TextureTransform& textureTransform)
			{
				constexpr vec3 centerOffset = vec3(0.5f, 0.5f, 0.0f);
				constexpr vec3 rotationAxis = vec3(0.0f, 0.0f, 1.0f);

				outTransform =
					glm::translate(mat4(1.0f), vec3(textureTransform.Translation, 0.0f))
					* glm::translate(mat4(1.0f), +centerOffset)
					* glm::rotate(mat4(1.0f), glm::radians(textureTransform.Rotation), rotationAxis)
					* glm::translate(outTransform, -centerOffset);
			};

			const RenderCommand3D::DynamicData* dynamic = command.SourceCommand.Dynamic;

			ConstantBuffers.Object.Data.DiffuseRGTC1 = false;
			ConstantBuffers.Object.Data.DiffuseScreenTexture = false;
			ConstantBuffers.Object.Data.AmbientTextureType = 0;

			std::array<ID3D11ShaderResourceView*, TextureSlot_MaterialTextureCount> textureResources = {};
			std::array<ID3D11SamplerState*, TextureSlot_MaterialTextureCount> textureSamplers = {};

			for (auto& materialTexture : material.Textures)
			{
				const bool secondColorMap = textureResources[TextureSlot_Diffuse] != nullptr;
				const auto correspondingTextureSlot = MaterialTextureTypeToTextureSlot(materialTexture.TextureFlags.Type, secondColorMap);

				if (correspondingTextureSlot >= TextureSlot_MaterialTextureCount || textureResources[correspondingTextureSlot] != nullptr)
					continue;

				if (!GetIsTextureSlotUsed(material.ShaderType, material.UsedTexturesFlags, correspondingTextureSlot))
					continue;

				const Cached_TexID* texID = &materialTexture.TextureID;
				auto samplerFlags = materialTexture.SamplerFlags;

				if (dynamic != nullptr)
				{
					for (auto& transform : dynamic->TextureTransforms)
					{
						if (*texID == transform.SourceID)
						{
							samplerFlags.RepeatU = transform.RepeatU.value_or<int>(samplerFlags.RepeatU);
							samplerFlags.RepeatV = transform.RepeatU.value_or<int>(samplerFlags.RepeatV);
						}
					}

					for (auto& pattern : dynamic->TexturePatterns)
					{
						if (*texID == pattern.SourceID && pattern.OverrideID != TexID::Invalid)
							texID = &pattern.OverrideID;
					}
				}

				auto tex = TexGetter(texID);
				if (tex == nullptr)
					continue;

				Current.RenderTarget->TextureSamplers.CreateIfNeeded(Current.RenderTarget->Param.AnistropicFiltering);

				textureResources[correspondingTextureSlot] = GetD3D11Texture2DView(D3D11, *tex);
				textureSamplers[correspondingTextureSlot] = Current.RenderTarget->TextureSamplers.GetSampler(samplerFlags).SamplerState.Get();

				if (correspondingTextureSlot == TextureSlot_Diffuse)
				{
					ConstantBuffers.Object.Data.Material.DiffuseTextureTransform = materialTexture.TextureCoordinateMatrix;

					if (dynamic != nullptr)
					{
						for (const auto& textureTransform : dynamic->TextureTransforms)
							if (textureTransform.SourceID == *texID)
								applyTextureTransform(ConstantBuffers.Object.Data.Material.DiffuseTextureTransform, textureTransform);

						if (*texID == dynamic->ScreenRenderTextureID)
						{
							ConstantBuffers.Object.Data.DiffuseScreenTexture = true;
							textureResources[correspondingTextureSlot] = Current.RenderTarget->Main.PreviousOrResolved().ColorTextureView.Get();

							// HACK: Flip to adjust for the expected OpenGL texture coordinates, problematic because it also effects all other textures using the first TEXCOORD attribute
							ConstantBuffers.Object.Data.Material.DiffuseTextureTransform *= glm::scale(mat4(1.0f), vec3(1.0f, -1.0f, 1.0f));
						}
					}

					if (tex->GetFormat() == TextureFormat::RGTC1)
						ConstantBuffers.Object.Data.DiffuseRGTC1 = true;
				}
				else if (correspondingTextureSlot == TextureSlot_Ambient)
				{
					ConstantBuffers.Object.Data.Material.AmbientTextureTransform = materialTexture.TextureCoordinateMatrix;

					if (dynamic != nullptr)
					{
						for (const auto& textureTransform : dynamic->TextureTransforms)
							if (textureTransform.SourceID == *texID)
								applyTextureTransform(ConstantBuffers.Object.Data.Material.AmbientTextureTransform, textureTransform);
					}

					const auto blendFlags = materialTexture.SamplerFlags.Blend;
					ConstantBuffers.Object.Data.AmbientTextureType = (blendFlags == 0b100) ? 2 : (blendFlags == 0b110) ? 1 : (blendFlags != 0b10000) ? 0 : 3;
				}
			}

			if (flags & RenderFlags_DiffuseTextureOnly)
			{
				D3D11.ImmediateContext->PSSetShaderResources(TextureSlot_Diffuse, 1, &textureResources[TextureSlot_Diffuse]);
				D3D11.ImmediateContext->PSSetSamplers(TextureSlot_Diffuse, 1, &textureSamplers[TextureSlot_Diffuse]);
			}
			else
			{
				D3D11.ImmediateContext->PSSetShaderResources(TextureSlot_Diffuse, static_cast<UINT>(textureResources.size()), textureResources.data());
				D3D11.ImmediateContext->PSSetSamplers(TextureSlot_Diffuse, static_cast<UINT>(textureSamplers.size()), textureSamplers.data());
			}

			u32 boundMaterialTexturesFlags = 0;
			for (size_t textureSlot = 0; textureSlot < textureResources.size(); textureSlot++)
			{
				if (textureResources[textureSlot] != nullptr)
					boundMaterialTexturesFlags |= (1 << textureSlot);
			}
			return boundMaterialTexturesFlags;
		}

		bool GetIsTextureSlotUsed(Material::ShaderTypeIdentifier shaderType, Material::MaterialUsedTextureFlags usedTextureFlags, u32 textureSlot)
		{
			switch (textureSlot)
			{
			case TextureSlot_Diffuse:
				// HACK: Eye materials only set the ColorL1 flag for their diffuse texture
				if (shaderType == Material::ShaderIdentifiers::EyeBall || shaderType == Material::ShaderIdentifiers::EyeLens || shaderType == Material::ShaderIdentifiers::GlassEye)
					return usedTextureFlags.Color || usedTextureFlags.ColorL1;
				return usedTextureFlags.Color;
			case TextureSlot_Ambient:
				return usedTextureFlags.ColorL1;
			case TextureSlot_Normal:
				return usedTextureFlags.Normal;
			case TextureSlot_Specular:
				return usedTextureFlags.Specular;
			case TextureSlot_Transparency:
				return usedTextureFlags.Transparency;
			case TextureSlot_Environment:
				return usedTextureFlags.Environment;
			case TextureSlot_Translucency:
				return usedTextureFlags.Translucency;
			case TextureSlot_Reserved:
				return false;

			default:
				assert(false);
				return false;
			}
		}

		void SetSubMeshRasterizerState(const Material& material)
		{
			if (material.BlendFlags.DoubleSided)
				SolidNoCullingRasterizerState.Bind(D3D11);
			else
				SolidBackfaceCullingRasterizerState.Bind(D3D11);
		}

		void SetObjectCBMaterialData(const Material& material, Detail::ObjectConstantData::MaterialData& outMaterialData) const
		{
			const float fresnel = (((material.ShaderFlags.Fresnel == 0) ? 7.0f : static_cast<float>(material.ShaderFlags.Fresnel) - 1.0f) * 0.12f) * 0.82f;
			const float lineLight = material.ShaderFlags.LineLight * 0.111f;
			outMaterialData.FresnelCoefficients = vec4(fresnel, 0.18f, lineLight, 0.0f);

			outMaterialData.Diffuse = material.Color.Diffuse;
			outMaterialData.Transparency = material.Color.Transparency;
			outMaterialData.Ambient = material.Color.Ambient;
			outMaterialData.Specular = material.Color.Specular;
			outMaterialData.Reflectivity = material.Color.Reflectivity;
			outMaterialData.Emission = material.Color.Emission;

			outMaterialData.Shininess = vec2(
				(material.Color.Shininess >= 0.0f ? material.Color.Shininess : 1.0f),
				(material.ShaderType != Material::ShaderIdentifiers::EyeBall) ? ((material.Color.Shininess - 16.0f) / 112.0f) : 10.0f);

			outMaterialData.Intensity = material.Color.Intensity;
			outMaterialData.BumpDepth = material.BumpDepth;
		}

		void SetObjectCBTransforms(const ObjRenderCommand& command, const Mesh& mesh, const SubMesh& subMesh, const Material& material, Detail::ObjectConstantData& outData) const
		{
			mat4 modelMatrix;
			if (Current.RenderTarget->Param.ObjectBillboarding && (mesh.Flags.FaceCameraPosition || mesh.Flags.FaceCameraView))
			{
				const auto& transform = command.SourceCommand.Transform;

				// TODO: if (mesh.Flags.FaceCameraView)
				const vec3 viewPoint = Current.Camera->ViewPoint;
				const float cameraAngle = glm::atan(transform.Translation.x - viewPoint.x, transform.Translation.z - viewPoint.z);

				modelMatrix = glm::rotate(command.ModelMatrix, cameraAngle - glm::pi<float>() - glm::radians(transform.Rotation.y), vec3(0.0f, 1.0f, 0.0f));
			}
			else
			{
				modelMatrix = command.ModelMatrix;
			}

			outData.Model = glm::transpose(modelMatrix);

			// HACK: Hacky optimization to avoid expensive inverse calculations
			if (&const_cast<Impl*>(this)->GetMaterialShader(command, mesh, subMesh, material) == &Shaders.GlassEye)
				outData.ModelInverse = glm::transpose(glm::inverse(modelMatrix));

#if 0 // TODO:
			outData.ModelView = glm::transpose(Current.Camera->GetView() * modelMatrix);
			outData.ModelViewProjection = glm::transpose(Current.Camera->GetViewProjection() * modelMatrix);
#else
			outData.ModelView = glm::transpose(glm::transpose(ConstantBuffers.Scene.Data.Scene.View) * modelMatrix);
			outData.ModelViewProjection = glm::transpose(glm::transpose(ConstantBuffers.Scene.Data.Scene.ViewProjection) * modelMatrix);
#endif
		}

		vec4 GetObjectCBMorphWeight(const ObjRenderCommand& command) const
		{
			if (command.SourceCommand.Dynamic != nullptr)
			{
				const float morphWeight = command.SourceCommand.Dynamic->MorphWeight;
				return vec4(morphWeight, 1.0f - morphWeight, 0.0f, 0.0f);
			}
			else
			{
				return vec4(0.0f, 0.0f, 0.0f, 0.0f);
			}
		}

		u32 GetObjectCBShaderFlags(const ObjRenderCommand& command, const Mesh& mesh, const SubMesh& subMesh, const Material& material, u32 boundMaterialTexturesFlags, RenderFlags flags) const
		{
			u32 result = 0;

			const bool hasVertexTangents = (mesh.AttributeFlags & VertexAttributeFlags_Tangent);
			const bool hasVertexTexCoords0 = (mesh.AttributeFlags & VertexAttributeFlags_TextureCoordinate0);
			const bool hasVertexTexCoords1 = (mesh.AttributeFlags & VertexAttributeFlags_TextureCoordinate1);

			if (Current.RenderTarget->Param.VertexColoring)
			{
				if (mesh.AttributeFlags & VertexAttributeFlags_Color0)
					result |= Detail::ShaderFlags_VertexColor;
			}

			if (Current.RenderTarget->Param.DiffuseMapping)
			{
				if (hasVertexTexCoords0 && (boundMaterialTexturesFlags & (1 << TextureSlot_Diffuse)))
					result |= Detail::ShaderFlags_DiffuseTexture;
			}

			if (Current.RenderTarget->Param.AmbientOcclusionMapping)
			{
				if (hasVertexTexCoords1 && (boundMaterialTexturesFlags & (1 << TextureSlot_Ambient)))
					result |= Detail::ShaderFlags_AmbientTexture;
			}

			if (Current.RenderTarget->Param.NormalMapping)
			{
				if (hasVertexTangents && (boundMaterialTexturesFlags & (1 << TextureSlot_Normal)))
					result |= Detail::ShaderFlags_NormalTexture;
			}

			if (Current.RenderTarget->Param.SpecularMapping)
			{
				if (hasVertexTexCoords0 && (boundMaterialTexturesFlags & (1 << TextureSlot_Specular)))
					result |= Detail::ShaderFlags_SpecularTexture;
			}

			if (Current.RenderTarget->Param.TransparencyMapping)
			{
				if (hasVertexTexCoords0 && (boundMaterialTexturesFlags & (1 << TextureSlot_Transparency)))
					result |= Detail::ShaderFlags_TransparencyTexture;
			}

			if (Current.RenderTarget->Param.EnvironmentMapping)
			{
				if (boundMaterialTexturesFlags & (1 << TextureSlot_Environment))
					result |= Detail::ShaderFlags_EnvironmentTexture;
			}

			if (Current.RenderTarget->Param.TranslucencyMapping)
			{
				if (hasVertexTexCoords0 && (boundMaterialTexturesFlags & (1 << TextureSlot_Translucency)))
					result |= Detail::ShaderFlags_TranslucencyTexture;
			}

			if (Current.RenderTarget->Param.RenderPunchThrough)
			{
				if ((flags & RenderFlags_ShadowPass) ? Detail::IsMeshShadowPunchThrough(mesh, subMesh, material) : Detail::IsMeshPunchThrough(mesh, subMesh, material))
					result |= Detail::ShaderFlags_PunchThrough;
			}

			if (Current.RenderTarget->Param.RenderFog)
			{
				if (!material.BlendFlags.NoFog && Current.SceneParam->Fog.Depth.Density > 0.0f)
					result |= Detail::ShaderFlags_LinearFog;
			}

			if (Current.RenderTarget->Param.ObjectMorphing)
			{
				if (command.SourceCommand.Dynamic != nullptr && command.SourceCommand.Dynamic->MorphObj != nullptr)
				{
					result |= Detail::ShaderFlags_Morph;
					result |= Detail::ShaderFlags_MorphColor;
				}
			}

			if (Current.RenderTarget->Param.ShadowMapping && IsAnyCommand.CastsShadow && IsAnyCommand.ReceiveShadow)
			{
				if (ReceivesShadows(command.SourceCommand, mesh, subMesh))
					result |= Detail::ShaderFlags_Shadow;
			}

			if (Current.RenderTarget->Param.ShadowMapping && Current.RenderTarget->Param.SelfShadowing && IsAnyCommand.CastsShadow && IsAnyCommand.ReceiveShadow)
			{
				if (ReceivesSelfShadow(command.SourceCommand, mesh, subMesh))
					result |= Detail::ShaderFlags_SelfShadow;
			}

			return result;
		}

		D3D11ShaderPair& GetMaterialShader(const ObjRenderCommand& command, const Mesh& mesh, const SubMesh& subMesh, const Material& material)
		{
			if (Current.RenderTarget->Param.AllowDebugShaderOverride)
			{
				if (command.SourceCommand.SourceObj->Debug.UseDebugMaterial || mesh.Debug.UseDebugMaterial || subMesh.Debug.UseDebugMaterial || material.Debug.UseDebugMaterial)
					return Shaders.DebugMaterial;
			}

			if (material.ShaderType == Material::ShaderIdentifiers::Blinn)
			{
				if (material.ShaderFlags.PhongShading)
					return (material.UsedTexturesFlags.Normal) ? Shaders.BlinnPerFrag : Shaders.BlinnPerVert;

				if (material.ShaderFlags.LambertShading)
					return Shaders.Lambert;

				return Shaders.Constant;
			}

			if (material.ShaderType == Material::ShaderIdentifiers::Item)
				return Shaders.ItemBlinn;

			if (material.ShaderType == Material::ShaderIdentifiers::Stage)
				return Shaders.StageBlinn;

			if (material.ShaderType == Material::ShaderIdentifiers::Skin)
				return Shaders.SkinDefault;

			if (material.ShaderType == Material::ShaderIdentifiers::Hair)
				return (material.ShaderFlags.AnisoDirection != AnisoDirection::Normal) ? Shaders.HairAniso : Shaders.HairDefault;

			if (material.ShaderType == Material::ShaderIdentifiers::Cloth)
				return (material.ShaderFlags.AnisoDirection != AnisoDirection::Normal) ? Shaders.ClothAniso : Shaders.ClothDefault;

			if (material.ShaderType == Material::ShaderIdentifiers::Tights)
				return Shaders.Tights;

			if (material.ShaderType == Material::ShaderIdentifiers::Sky)
				return Shaders.SkyDefault;

			if (material.ShaderType == Material::ShaderIdentifiers::EyeBall)
				return (true) ? Shaders.GlassEye : Shaders.EyeBall;

			if (material.ShaderType == Material::ShaderIdentifiers::EyeLens)
				return Shaders.EyeLens;

			if (material.ShaderType == Material::ShaderIdentifiers::GlassEye)
				return Shaders.GlassEye;

			if (material.ShaderType == Material::ShaderIdentifiers::Water01 || material.ShaderType == Material::ShaderIdentifiers::Water02)
				return Shaders.Water;

			if (material.ShaderType == Material::ShaderIdentifiers::Floor)
				return Shaders.Floor;

			return Shaders.DebugMaterial;
		}

		D3D11ShaderPair& GetSSSMaterialShader(const Material& material)
		{
			if (Detail::UsesSSSSkinConst(material))
				return Shaders.SSSSkinConst;

			return Shaders.SSSSkin;
		}

		void SubmitSubMeshDrawCall(const SubMesh& subMesh)
		{
			const size_t indexCount = subMesh.GetIndexCount();

			if (auto* indexBuffer = GetD3D11IndexBuffer(D3D11, subMesh); indexBuffer != nullptr)
				indexBuffer->Bind(D3D11);

			D3D11.ImmediateContext->IASetPrimitiveTopology(PrimitiveTypeToD3DTopology(subMesh.Primitive));
			D3D11.ImmediateContext->DrawIndexed(static_cast<UINT>(indexCount), 0, 0);

			Statistics.VerticesRendered += indexCount;
		}

		void SubmitQuadDrawCall()
		{
			constexpr UINT quadVertexCount = 6;
			D3D11.ImmediateContext->Draw(quadVertexCount, 0);
		}

		Sphere CalculateShadowViewFrustumSphere() const
		{
			// TODO: If larger than some threshold, split into two (or more)
			// TODO: Shadow casting objects which don't lie within the view frustum *nor* the light frustum should be ignored

			if (!IsAnyCommand.CastsShadow)
				return Sphere { vec3(0.0f), 1.0f };

			bool firstShadowCaster = true;
			vec3 min = {}, max = {};

			auto updateMinMax = [&](const auto& boundingSphere)
			{
				if (firstShadowCaster)
				{
					min.x = (boundingSphere.Center.x - boundingSphere.Radius);
					min.y = (boundingSphere.Center.y - boundingSphere.Radius);
					min.z = (boundingSphere.Center.z - boundingSphere.Radius);

					max.x = (boundingSphere.Center.x + boundingSphere.Radius);
					max.y = (boundingSphere.Center.y + boundingSphere.Radius);
					max.z = (boundingSphere.Center.z + boundingSphere.Radius);
					firstShadowCaster = false;
				}
				else
				{
					min.x = std::min(min.x, boundingSphere.Center.x - boundingSphere.Radius);
					min.y = std::min(min.y, boundingSphere.Center.y - boundingSphere.Radius);
					min.z = std::min(min.z, boundingSphere.Center.z - boundingSphere.Radius);

					max.x = std::max(max.x, boundingSphere.Center.x + boundingSphere.Radius);
					max.y = std::max(max.y, boundingSphere.Center.y + boundingSphere.Radius);
					max.z = std::max(max.z, boundingSphere.Center.z + boundingSphere.Radius);
				}
			};

			for (auto& command : DefaultCommandList.OpaqueAndTransparent)
			{
				if (!command.SourceCommand.Flags.CastsShadow)
					continue;

				// NOTE: Performance optimization to avoid transforming all sub mesh bounding spheres
				if (AllSubMeshesCastShadows(command.SourceCommand))
				{
					updateMinMax(command.TransformedBoundingSphere);
				}
				else
				{
					IterateCommandMeshesAndSubMeshes(command.SourceCommand, [&](auto& mesh, auto& subMesh, auto& material)
					{
						if (!CastsShadow(command.SourceCommand, mesh, subMesh, material))
							return;

						auto transformedBoundingSphere = subMesh.BoundingSphere;
						transformedBoundingSphere.Transform(command.ModelMatrix, command.SourceCommand.Transform.Scale);

						updateMinMax(transformedBoundingSphere);
					});
				}
			}

			constexpr float radiusPadding = 0.05f;
			const vec3 size = (max - min) / 2.0f;

			return Sphere { (min + size), (std::max(size.x, std::max(size.y, size.z))) + radiusPadding };
		}

		bool IntersectsCameraFrustum(const ObjRenderCommand& command) const
		{
			if (!Current.RenderTarget->Param.FrustumCulling)
				return true;

			return Current.Camera->IntersectsViewFrustum(command.TransformedBoundingSphere);
		}

		bool IntersectsCameraFrustum(const Sphere& boundingSphere, const ObjRenderCommand& command) const
		{
			if (!Current.RenderTarget->Param.FrustumCulling)
				return true;

			return Current.Camera->IntersectsViewFrustum(boundingSphere * command.SourceCommand.Transform);
		}

		bool IntersectsCameraFrustum(const Sphere& boundingSphere, const SubMeshRenderCommand& command) const
		{
			return IntersectsCameraFrustum(boundingSphere, *command.ObjCommand);
		}

		bool IsDebugRenderFlagSet(int bitIndex) const
		{
			return Current.RenderTarget->Param.DebugFlags & (1 << bitIndex);
		}
	};

	Renderer3D::Renderer3D(TexGetter texGetter) : impl(std::make_unique<Impl>())
	{
		impl->TexGetter = texGetter;
	}

	Renderer3D::~Renderer3D()
	{
	}

	void Renderer3D::Begin(Camera3D& camera, RenderTarget3D& renderTarget, const SceneParam3D& sceneParam)
	{
		assert(impl->Current.Camera == nullptr);
		D3D11_BeginDebugEvent("Renderer3D::Begin - End");

		impl->LastFrameStatistics = impl->Statistics;
		impl->Statistics = {};

		impl->IsAnyCommand = {};
		impl->Current = { &camera, static_cast<Detail::RenderTarget3DImpl*>(&renderTarget), &sceneParam };
	}

	void Renderer3D::Draw(const RenderCommand3D& command)
	{
		assert(impl->Current.Camera != nullptr);

		if (command.SourceObj == nullptr)
			return;

		if (IsMeshOrSubMeshIndexSpecified(command.Flags.MeshIndex))
		{
			if (command.Flags.MeshIndex >= command.SourceObj->Meshes.size())
				return;

			if (IsMeshOrSubMeshIndexSpecified(command.Flags.SubMeshIndex))
				if (command.Flags.SubMeshIndex >= command.SourceObj->Meshes[command.Flags.MeshIndex].SubMeshes.size())
					return;
		}

		auto& commandList = (command.Flags.IsReflection) ? impl->ReflectionCommandList : impl->DefaultCommandList;

		ObjRenderCommand& newRenderCommand = commandList.OpaqueAndTransparent.emplace_back();
		newRenderCommand.SourceCommand = command;
		newRenderCommand.AreAllMeshesTransparent = false;
		newRenderCommand.ModelMatrix = command.Transform.CalculateMatrix();
		newRenderCommand.TransformedBoundingSphere = command.SourceObj->BoundingSphere;
		newRenderCommand.TransformedBoundingSphere.Transform(newRenderCommand.ModelMatrix, command.Transform.Scale);

		impl->UpdateIsAnyCommandFlags(command);
	}

	void Renderer3D::End()
	{
		assert(impl->Current.Camera != nullptr);

		impl->Flush();
		impl->Current = {};
		D3D11_EndDebugEvent();
	}

	const Graphics::Tex* Renderer3D::GetTexFromTextureID(const Cached_TexID* textureID) const
	{
		return impl->TexGetter(textureID);
	}

	std::unique_ptr<RenderTarget3D> Renderer3D::CreateRenderTarget()
	{
		return std::make_unique<Detail::RenderTarget3DImpl>();
	}
}
