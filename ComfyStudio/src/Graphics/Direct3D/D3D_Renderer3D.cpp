#include "D3D_Renderer3D.h"

namespace Graphics
{
	namespace
	{
		void TryBindLightMap(LightMap& lightMap, int slot)
		{
			if (lightMap.CubeMap != nullptr)
				lightMap.CubeMap->Bind(slot);
		}

		constexpr D3D11_PRIMITIVE_TOPOLOGY GetD3DPrimitiveTopolgy(PrimitiveType primitive)
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

		constexpr D3D11_BLEND GetD3DBlend(BlendFactor materialBlendFactor)
		{
			switch (materialBlendFactor)
			{
			case BlendFactor_ZERO:
				return D3D11_BLEND_ZERO;

			case BlendFactor_ONE:
				return D3D11_BLEND_ONE;

			case BlendFactor_SRC_COLOR:
				return D3D11_BLEND_SRC_COLOR;

			case BlendFactor_ISRC_COLOR:
				return D3D11_BLEND_INV_SRC_COLOR;

			case BlendFactor_SRC_ALPHA:
				return D3D11_BLEND_SRC_ALPHA;

			case BlendFactor_ISRC_ALPHA:
				return D3D11_BLEND_INV_SRC_ALPHA;

			case BlendFactor_DST_ALPHA:
				return D3D11_BLEND_DEST_ALPHA;

			case BlendFactor_IDST_ALPHA:
				return D3D11_BLEND_INV_DEST_ALPHA;

			case BlendFactor_DST_COLOR:
				return D3D11_BLEND_DEST_COLOR;

			case BlendFactor_IDST_COLOR:
				return D3D11_BLEND_INV_DEST_COLOR;

			default:
				assert(false);
				return D3D11_BLEND_ZERO;
			}
		}

		Txp* FindObjSetTxpFromID(ObjSet* objSet, uint32_t textureID)
		{
			if (textureID == -1)
				return nullptr;

			for (uint32_t i = 0; i < objSet->TextureIDs.size(); i++)
			{
				if (objSet->TextureIDs[i] == textureID)
					return &objSet->TxpSet->Txps[i];
			}

			return nullptr;
		}

		bool IsTransparent(ObjSet* objSet, Mesh& mesh, Material& material)
		{
			if (material.BlendFlags.EnableBlend)
				return true;

			if (mesh.Flags.Transparent)
				return true;

			return false;
		}
	}

	D3D_Renderer3D::D3D_Renderer3D()
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

		genericInputLayout = MakeUnique<D3D_InputLayout>(elements, std::size(elements), shaders.Debug.VS);
	}

	void D3D_Renderer3D::Begin(SceneContext& scene)
	{
		sceneContext = &scene;
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
		sceneContext = nullptr;
	}

	const SceneContext* D3D_Renderer3D::GetSceneContext() const
	{
		return sceneContext;
	}

	void D3D_Renderer3D::InternalFlush()
	{
		InternalPrepareRenderCommands();
		InternalRenderItems();
		InternalRenderPostProcessing();

		transparentSubMeshCommands.clear();
		renderCommandList.clear();
	}

	void D3D_Renderer3D::InternalPrepareRenderCommands()
	{
		for (auto& command : renderCommandList)
		{
			for (auto& mesh : command.Obj->Meshes)
			{
				for (auto& subMesh : mesh.SubMeshes)
				{
					auto& material = command.Obj->Materials[subMesh.MaterialIndex];

					if (IsTransparent(command.ObjSet, mesh, material))
					{
						float cameraDistance = glm::distance(command.Position + subMesh.BoundingSphere.Center, sceneContext->Camera.Position);
						transparentSubMeshCommands.push_back({ &command, &mesh, &subMesh, cameraDistance });
					}
				}
			}
		}

		if (sceneContext->RenderParameters.AlphaSort)
		{
			std::sort(transparentSubMeshCommands.begin(), transparentSubMeshCommands.end(), [](SubMeshRenderCommand& a, SubMeshRenderCommand& b)
			{
				return a.CameraDistance > b.CameraDistance;
			});
		}
	}

	void D3D_Renderer3D::InternalRenderItems()
	{
		sceneConstantBuffer.Data.IrradianceRed = glm::transpose(sceneContext->IBL.Stage.IrradianceRGB[0]);
		sceneConstantBuffer.Data.IrradianceGreen = glm::transpose(sceneContext->IBL.Stage.IrradianceRGB[1]);
		sceneConstantBuffer.Data.IrradianceBlue = glm::transpose(sceneContext->IBL.Stage.IrradianceRGB[2]);
		sceneConstantBuffer.Data.Scene.View = glm::transpose(sceneContext->Camera.GetViewMatrix());
		sceneConstantBuffer.Data.Scene.ViewProjection = glm::transpose(sceneContext->Camera.GetProjectionMatrix() * sceneContext->Camera.GetViewMatrix());
		sceneConstantBuffer.Data.Scene.EyePosition = vec4(sceneContext->Camera.Position, 0.0f);
		sceneConstantBuffer.Data.LightColor = vec4(sceneContext->IBL.Stage.LightColor, 1.0f);
		sceneConstantBuffer.Data.StageLight.Ambient = vec4(sceneContext->Light.Stage.Ambient, 1.0f);
		sceneConstantBuffer.Data.StageLight.Diffuse = vec4(sceneContext->Light.Stage.Diffuse, 1.0f);
		sceneConstantBuffer.Data.StageLight.Specular = vec4(sceneContext->Light.Stage.Specular, 1.0f);
		sceneConstantBuffer.Data.StageLight.Direction = vec4(glm::normalize(sceneContext->Light.Stage.Position), 1.0f);
		sceneConstantBuffer.UploadData();

		sceneConstantBuffer.BindShaders();
		objectConstantBuffer.BindShaders();

		TryBindLightMap(sceneContext->IBL.Character.LightMap, 9);
		TryBindLightMap(sceneContext->IBL.Sun.LightMap, 10);
		TryBindLightMap(sceneContext->IBL.Reflect.LightMap, 11);
		TryBindLightMap(sceneContext->IBL.Shadow.LightMap, 12);
		TryBindLightMap(sceneContext->IBL.CharacterColor.LightMap, 13);

		if (sceneContext->RenderParameters.Wireframe)
			wireframeRasterizerState.Bind();

		genericInputLayout->Bind();

		sceneContext->RenderTarget.Bind();
		D3D.SetViewport(sceneContext->RenderTarget.GetSize());

		if (sceneContext->RenderParameters.Clear)
			sceneContext->RenderTarget.Clear(sceneContext->RenderParameters.ClearColor);
		else
			sceneContext->RenderTarget.GetDepthBuffer()->Clear();

		if (sceneContext->RenderParameters.RenderOpaque)
		{
			D3D.Context->OMSetBlendState(nullptr, nullptr, D3D11_DEFAULT_SAMPLE_MASK);

			for (auto& command : renderCommandList)
				InternalRenderOpaqueObjCommand(command);
		}

		if (sceneContext->RenderParameters.WireframeOverlay)
			InternalRenderWireframeOverlay();

		if (sceneContext->RenderParameters.RenderTransparent)
		{
			transparencyPassDepthStencilState.Bind();

			for (auto& command : transparentSubMeshCommands)
				InternalRenderTransparentSubMeshCommand(command);

			transparencyPassDepthStencilState.UnBind();
		}

		sceneContext->RenderTarget.UnBind();
	}

	void D3D_Renderer3D::InternalRenderOpaqueObjCommand(ObjRenderCommand& command)
	{
		for (auto& mesh : command.Obj->Meshes)
		{
			BindMeshVertexBuffers(mesh);

			for (auto& subMesh : mesh.SubMeshes)
			{
				auto& material = command.Obj->Materials[subMesh.MaterialIndex];
				if (IsTransparent(command.ObjSet, mesh, material))
					continue;

				PrepareAndRenderSubMesh(command, mesh, subMesh, material, command.Transform);
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

		PrepareAndRenderSubMesh(*command.ObjCommand, mesh, subMesh, material, command.ObjCommand->Transform);
	}

	void D3D_Renderer3D::InternalRenderWireframeOverlay()
	{
		wireframeRasterizerState.Bind();
		shaders.Debug.Bind();
		currentlyRenderingWireframeOverlay = true;

		for (auto& command : renderCommandList)
		{
			for (auto& mesh : command.Obj->Meshes)
			{
				BindMeshVertexBuffers(mesh);
				for (auto& subMesh : mesh.SubMeshes)
					PrepareAndRenderSubMesh(command, mesh, subMesh, command.Obj->Materials[subMesh.MaterialIndex], command.Transform);
			}
		}

		currentlyRenderingWireframeOverlay = false;
	}

	void D3D_Renderer3D::InternalRenderPostProcessing()
	{
		if (toneMapData.NeedsUpdating(sceneContext))
		{
			toneMapData.Glow = sceneContext->Glow;
			toneMapData.Update();
		}

		postProcessConstantBuffer.Data.Exposure = sceneContext->Glow.Exposure;
		postProcessConstantBuffer.Data.Gamma = sceneContext->Glow.Gamma;
		postProcessConstantBuffer.Data.SaturatePower = sceneContext->Glow.SaturatePower;
		postProcessConstantBuffer.Data.SaturateCoefficient = sceneContext->Glow.SaturateCoefficient;
		postProcessConstantBuffer.UploadData();
		postProcessConstantBuffer.BindVertexShader();

		postProcessInputLayout.Bind();

		std::array<ID3D11Buffer*, VertexAttribute_Count> buffers = {};
		std::array<UINT, VertexAttribute_Count> strides = {}, offsets = {};
		D3D.Context->IASetVertexBuffers(0, VertexAttribute_Count, buffers.data(), strides.data(), offsets.data());

		solidNoCullingRasterizerState.Bind();
		D3D.Context->OMSetBlendState(nullptr, nullptr, D3D11_DEFAULT_SAMPLE_MASK);

		sceneContext->OutputRenderTarget->Bind();
		D3D.SetViewport(sceneContext->OutputRenderTarget->GetSize());

		std::array<ID3D11SamplerState*, 1> samplers = { nullptr };
		D3D.Context->PSSetSamplers(0, static_cast<UINT>(samplers.size()), samplers.data());

		std::array renderTargetResourceViews = { sceneContext->RenderTarget.GetResourceView(), toneMapData.LookupTexture->GetResourceView() };
		D3D.Context->PSSetShaderResources(0, static_cast<UINT>(renderTargetResourceViews.size()), renderTargetResourceViews.data());

		shaders.ToneMap.Bind();

		constexpr UINT rectangleVertexCount = 6;
		D3D.Context->Draw(rectangleVertexCount, 0);

		renderTargetResourceViews = { nullptr, nullptr };
		D3D.Context->PSSetShaderResources(0, static_cast<UINT>(renderTargetResourceViews.size()), renderTargetResourceViews.data());
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

	void D3D_Renderer3D::PrepareAndRenderSubMesh(ObjRenderCommand& command, Mesh& mesh, SubMesh& subMesh, Material& material, const mat4& model)
	{
		auto& materialShader = GetMaterialShader(material);
		materialShader.Bind();

		ObjSet* objSet = command.ObjSet;
		CheckBindMaterialTexture(objSet, material.Diffuse, 0, objectConstantBuffer.Data.TextureFormats.Diffuse);
		CheckBindMaterialTexture(objSet, material.Ambient, 1, objectConstantBuffer.Data.TextureFormats.Ambient);
		CheckBindMaterialTexture(objSet, material.Normal, 2, objectConstantBuffer.Data.TextureFormats.Normal);
		CheckBindMaterialTexture(objSet, material.Specular, 3, objectConstantBuffer.Data.TextureFormats.Specular);
		CheckBindMaterialTexture(objSet, material.Reflection, 5, objectConstantBuffer.Data.TextureFormats.Reflection);

		if (!currentlyRenderingWireframeOverlay && !sceneContext->RenderParameters.Wireframe)
			((material.BlendFlags.DoubleSidedness != DoubleSidedness_Off) ? solidNoCullingRasterizerState : solidBackfaceCullingRasterizerState).Bind();

		objectConstantBuffer.Data.Material.Diffuse = material.DiffuseColor;
		objectConstantBuffer.Data.Material.Transparency = material.Transparency;
		objectConstantBuffer.Data.Material.Ambient = material.AmbientColor;
		objectConstantBuffer.Data.Material.Specular = material.SpecularColor;
		objectConstantBuffer.Data.Material.Reflectivity = material.Reflectivity;
		objectConstantBuffer.Data.Material.Emission = material.EmissionColor;
		objectConstantBuffer.Data.Material.Shininess = (material.Shininess - 16.0f) / 112.0f;
		objectConstantBuffer.Data.Material.Intensity = material.Intensity;
		objectConstantBuffer.Data.Material.BumpDepth = material.BumpDepth;
		objectConstantBuffer.Data.Material.DiffuseTextureTransform = glm::transpose(material.Diffuse.TextureCoordinateMatrix);
		objectConstantBuffer.Data.Material.AmbientTextureTransform = glm::transpose(material.Ambient.TextureCoordinateMatrix);

		if (mesh.Flags.FaceCamera)
		{
			const float cameraAngle = glm::atan(command.Position.x - sceneContext->Camera.Position.x, command.Position.z - sceneContext->Camera.Position.z);
			const mat4 billboardModel = glm::rotate(glm::translate(glm::mat4(1.0f), command.Position), cameraAngle - glm::pi<float>(), vec3(0.0f, 1.0f, 0.0f));

			objectConstantBuffer.Data.Model = glm::transpose(billboardModel);
		}
		else
		{
			objectConstantBuffer.Data.Model = glm::transpose(model);
		}

		objectConstantBuffer.Data.ShaderFlags = 0;

		if (mesh.AttributeFlags & VertexAttributeFlags_Color0)
			objectConstantBuffer.Data.ShaderFlags |= ShaderFlags_VertexColor;

		if (material.Flags.UseDiffuseTexture || material.Diffuse.TextureID != -1)
			objectConstantBuffer.Data.ShaderFlags |= ShaderFlags_DiffuseTexture;

		if (material.Flags.UseAmbientTexture || material.Ambient.TextureID != -1)
			objectConstantBuffer.Data.ShaderFlags |= ShaderFlags_AmbientTexture;

		if (material.Flags.UseNormalTexture || material.Normal.TextureID != -1)
			objectConstantBuffer.Data.ShaderFlags |= ShaderFlags_NormalTexture;

		if (material.Flags.UseSpecularTexture || material.Specular.TextureID != -1)
			objectConstantBuffer.Data.ShaderFlags |= ShaderFlags_SpecularTexture;

		if (material.BlendFlags.EnableAlphaTest && !(material.BlendFlags.EnableBlend || mesh.Flags.Transparent))
			objectConstantBuffer.Data.ShaderFlags |= ShaderFlags_AlphaTest;

		if (material.Flags.UseCubeMapReflection || material.Reflection.TextureID != -1)
			objectConstantBuffer.Data.ShaderFlags |= ShaderFlags_CubeMapReflection;

		objectConstantBuffer.UploadData();

		SubmitSubMeshDrawCall(subMesh);
	}

	D3D_BlendState D3D_Renderer3D::CreateMaterialBlendState(Material& material)
	{
		return { GetD3DBlend(material.BlendFlags.SrcBlendFactor), GetD3DBlend(material.BlendFlags.DstBlendFactor), D3D11_BLEND_INV_DEST_ALPHA, D3D11_BLEND_ONE };
	}

	D3D_ShaderPair& D3D_Renderer3D::GetMaterialShader(Material& material)
	{
		if (std::strcmp(material.Shader, "BLINN") == 0)
		{
			if (material.ShaderFlags.LightingModel_Phong)
			{
				return (material.Flags.UseNormalTexture) ? shaders.BlinnPerFrag : shaders.BlinnPerVert;
			}
			else if (material.ShaderFlags.LightingModel_Lambert)
			{
				return shaders.Lambert;
			}
			else
			{
				return shaders.Constant;
			}
		}
		else if (std::strcmp(material.Shader, "STAGE") == 0)
		{
			if (sceneContext->RenderParameters.DebugFlags & (1 << 0))
				return shaders.BlinnPerFrag;

			return shaders.StageBlinn;
		}
		else if (std::strcmp(material.Shader, "ITEM") == 0)
		{
			return shaders.ItemBlinn;
		}
		else if (std::strcmp(material.Shader, "SKY") == 0)
		{
			return shaders.SkyDefault;
		}
		else if (std::strcmp(material.Shader, "WATER01") == 0 || std::strcmp(material.Shader, "WATER02") == 0)
		{
			return shaders.Water;
		}
		else
		{
			return shaders.Debug;
		}
	}

	D3D_TextureSampler D3D_Renderer3D::CreateTextureSampler(MaterialTexture& materialTexture, TextureFormat format)
	{
		const auto flags = materialTexture.Flags;
		return
		{
			(sceneContext->RenderParameters.AnistropicFiltering > D3D11_MIN_MAXANISOTROPY) ? D3D11_FILTER_ANISOTROPIC : D3D11_FILTER_MIN_MAG_MIP_LINEAR,
			flags.TextureAddressMode_U_Mirror ? D3D11_TEXTURE_ADDRESS_MIRROR : flags.TextureAddressMode_U_Repeat ? D3D11_TEXTURE_ADDRESS_WRAP : D3D11_TEXTURE_ADDRESS_CLAMP,
			flags.TextureAddressMode_V_Mirror ? D3D11_TEXTURE_ADDRESS_MIRROR : flags.TextureAddressMode_V_Repeat ? D3D11_TEXTURE_ADDRESS_WRAP : D3D11_TEXTURE_ADDRESS_CLAMP,
			// TODO: This might need to be scaled first, or is this even used (?)
			// static_cast<float>(materialTexture.Flags.MipMapBias),
			((sceneContext->RenderParameters.DebugFlags & (1 << 1)) && format != TextureFormat::RGTC1 && format != TextureFormat::RGTC2) ? -1.0f : 0.0f,
			sceneContext->RenderParameters.AnistropicFiltering
		};
	}

	void D3D_Renderer3D::CheckBindMaterialTexture(ObjSet* objSet, MaterialTexture& materialTexture, int slot, TextureFormat& constantBufferTextureFormat)
	{
		const Txp* txp = FindObjSetTxpFromID(objSet, materialTexture.TextureID);

		if (txp == nullptr || (txp->Texture2D == nullptr && txp->CubeMap == nullptr))
		{
			ID3D11ShaderResourceView* resourceView = nullptr;
			D3D.Context->PSSetShaderResources(slot, 1, &resourceView);

			constantBufferTextureFormat = TextureFormat::Unknown;
			return;
		}

		if (txp->Texture2D != nullptr)
		{
			constantBufferTextureFormat = txp->Texture2D->GetTextureFormat();
			txp->Texture2D->Bind(slot);
		}
		else if (txp->CubeMap != nullptr)
		{
			constantBufferTextureFormat = txp->CubeMap->GetTextureFormat();
			txp->CubeMap->Bind(slot);
		}

		auto sampler = CreateTextureSampler(materialTexture, constantBufferTextureFormat);
		sampler.Bind(slot);
	}

	void D3D_Renderer3D::SubmitSubMeshDrawCall(SubMesh& subMesh)
	{
		subMesh.GraphicsIndexBuffer->Bind();

		D3D.Context->IASetPrimitiveTopology(GetD3DPrimitiveTopolgy(subMesh.Primitive));
		D3D.Context->DrawIndexed(static_cast<UINT>(subMesh.Indices.size()), 0, 0);
	}

	bool D3D_Renderer3D::ToneMapData::NeedsUpdating(const SceneContext* sceneContext)
	{
		if (LookupTexture == nullptr)
			return true;

		if (Glow.Gamma != sceneContext->Glow.Gamma)
			return true;

		if (Glow.SaturatePower != sceneContext->Glow.SaturatePower)
			return true;

		if (Glow.SaturateCoefficient != sceneContext->Glow.SaturateCoefficient)
			return true;

		return false;
	}

	void D3D_Renderer3D::ToneMapData::Update()
	{
		GenerateLookupData();
		UpdateTexture();
	}

	void D3D_Renderer3D::ToneMapData::GenerateLookupData()
	{
		const float pixelCount = static_cast<float>(TextureData.size());
		const float gammaPower = 1.0f * Glow.Gamma * 1.5f;
		const int saturatePowerCount = Glow.SaturatePower * 4;

		TextureData[0] = vec2(0.0f, 0.0f);
		for (int i = 1; i < static_cast<int>(TextureData.size()); i++)
		{
			const float step = (static_cast<float>(i) * 16.0f) / pixelCount;
			const float gamma = glm::pow((1.0f - glm::exp(-step)), gammaPower);

			float saturation = (gamma * 2.0f) - 1.0f;
			for (int j = 0; j < saturatePowerCount; j++)
				saturation *= saturation;

			TextureData[i].x = gamma;
			TextureData[i].y = ((gamma * Glow.SaturateCoefficient) / step) * (1.0f - saturation);
		}
	}

	void D3D_Renderer3D::ToneMapData::UpdateTexture()
	{
		if (LookupTexture == nullptr)
		{
			LookupTexture = MakeUnique<D3D_Texture1D>(static_cast<int32_t>(TextureData.size()), TextureData.data(), DXGI_FORMAT_R32G32_FLOAT);
		}
		else
		{
			LookupTexture->UploadData(sizeof(toneMapData.TextureData), TextureData.data());
		}
	}
}
