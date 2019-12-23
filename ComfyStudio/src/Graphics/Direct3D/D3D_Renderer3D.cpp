#include "D3D_Renderer3D.h"

namespace Graphics
{
	namespace
	{
		constexpr UINT RectangleVertexCount = 6;

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

		bool IsMeshTransparent(const Mesh& mesh, const Material& material)
		{
			if (material.BlendFlags.EnableBlend)
				return true;

			if (mesh.Flags.Transparent)
				return true;

			return false;
		}

		vec4 GetPackedTextureSize(const D3D_RenderTarget& renderTarget)
		{
			vec2 renderTargetSize = renderTarget.GetSize();
			return vec4(1.0f / renderTargetSize, renderTargetSize);
		};

		void CalculateGaussianBlurKernel(const GlowParameter& glow, PPGaussCoefConstantData* outData)
		{
			constexpr float powStart = 1.0f, powIncrement = 1.0f;
			constexpr float sigmaFactor = 0.8f, intensityFactor = 1.0f;

			const float firstCoef = (powStart - (powIncrement * 0.5f)) * 2.0f;

			for (int channel = 0; channel < 3; channel++)
			{
				const float channelSigma = glow.Sigma[channel] * sigmaFactor;
				const float reciprocalSigma = 1.0f / ((channelSigma * 2.0f) * channelSigma);

				float accumilatedExpResult = firstCoef;
				float accumilatingPow = powStart;

				std::array<float, 8> results = { firstCoef };
				for (int i = 1; i < 7; i++)
				{
					const float result = glm::exp(-((accumilatingPow * accumilatingPow) * reciprocalSigma)) * powIncrement;
					accumilatingPow += powIncrement;

					results[i] = result;
					accumilatedExpResult += result;
				}

				const float channelIntensity = glow.Intensity[channel] * (intensityFactor * 0.5f);

				for (int i = 0; i < outData->Coefficient.size(); i++)
					outData->Coefficient[i][channel] = (results[i] / accumilatedExpResult) * channelIntensity;
			}
		}

		const struct MaterialIdentifiers
		{
			std::array<char, 8> BLINN { "BLINN" };
			std::array<char, 8> ITEM { "ITEM" };
			std::array<char, 8> STAGE { "STAGE" };
			std::array<char, 8> SKIN { "SKIN" };
			std::array<char, 8> HAIR { "HAIR" };
			std::array<char, 8> CLOTH { "CLOTH" };
			std::array<char, 8> TIGHTS { "TIGHTS" };
			std::array<char, 8> SKY { "SKY" };
			std::array<char, 8> EYEBALL { "EYEBALL" };
			std::array<char, 8> EYELENS { "EYELENS" };
			std::array<char, 8> GLASEYE { "GLASEYE" };
			std::array<char, 8> WATER01 { "WATER01" };
			std::array<char, 8> WATER02 { "WATER02" };
			std::array<char, 8> FLOOR { "FLOOR" };
		} MaterialIdentifiers;

		enum Renderer3DTextureSlot : int32_t
		{
			TextureSlot_Diffuse = 0,
			TextureSlot_Ambient = 1,
			TextureSlot_Normal = 2,
			TextureSlot_Specular = 3,
			TextureSlot_ToonCurve = 4,
			TextureSlot_Reflection = 5,
			TextureSlot_Tangent = 6,
			TextureSlot_Reserved = 7,

			TextureSlot_CharacterLightMap = 9,
			TextureSlot_SunLightMap = 10,
			TextureSlot_ReflectLightMap = 11,
			TextureSlot_ShadowLightMap = 12,
			TextureSlot_CharacterColorLightMap = 13,

			TextureSlot_ScreenReflection = 15,
		};
	}

	D3D_Renderer3D::D3D_Renderer3D()
	{
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
		D3D_SetObjectDebugName(genericInputLayout->GetLayout(), "Renderer3D::GenericInputLayout");
	}

	void D3D_Renderer3D::Begin(SceneContext& scene)
	{
		sceneContext = &scene;
	}

	void D3D_Renderer3D::Draw(const RenderCommand& command)
	{
		ObjRenderCommand renderCommand;
		renderCommand.Obj = command.SourceObj;
		renderCommand.Transform = glm::translate(mat4(1.0f), command.Position);
		renderCommand.Position = command.Position;
		renderCommand.AreAllMeshesTransparent = false;

		auto& commandList = (command.Flags.IsReflection) ? reflectionCommandList : renderCommandList;
		commandList.push_back(renderCommand);
	}

	void D3D_Renderer3D::End()
	{
		InternalFlush();
		sceneContext = nullptr;
	}

	void D3D_Renderer3D::ClearTextureIDs()
	{
		textureIDTxpMap.clear();
	}

	void D3D_Renderer3D::RegisterTextureIDs(const TxpSet& txpSet)
	{
		for (auto& txp : txpSet.Txps)
		{
			if (txp.TextureID != -1)
				textureIDTxpMap[txp.TextureID] = &txp;
		}
	}

	const SceneContext* D3D_Renderer3D::GetSceneContext() const
	{
		return sceneContext;
	}

	std::unordered_map<uint32_t, const Txp*>& D3D_Renderer3D::GetTextureIDTxpMap()
	{
		return textureIDTxpMap;
	}

	const Txp* D3D_Renderer3D::GetTxpFromTextureID(uint32_t textureID) const
	{
		if (textureID == -1)
			return nullptr;

		auto found = textureIDTxpMap.find(textureID);
		if (found != textureIDTxpMap.end())
			return found->second;

		return nullptr;
	}

	void D3D_Renderer3D::InternalFlush()
	{
		InternalPrepareRenderCommands();
		InternalRenderItems();
		InternalRenderPostProcessing();

		transparentSubMeshCommands.clear();
		renderCommandList.clear();
		reflectionCommandList.clear();
	}

	void D3D_Renderer3D::InternalPrepareRenderCommands()
	{
		for (auto& command : renderCommandList)
		{
			command.AreAllMeshesTransparent = true;

			for (auto& mesh : command.Obj->Meshes)
			{
				for (auto& subMesh : mesh.SubMeshes)
				{
					if (IsMeshTransparent(mesh, subMesh.GetMaterial(*command.Obj)))
					{
						const float cameraDistance = glm::distance(command.Position + subMesh.BoundingSphere.Center, sceneContext->Camera.Position);
						transparentSubMeshCommands.push_back({ &command, &mesh, &subMesh, cameraDistance });
					}
					else
					{
						command.AreAllMeshesTransparent = false;
					}
				}
			}
		}

		if (sceneContext->RenderParameters.AlphaSort)
		{
			std::sort(transparentSubMeshCommands.begin(), transparentSubMeshCommands.end(), [](SubMeshRenderCommand& a, SubMeshRenderCommand& b)
			{
				constexpr float comparisonThreshold = 0.001f;
				const bool sameDistance = std::abs(a.CameraDistance - b.CameraDistance) < comparisonThreshold;

				if (sameDistance)
					return a.SubMesh->BoundingSphere.Radius > b.SubMesh->BoundingSphere.Radius;

				return (a.CameraDistance > b.CameraDistance);
			});
		}
	}

	void D3D_Renderer3D::InternalRenderItems()
	{
		sceneCB.Data.RenderResolution = GetPackedTextureSize(sceneContext->RenderData.RenderTarget);
		sceneCB.Data.IrradianceRed = glm::transpose(sceneContext->IBL.Stage.IrradianceRGB[0]);
		sceneCB.Data.IrradianceGreen = glm::transpose(sceneContext->IBL.Stage.IrradianceRGB[1]);
		sceneCB.Data.IrradianceBlue = glm::transpose(sceneContext->IBL.Stage.IrradianceRGB[2]);
		sceneCB.Data.Scene.View = glm::transpose(sceneContext->Camera.GetViewMatrix());
		sceneCB.Data.Scene.ViewProjection = glm::transpose(sceneContext->Camera.GetProjectionMatrix() * sceneContext->Camera.GetViewMatrix());
		sceneCB.Data.Scene.EyePosition = vec4(sceneContext->Camera.Position, 0.0f);
		sceneCB.Data.LightColor = vec4(sceneContext->IBL.Stage.LightColor, 1.0f);
		sceneCB.Data.CharacterLight.Ambient = vec4(sceneContext->Light.Character.Ambient, 1.0f);
		sceneCB.Data.CharacterLight.Diffuse = vec4(sceneContext->Light.Character.Diffuse, 1.0f);
		sceneCB.Data.CharacterLight.Specular = vec4(sceneContext->Light.Character.Specular, 1.0f);
		sceneCB.Data.CharacterLight.Direction = vec4(glm::normalize(sceneContext->Light.Character.Position), 1.0f);
		sceneCB.Data.StageLight.Ambient = vec4(sceneContext->Light.Stage.Ambient, 1.0f);
		sceneCB.Data.StageLight.Diffuse = vec4(sceneContext->Light.Stage.Diffuse, 1.0f);
		sceneCB.Data.StageLight.Specular = vec4(sceneContext->Light.Stage.Specular, 1.0f);
		sceneCB.Data.StageLight.Direction = vec4(glm::normalize(sceneContext->Light.Stage.Position), 1.0f);
		sceneCB.Data.DepthFog.Parameters = vec4(sceneContext->RenderParameters.RenderFog ? sceneContext->Fog.Depth.Density : 0.0f, sceneContext->Fog.Depth.Start, sceneContext->Fog.Depth.End, 1.0f / (sceneContext->Fog.Depth.End - sceneContext->Fog.Depth.Start));
		sceneCB.Data.DepthFog.Color = vec4(sceneContext->Fog.Depth.Color, 1.0f);
		sceneCB.UploadData();

		sceneCB.BindShaders();
		objectCB.BindShaders();

		D3D_ShaderResourceView::BindArray<7>(TextureSlot_CharacterLightMap,
			{
				sceneContext->IBL.Character.LightMap.CubeMap.get(),
				sceneContext->IBL.Sun.LightMap.CubeMap.get(),
				sceneContext->IBL.Reflect.LightMap.CubeMap.get(),
				sceneContext->IBL.Shadow.LightMap.CubeMap.get(),
				sceneContext->IBL.CharacterColor.LightMap.CubeMap.get(),

				// NOTE: Also unbind screen reflection render target
				nullptr,
				nullptr,
			});

		if (sceneContext->RenderParameters.Wireframe)
			wireframeRasterizerState.Bind();

		genericInputLayout->Bind();
		D3D.Context->OMSetBlendState(nullptr, nullptr, D3D11_DEFAULT_SAMPLE_MASK);

		if (sceneContext->RenderParameters.RenderReflection)
		{
			if (!reflectionCommandList.empty())
			{
				sceneContext->RenderData.ReflectionRenderTarget.ResizeIfDifferent(sceneContext->RenderParameters.ReflectionRenderResolution);
				sceneContext->RenderData.ReflectionRenderTarget.BindSetViewport();

				if (sceneContext->RenderParameters.ClearReflection)
					sceneContext->RenderData.ReflectionRenderTarget.Clear(sceneContext->RenderParameters.ClearColor);
				else
					sceneContext->RenderData.ReflectionRenderTarget.GetDepthBuffer()->Clear();

				// TODO: Render using reflection shader
				for (auto& command : reflectionCommandList)
					InternalRenderOpaqueObjCommand(command);

				sceneContext->RenderData.ReflectionRenderTarget.UnBind();
				sceneContext->RenderData.ReflectionRenderTarget.BindResource(TextureSlot_ScreenReflection);
			}
		}

		cachedTextureSamplers.CreateIfNeeded(sceneContext->RenderParameters);

		sceneContext->RenderData.RenderTarget.SetMultiSampleCountIfDifferent(sceneContext->RenderParameters.MultiSampleCount);
		sceneContext->RenderData.RenderTarget.ResizeIfDifferent(sceneContext->RenderParameters.RenderResolution);
		sceneContext->RenderData.RenderTarget.BindSetViewport();

		if (sceneContext->RenderParameters.Clear)
			sceneContext->RenderData.RenderTarget.Clear(sceneContext->RenderParameters.ClearColor);
		else
			sceneContext->RenderData.RenderTarget.GetDepthBuffer()->Clear();

		if (sceneContext->RenderParameters.RenderOpaque)
		{
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

		sceneContext->RenderData.RenderTarget.UnBind();
		genericInputLayout->UnBind();
	}

	void D3D_Renderer3D::InternalRenderOpaqueObjCommand(ObjRenderCommand& command)
	{
		if (command.AreAllMeshesTransparent)
			return;

		for (auto& mesh : command.Obj->Meshes)
		{
			BindMeshVertexBuffers(mesh);

			for (auto& subMesh : mesh.SubMeshes)
			{
				auto& material = subMesh.GetMaterial(*command.Obj);
				if (IsMeshTransparent(mesh, material))
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
		auto& material = subMesh.GetMaterial(*command.ObjCommand->Obj);

		if (!material.BlendFlags.IgnoreBlendFactors)
			cachedBlendStates.GetState(material.BlendFlags.SrcBlendFactor, material.BlendFlags.DstBlendFactor).Bind();
		
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
					PrepareAndRenderSubMesh(command, mesh, subMesh, subMesh.GetMaterial(*command.Obj), command.Transform);
			}
		}

		currentlyRenderingWireframeOverlay = false;
	}

	void D3D_Renderer3D::InternalRenderPostProcessing()
	{
		D3D.Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		std::array<ID3D11Buffer*, VertexAttribute_Count> buffers = {};
		std::array<UINT, VertexAttribute_Count> strides = {}, offsets = {};
		D3D.Context->IASetVertexBuffers(0, VertexAttribute_Count, buffers.data(), strides.data(), offsets.data());

		solidNoCullingRasterizerState.Bind();
		D3D.Context->OMSetBlendState(nullptr, nullptr, D3D11_DEFAULT_SAMPLE_MASK);

		D3D_TextureSampler::BindArray<1>(0, { nullptr });

		if (sceneContext->RenderParameters.RenderBloom)
			InternalRenderBloom();

		sceneContext->RenderData.OutputRenderTarget->BindSetViewport();

		if (toneMapData.NeedsUpdating(sceneContext))
		{
			toneMapData.Glow = sceneContext->Glow;
			toneMapData.Update();
		}

		D3D_ShaderResourceView::BindArray<3>(0, { &sceneContext->RenderData.RenderTarget, (sceneContext->RenderParameters.RenderBloom) ? &sceneContext->BloomRenderData.CombinedBlurRenderTarget : nullptr, toneMapData.LookupTexture.get() });

		toneMapCB.Data.Exposure = sceneContext->Glow.Exposure;
		toneMapCB.Data.Gamma = sceneContext->Glow.Gamma;
		toneMapCB.Data.SaturatePower = sceneContext->Glow.SaturatePower;
		toneMapCB.Data.SaturateCoefficient = sceneContext->Glow.SaturateCoefficient;
		toneMapCB.UploadData();
		toneMapCB.BindVertexShader();

		shaders.ToneMap.Bind();

		D3D.Context->Draw(RectangleVertexCount, 0);

		D3D_ShaderResourceView::BindArray<3>(0, { nullptr, nullptr, nullptr });
	}

	void D3D_Renderer3D::InternalRenderBloom()
	{
		auto& bloom = sceneContext->BloomRenderData;

		bloom.BaseRenderTarget.ResizeIfDifferent(sceneContext->RenderParameters.RenderResolution / 2);

		reduceTexCB.Data.CombineBlurred = false;
		reduceTexCB.BindShaders();
		shaders.ReduceTex.Bind();

		for (int i = -1; i < static_cast<int>(bloom.ReduceRenderTargets.size()); i++)
		{
			auto& renderTarget = (i < 0) ? bloom.BaseRenderTarget : bloom.ReduceRenderTargets[i];
			auto& lastRenderTarget = (i < 0) ? sceneContext->RenderData.RenderTarget : (i == 0) ? bloom.BaseRenderTarget : bloom.ReduceRenderTargets[i - 1];

			reduceTexCB.Data.TextureSize = GetPackedTextureSize(lastRenderTarget);
			reduceTexCB.Data.ExtractBrightness = (i == 0);
			reduceTexCB.UploadData();

			renderTarget.BindSetViewport();
			lastRenderTarget.BindResource(0);

			D3D.Context->Draw(RectangleVertexCount, 0);
		}

		CalculateGaussianBlurKernel(sceneContext->Glow, &ppGaussCoefCB.Data);
		ppGaussCoefCB.UploadData();
		ppGaussCoefCB.BindPixelShader();

		ppGaussTexCB.Data.FinalPass = false;
		ppGaussTexCB.BindPixelShader();

		shaders.PPGauss.Bind();

		for (int i = 1; i < 4; i++)
		{
			auto* sourceTarget = &bloom.ReduceRenderTargets[i];
			auto* destinationTarget = &bloom.BlurRenderTargets[i];

			ppGaussTexCB.Data.TextureSize = GetPackedTextureSize(*sourceTarget);
			ppGaussTexCB.UploadData();

			for (int j = 0; j < 2; j++)
			{
				sourceTarget->BindResource(0);
				destinationTarget->BindSetViewport();
				D3D.Context->Draw(RectangleVertexCount, 0);
				destinationTarget->UnBind();

				// NOTE: Ping pong between them to avoid having to use additional render targets
				std::swap(sourceTarget, destinationTarget);
			}
		}

		ppGaussTexCB.Data.TextureSize = GetPackedTextureSize(bloom.ReduceRenderTargets[0]);
		ppGaussTexCB.Data.FinalPass = true;
		ppGaussTexCB.UploadData();

		bloom.BlurRenderTargets[0].BindSetViewport();
		bloom.ReduceRenderTargets[0].BindResource(0);
		D3D.Context->Draw(RectangleVertexCount, 0);

		std::array<D3D_ShaderResourceView*, 4> combinedBlurInputTargets =
		{
			&bloom.BlurRenderTargets[0],
			// NOTE: Use the reduce targets because of the ping pong blur rendering
			&bloom.ReduceRenderTargets[1],
			&bloom.ReduceRenderTargets[2],
			&bloom.ReduceRenderTargets[3],
		};

		bloom.CombinedBlurRenderTarget.BindSetViewport();
		D3D_ShaderResourceView::BindArray(0, combinedBlurInputTargets);

		reduceTexCB.Data.TextureSize = GetPackedTextureSize(bloom.ReduceRenderTargets[3]);
		reduceTexCB.Data.ExtractBrightness = false;
		reduceTexCB.Data.CombineBlurred = true;
		reduceTexCB.UploadData();
		reduceTexCB.BindShaders();
		shaders.ReduceTex.Bind();

		D3D.Context->Draw(RectangleVertexCount, 0);
	}

	void D3D_Renderer3D::BindMeshVertexBuffers(const Mesh& mesh)
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

	void D3D_Renderer3D::PrepareAndRenderSubMesh(const ObjRenderCommand& command, const Mesh& mesh, const SubMesh& subMesh, const Material& material, const mat4& model)
	{
		auto& materialShader = GetMaterialShader(material);
		materialShader.Bind();

		std::array<D3D_ShaderResourceView*, 8> textureResources;
		std::array<D3D_TextureSampler*, 8> textureSamplers;

		for (size_t i = 0; i < 8; i++)
		{
			auto& materialTexture = (&material.Diffuse)[i];
			auto& textureFormat = (&objectCB.Data.TextureFormats.Diffuse)[i];
			auto txp = GetTxpFromTextureID(materialTexture.TextureID);

			if (txp != nullptr)
			{
				textureFormat = txp->GetFormat();
				textureResources[i] = (txp->Texture2D != nullptr) ? static_cast<D3D_TextureResource*>(txp->Texture2D.get()) : (txp->CubeMap != nullptr) ? (txp->CubeMap.get()) : nullptr;
				textureSamplers[i] = &cachedTextureSamplers.GetSampler(materialTexture.Flags);
			}
			else
			{
				textureFormat = TextureFormat::Unknown;
				textureResources[i] = nullptr;
				textureSamplers[i] = nullptr;
			}
		}

		D3D_ShaderResourceView::BindArray<8>(TextureSlot_Diffuse, textureResources);
		D3D_TextureSampler::BindArray<8>(TextureSlot_Diffuse, textureSamplers);

		if (!currentlyRenderingWireframeOverlay && !sceneContext->RenderParameters.Wireframe)
			((material.BlendFlags.DoubleSidedness != DoubleSidedness_Off) ? solidNoCullingRasterizerState : solidBackfaceCullingRasterizerState).Bind();

		const float fresnel = (((material.ShaderFlags.Fresnel == 0) ? 7.0f : static_cast<float>(material.ShaderFlags.Fresnel) - 1.0f) * 0.12f) * 0.82f;
		const float lineLight = material.ShaderFlags.LineLight * 0.111f;
		objectCB.Data.Material.FresnelCoefficient = vec4(fresnel, 0.18f, lineLight, 0.0f);
		objectCB.Data.Material.Diffuse = material.DiffuseColor;
		objectCB.Data.Material.Transparency = material.Transparency;
		objectCB.Data.Material.Ambient = material.AmbientColor;
		objectCB.Data.Material.Specular = material.SpecularColor;
		objectCB.Data.Material.Reflectivity = material.Reflectivity;
		objectCB.Data.Material.Emission = material.EmissionColor;
		objectCB.Data.Material.Shininess = (material.Shininess - 16.0f) / 112.0f;
		objectCB.Data.Material.Intensity = material.Intensity;
		objectCB.Data.Material.BumpDepth = material.BumpDepth;
		objectCB.Data.Material.DiffuseTextureTransform = glm::transpose(material.Diffuse.TextureCoordinateMatrix);
		objectCB.Data.Material.AmbientTextureTransform = glm::transpose(material.Ambient.TextureCoordinateMatrix);

		mat4 modelMatrix;
		if (mesh.Flags.FaceCamera)
		{
			const float cameraAngle = glm::atan(command.Position.x - sceneContext->Camera.Position.x, command.Position.z - sceneContext->Camera.Position.z);
			modelMatrix = glm::rotate(glm::translate(glm::mat4(1.0f), command.Position), cameraAngle - glm::pi<float>(), vec3(0.0f, 1.0f, 0.0f));
		}
		else
		{
			modelMatrix = model;
		}

		objectCB.Data.Model = glm::transpose(modelMatrix);
		objectCB.Data.ModelView = glm::transpose(sceneContext->Camera.GetViewMatrix() * modelMatrix);
		objectCB.Data.ModelViewProjection = glm::transpose(sceneContext->Camera.GetProjectionMatrix() * sceneContext->Camera.GetViewMatrix() * modelMatrix);

		objectCB.Data.ShaderFlags = 0;

		if (mesh.AttributeFlags & VertexAttributeFlags_Color0)
			objectCB.Data.ShaderFlags |= ShaderFlags_VertexColor;

		if (material.Flags.UseDiffuseTexture || material.Diffuse.TextureID != -1)
			objectCB.Data.ShaderFlags |= ShaderFlags_DiffuseTexture;

		if (material.Flags.UseAmbientTexture || material.Ambient.TextureID != -1)
			objectCB.Data.ShaderFlags |= ShaderFlags_AmbientTexture;

		if (material.Flags.UseNormalTexture || material.Normal.TextureID != -1)
			objectCB.Data.ShaderFlags |= ShaderFlags_NormalTexture;

		if (material.Flags.UseSpecularTexture || material.Specular.TextureID != -1)
			objectCB.Data.ShaderFlags |= ShaderFlags_SpecularTexture;

		if (material.BlendFlags.EnableAlphaTest && !(material.BlendFlags.EnableBlend || mesh.Flags.Transparent))
			objectCB.Data.ShaderFlags |= ShaderFlags_AlphaTest;

		if (material.Flags.UseCubeMapReflection || material.Reflection.TextureID != -1)
			objectCB.Data.ShaderFlags |= ShaderFlags_CubeMapReflection;

		if (sceneContext->RenderParameters.RenderFog)
			objectCB.Data.ShaderFlags |= ShaderFlags_LinearFog;

		objectCB.UploadData();

		SubmitSubMeshDrawCall(subMesh);
	}

	D3D_ShaderPair& D3D_Renderer3D::GetMaterialShader(const Material& material)
	{
		if (material.MaterialType == MaterialIdentifiers.BLINN)
		{
			if (material.ShaderFlags.PhongShading)
			{
				return (material.Flags.UseNormalTexture) ? shaders.BlinnPerFrag : shaders.BlinnPerVert;
			}
			else if (material.ShaderFlags.LambertShading)
			{
				return shaders.Lambert;
			}
			else
			{
				return shaders.Constant;
			}
		}
		else if (material.MaterialType == MaterialIdentifiers.ITEM)
		{
			return shaders.ItemBlinn;
		}
		else if (material.MaterialType == MaterialIdentifiers.STAGE)
		{
			return shaders.StageBlinn;
		}
		else if (material.MaterialType == MaterialIdentifiers.SKIN)
		{
			return shaders.SkinDefault;
		}
		else if (material.MaterialType == MaterialIdentifiers.HAIR)
		{
			return (false) ? shaders.HairAniso : shaders.HairDefault;
		}
		else if (material.MaterialType == MaterialIdentifiers.CLOTH)
		{
			return (false) ? shaders.ClothAniso : shaders.ClothDefault;
		}
		else if (material.MaterialType == MaterialIdentifiers.TIGHTS)
		{
			return shaders.Tights;
		}
		else if (material.MaterialType == MaterialIdentifiers.SKY)
		{
			return shaders.SkyDefault;
		}
		else if (material.MaterialType == MaterialIdentifiers.EYEBALL)
		{
			return shaders.EyeBall;
		}
		else if (material.MaterialType == MaterialIdentifiers.EYELENS)
		{
			return shaders.EyeLens;
		}
		else if (material.MaterialType == MaterialIdentifiers.GLASEYE)
		{
			return shaders.GlassEye;
		}
		else if (material.MaterialType == MaterialIdentifiers.WATER01 || material.MaterialType == MaterialIdentifiers.WATER02)
		{
			return shaders.Water;
		}
		else if (material.MaterialType == MaterialIdentifiers.FLOOR)
		{
			return shaders.Floor;
		}
		else
		{
			return shaders.Debug;
		}
	}

	void D3D_Renderer3D::SubmitSubMeshDrawCall(const SubMesh& subMesh)
	{
		subMesh.GraphicsIndexBuffer->Bind();

		D3D.Context->IASetPrimitiveTopology(GetD3DPrimitiveTopolgy(subMesh.Primitive));
		D3D.Context->DrawIndexed(static_cast<UINT>(subMesh.Indices.size()), 0, 0);
	}

	bool D3D_Renderer3D::IsDebugRenderFlagSet(int bitIndex) const
	{
		return sceneContext->RenderParameters.DebugFlags & (1 << bitIndex);
	}

	void D3D_Renderer3D::TextureSamplerCache::CreateIfNeeded(const RenderParameters& renderParameters)
	{
		if (samplers[0][0] == nullptr || lastAnistropicFiltering != renderParameters.AnistropicFiltering)
		{
			const auto filter = (renderParameters.AnistropicFiltering > D3D11_MIN_MAXANISOTROPY) ? D3D11_FILTER_ANISOTROPIC : D3D11_FILTER_MIN_MAG_MIP_LINEAR;

			constexpr std::array d3dAddressModes = { D3D11_TEXTURE_ADDRESS_MIRROR, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_CLAMP };
			constexpr std::array addressModeNames = { "Mirror", "Repeat", "Clamp" };

			for (int u = 0; u < AddressMode_Count; u++)
			{
				for (int v = 0; v < AddressMode_Count; v++)
				{
					samplers[u][v] = MakeUnique<D3D_TextureSampler>(filter, d3dAddressModes[u], d3dAddressModes[v], 0.0f, renderParameters.AnistropicFiltering);
					D3D_SetObjectDebugName(samplers[u][v]->GetSampler(), "Renderer3D::Sampler::%s-%s", addressModeNames[u], addressModeNames[v]);
				}
			}

			lastAnistropicFiltering = renderParameters.AnistropicFiltering;
		}
	}

	D3D_TextureSampler& D3D_Renderer3D::TextureSamplerCache::GetSampler(MaterialTextureFlags flags)
	{
		auto u = flags.TextureAddressMode_U_Mirror ? Mirror : flags.TextureAddressMode_U_Repeat ? Repeat : Clamp;
		auto v = flags.TextureAddressMode_V_Mirror ? Mirror : flags.TextureAddressMode_V_Repeat ? Repeat : Clamp;
		return *samplers[u][v];
	}

	D3D_Renderer3D::BlendStateCache::BlendStateCache()
	{
		constexpr std::array d3dBlendFactors = { D3D11_BLEND_ZERO, D3D11_BLEND_ONE, D3D11_BLEND_SRC_COLOR, D3D11_BLEND_INV_SRC_COLOR, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_DEST_ALPHA, D3D11_BLEND_INV_DEST_ALPHA, D3D11_BLEND_DEST_COLOR, D3D11_BLEND_INV_DEST_COLOR, };
		constexpr std::array blendFactorNames =  { "Zero", "One", "SrcColor", "ISrcColor", "SrcAlpha", "ISrcAlpha", "DstAlpha", "IDstAlpha", "DstColor", "IDstColor", };

		for (int src = 0; src < BlendFactor_Count; src++)
		{
			for (int dst = 0; dst < BlendFactor_Count; dst++)
			{
				states[src][dst] = MakeUnique<D3D_BlendState>(d3dBlendFactors[src], d3dBlendFactors[dst], D3D11_BLEND_INV_DEST_ALPHA, D3D11_BLEND_ONE);
				D3D_SetObjectDebugName(states[src][dst]->GetBlendState(), "Renderer3D::BlendState::%s-%s", blendFactorNames[src], blendFactorNames[dst]);
			}
		}
	}

	D3D_BlendState& D3D_Renderer3D::BlendStateCache::GetState(BlendFactor source, BlendFactor destination)
	{
		return *states[source][destination];
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
