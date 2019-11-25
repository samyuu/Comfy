#include "D3D_Renderer3D.h"
#include "ShaderBytecode/ShaderBytecode.h"

namespace Graphics
{
	namespace
	{
		constexpr D3D11_PRIMITIVE_TOPOLOGY GetPrimitiveTopolgy(PrimitiveType primitive)
		{
			switch (primitive)
			{
			case PrimitiveType::Points:
				return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;

			case PrimitiveType::Lines:
				return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;

			case PrimitiveType::LineStrip:
				return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;

			case PrimitiveType::LineLoop:
				return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;

			case PrimitiveType::Triangles:
				return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

			case PrimitiveType::TriangleStrip:
				return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

			case PrimitiveType::TriangleFan:
			case PrimitiveType::Quads:
			case PrimitiveType::QuadStrip:
			case PrimitiveType::Polygon:
				return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;

			default:
				assert(false);
				return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
			}
		}
	}

	D3D_Renderer3D::D3D_Renderer3D()
		: testShader(Test_VS(), Test_PS()), constantShader(Constant_VS(), Constant_PS()), lambertShader(Lambert_VS(), Lambert_PS())
	{
		// TODO: Give names to all graphics resources
		// D3D_SetObjectDebugName(..., "Renderer3D::...");

		InputElement elements[] =
		{
			{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, VertexAttribute_Position },
			{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, VertexAttribute_Normal },
			{ "TANGENT",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, VertexAttribute_Tangent },
			{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, VertexAttribute_TextureCoordinate0 },
			{ "TEXCOORD",	1, DXGI_FORMAT_R32G32_FLOAT,		0, VertexAttribute_TextureCoordinate1 },
			{ "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, VertexAttribute_Color0 },
			{ "COLOR",		1, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, VertexAttribute_Color1 },
		};

		inputLayout = MakeUnique<D3D_InputLayout>(elements, std::size(elements), testShader.VS);
	}

	void D3D_Renderer3D::Begin(const PerspectiveCamera& camera, const vec4& diffuse, const ParallelLight& stageLight)
	{
		perspectiveCamera = &camera;
		lightDiffuse = &diffuse;
		parallelStageLight = &stageLight;
	}

	void D3D_Renderer3D::Draw(ObjSet* objSet, Obj* obj, vec3 position)
	{
		renderCommandList.emplace_back();
		auto& back = renderCommandList.back();

		back.ObjSet = objSet;
		back.Obj = obj;
		back.Transform = glm::translate(mat4(1.0f), position);
		back.Position = position;
	}

	void D3D_Renderer3D::End()
	{
		InternalFlush();
		perspectiveCamera = nullptr;
	}

	const PerspectiveCamera* D3D_Renderer3D::GetCamera() const
	{
		return perspectiveCamera;
	}

	void D3D_Renderer3D::InternalFlush()
	{
		for (auto& command : renderCommandList)
		{
			for (auto& mesh : command.Obj->Meshes)
			{
				for (auto& subMesh : mesh.SubMeshes)
				{
					if (command.Obj->Materials[subMesh.MaterialIndex].BlendFlags.EnableBlend)
					{
						float cameraDistance = glm::distance(/*command.Position +*/ subMesh.BoundingSphere.Center, perspectiveCamera->Position);
						transparentSubMeshCommands.push_back({ &command, &mesh, &subMesh, cameraDistance });
					}
				}
			}
		}

		if (DEBUG_AlphaSort)
			std::sort(transparentSubMeshCommands.begin(), transparentSubMeshCommands.end(), [](SubMeshRenderCommand& a, SubMeshRenderCommand& b) { return a.CameraDistance > b.CameraDistance; });

		InternalRenderItems();

		transparentSubMeshCommands.clear();
		renderCommandList.clear();
	}

	void D3D_Renderer3D::InternalRenderItems()
	{
		sceneConstantBuffer.Data.Scene.ViewProjection = glm::transpose(perspectiveCamera->GetProjectionMatrix() * perspectiveCamera->GetViewMatrix());
		sceneConstantBuffer.Data.Scene.EyePosition = vec4(perspectiveCamera->Position, 0.0f);
		sceneConstantBuffer.Data.LightDiffuse = *lightDiffuse;
		sceneConstantBuffer.Data.StageLight = *parallelStageLight;
		sceneConstantBuffer.Data.StageLight.Direction = glm::normalize(parallelStageLight->Position);
		sceneConstantBuffer.UploadData();

		sceneConstantBuffer.BindShaders();
		objectConstantBuffer.BindShaders();

		(DEBUG_RenderWireframe ? wireframeRasterizerState : solidBackfaceCullingRasterizerState).Bind();
		inputLayout->Bind();

		if (DEBUG_RenderOpaque)
		{
			D3D.Context->OMSetBlendState(nullptr, nullptr, D3D11_DEFAULT_SAMPLE_MASK);

			for (auto& command : renderCommandList)
				InternalRenderOpaqueObjCommand(command);
		}

		if (DEBUG_RenderTransparent)
		{
			transparencyPassDepthStencilState.Bind();

			for (auto& command : transparentSubMeshCommands)
				InternalRenderTransparentSubMeshCommand(command);

			transparencyPassDepthStencilState.UnBind();
		}
	}

	void D3D_Renderer3D::InternalRenderOpaqueObjCommand(ObjRenderCommand& command)
	{
		for (auto& mesh : command.Obj->Meshes)
		{
			BindMeshVertexBuffers(mesh);

			for (auto& subMesh : mesh.SubMeshes)
			{
				auto& material = command.Obj->Materials[subMesh.MaterialIndex];
				if (material.BlendFlags.EnableBlend)
					continue;

				UpdateSubMeshShaderState(subMesh, material, command.ObjSet);
				UpdateObjectConstantBuffer(mesh, material, command.Transform);
				SubmitSubMeshDrawCall(subMesh);
			}
		}
	}

	void D3D_Renderer3D::InternalRenderTransparentSubMeshCommand(SubMeshRenderCommand& command)
	{
		auto& subMesh = *command.SubMesh;
		auto& mesh = *command.ParentMesh;

		BindMeshVertexBuffers(mesh);
		auto& material = command.ObjCommand->Obj->Materials[subMesh.MaterialIndex];

		auto blendState = CreateMaterialBlendState(material);
		blendState.Bind();

		UpdateObjectConstantBuffer(mesh, material, command.ObjCommand->Transform);
		UpdateSubMeshShaderState(subMesh, material, command.ObjCommand->ObjSet);
		SubmitSubMeshDrawCall(subMesh);
	}

	void D3D_Renderer3D::BindMeshVertexBuffers(Mesh& mesh)
	{
		std::array<ID3D11Buffer*, VertexAttribute_Count> buffers;
		std::array<UINT, VertexAttribute_Count> strides;
		std::array<UINT, VertexAttribute_Count> offsets;

		for (VertexAttribute i = 0; i < VertexAttribute_Count; i++)
		{
			D3D_StaticVertexBuffer* vertexBuffer = mesh.GraphicsAttributeBuffers[i].get();

			if (vertexBuffer != nullptr)
			{
				buffers[i] = vertexBuffer->GetBuffer();
				strides[i] = vertexBuffer->GetDescription().StructureByteStride;
				offsets[i] = 0;
			}
			else
			{
				buffers[i] = nullptr;
				strides[i] = 0;
				offsets[i] = 0;
			}
		}

		D3D.Context->IASetVertexBuffers(0, VertexAttribute_Count, buffers.data(), strides.data(), offsets.data());
	}

	void D3D_Renderer3D::UpdateObjectConstantBuffer(Mesh& mesh, Material& material, const mat4& model)
	{
		objectConstantBuffer.Data.Material.DiffuseColor = material.DiffuseColor;
		objectConstantBuffer.Data.Material.Transparency = material.Transparency;
		objectConstantBuffer.Data.Material.AmbientColor = material.AmbientColor;
		objectConstantBuffer.Data.Material.SpecularColor = material.SpecularColor;
		objectConstantBuffer.Data.Material.Reflectivity = material.Reflectivity;
		objectConstantBuffer.Data.Material.EmissionColor = material.EmissionColor;
		objectConstantBuffer.Data.Material.Shininess = material.Shininess;
		objectConstantBuffer.Data.Material.Intensity = material.Intensity;
		objectConstantBuffer.Data.Material.BumpDepth = material.BumpDepth;
		objectConstantBuffer.Data.Material.DiffuseTextureTransform = glm::transpose(material.Diffuse.TextureCoordinateMatrix);
		objectConstantBuffer.Data.Material.AmbientTextureTransform = glm::transpose(material.Ambient.TextureCoordinateMatrix);

		objectConstantBuffer.Data.Model = glm::transpose(model);
		objectConstantBuffer.Data.ShaderFlags = 0;

		if (mesh.AttributeFlags & VertexAttributeFlags_Color0)
			objectConstantBuffer.Data.ShaderFlags |= ShaderFlags_VertexColor;

		if (material.Flags.UseDiffuseTexture || material.Diffuse.TextureID != -1)
			objectConstantBuffer.Data.ShaderFlags |= ShaderFlags_DiffuseTexture;

		if (material.Flags.UseAmbientTexture || material.Ambient.TextureID != -1)
			objectConstantBuffer.Data.ShaderFlags |= ShaderFlags_AmbientTexture;

		// TODO: Is this correct (?)
		if (material.BlendFlags.EnableAlphaTest)
			objectConstantBuffer.Data.ShaderFlags |= ShaderFlags_AlphaTest;

		objectConstantBuffer.UploadData();
	}

	D3D_BlendState D3D_Renderer3D::CreateMaterialBlendState(Material& material)
	{
		auto getD3DBlend = [](BlendFactor materialBlendFactor)
		{
			switch (materialBlendFactor)
			{
			case BlendFactor_ZERO: return D3D11_BLEND_ZERO;
			case BlendFactor_ONE: return D3D11_BLEND_ONE;
			case BlendFactor_SRC_COLOR: return D3D11_BLEND_SRC_COLOR;
			case BlendFactor_ISRC_COLOR: return D3D11_BLEND_INV_SRC_COLOR;
			case BlendFactor_SRC_ALPHA:	return D3D11_BLEND_SRC_ALPHA;
			case BlendFactor_ISRC_ALPHA: return D3D11_BLEND_INV_SRC_ALPHA;
			case BlendFactor_DST_ALPHA: return D3D11_BLEND_DEST_ALPHA;
			case BlendFactor_IDST_ALPHA: return D3D11_BLEND_INV_DEST_ALPHA;
			case BlendFactor_DST_COLOR: return D3D11_BLEND_DEST_COLOR;
			case BlendFactor_IDST_COLOR: return D3D11_BLEND_INV_DEST_COLOR;
			default: return D3D11_BLEND_ZERO;
			}
		};

		return { getD3DBlend(material.BlendFlags.SrcBlendFactor), getD3DBlend(material.BlendFlags.DstBlendFactor), D3D11_BLEND_INV_DEST_ALPHA, D3D11_BLEND_ONE };
	}

	D3D_ShaderPair& D3D_Renderer3D::GetMaterialShader(Material& material)
	{
		if (strcmp(material.Shader, "BLINN") == 0)
		{
			if (material.ShaderFlags.LightingModel_Lambert)
				return lambertShader;
			else if (material.ShaderFlags.LightingModel_Phong)
				return lambertShader /*blinnPerVertex*/;
			else
				return constantShader;
		}
		else
		{
			return testShader;
		}
	}

	void D3D_Renderer3D::UpdateSubMeshShaderState(SubMesh& subMesh, Material& material, ObjSet* objSet)
	{
		auto& materialShader = GetMaterialShader(material);
		materialShader.Bind();

		auto bindMaterialTexture = [](ObjSet* objSet, MaterialTexture& materialTexture, int slot)
		{
			if (materialTexture.TextureID == -1)
			{
				ID3D11ShaderResourceView* resourceView = nullptr;
				D3D.Context->PSSetShaderResources(slot, 1, &resourceView);
				return;
			}

			for (uint32_t i = 0; i < objSet->TextureIDs.size(); i++)
			{
				if (objSet->TextureIDs[i] == materialTexture.TextureID)
				{
					if (objSet->TxpSet->Txps[i].Texture2D != nullptr)
					{
						objSet->TxpSet->Txps[i].Texture2D->Bind(slot);
					}
					else if (objSet->TxpSet->Txps[i].CubeMap != nullptr)
					{
						objSet->TxpSet->Txps[i].CubeMap->Bind(slot);
					}
					else
					{
						ID3D11ShaderResourceView* resourceView = nullptr;
						D3D.Context->PSSetShaderResources(slot, 1, &resourceView);
					}

					const auto flags = materialTexture.Flags;
					D3D_TextureSampler diffuseSampler =
					{
						D3D11_FILTER_MIN_MAG_MIP_LINEAR,
						flags.TextureAddressMode_U_Mirror ? D3D11_TEXTURE_ADDRESS_MIRROR : flags.TextureAddressMode_U_Repeat ? D3D11_TEXTURE_ADDRESS_WRAP : D3D11_TEXTURE_ADDRESS_CLAMP,
						flags.TextureAddressMode_V_Mirror ? D3D11_TEXTURE_ADDRESS_MIRROR : flags.TextureAddressMode_V_Repeat ? D3D11_TEXTURE_ADDRESS_WRAP : D3D11_TEXTURE_ADDRESS_CLAMP,
					};
					diffuseSampler.Bind(slot);

					return;
				}
			}
		};

		bindMaterialTexture(objSet, material.Diffuse, 0);
		bindMaterialTexture(objSet, material.Ambient, 1);
		bindMaterialTexture(objSet, material.Reflection, 2);

		if (!DEBUG_RenderWireframe)
			((material.BlendFlags.DoubleSidedness != DoubleSidedness_Off) ? solidNoCullingRasterizerState : solidBackfaceCullingRasterizerState).Bind();
	}

	void D3D_Renderer3D::SubmitSubMeshDrawCall(SubMesh& subMesh)
	{
		subMesh.GraphicsIndexBuffer->Bind();

		D3D.Context->IASetPrimitiveTopology(GetPrimitiveTopolgy(subMesh.Primitive));
		D3D.Context->DrawIndexed(static_cast<UINT>(subMesh.Indices.size()), 0, 0);
	}
}
