#include "D3D_Renderer3D.h"

namespace Graphics
{
	namespace
	{
		constexpr std::array<vec4, 2> DefaultShadowCoefficients =
		{
			vec4(0.199471f, 0.176033f, 0.120985f, 0.064759f),
			vec4(0.026995f, 0.008764f, 0.002216f, 0.000436f),
		};
		constexpr float DefaultShadowAmbient = 0.4f;
		constexpr float DefaultShadowExpontent = 80.0f * (9.95f * 2.0f) * 1.442695f;
		constexpr float DefaultShadowTexelOffset = 0.05f / (9.95f * 2.0f);
		constexpr float DefaultSSSParameter = 0.6f;

		constexpr UINT RectangleVertexCount = 6;

		constexpr uint32_t MorphVertexAttributeOffset = VertexAttribute_Count;

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

		constexpr bool ReceivesShadows(const RenderCommand& command, const Mesh& mesh, const SubMesh& subMesh)
		{
			return (command.Flags.ReceivesShadow && subMesh.ShadowFlags);
		}

		constexpr bool ReceivesSelfShadow(const RenderCommand& command, const Mesh& mesh, const SubMesh& subMesh)
		{
			return (command.Flags.ReceivesShadow);
		}

		constexpr bool IsMeshTransparent(const Mesh& mesh, const SubMesh& subMesh, const Material& material)
		{
			if (material.BlendFlags.EnableBlend)
				return true;

			if (material.BlendFlags.EnableAlphaTest && !material.BlendFlags.OpaqueAlphaTest)
				return true;

			return false;
		}

		bool UsesSSSSkin(const Material& material)
		{
			if (material.MaterialType == Material::Identifiers.SKIN || material.MaterialType == Material::Identifiers.EYEBALL || material.MaterialType == Material::Identifiers.EYELENS)
				return true;

			return false;
		}

		bool UsesSSSSkinConst(const Material& material)
		{
			if (material.MaterialType == Material::Identifiers.HAIR || material.MaterialType == Material::Identifiers.CLOTH || material.MaterialType == Material::Identifiers.TIGHTS)
				return true;

			return false;
		}

		const Mesh* MeshOrDefault(const Obj* obj, size_t index)
		{
			if (obj == nullptr)
				return nullptr;

			return index < (obj->Meshes.size()) ? &obj->Meshes[index] : nullptr;
		}

		vec4 GetPackedTextureSize(vec2 size)
		{
			return vec4(1.0f / size, size);
		};

		vec4 GetPackedTextureSize(const D3D_RenderTargetBase& renderTarget)
		{
			return GetPackedTextureSize(vec2(renderTarget.GetSize()));
		};

		inline float RootMeanSquare(const vec3 value)
		{
			const vec3 squared = (value * value);
			return glm::sqrt(squared.x + squared.y + squared.z);
		}

		double CalculateSSSCameraCoefficient(const vec3 viewPoint, const vec3 interest, const float fieldOfView, const std::array<std::optional<vec3>, 2>& characterHeadPositions)
		{
			std::array<float, 2> meanSquareRoots;
			std::array<vec3, 2> headPositions;

			for (size_t i = 0; i < meanSquareRoots.size(); i++)
			{
				if (characterHeadPositions[i].has_value())
				{
					headPositions[i] = characterHeadPositions[i].value();
					meanSquareRoots[i] = RootMeanSquare(viewPoint - characterHeadPositions[i].value());
				}
				else
				{
					headPositions[i] = interest;
					meanSquareRoots[i] = 999999.0f;
				}
			}

			vec3 headPosition;
			if (meanSquareRoots[0] <= meanSquareRoots[1])
			{
				headPosition = headPositions[0];
			}
			else
			{
				headPosition = headPositions[1];
				headPositions[0] = headPositions[1];
			}

			const double result = 1.0f / std::clamp(
				std::max(glm::tan(glm::radians(fieldOfView * 0.5f)) * 5.0f, 0.25f) *
				std::max(RootMeanSquare(viewPoint - ((RootMeanSquare(interest - headPosition) > 1.25f) ? headPositions[0] : interest)), 0.25f),
				0.25f, 100.0f);

			return result;
		}

		std::array<vec4, 36> CalculateSSSFilterCoefficient(double cameraCoefficient)
		{
			constexpr std::array<std::array<double, 3>, 4> weights =
			{
				std::array { 1.0, 2.0, 5.0, },
				std::array { 0.2, 0.4, 1.2, },
				std::array { 0.3, 0.7, 2.0, },
				std::array { 0.4, 0.3, 0.3, },
			};
			constexpr double expFactorIncrement = 1.0;

			std::array<vec4, 36> coefficients = {};

			for (int iteration = 0; iteration < 3; iteration++)
			{
				for (int component = 0; component < 3; component++)
				{
					const double reciprocalWeight = 1.0 / (cameraCoefficient * weights[component][iteration]);

					double expSum = 0.0;
					double expFactorSum = 0.0;

					std::array<double, 6> exponentials;
					for (double& exp : exponentials)
					{
						const double expResult = glm::exp(reciprocalWeight * expFactorSum * -0.5 * (reciprocalWeight * expFactorSum));
						exp = expResult;

						expSum += expResult;
						expFactorSum += expFactorIncrement;
					}

					const double reciprocalExpSum = 1.0 / expSum;
					for (double& exp : exponentials)
						exp *= reciprocalExpSum;

					for (int i = 0; i < 6; i++)
					{
						const double weight = weights.back()[iteration] * exponentials[i];
						for (int j = 0; j < 6; j++)
						{
							float& coef = coefficients[(i * 6) + j][component];
							coef = static_cast<float>(static_cast<double>(coef) + exponentials[j] * weight);
						}
					}
				}
			}

			return coefficients;
		}

		void CalculateSSSCoefficients(const PerspectiveCamera& camera, SSSFilterConstantData& outData)
		{
			const std::array<std::optional<vec3>, 2> characterHeadPositions = { vec3(0.0f, 1.055f, 0.0f) };
			const double cameraCoefficient = CalculateSSSCameraCoefficient(camera.ViewPoint, camera.Interest, camera.FieldOfView, characterHeadPositions);

			outData.Coefficients = CalculateSSSFilterCoefficient(cameraCoefficient);
		}

		void CalculateGaussianBlurKernel(const GlowParameter& glow, PPGaussCoefConstantData& outData)
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

				for (int i = 0; i < outData.Coefficients.size(); i++)
					outData.Coefficients[i][channel] = (results[i] / accumilatedExpResult) * channelIntensity;
			}
		}

		void CalculateExposureSpotCoefficients(ExposureConstantData& outData)
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

		enum Renderer3DTextureSlot : int32_t
		{
			TextureSlot_Diffuse = 0,
			TextureSlot_Ambient = 1,
			TextureSlot_Normal = 2,
			TextureSlot_Specular = 3,
			TextureSlot_Tranparency = 4,
			TextureSlot_Environment = 5,
			TextureSlot_Translucency = 6,
			TextureSlot_Reserved = 7,

			TextureSlot_IBLCharacterLightMap = 9,
			TextureSlot_IBLSunLightMap = 10,
			TextureSlot_IBLReflectLightMap = 11,
			TextureSlot_IBLShadowLightMap = 12,
			TextureSlot_IBLCharacterColorLightMap = 13,

			TextureSlot_ScreenReflection = 15,
			TextureSlot_SubsurfaceScattering = 16,

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
	}

	D3D_Renderer3D::D3D_Renderer3D()
	{
		static constexpr InputElement genericElements[] =
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

		genericInputLayout = MakeUnique<D3D_InputLayout>(genericElements, std::size(genericElements), shaders.Debug.VS);
		D3D_SetObjectDebugName(genericInputLayout->GetLayout(), "Renderer3D::GenericInputLayout");

		static constexpr InputElement silhouetteElements[] =
		{
			{ "POSITION",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, VertexAttribute_Position },
			{ "TEXCOORD",		0, DXGI_FORMAT_R32G32_FLOAT,		0, VertexAttribute_TextureCoordinate0 },
			{ "BONE_WEIGHT",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, VertexAttribute_BoneWeight },
			{ "BONE_INDEX",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, VertexAttribute_BoneIndex },
			{ "POSITION",		1, DXGI_FORMAT_R32G32B32_FLOAT,		0, MorphVertexAttributeOffset + VertexAttribute_Position },
			{ "TEXCOORD",		4, DXGI_FORMAT_R32G32_FLOAT,		0, MorphVertexAttributeOffset + VertexAttribute_TextureCoordinate0 },
		};

		shadowSilhouetteInputLayout = MakeUnique<D3D_InputLayout>(silhouetteElements, std::size(silhouetteElements), shaders.Silhouette.VS);
		D3D_SetObjectDebugName(shadowSilhouetteInputLayout->GetLayout(), "Renderer3D::ShadowSilhouetteInputLayout");

		constexpr size_t reasonableInitialCapacity = 64;

		defaultCommandList.OpaqueAndTransparent.reserve(reasonableInitialCapacity);
		defaultCommandList.Transparent.reserve(reasonableInitialCapacity);

		reflectionCommandList.OpaqueAndTransparent.reserve(reasonableInitialCapacity);
		reflectionCommandList.Transparent.reserve(reasonableInitialCapacity);
	}

	void D3D_Renderer3D::Begin(SceneContext& scene)
	{
		lastFrameStatistics = statistics;
		statistics = {};

		isAnyCommand = {};

		sceneContext = &scene;
		renderParameters = &scene.RenderParameters;
		renderData = &scene.RenderData;
	}

	void D3D_Renderer3D::Draw(const RenderCommand& command)
	{
		assert(command.SourceObj != nullptr);
		const auto& transform = command.Transform;

		ObjRenderCommand renderCommand;
		renderCommand.SourceCommand = command;
		renderCommand.AreAllMeshesTransparent = false;
		renderCommand.ModelMatrix = command.Transform.CalculateMatrix();

		renderCommand.TransformedBoundingSphere = command.SourceObj->BoundingSphere;
		renderCommand.TransformedBoundingSphere.Transform(renderCommand.ModelMatrix, command.Transform.Scale);

		auto& commandList = (command.Flags.IsReflection) ? reflectionCommandList : defaultCommandList;
		commandList.OpaqueAndTransparent.push_back(renderCommand);

		UpdateIsAnyCommandFlags(command);
	}

	void D3D_Renderer3D::End()
	{
		InternalFlush();
		sceneContext = nullptr;
		renderParameters = nullptr;
		renderData = nullptr;
	}

	void D3D_Renderer3D::UpdateIsAnyCommandFlags(const RenderCommand& command)
	{
		if (command.Flags.IsReflection)
			isAnyCommand.ScreenReflection = true;

		if (command.Flags.SilhouetteOutline)
			isAnyCommand.SilhouetteOutline = true;

		if (command.Flags.CastsShadow)
			isAnyCommand.CastShadow = true;

		if (!isAnyCommand.ReceiveShadow)
		{
			for (auto& mesh : command.SourceObj->Meshes)
			{
				for (auto& subMesh : mesh.SubMeshes)
				{
					if (ReceivesShadows(command, mesh, subMesh) || ReceivesSelfShadow(command, mesh, subMesh))
						isAnyCommand.ReceiveShadow = true;
				}
			}
		}

		if (!isAnyCommand.SubsurfaceScattering)
		{
			for (auto& mesh : command.SourceObj->Meshes)
			{
				for (auto& subMesh : mesh.SubMeshes)
				{
					if (UsesSSSSkin(subMesh.GetMaterial(*command.SourceObj)))
						isAnyCommand.SubsurfaceScattering = true;
				}
			}
		}
	}

	void D3D_Renderer3D::ClearTextureIDs()
	{
		textureIDTxpMap.clear();
	}

	void D3D_Renderer3D::RegisterTextureIDs(const TxpSet& txpSet)
	{
		for (auto& txp : txpSet.Txps)
		{
			if (txp.ID != TxpID::Invalid)
				textureIDTxpMap[txp.ID] = &txp;
		}
	}

	void D3D_Renderer3D::UnRegisterTextureIDs(const TxpSet& txpSet)
	{
		for (auto& txp : txpSet.Txps)
		{
			if (txp.ID != TxpID::Invalid)
				textureIDTxpMap.erase(txp.ID);
		}
	}

	const SceneContext* D3D_Renderer3D::GetSceneContext() const
	{
		return sceneContext;
	}

	std::unordered_map<TxpID, const Txp*>& D3D_Renderer3D::GetTextureIDTxpMap()
	{
		return textureIDTxpMap;
	}

	const Txp* D3D_Renderer3D::GetTxpFromTextureID(TxpID textureID) const
	{
		if (textureID == TxpID::Invalid)
			return nullptr;

		auto found = textureIDTxpMap.find(textureID);
		if (found != textureIDTxpMap.end())
			return found->second;

		return nullptr;
	}

	void D3D_Renderer3D::InternalFlush()
	{
		InternalPrepareRenderCommands(defaultCommandList);
		InternalPrepareRenderCommands(reflectionCommandList);

		InternalRenderScene();
		InternalRenderPostProcessing();

		if (isAnyCommand.SilhouetteOutline)
			InternalRenderSilhouetteOutlineOverlay();

		defaultCommandList.OpaqueAndTransparent.clear();
		defaultCommandList.Transparent.clear();

		reflectionCommandList.OpaqueAndTransparent.clear();
		reflectionCommandList.Transparent.clear();
	}

	void D3D_Renderer3D::InternalPrepareRenderCommands(RenderPassCommandLists& commandList)
	{
		if (commandList.OpaqueAndTransparent.empty())
			return;

		for (auto& command : commandList.OpaqueAndTransparent)
		{
			command.AreAllMeshesTransparent = true;

			for (auto& mesh : command.SourceCommand.SourceObj->Meshes)
			{
				for (auto& subMesh : mesh.SubMeshes)
				{
					if (IsMeshTransparent(mesh, subMesh, subMesh.GetMaterial(*command.SourceCommand.SourceObj)))
					{
						const auto boundingSphere = (subMesh.BoundingSphere * command.SourceCommand.Transform);
						const float cameraDistance = glm::distance(boundingSphere.Center, sceneContext->Camera.ViewPoint);

						commandList.Transparent.push_back({ &command, &mesh, &subMesh, cameraDistance });
					}
					else
					{
						command.AreAllMeshesTransparent = false;
					}
				}
			}
		}

		if (renderParameters->AlphaSort)
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

	void D3D_Renderer3D::InternalRenderScene()
	{
		const auto& camera = sceneContext->Camera;
		const auto& fog = sceneContext->Fog;
		const auto& lightParam = sceneContext->Light;
		const auto& ibl = sceneContext->IBL;

		sceneCB.Data.RenderResolution = GetPackedTextureSize(renderData->Main.Current());
		sceneCB.Data.IBLIrradianceRed = glm::transpose(ibl.Stage.IrradianceRGB[0]);
		sceneCB.Data.IBLIrradianceGreen = glm::transpose(ibl.Stage.IrradianceRGB[1]);
		sceneCB.Data.IBLIrradianceBlue = glm::transpose(ibl.Stage.IrradianceRGB[2]);
		sceneCB.Data.Scene.View = glm::transpose(camera.GetView());
		sceneCB.Data.Scene.ViewProjection = glm::transpose(camera.GetViewProjection());
		sceneCB.Data.Scene.EyePosition = vec4(camera.ViewPoint, 1.0f);
		sceneCB.Data.IBLStageColor = vec4(ibl.Stage.LightColor, 1.0f);
		sceneCB.Data.IBLCharaColor = vec4(ibl.Character.LightColor, 1.0f);
		sceneCB.Data.IBLSunColor = vec4(ibl.Sun.LightColor, 1.0f);
		sceneCB.Data.CharaLight.Ambient = vec4(lightParam.Character.Ambient, 1.0f);
		sceneCB.Data.CharaLight.Diffuse = vec4(lightParam.Character.Diffuse, 1.0f);
		sceneCB.Data.CharaLight.Specular = vec4(lightParam.Character.Specular, 1.0f);
		sceneCB.Data.CharaLight.Direction = vec4(glm::normalize(lightParam.Character.Position), 1.0f);
		sceneCB.Data.StageLight.Ambient = vec4(lightParam.Stage.Ambient, 1.0f);
		sceneCB.Data.StageLight.Diffuse = vec4(lightParam.Stage.Diffuse, 1.0f);
		sceneCB.Data.StageLight.Specular = vec4(lightParam.Stage.Specular, 1.0f);
		sceneCB.Data.StageLight.Direction = vec4(glm::normalize(lightParam.Stage.Position), 1.0f);
		sceneCB.Data.DepthFog.Parameters = vec4(renderParameters->RenderFog ? fog.Depth.Density : 0.0f, fog.Depth.Start, fog.Depth.End, 1.0f / (fog.Depth.End - fog.Depth.Start));
		sceneCB.Data.DepthFog.Color = vec4(fog.Depth.Color, 1.0f);

		sceneCB.Data.ShadowAmbient = vec4(DefaultShadowAmbient, DefaultShadowAmbient, DefaultShadowAmbient, 1.0);
		sceneCB.Data.OneMinusShadowAmbient = vec4(1.0f) - sceneCB.Data.ShadowAmbient;
		sceneCB.Data.ShadowExponent = DefaultShadowExpontent;

		sceneCB.Data.SubsurfaceScatteringParameter = renderParameters->RenderSubsurfaceScattering ? DefaultSSSParameter : 0.0f;

		sceneCB.Data.DebugFlags = renderParameters->ShaderDebugFlags;

		sceneCB.UploadData();

		sceneCB.BindShaders();
		objectCB.BindShaders();

		D3D_ShaderResourceView::BindArray<TextureSlot_Count>(TextureSlot_Diffuse,
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

				// NOTE: IBLCharacterLightMap = 9
				ibl.Character.LightMap.D3D_CubeMap.get(),
				// NOTE: IBLSunLightMap = 10
				ibl.Sun.LightMap.D3D_CubeMap.get(),
				// NOTE: IBLReflectLightMap = 11
				ibl.Reflect.LightMap.D3D_CubeMap.get(),
				// NOTE: IBLShadowLightMap = 12
				ibl.Shadow.LightMap.D3D_CubeMap.get(),
				// NOTE: IBLCharacterColorLightMap = 13
				ibl.CharacterColor.LightMap.D3D_CubeMap.get(),

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
			});

		cachedTextureSamplers.CreateIfNeeded(*renderParameters);

		if (renderParameters->ShadowMapping && isAnyCommand.CastShadow && isAnyCommand.ReceiveShadow)
		{
			InternalPreRenderShadowMap();
			InternalPreRenderReduceFilterShadowMap();
		}

		genericInputLayout->Bind();

		if (renderParameters->RenderReflection && isAnyCommand.ScreenReflection)
		{
			InternalPreRenderScreenReflection();
			renderData->Reflection.RenderTarget.BindResource(TextureSlot_ScreenReflection);
		}

		if (renderParameters->RenderSubsurfaceScattering && isAnyCommand.SubsurfaceScattering)
		{
			InternalPreRenderSubsurfaceScattering();
			InternalPreRenderReduceFilterSubsurfaceScattering();
			renderData->SubsurfaceScattering.FilterRenderTargets.back().BindResource(TextureSlot_SubsurfaceScattering);
		}

		renderData->Main.Current().SetMultiSampleCountIfDifferent(renderParameters->MultiSampleCount);
		renderData->Main.Current().ResizeIfDifferent(renderParameters->RenderResolution);
		renderData->Main.Current().BindSetViewport();

		if (renderParameters->Clear)
			renderData->Main.Current().Clear(renderParameters->ClearColor);
		else
			renderData->Main.Current().GetDepthBuffer()->Clear();

		if (renderParameters->Wireframe)
			wireframeRasterizerState.Bind();

		if (renderParameters->RenderOpaque && !defaultCommandList.OpaqueAndTransparent.empty())
		{
			D3D.Context->OMSetBlendState(nullptr, nullptr, D3D11_DEFAULT_SAMPLE_MASK);

			for (auto& command : defaultCommandList.OpaqueAndTransparent)
				InternalRenderOpaqueObjCommand(command);
		}

		if (renderParameters->RenderTransparent && !defaultCommandList.Transparent.empty())
		{
			transparencyPassDepthStencilState.Bind();

			for (auto& command : defaultCommandList.Transparent)
				InternalRenderTransparentSubMeshCommand(command);

			transparencyPassDepthStencilState.UnBind();
		}

		if (isAnyCommand.SilhouetteOutline)
			InternalRenderSilhouette();

		renderData->Main.Current().UnBind();
		genericInputLayout->UnBind();

		if (renderData->Main.MSAAEnabled())
		{
			auto& current = renderData->Main.Current();
			auto& currentResolved = renderData->Main.CurrentResolved();
			
			currentResolved.ResizeIfDifferent(current.GetSize());
			D3D.Context->ResolveSubresource(currentResolved.GetResource(), 0, current.GetResource(), 0, current.GetBackBufferDescription().Format);
		}
	}

	void D3D_Renderer3D::InternalPreRenderShadowMap()
	{
		const auto& light = sceneContext->Light.Character;
		const auto& camera = sceneContext->Camera;

		shadowSilhouetteInputLayout->Bind();

		// solidBackfaceCullingRasterizerState.Bind();
		// solidFrontfaceCullingRasterizerState.Bind();
		solidNoCullingRasterizerState.Bind();

		const Sphere frustumSphere = CalculateShadowViewFrustumSphere();

		const float lightDistance = frustumSphere.Radius;
		const float nearFarPadding = 0.1f;
		const vec2 nearFarPlane = { -nearFarPadding, (frustumSphere.Radius * 2.0f) + nearFarPadding };

		const vec3 lightViewPoint = glm::normalize(light.Position) * lightDistance;
		const vec3 lightInterest = vec3(0.0f, 0.0f, 0.0f);

		const mat4 lightView = glm::lookAt(
			lightViewPoint + frustumSphere.Center,
			lightInterest + frustumSphere.Center,
			camera.UpDirection);

		const mat4 lightProjection = glm::ortho(
			-frustumSphere.Radius, +frustumSphere.Radius,
			-frustumSphere.Radius, +frustumSphere.Radius,
			nearFarPlane.x, nearFarPlane.y);

		sceneCB.Data.Scene.View = glm::transpose(lightView);
		sceneCB.Data.Scene.ViewProjection = glm::transpose(lightProjection * lightView);
		sceneCB.UploadData();

		renderData->Shadow.RenderTarget.ResizeIfDifferent(renderParameters->ShadowMapResolution);
		renderData->Shadow.RenderTarget.BindSetViewport();
		renderData->Shadow.RenderTarget.Clear(vec4(0.0f));
		D3D.Context->OMSetBlendState(nullptr, nullptr, D3D11_DEFAULT_SAMPLE_MASK);

		shaders.Silhouette.Bind();
		for (auto& command : defaultCommandList.OpaqueAndTransparent)
		{
			if (command.SourceCommand.Flags.CastsShadow)
				InternalRenderOpaqueObjCommand(command, RenderFlags_NoMaterialShader | RenderFlags_NoRasterizerState | RenderFlags_NoDoFrustumCulling | RenderFlags_DiffuseTextureOnly);
		}

		renderData->Shadow.RenderTarget.UnBind();

		sceneCB.Data.Scene.View = glm::transpose(camera.GetView());
		sceneCB.Data.Scene.ViewProjection = glm::transpose(camera.GetViewProjection());
		sceneCB.Data.Scene.LightSpace = glm::transpose(lightProjection * lightView);
		sceneCB.UploadData();
	}

	void D3D_Renderer3D::InternalPreRenderReduceFilterShadowMap()
	{
		solidNoCullingRasterizerState.Bind();

		D3D.Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		D3D.Context->OMSetBlendState(nullptr, nullptr, D3D11_DEFAULT_SAMPLE_MASK);

		D3D_TextureSampler::BindArray<1>(0, { nullptr });

		const ivec2 fullResolution = renderData->Shadow.RenderTarget.GetSize();
		const ivec2 halfResolution = fullResolution / 2;
		const ivec2 blurResolution = fullResolution / 4;

		// NOTE: ESM
		{
			esmFilterCB.BindPixelShader();

			for (auto& renderTarget : renderData->Shadow.ExponentialRenderTargets)
				renderTarget.ResizeIfDifferent(fullResolution);

			shaders.ESMGauss.Bind();
			renderData->Shadow.ExponentialRenderTargets[0].BindSetViewport();
			renderData->Shadow.RenderTarget.BindResource(0);
			esmFilterCB.Data.Coefficients = DefaultShadowCoefficients;
			esmFilterCB.Data.TextureStep = vec2(1.0f / fullResolution.x, 0.0f);
			esmFilterCB.Data.FarTexelOffset = vec2(DefaultShadowTexelOffset, DefaultShadowTexelOffset);
			esmFilterCB.UploadData();
			D3D.Context->Draw(RectangleVertexCount, 0);
			renderData->Shadow.ExponentialRenderTargets[0].UnBind();

			renderData->Shadow.ExponentialRenderTargets[1].Bind();
			renderData->Shadow.ExponentialRenderTargets[0].BindResource(0);
			esmFilterCB.Data.TextureStep = vec2(0.0f, 1.0f / fullResolution.y);
			esmFilterCB.UploadData();
			D3D.Context->Draw(RectangleVertexCount, 0);
			renderData->Shadow.ExponentialRenderTargets[1].UnBind();
			renderData->Shadow.ExponentialRenderTargets[1].BindResource(TextureSlot_ESMFull);

			for (auto& renderTarget : renderData->Shadow.ExponentialBlurRenderTargets)
				renderTarget.ResizeIfDifferent(blurResolution);

			shaders.ESMFilterMin.Bind();
			renderData->Shadow.ExponentialBlurRenderTargets[0].BindSetViewport();
			renderData->Shadow.ExponentialRenderTargets[1].BindResource(0);
			esmFilterCB.Data.TextureStep = vec2(1.0f) / vec2(fullResolution);
			esmFilterCB.UploadData();
			D3D.Context->Draw(RectangleVertexCount, 0);
			renderData->Shadow.ExponentialBlurRenderTargets[0].UnBind();

			shaders.ESMFilterErosion.Bind();
			renderData->Shadow.ExponentialBlurRenderTargets[1].Bind();
			renderData->Shadow.ExponentialBlurRenderTargets[0].BindResource(0);
			esmFilterCB.Data.TextureStep = vec2(0.75f) / vec2(blurResolution);
			esmFilterCB.UploadData();
			D3D.Context->Draw(RectangleVertexCount, 0);
			renderData->Shadow.ExponentialBlurRenderTargets[1].UnBind();
			renderData->Shadow.ExponentialBlurRenderTargets[1].BindResource(TextureSlot_ESMGauss);
		}

		{
			// NOTE: This is the alternative to rendering to a depth and color buffer, is this more performant though, I'm not sure
			shaders.DepthThreshold.Bind();

			renderData->Shadow.ThresholdRenderTarget.ResizeIfDifferent(halfResolution);
			renderData->Shadow.ThresholdRenderTarget.BindSetViewport();
			renderData->Shadow.RenderTarget.BindResource(0);
			D3D.Context->Draw(RectangleVertexCount, 0);
			renderData->Shadow.ThresholdRenderTarget.UnBind();

			for (auto& renderTarget : renderData->Shadow.BlurRenderTargets)
				renderTarget.ResizeIfDifferent(blurResolution);

			const int blurTargets = static_cast<int>(renderData->Shadow.BlurRenderTargets.size());
			const int blurPasses = renderParameters->ShadowBlurPasses + 1;

			D3D.SetViewport(blurResolution);
			shaders.ImgFilter.Bind();

			for (int passIndex = 0; passIndex < blurPasses; passIndex++)
			{
				const int blurIndex = (passIndex % blurTargets);
				const int previousBlurIndex = ((passIndex - 1) + blurTargets) % blurTargets;

				auto& sourceTarget = (passIndex == 0) ? renderData->Shadow.ThresholdRenderTarget : renderData->Shadow.BlurRenderTargets[previousBlurIndex];
				auto& destinationTarget = renderData->Shadow.BlurRenderTargets[blurIndex];

				if (passIndex == 1)
					shaders.ImgFilterBlur.Bind();

				sourceTarget.BindResource(0);
				destinationTarget.Bind();
				D3D.Context->Draw(RectangleVertexCount, 0);
				destinationTarget.UnBind();

				if (passIndex == (blurPasses - 1))
					destinationTarget.BindResource(TextureSlot_ShadowMap);
			}
		}
	}

	void D3D_Renderer3D::InternalPreRenderScreenReflection()
	{
		renderData->Reflection.RenderTarget.ResizeIfDifferent(renderParameters->ReflectionRenderResolution);
		renderData->Reflection.RenderTarget.BindSetViewport();

		if (renderParameters->ClearReflection)
			renderData->Reflection.RenderTarget.Clear(renderParameters->ClearColor);
		else
			renderData->Reflection.RenderTarget.GetDepthBuffer()->Clear();

		if (!reflectionCommandList.OpaqueAndTransparent.empty())
		{
			// TODO: Render using cheaper reflection shaders
			if (renderParameters->RenderOpaque)
			{
				D3D.Context->OMSetBlendState(nullptr, nullptr, D3D11_DEFAULT_SAMPLE_MASK);

				for (auto& command : reflectionCommandList.OpaqueAndTransparent)
					InternalRenderOpaqueObjCommand(command);
			}

			if (renderParameters->RenderTransparent && !reflectionCommandList.Transparent.empty())
			{
				transparencyPassDepthStencilState.Bind();

				for (auto& command : reflectionCommandList.Transparent)
					InternalRenderTransparentSubMeshCommand(command);

				transparencyPassDepthStencilState.UnBind();
			}
		}

		renderData->Reflection.RenderTarget.UnBind();
	}

	void D3D_Renderer3D::InternalPreRenderSubsurfaceScattering()
	{
		renderData->SubsurfaceScattering.RenderTarget.ResizeIfDifferent(renderParameters->RenderResolution);

		renderData->SubsurfaceScattering.RenderTarget.BindSetViewport();
		renderData->SubsurfaceScattering.RenderTarget.Clear(vec4(0.0f));

		if (!defaultCommandList.OpaqueAndTransparent.empty())
		{
			D3D.Context->OMSetBlendState(nullptr, nullptr, D3D11_DEFAULT_SAMPLE_MASK);

			for (auto& command : defaultCommandList.OpaqueAndTransparent)
				InternalRenderOpaqueObjCommand(command, RenderFlags_SSSPass);

			// TODO: defaultCommandList.Transparent (?)
		}

		renderData->SubsurfaceScattering.RenderTarget.UnBind();
	}

	void D3D_Renderer3D::InternalPreRenderReduceFilterSubsurfaceScattering()
	{
		solidNoCullingRasterizerState.Bind();

		D3D.Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		D3D.Context->OMSetBlendState(nullptr, nullptr, D3D11_DEFAULT_SAMPLE_MASK);

		D3D_TextureSampler::BindArray<1>(0, { nullptr });

		shaders.SSSFilterCopy.Bind();
		renderData->SubsurfaceScattering.RenderTarget.BindResource(0);
		renderData->SubsurfaceScattering.FilterRenderTargets[0].BindSetViewport();
		D3D.Context->Draw(RectangleVertexCount, 0);
		renderData->SubsurfaceScattering.FilterRenderTargets[0].UnBind();

		shaders.SSSFilterMin.Bind();
		renderData->SubsurfaceScattering.FilterRenderTargets[0].BindResource(0);
		renderData->SubsurfaceScattering.FilterRenderTargets[1].BindSetViewport();
		D3D.Context->Draw(RectangleVertexCount, 0);
		renderData->SubsurfaceScattering.FilterRenderTargets[1].UnBind();

		sssFilterCB.Data.TextureSize = GetPackedTextureSize(renderData->SubsurfaceScattering.FilterRenderTargets[1]);
		CalculateSSSCoefficients(sceneContext->Camera, sssFilterCB.Data);
		sssFilterCB.BindPixelShader();
		sssFilterCB.UploadData();
		shaders.SSSFilterGauss2D.Bind();
		renderData->SubsurfaceScattering.FilterRenderTargets[1].BindResource(0);
		renderData->SubsurfaceScattering.FilterRenderTargets[2].BindSetViewport();
		D3D.Context->Draw(RectangleVertexCount, 0);
		renderData->SubsurfaceScattering.FilterRenderTargets[2].UnBind();
	}

	void D3D_Renderer3D::InternalRenderOpaqueObjCommand(ObjRenderCommand& command, RenderFlags flags)
	{
		if (command.AreAllMeshesTransparent)
			return;

		auto& transform = command.SourceCommand.Transform;
		auto& obj = *command.SourceCommand.SourceObj;

		const bool doFrustumCulling = !(flags & RenderFlags_NoDoFrustumCulling);

		if (doFrustumCulling && !IntersectsCameraFrustum(obj.BoundingSphere, command))
			return;

		for (size_t meshIndex = 0; meshIndex < obj.Meshes.size(); meshIndex++)
		{
			auto& mesh = obj.Meshes[meshIndex];
			if (doFrustumCulling && !IntersectsCameraFrustum(mesh.BoundingSphere, command))
				continue;

			auto* morphMesh = MeshOrDefault(command.SourceCommand.SourceMorphObj, meshIndex);
			BindMeshVertexBuffers(mesh, morphMesh);

			for (auto& subMesh : mesh.SubMeshes)
			{
				auto& material = subMesh.GetMaterial(obj);
				if (IsMeshTransparent(mesh, subMesh, material))
					continue;

				if ((flags & RenderFlags_SSSPass) && !(UsesSSSSkin(material) || UsesSSSSkinConst(material)))
					continue;

				if (doFrustumCulling && !IntersectsCameraFrustum(subMesh.BoundingSphere, command))
					continue;

				PrepareAndRenderSubMesh(command, mesh, subMesh, material, flags);
			}
		}
	}

	void D3D_Renderer3D::InternalRenderTransparentSubMeshCommand(SubMeshRenderCommand& command)
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

		auto* morphMesh = MeshOrDefault(objCommand->SourceCommand.SourceMorphObj, std::distance(&obj.Meshes.front(), &mesh));
		BindMeshVertexBuffers(mesh, morphMesh);

		auto& material = subMesh.GetMaterial(obj);
		cachedBlendStates.GetState(material.BlendFlags.SrcBlendFactor, material.BlendFlags.DstBlendFactor).Bind();

		PrepareAndRenderSubMesh(*command.ObjCommand, mesh, subMesh, material);
	}

	void D3D_Renderer3D::InternalRenderSilhouette()
	{
		renderData->Silhouette.RenderTarget.ResizeIfDifferent(renderParameters->RenderResolution);
		renderData->Silhouette.RenderTarget.BindSetViewport();
		renderData->Silhouette.RenderTarget.Clear(vec4(0.0f));

		if (!defaultCommandList.OpaqueAndTransparent.empty())
		{
			D3D.Context->OMSetBlendState(nullptr, nullptr, D3D11_DEFAULT_SAMPLE_MASK);

			for (auto& command : defaultCommandList.OpaqueAndTransparent)
				InternalRenderOpaqueObjCommand(command, RenderFlags_SilhouetteOutlinePass | RenderFlags_NoMaterialShader | RenderFlags_NoMaterialTextures);
		}

		renderData->Silhouette.RenderTarget.UnBind();
	}

	void D3D_Renderer3D::InternalRenderSilhouetteOutlineOverlay()
	{
		renderData->Silhouette.RenderTarget.BindResource(0);

		shaders.SilhouetteOutline.Bind();
		D3D.Context->Draw(RectangleVertexCount, 0);
	}

	void D3D_Renderer3D::InternalRenderPostProcessing()
	{
		D3D.Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		constexpr uint32_t attributesToReset = (VertexAttribute_Count * 2);

		std::array<ID3D11Buffer*, attributesToReset> buffers = {};
		std::array<UINT, attributesToReset> strides = {}, offsets = {};
		D3D.Context->IASetVertexBuffers(0, attributesToReset, buffers.data(), strides.data(), offsets.data());

		solidNoCullingRasterizerState.Bind();
		D3D.Context->OMSetBlendState(nullptr, nullptr, D3D11_DEFAULT_SAMPLE_MASK);

		D3D_TextureSampler::BindArray<1>(0, { nullptr });

		if (renderParameters->RenderBloom)
			InternalRenderBloom();

		renderData->Output.RenderTarget.ResizeIfDifferent(renderParameters->RenderResolution);
		renderData->Output.RenderTarget.BindSetViewport();

		if (toneMapData.NeedsUpdating(sceneContext))
		{
			toneMapData.Glow = sceneContext->Glow;
			toneMapData.Update();
		}

		const bool autoExposureEnabled = (renderParameters->AutoExposure && sceneContext->Glow.AutoExposure && renderParameters->RenderBloom);

		D3D_ShaderResourceView::BindArray<4>(0, 
			{ 
				&renderData->Main.CurrentOrResolved(),
				(renderParameters->RenderBloom) ? &renderData->Bloom.CombinedBlurRenderTarget : nullptr,
				toneMapData.LookupTexture.get(),
				autoExposureEnabled ? &renderData->Bloom.ExposureRenderTargets.back() : nullptr
			});

		toneMapCB.Data.Exposure = sceneContext->Glow.Exposure;
		toneMapCB.Data.Gamma = sceneContext->Glow.Gamma;
		toneMapCB.Data.SaturatePower = static_cast<float>(sceneContext->Glow.SaturatePower);
		toneMapCB.Data.SaturateCoefficient = sceneContext->Glow.SaturateCoefficient;
		toneMapCB.Data.AlphaLerp = renderParameters->ToneMapPreserveAlpha ? 0.0f : 1.0f;
		toneMapCB.Data.AlphaValue = 1.0f;
		toneMapCB.Data.AutoExposure = autoExposureEnabled;
		toneMapCB.UploadData();
		toneMapCB.BindPixelShader();

		shaders.ToneMap.Bind();

		D3D.Context->Draw(RectangleVertexCount, 0);

		D3D_ShaderResourceView::BindArray<3>(0, { nullptr, nullptr, nullptr });
		renderData->Main.AdvanceRenderTarget();
	}

	void D3D_Renderer3D::InternalRenderBloom()
	{
		auto& bloom = renderData->Bloom;

		bloom.BaseRenderTarget.ResizeIfDifferent(renderParameters->RenderResolution / 2);

		reduceTexCB.Data.CombineBlurred = false;
		reduceTexCB.BindPixelShader();
		shaders.ReduceTex.Bind();

		for (int i = -1; i < static_cast<int>(bloom.ReduceRenderTargets.size()); i++)
		{
			auto& renderTarget = (i < 0) ? bloom.BaseRenderTarget : bloom.ReduceRenderTargets[i];
			auto& lastRenderTarget = (i < 0) ? renderData->Main.CurrentOrResolved() : (i == 0) ? bloom.BaseRenderTarget : bloom.ReduceRenderTargets[i - 1];

			reduceTexCB.Data.TextureSize = GetPackedTextureSize(lastRenderTarget);
			reduceTexCB.Data.ExtractBrightness = (i == 0);
			reduceTexCB.UploadData();

			renderTarget.BindSetViewport();
			lastRenderTarget.BindResource(0);

			D3D.Context->Draw(RectangleVertexCount, 0);
		}

		if (renderParameters->AutoExposure && sceneContext->Glow.AutoExposure)
			InternalRenderExposurePreBloom();

		CalculateGaussianBlurKernel(sceneContext->Glow, ppGaussCoefCB.Data);
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

			for (int j = 0; j < 2; j++)
			{
				constexpr vec4 horizontalOffsets = vec4(1.0f, 0.0f, 1.0f, 0.0f);
				constexpr vec4 verticalOffsets = vec4(0.0f, 1.0f, 0.0f, 1.0f);

				ppGaussTexCB.Data.TextureOffsets = (j % 2 == 0) ? horizontalOffsets : verticalOffsets;
				ppGaussTexCB.UploadData();

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
		reduceTexCB.BindPixelShader();
		shaders.ReduceTex.Bind();

		D3D.Context->Draw(RectangleVertexCount, 0);

		if (renderParameters->AutoExposure && sceneContext->Glow.AutoExposure)
			InternalRenderExposurePostBloom();
	}

	void D3D_Renderer3D::InternalRenderExposurePreBloom()
	{
		shaders.ExposureMinify.Bind();
		renderData->Bloom.ExposureRenderTargets[0].BindSetViewport();
		renderData->Bloom.ReduceRenderTargets.back().BindResource(0);
		D3D.Context->Draw(RectangleVertexCount, 0);
		renderData->Bloom.ExposureRenderTargets[0].UnBind();
	}

	void D3D_Renderer3D::InternalRenderExposurePostBloom()
	{
		CalculateExposureSpotCoefficients(exposureCB.Data);
		exposureCB.UploadData();
		exposureCB.BindPixelShader();

		shaders.ExposureMeasure.Bind();
		renderData->Bloom.ExposureRenderTargets[1].BindSetViewport();
		renderData->Bloom.ExposureRenderTargets[0].BindResource(0);
		D3D.Context->Draw(RectangleVertexCount, 0);
		renderData->Bloom.ExposureRenderTargets[1].UnBind();

		shaders.ExposureAverage.Bind();
		renderData->Bloom.ExposureRenderTargets[2].BindSetViewport();
		renderData->Bloom.ExposureRenderTargets[1].BindResource(0);
		D3D.Context->Draw(RectangleVertexCount, 0);
		renderData->Bloom.ExposureRenderTargets[2].UnBind();
	}

	void D3D_Renderer3D::BindMeshVertexBuffers(const Mesh& primaryMesh, const Mesh* morphMesh)
	{
		std::array<ID3D11Buffer*, VertexAttribute_Count> buffers;
		std::array<UINT, VertexAttribute_Count> strides;
		std::array<UINT, VertexAttribute_Count> offsets;

		const int meshCount = (morphMesh == nullptr) ? 1 : 2;
		for (int meshIndex = 0; meshIndex < meshCount; meshIndex++)
		{
			const Mesh& mesh = (meshIndex == 0) ? primaryMesh : *morphMesh;;
			for (VertexAttribute i = 0; i < VertexAttribute_Count; i++)
			{
				D3D_StaticVertexBuffer* vertexBuffer = mesh.D3D_VertexBuffers[i].get();

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

			const UINT startSlot = MorphVertexAttributeOffset * meshIndex;
			D3D.Context->IASetVertexBuffers(startSlot, VertexAttribute_Count, buffers.data(), strides.data(), offsets.data());
		}
	}

	void D3D_Renderer3D::PrepareAndRenderSubMesh(const ObjRenderCommand& command, const Mesh& mesh, const SubMesh& subMesh, const Material& material, RenderFlags flags)
	{
		if (!(flags & RenderFlags_NoMaterialShader))
		{
			auto& materialShader = (flags & RenderFlags_SSSPass) ? GetSSSMaterialShader(material) : GetMaterialShader(material);
			materialShader.Bind();
		}

		if (flags & RenderFlags_SilhouetteOutlinePass)
		{
			if (command.SourceCommand.Flags.SilhouetteOutline)
				shaders.SolidWhite.Bind();
			else
				shaders.SolidBlack.Bind();
		}

		if (!(flags & RenderFlags_NoMaterialTextures))
		{
			constexpr size_t textureTypeCount = 7;
			std::array<D3D_ShaderResourceView*, textureTypeCount> textureResources = {};
			std::array<D3D_TextureSampler*, textureTypeCount> textureSamplers = {};

			objectCB.Data.DiffuseRGTC1 = false;
			objectCB.Data.DiffuseScreenTexture = false;

			const size_t texturesToBind = (flags & RenderFlags_DiffuseTextureOnly) ? 1 : textureTypeCount;

			for (size_t i = 0; i < texturesToBind; i++)
			{
				const MaterialTexture& materialTexture = (&material.DiffuseMap)[i];

				TxpID txpID = materialTexture.TextureID;
				MaterialTextureFlags textureFlags = materialTexture.Flags;

				if (command.SourceCommand.Animation != nullptr)
				{
					for (auto& transform : command.SourceCommand.Animation->TextureTransforms)
					{
						if (txpID == transform.ID)
						{
							textureFlags.TextureAddressMode_U_Repeat = transform.RepeatU.value_or<int>(textureFlags.TextureAddressMode_U_Repeat);
							textureFlags.TextureAddressMode_V_Repeat = transform.RepeatU.value_or<int>(textureFlags.TextureAddressMode_V_Repeat);
						}
					}

					for (auto& pattern : command.SourceCommand.Animation->TexturePatterns)
					{
						if (txpID == pattern.ID && pattern.IDOverride != TxpID::Invalid)
							txpID = pattern.IDOverride;
					}
				}

				if (auto txp = GetTxpFromTextureID(txpID); txp != nullptr)
				{
					textureResources[i] = (txp->D3D_Texture2D != nullptr) ? static_cast<D3D_TextureResource*>(txp->D3D_Texture2D.get()) : (txp->D3D_CubeMap != nullptr) ? (txp->D3D_CubeMap.get()) : nullptr;
					textureSamplers[i] = &cachedTextureSamplers.GetSampler(textureFlags);

					if (&materialTexture == &material.DiffuseMap)
					{
						if (command.SourceCommand.Animation != nullptr && txpID == command.SourceCommand.Animation->ScreenRenderTextureID)
						{
							objectCB.Data.DiffuseScreenTexture = true;
							textureResources[i] = &renderData->Main.PreviousOrResolved();
						}

						if (txp->GetFormat() == TextureFormat::RGTC1)
							objectCB.Data.DiffuseRGTC1 = true;
					}
				}
			}

			auto ambientTypeFlags = material.AmbientMap.Flags.AmbientTypeFlags;
			objectCB.Data.AmbientTextureType = (ambientTypeFlags == 0b100) ? 2 : (ambientTypeFlags == 0b110) ? 1 : (ambientTypeFlags != 0b10000) ? 0 : 3;

			D3D_ShaderResourceView::BindArray(TextureSlot_Diffuse, textureResources);
			D3D_TextureSampler::BindArray(TextureSlot_Diffuse, textureSamplers);

			objectCB.Data.Material.DiffuseTextureTransform = material.DiffuseMap.TextureCoordinateMatrix;
			objectCB.Data.Material.AmbientTextureTransform = material.AmbientMap.TextureCoordinateMatrix;

			if (command.SourceCommand.Animation != nullptr)
			{
				for (auto& textureTransform : command.SourceCommand.Animation->TextureTransforms)
				{
					mat4* output =
						(textureTransform.ID == material.DiffuseMap.TextureID) ? &objectCB.Data.Material.DiffuseTextureTransform :
						(textureTransform.ID == material.AmbientMap.TextureID) ? &objectCB.Data.Material.AmbientTextureTransform :
						nullptr;

					if (output == nullptr)
						continue;

					constexpr vec3 centerOffset = vec3(0.5f, 0.5f, 0.0f);
					constexpr vec3 rotationAxis = vec3(0.0f, 0.0f, 1.0f);

					*output =
						glm::translate(mat4(1.0f), vec3(textureTransform.Translation, 0.0f))
						* glm::translate(mat4(1.0f), +centerOffset)
						* glm::rotate(mat4(1.0f), glm::radians(textureTransform.Rotation), rotationAxis)
						* glm::translate(*output, -centerOffset);
				}
			}

			// HACK: Flip to adjust for the expected OpenGL texture coordinates, problematic because it also effects all other textures using the first TEXCOORD attribute
			if (objectCB.Data.DiffuseScreenTexture)
				objectCB.Data.Material.DiffuseTextureTransform *= glm::scale(mat4(1.0f), vec3(1.0f, -1.0f, 1.0f));
		}

		if (!renderParameters->Wireframe && !(flags & RenderFlags_NoRasterizerState))
			((material.BlendFlags.DoubleSidedness != DoubleSidedness_Off) ? solidNoCullingRasterizerState : solidBackfaceCullingRasterizerState).Bind();

		const float fresnel = (((material.ShaderFlags.Fresnel == 0) ? 7.0f : static_cast<float>(material.ShaderFlags.Fresnel) - 1.0f) * 0.12f) * 0.82f;
		const float lineLight = material.ShaderFlags.LineLight * 0.111f;
		objectCB.Data.Material.FresnelCoefficients = vec4(fresnel, 0.18f, lineLight, 0.0f);
		objectCB.Data.Material.Diffuse = material.DiffuseColor;
		objectCB.Data.Material.Transparency = material.Transparency;
		objectCB.Data.Material.Ambient = material.AmbientColor;
		objectCB.Data.Material.Specular = material.SpecularColor;
		objectCB.Data.Material.Reflectivity = material.Reflectivity;
		objectCB.Data.Material.Emission = material.EmissionColor;

		objectCB.Data.Material.Shininess = vec2(
			(material.Shininess >= 0.0f ? material.Shininess : 1.0f),
			(material.MaterialType != Material::Identifiers.EYEBALL) ? ((material.Shininess - 16.0f) / 112.0f) : 10.0f);

		objectCB.Data.Material.Intensity = material.Intensity;
		objectCB.Data.Material.BumpDepth = material.BumpDepth;

		const float morphWeight = (command.SourceCommand.Animation != nullptr) ? command.SourceCommand.Animation->MorphWeight : 0.0f;
		objectCB.Data.MorphWeight = vec4(morphWeight, 1.0f - morphWeight, 0.0f, 0.0f);

		mat4 modelMatrix;
		if (mesh.Flags.FaceCameraPosition || mesh.Flags.FaceCameraView)
		{
			const auto& transform = command.SourceCommand.Transform;

			// TODO: if (mesh.Flags.FaceCameraView)
			const vec3 viewPoint = sceneContext->Camera.ViewPoint;
			const float cameraAngle = glm::atan(transform.Translation.x - viewPoint.x, transform.Translation.z - viewPoint.z);

			modelMatrix = glm::rotate(command.ModelMatrix, cameraAngle - glm::pi<float>() - glm::radians(transform.Rotation.y), vec3(0.0f, 1.0f, 0.0f));
		}
		else
		{
			modelMatrix = command.ModelMatrix;
		}

		objectCB.Data.Model = glm::transpose(modelMatrix);
#if 0 // TODO:
		objectCB.Data.ModelView = glm::transpose(sceneContext->Camera.GetView() * modelMatrix);
		objectCB.Data.ModelViewProjection = glm::transpose(sceneContext->Camera.GetViewProjection() * modelMatrix);
#else
		objectCB.Data.ModelView = glm::transpose(glm::transpose(sceneCB.Data.Scene.View) * modelMatrix);
		objectCB.Data.ModelViewProjection = glm::transpose(glm::transpose(sceneCB.Data.Scene.ViewProjection) * modelMatrix);
#endif

		objectCB.Data.ShaderFlags = 0;

		if (renderParameters->VertexColoring)
		{
			if (mesh.AttributeFlags & VertexAttributeFlags_Color0)
				objectCB.Data.ShaderFlags |= ShaderFlags_VertexColor;
		}

		if (renderParameters->DiffuseMapping)
		{
			if (material.DiffuseMap.TextureID != TxpID::Invalid)
				objectCB.Data.ShaderFlags |= ShaderFlags_DiffuseTexture;
		}

		if (renderParameters->AmbientOcclusionMapping)
		{
			if (material.AmbientMap.TextureID != TxpID::Invalid)
				objectCB.Data.ShaderFlags |= ShaderFlags_AmbientTexture;
		}

		if (renderParameters->NormalMapping)
		{
			if (material.NormalMap.TextureID != TxpID::Invalid)
				objectCB.Data.ShaderFlags |= ShaderFlags_NormalTexture;
		}

		if (renderParameters->SpecularMapping)
		{
			if (material.SpecularMap.TextureID != TxpID::Invalid)
				objectCB.Data.ShaderFlags |= ShaderFlags_SpecularTexture;
		}

		if (renderParameters->TransparencyMapping)
		{
			if (material.TransparencyMap.TextureID != TxpID::Invalid)
				objectCB.Data.ShaderFlags |= ShaderFlags_TransparencyTexture;
		}

		if (renderParameters->EnvironmentMapping)
		{
			if (material.EnvironmentMap.TextureID != TxpID::Invalid)
				objectCB.Data.ShaderFlags |= ShaderFlags_EnvironmentTexture;
		}

		if (renderParameters->TranslucencyMapping)
		{
			if (material.TranslucencyMap.TextureID != TxpID::Invalid)
				objectCB.Data.ShaderFlags |= ShaderFlags_TranslucencyTexture;
		}

		if (renderParameters->RenderPunchThrough)
		{
			if (material.BlendFlags.EnableAlphaTest && !IsMeshTransparent(mesh, subMesh, material))
				objectCB.Data.ShaderFlags |= ShaderFlags_PunchThrough;
		}

		if (renderParameters->RenderFog)
		{
			if (sceneContext->Fog.Depth.Density > 0.0f)
				objectCB.Data.ShaderFlags |= ShaderFlags_LinearFog;
		}

		if (renderParameters->ObjectMorphing)
		{
			if (command.SourceCommand.SourceMorphObj != nullptr)
			{
				objectCB.Data.ShaderFlags |= ShaderFlags_Morph;
				objectCB.Data.ShaderFlags |= ShaderFlags_MorphColor;
			}
		}

		if (renderParameters->ShadowMapping && isAnyCommand.CastShadow && isAnyCommand.ReceiveShadow)
		{
			if (ReceivesShadows(command.SourceCommand, mesh, subMesh))
				objectCB.Data.ShaderFlags |= ShaderFlags_Shadow;
		}

		if (renderParameters->ShadowMapping && renderParameters->SelfShadowing && isAnyCommand.CastShadow && isAnyCommand.ReceiveShadow)
		{
			if (ReceivesSelfShadow(command.SourceCommand, mesh, subMesh))
				objectCB.Data.ShaderFlags |= ShaderFlags_SelfShadow;
		}

		objectCB.UploadData();

		SubmitSubMeshDrawCall(subMesh);
	}

	D3D_ShaderPair& D3D_Renderer3D::GetMaterialShader(const Material& material)
	{
		if (material.MaterialType == Material::Identifiers.BLINN)
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
		else if (material.MaterialType == Material::Identifiers.ITEM)
		{
			return shaders.ItemBlinn;
		}
		else if (material.MaterialType == Material::Identifiers.STAGE)
		{
			return shaders.StageBlinn;
		}
		else if (material.MaterialType == Material::Identifiers.SKIN)
		{
			return shaders.SkinDefault;
		}
		else if (material.MaterialType == Material::Identifiers.HAIR)
		{
			return (material.ShaderFlags.AnisoDirection != AnisoDirection_Normal) ? shaders.HairAniso : shaders.HairDefault;
		}
		else if (material.MaterialType == Material::Identifiers.CLOTH)
		{
			return (material.ShaderFlags.AnisoDirection != AnisoDirection_Normal) ? shaders.ClothAniso : shaders.ClothDefault;
		}
		else if (material.MaterialType == Material::Identifiers.TIGHTS)
		{
			return shaders.Tights;
		}
		else if (material.MaterialType == Material::Identifiers.SKY)
		{
			return shaders.SkyDefault;
		}
		else if (material.MaterialType == Material::Identifiers.EYEBALL)
		{
			return (true) ? shaders.GlassEye : shaders.EyeBall;
		}
		else if (material.MaterialType == Material::Identifiers.EYELENS)
		{
			return shaders.EyeLens;
		}
		else if (material.MaterialType == Material::Identifiers.GLASEYE)
		{
			return shaders.GlassEye;
		}
		else if (material.MaterialType == Material::Identifiers.WATER01 || material.MaterialType == Material::Identifiers.WATER02)
		{
			return shaders.Water;
		}
		else if (material.MaterialType == Material::Identifiers.FLOOR)
		{
			return shaders.Floor;
		}
		else
		{
			return shaders.Debug;
		}
	}

	D3D_ShaderPair& D3D_Renderer3D::GetSSSMaterialShader(const Material& material)
	{
		if (UsesSSSSkinConst(material))
			return shaders.SSSSkinConst;

		return shaders.SSSSkin;
	}

	void D3D_Renderer3D::SubmitSubMeshDrawCall(const SubMesh& subMesh)
	{
		subMesh.D3D_IndexBuffer->Bind();

		D3D.Context->IASetPrimitiveTopology(GetD3DPrimitiveTopolgy(subMesh.Primitive));
		D3D.Context->DrawIndexed(static_cast<UINT>(subMesh.Indices.size()), 0, 0);

		statistics.VerticesRendered += subMesh.Indices.size();
	}

	Sphere D3D_Renderer3D::CalculateShadowViewFrustumSphere() const
	{
		// TODO: If larger than some threshold, split into two (or more)
		// TODO: Shadow casting objects which don't lie within the view frustum *nor* the light frustum should be ignored

		if (!isAnyCommand.CastShadow)
			return Sphere { vec3(0.0f), 1.0f };

		vec3 min, max;
		for (auto& command : defaultCommandList.OpaqueAndTransparent)
		{
			if (!command.SourceCommand.Flags.CastsShadow)
				continue;

			min = max = command.TransformedBoundingSphere.Center;
			break;
		}

		for (auto& command : defaultCommandList.OpaqueAndTransparent)
		{
			if (!command.SourceCommand.Flags.CastsShadow)
				continue;

			const auto sphere = command.TransformedBoundingSphere;
			const float radius = sphere.Radius;

			min.x = std::min(min.x, sphere.Center.x - radius);
			min.y = std::min(min.y, sphere.Center.y - radius);
			min.z = std::min(min.z, sphere.Center.z - radius);

			max.x = std::max(max.x, sphere.Center.x + radius);
			max.y = std::max(max.y, sphere.Center.y + radius);
			max.z = std::max(max.z, sphere.Center.z + radius);
		}

		const vec3 size = (max - min) / 2.0f;
		return Sphere { (min + size), (std::max(size.x, std::max(size.y, size.z))) };
	}

	bool D3D_Renderer3D::IntersectsCameraFrustum(const ObjRenderCommand& command) const
	{
		if (!renderParameters->FrustumCulling)
			return true;

		return sceneContext->Camera.IntersectsViewFrustum(command.TransformedBoundingSphere);
	}

	bool D3D_Renderer3D::IntersectsCameraFrustum(const Sphere& boundingSphere, const ObjRenderCommand& command) const
	{
		if (!renderParameters->FrustumCulling)
			return true;

		return sceneContext->Camera.IntersectsViewFrustum(boundingSphere * command.SourceCommand.Transform);
	}

	bool D3D_Renderer3D::IntersectsCameraFrustum(const Sphere& boundingSphere, const SubMeshRenderCommand& command) const
	{
		return IntersectsCameraFrustum(boundingSphere, *command.ObjCommand);
	}

	bool D3D_Renderer3D::IsDebugRenderFlagSet(int bitIndex) const
	{
		return renderParameters->DebugFlags & (1 << bitIndex);
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
		constexpr std::array blendFactorNames = { "Zero", "One", "SrcColor", "ISrcColor", "SrcAlpha", "ISrcAlpha", "DstAlpha", "IDstAlpha", "DstColor", "IDstColor", };

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
