#include "D3D_Renderer3D.h"
#include "Core/TimeSpan.h"
#include "ImGui/Gui.h"

namespace Comfy::Graphics
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

		const Material& GetSubMeshMaterial(const SubMesh& subMesh, const RenderCommand& command)
		{
			if (command.Animation != nullptr)
			{
				for (auto& materialOverride : command.Animation->MaterialOverrides)
				{
					if (&subMesh == materialOverride.SubMeshToReplace && materialOverride.NewMaterial != nullptr)
						return *materialOverride.NewMaterial;
				}
			}

			const auto& parentObj = *command.SourceObj;

			if (InBounds(subMesh.MaterialIndex, parentObj.Materials))
				return parentObj.Materials[subMesh.MaterialIndex];

			static Material dummyMaterial = {};
			return dummyMaterial;
		}

		constexpr bool IsMeshOrSubMeshIndexSpecified(int index)
		{
			return (index >= 0);
		}

		template <typename Func>
		void IterateCommandMeshes(const RenderCommand& command, Func func)
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
		void IterateCommandSubMeshes(const RenderCommand& command, const Mesh& mesh, Func func)
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
		void IterateCommandMeshesAndSubMeshes(const RenderCommand& command, Func func)
		{
			IterateCommandMeshes(command, [&](auto& mesh)
			{
				IterateCommandSubMeshes(command, mesh, [&](auto& subMesh, auto& material)
				{
					func(mesh, subMesh, material);
				});
			});
		}

		constexpr bool ReceivesShadows(const RenderCommand& command, const Mesh& mesh, const SubMesh& subMesh)
		{
			return (command.Flags.ReceivesShadow && subMesh.Flags.ReceivesShadows);
		}

		constexpr bool ReceivesSelfShadow(const RenderCommand& command, const Mesh& mesh, const SubMesh& subMesh)
		{
			return (command.Flags.ReceivesShadow);
		}

		constexpr bool IsMeshTransparent(const Mesh& mesh, const SubMesh& subMesh, const Material& material)
		{
			if (material.BlendFlags.AlphaMaterial)
				return true;

			if (material.BlendFlags.AlphaTexture && !material.BlendFlags.PunchThrough)
				return true;

			if (subMesh.Flags.Transparent)
				return true;

			return false;
		}

		bool UsesSSSSkin(const Material& material)
		{
			if (material.ShaderType == Material::ShaderIdentifiers::Skin || material.ShaderType == Material::ShaderIdentifiers::EyeBall || material.ShaderType == Material::ShaderIdentifiers::EyeLens)
				return true;

			return false;
		}

		bool UsesSSSSkinConst(const Material& material)
		{
			if (material.ShaderType == Material::ShaderIdentifiers::Hair || material.ShaderType == Material::ShaderIdentifiers::Cloth || material.ShaderType == Material::ShaderIdentifiers::Tights)
				return true;

			return false;
		}

		const Mesh* GetMorphMesh(const Obj& obj, const Obj* morphObj, const Mesh& mesh)
		{
			if (morphObj == nullptr)
				return nullptr;

			const size_t meshIndex = static_cast<size_t>(std::distance(&obj.Meshes.front(), &mesh));
			return (meshIndex < morphObj->Meshes.size()) ? &morphObj->Meshes[meshIndex] : nullptr;
		}

		vec4 GetPackedTextureSize(vec2 size)
		{
			return vec4(1.0f / size, size);
		}

		vec4 GetPackedTextureSize(const D3D_RenderTargetBase& renderTarget)
		{
			return GetPackedTextureSize(vec2(renderTarget.GetSize()));
		}

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
	}

	D3D_Renderer3D::D3D_Renderer3D(TxpGetterFunction txpGetter)
	{
		this->txpGetter = txpGetter;

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

		genericInputLayout = MakeUnique<D3D_InputLayout>(genericElements, std::size(genericElements), shaders.DebugMaterial.VS);
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

	void D3D_Renderer3D::Begin(SceneViewport& viewport, const SceneParameters& scene)
	{
		lastFrameStatistics = statistics;
		statistics = {};

		isAnyCommand = {};
		current = { &viewport, &scene };
	}

	void D3D_Renderer3D::Draw(const RenderCommand& command)
	{
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

		auto& commandList = (command.Flags.IsReflection) ? reflectionCommandList : defaultCommandList;
		ObjRenderCommand& newRenderCommand = commandList.OpaqueAndTransparent.emplace_back();

		newRenderCommand.SourceCommand = command;
		newRenderCommand.AreAllMeshesTransparent = false;
		newRenderCommand.ModelMatrix = command.Transform.CalculateMatrix();

		newRenderCommand.TransformedBoundingSphere = command.SourceObj->BoundingSphere;
		newRenderCommand.TransformedBoundingSphere.Transform(newRenderCommand.ModelMatrix, command.Transform.Scale);

		UpdateIsAnyCommandFlags(command);
	}

	void D3D_Renderer3D::End()
	{
		assert(current.Scene != nullptr && current.Viewport != nullptr);

		InternalFlush();
		current = {};
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
			IterateCommandMeshesAndSubMeshes(command, [&](auto& mesh, auto& subMesh, auto& material)
			{
				if (ReceivesShadows(command, mesh, subMesh) || ReceivesSelfShadow(command, mesh, subMesh))
					isAnyCommand.ReceiveShadow = true;
			});
		}

		if (!isAnyCommand.SubsurfaceScattering)
		{
			IterateCommandMeshesAndSubMeshes(command, [&](auto& mesh, auto& subMesh, auto& material)
			{
				if (UsesSSSSkin(GetSubMeshMaterial(subMesh, command)))
					isAnyCommand.SubsurfaceScattering = true;
			});
		}
	}

	const Txp* D3D_Renderer3D::GetTxpFromTextureID(const Cached_TxpID* textureID) const
	{
		return txpGetter(textureID);
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

			IterateCommandMeshesAndSubMeshes(command.SourceCommand, [&](auto& mesh, auto& subMesh, auto& material)
			{
				if (IsMeshTransparent(mesh, subMesh, material))
				{
					const auto boundingSphere = (subMesh.BoundingSphere * command.SourceCommand.Transform);
					const float cameraDistance = glm::distance(boundingSphere.Center, current.Viewport->Camera.ViewPoint);

					commandList.Transparent.push_back({ &command, &mesh, &subMesh, cameraDistance });
				}
				else
				{
					command.AreAllMeshesTransparent = false;
				}
			});
		}

		if (current.Viewport->Parameters.AlphaSort)
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
		InternalSetUploadSceneCB();

		sceneCB.BindShaders();
		objectCB.BindShaders();
		skeletonCB.BindVertexShader();

		InternalBindSceneTextures();

		cachedTextureSamplers.CreateIfNeeded(current.Viewport->Parameters);

		if (current.Viewport->Parameters.ShadowMapping && isAnyCommand.CastShadow && isAnyCommand.ReceiveShadow)
		{
			InternalPreRenderShadowMap();
			InternalPreRenderReduceFilterShadowMap();
		}

		genericInputLayout->Bind();

		if (current.Viewport->Parameters.RenderReflection && isAnyCommand.ScreenReflection)
		{
			InternalPreRenderScreenReflection();
			current.Viewport->Data.Reflection.RenderTarget.BindResource(TextureSlot_ScreenReflection);
		}

		if (current.Viewport->Parameters.RenderSubsurfaceScattering && isAnyCommand.SubsurfaceScattering)
		{
			InternalPreRenderSubsurfaceScattering();
			InternalPreRenderReduceFilterSubsurfaceScattering();
			current.Viewport->Data.SubsurfaceScattering.FilterRenderTargets.back().BindResource(TextureSlot_SubsurfaceScattering);
		}

		current.Viewport->Data.Main.Current().SetMultiSampleCountIfDifferent(current.Viewport->Parameters.MultiSampleCount);
		current.Viewport->Data.Main.Current().ResizeIfDifferent(current.Viewport->Parameters.RenderResolution);
		current.Viewport->Data.Main.Current().BindSetViewport();

		if (current.Viewport->Parameters.Clear)
			current.Viewport->Data.Main.Current().Clear(current.Viewport->Parameters.ClearColor);
		else
			current.Viewport->Data.Main.Current().GetDepthBuffer()->Clear();

		if (current.Viewport->Parameters.Wireframe)
			wireframeRasterizerState.Bind();

		if (current.Viewport->Parameters.RenderOpaque && !defaultCommandList.OpaqueAndTransparent.empty())
		{
			D3D.Context->OMSetBlendState(nullptr, nullptr, D3D11_DEFAULT_SAMPLE_MASK);

			for (auto& command : defaultCommandList.OpaqueAndTransparent)
				InternalRenderOpaqueObjCommand(command);

			if (current.Viewport->Parameters.RenderLensFlare && current.Scene->LensFlare.SunPosition.has_value())
				InternalQueryRenderLensFlare();
		}

		if (current.Viewport->Parameters.RenderTransparent && !defaultCommandList.Transparent.empty())
		{
			transparencyPassDepthStencilState.Bind();

			for (auto& command : defaultCommandList.Transparent)
				InternalRenderTransparentSubMeshCommand(command);

			if (current.Viewport->Parameters.RenderLensFlare && current.Scene->LensFlare.SunPosition.has_value())
				InternalRenderLensFlare();

			transparencyPassDepthStencilState.UnBind();
		}

		if (isAnyCommand.SilhouetteOutline)
			InternalRenderSilhouette();

		current.Viewport->Data.Main.Current().UnBind();
		genericInputLayout->UnBind();

		if (current.Viewport->Data.Main.MSAAEnabled())
		{
			auto& currentMain = current.Viewport->Data.Main.Current();
			auto& currentMainResolved = current.Viewport->Data.Main.CurrentResolved();

			currentMainResolved.ResizeIfDifferent(currentMain.GetSize());
			D3D.Context->ResolveSubresource(currentMainResolved.GetResource(), 0, currentMain.GetResource(), 0, currentMain.GetBackBufferDescription().Format);
		}
	}

	void D3D_Renderer3D::InternalSetUploadSceneCB()
	{
		sceneCB.Data.RenderResolution = GetPackedTextureSize(current.Viewport->Data.Main.Current());

		const vec4 renderTimeNow = static_cast<float>(TimeSpan::GetTimeNow().TotalSeconds()) * SceneConstantData::RenderTime::Scales;
		sceneCB.Data.RenderTime.Time = renderTimeNow;
		sceneCB.Data.RenderTime.TimeSin = (glm::sin(renderTimeNow) + 1.0f) * 0.5f;
		sceneCB.Data.RenderTime.TimeCos = (glm::cos(renderTimeNow) + 1.0f) * 0.5f;

		const auto& ibl = current.Scene->IBL;
		for (size_t component = 0; component < ibl.Lights[1].IrradianceRGB.size(); component++)
			sceneCB.Data.IBL.IrradianceRGB[component] = glm::transpose(ibl.Lights[1].IrradianceRGB[component]);

		for (size_t i = 0; i < ibl.Lights.size(); i++)
			sceneCB.Data.IBL.LightColors[i] = vec4(ibl.Lights[i].LightColor, 1.0f);

		const auto& camera = current.Viewport->Camera;
		sceneCB.Data.Scene.View = glm::transpose(camera.GetView());
		sceneCB.Data.Scene.ViewProjection = glm::transpose(camera.GetViewProjection());
		sceneCB.Data.Scene.EyePosition = vec4(camera.ViewPoint, 1.0f);

		const auto& light = current.Scene->Light;
		sceneCB.Data.CharaLight.Ambient = vec4(light.Character.Ambient, 1.0f);
		sceneCB.Data.CharaLight.Diffuse = vec4(light.Character.Diffuse, 1.0f);
		sceneCB.Data.CharaLight.Specular = vec4(light.Character.Specular, 1.0f);
		sceneCB.Data.CharaLight.Direction = vec4(glm::normalize(light.Character.Position), 1.0f);

		sceneCB.Data.StageLight.Ambient = vec4(light.Stage.Ambient, 1.0f);
		sceneCB.Data.StageLight.Diffuse = vec4(light.Stage.Diffuse, 1.0f);
		sceneCB.Data.StageLight.Specular = vec4(light.Stage.Specular, 1.0f);
		sceneCB.Data.StageLight.Direction = vec4(glm::normalize(light.Stage.Position), 1.0f);

		const auto& depthFog = current.Scene->Fog.Depth;
		sceneCB.Data.DepthFog.Parameters = vec4(current.Viewport->Parameters.RenderFog ? depthFog.Density : 0.0f, depthFog.Start, depthFog.End, 1.0f / (depthFog.End - depthFog.Start));
		sceneCB.Data.DepthFog.Color = vec4(depthFog.Color, 1.0f);

		sceneCB.Data.ShadowAmbient = vec4(DefaultShadowAmbient, DefaultShadowAmbient, DefaultShadowAmbient, 1.0);
		sceneCB.Data.OneMinusShadowAmbient = vec4(1.0f) - sceneCB.Data.ShadowAmbient;
		sceneCB.Data.ShadowExponent = DefaultShadowExpontent;

		sceneCB.Data.SubsurfaceScatteringParameter = current.Viewport->Parameters.RenderSubsurfaceScattering ? DefaultSSSParameter : 0.0f;

		sceneCB.Data.DebugFlags = current.Viewport->Parameters.ShaderDebugFlags;
		sceneCB.Data.DebugValue = current.Viewport->Parameters.ShaderDebugValue;

		sceneCB.UploadData();
	}

	void D3D_Renderer3D::InternalBindSceneTextures()
	{
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

				// NOTE: IBLLightMaps_0 = 9
				current.Scene->IBL.LightMaps[0].GPU_CubeMap.get(),
				// NOTE: IBLLightMaps_1 = 10
				current.Scene->IBL.LightMaps[1].GPU_CubeMap.get(),
				// NOTE: IBLLightMaps_2 = 11
				current.Scene->IBL.LightMaps[2].GPU_CubeMap.get(),

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
			});
	}

	void D3D_Renderer3D::InternalPreRenderShadowMap()
	{
		const auto& light = current.Scene->Light.Character;

		shadowSilhouetteInputLayout->Bind();

		solidNoCullingRasterizerState.Bind();

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
			current.Viewport->Camera.UpDirection);

		const mat4 lightProjection = glm::ortho(
			-frustumSphere.Radius, +frustumSphere.Radius,
			-frustumSphere.Radius, +frustumSphere.Radius,
			nearFarPlane.x, nearFarPlane.y);

		sceneCB.Data.Scene.View = glm::transpose(lightView);
		sceneCB.Data.Scene.ViewProjection = glm::transpose(lightProjection * lightView);
		sceneCB.UploadData();

		current.Viewport->Data.Shadow.RenderTarget.ResizeIfDifferent(current.Viewport->Parameters.ShadowMapResolution);
		current.Viewport->Data.Shadow.RenderTarget.BindSetViewport();
		current.Viewport->Data.Shadow.RenderTarget.Clear(vec4(0.0f));
		D3D.Context->OMSetBlendState(nullptr, nullptr, D3D11_DEFAULT_SAMPLE_MASK);

		shaders.Silhouette.Bind();
		for (auto& command : defaultCommandList.OpaqueAndTransparent)
		{
			if (command.SourceCommand.Flags.CastsShadow)
				InternalRenderOpaqueObjCommand(command, RenderFlags_NoMaterialShader | RenderFlags_NoRasterizerState | RenderFlags_NoFrustumCulling | RenderFlags_DiffuseTextureOnly);
		}

		current.Viewport->Data.Shadow.RenderTarget.UnBind();

		sceneCB.Data.Scene.View = glm::transpose(current.Viewport->Camera.GetView());
		sceneCB.Data.Scene.ViewProjection = glm::transpose(current.Viewport->Camera.GetViewProjection());
		sceneCB.Data.Scene.LightSpace = glm::transpose(lightProjection * lightView);
		sceneCB.UploadData();
	}

	void D3D_Renderer3D::InternalPreRenderReduceFilterShadowMap()
	{
		solidNoCullingRasterizerState.Bind();

		D3D.Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		D3D.Context->OMSetBlendState(nullptr, nullptr, D3D11_DEFAULT_SAMPLE_MASK);

		D3D_TextureSampler::BindArray<1>(0, { nullptr });

		const ivec2 fullResolution = current.Viewport->Data.Shadow.RenderTarget.GetSize();
		const ivec2 halfResolution = fullResolution / 2;
		const ivec2 blurResolution = fullResolution / 4;

		// NOTE: ESM
		{
			esmFilterCB.BindPixelShader();

			for (auto& renderTarget : current.Viewport->Data.Shadow.ExponentialRenderTargets)
				renderTarget.ResizeIfDifferent(fullResolution);

			shaders.ESMGauss.Bind();
			current.Viewport->Data.Shadow.ExponentialRenderTargets[0].BindSetViewport();
			current.Viewport->Data.Shadow.RenderTarget.BindResource(0);
			esmFilterCB.Data.Coefficients = DefaultShadowCoefficients;
			esmFilterCB.Data.TextureStep = vec2(1.0f / fullResolution.x, 0.0f);
			esmFilterCB.Data.FarTexelOffset = vec2(DefaultShadowTexelOffset, DefaultShadowTexelOffset);
			esmFilterCB.UploadData();
			D3D.Context->Draw(RectangleVertexCount, 0);
			current.Viewport->Data.Shadow.ExponentialRenderTargets[0].UnBind();

			current.Viewport->Data.Shadow.ExponentialRenderTargets[1].Bind();
			current.Viewport->Data.Shadow.ExponentialRenderTargets[0].BindResource(0);
			esmFilterCB.Data.TextureStep = vec2(0.0f, 1.0f / fullResolution.y);
			esmFilterCB.UploadData();
			D3D.Context->Draw(RectangleVertexCount, 0);
			current.Viewport->Data.Shadow.ExponentialRenderTargets[1].UnBind();
			current.Viewport->Data.Shadow.ExponentialRenderTargets[1].BindResource(TextureSlot_ESMFull);

			for (auto& renderTarget : current.Viewport->Data.Shadow.ExponentialBlurRenderTargets)
				renderTarget.ResizeIfDifferent(blurResolution);

			shaders.ESMFilterMin.Bind();
			current.Viewport->Data.Shadow.ExponentialBlurRenderTargets[0].BindSetViewport();
			current.Viewport->Data.Shadow.ExponentialRenderTargets[1].BindResource(0);
			esmFilterCB.Data.TextureStep = vec2(1.0f) / vec2(fullResolution);
			esmFilterCB.UploadData();
			D3D.Context->Draw(RectangleVertexCount, 0);
			current.Viewport->Data.Shadow.ExponentialBlurRenderTargets[0].UnBind();

			shaders.ESMFilterErosion.Bind();
			current.Viewport->Data.Shadow.ExponentialBlurRenderTargets[1].Bind();
			current.Viewport->Data.Shadow.ExponentialBlurRenderTargets[0].BindResource(0);
			esmFilterCB.Data.TextureStep = vec2(0.75f) / vec2(blurResolution);
			esmFilterCB.UploadData();
			D3D.Context->Draw(RectangleVertexCount, 0);
			current.Viewport->Data.Shadow.ExponentialBlurRenderTargets[1].UnBind();
			current.Viewport->Data.Shadow.ExponentialBlurRenderTargets[1].BindResource(TextureSlot_ESMGauss);
		}

		{
			// NOTE: This is the alternative to rendering to a depth and color buffer, is this more performant though, I'm not sure
			shaders.DepthThreshold.Bind();

			current.Viewport->Data.Shadow.ThresholdRenderTarget.ResizeIfDifferent(halfResolution);
			current.Viewport->Data.Shadow.ThresholdRenderTarget.BindSetViewport();
			current.Viewport->Data.Shadow.RenderTarget.BindResource(0);
			D3D.Context->Draw(RectangleVertexCount, 0);
			current.Viewport->Data.Shadow.ThresholdRenderTarget.UnBind();

			for (auto& renderTarget : current.Viewport->Data.Shadow.BlurRenderTargets)
				renderTarget.ResizeIfDifferent(blurResolution);

			const int blurTargets = static_cast<int>(current.Viewport->Data.Shadow.BlurRenderTargets.size());
			const int blurPasses = current.Viewport->Parameters.ShadowBlurPasses + 1;

			D3D.SetViewport(blurResolution);
			shaders.ImgFilter.Bind();

			for (int passIndex = 0; passIndex < blurPasses; passIndex++)
			{
				const int blurIndex = (passIndex % blurTargets);
				const int previousBlurIndex = ((passIndex - 1) + blurTargets) % blurTargets;

				auto& sourceTarget = (passIndex == 0) ? current.Viewport->Data.Shadow.ThresholdRenderTarget : current.Viewport->Data.Shadow.BlurRenderTargets[previousBlurIndex];
				auto& destinationTarget = current.Viewport->Data.Shadow.BlurRenderTargets[blurIndex];

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
		current.Viewport->Data.Reflection.RenderTarget.ResizeIfDifferent(current.Viewport->Parameters.ReflectionRenderResolution);
		current.Viewport->Data.Reflection.RenderTarget.BindSetViewport();

		if (current.Viewport->Parameters.ClearReflection)
			current.Viewport->Data.Reflection.RenderTarget.Clear(current.Viewport->Parameters.ClearColor);
		else
			current.Viewport->Data.Reflection.RenderTarget.GetDepthBuffer()->Clear();

		if (!reflectionCommandList.OpaqueAndTransparent.empty())
		{
			// TODO: Render using cheaper reflection shaders
			if (current.Viewport->Parameters.RenderOpaque)
			{
				D3D.Context->OMSetBlendState(nullptr, nullptr, D3D11_DEFAULT_SAMPLE_MASK);

				for (auto& command : reflectionCommandList.OpaqueAndTransparent)
					InternalRenderOpaqueObjCommand(command);
			}

			if (current.Viewport->Parameters.RenderTransparent && !reflectionCommandList.Transparent.empty())
			{
				transparencyPassDepthStencilState.Bind();

				for (auto& command : reflectionCommandList.Transparent)
					InternalRenderTransparentSubMeshCommand(command);

				transparencyPassDepthStencilState.UnBind();
			}
		}

		current.Viewport->Data.Reflection.RenderTarget.UnBind();
	}

	void D3D_Renderer3D::InternalPreRenderSubsurfaceScattering()
	{
		current.Viewport->Data.SubsurfaceScattering.RenderTarget.ResizeIfDifferent(current.Viewport->Parameters.RenderResolution);

		current.Viewport->Data.SubsurfaceScattering.RenderTarget.BindSetViewport();
		current.Viewport->Data.SubsurfaceScattering.RenderTarget.Clear(vec4(0.0f));

		if (!defaultCommandList.OpaqueAndTransparent.empty())
		{
			D3D.Context->OMSetBlendState(nullptr, nullptr, D3D11_DEFAULT_SAMPLE_MASK);

			for (auto& command : defaultCommandList.OpaqueAndTransparent)
				InternalRenderOpaqueObjCommand(command, RenderFlags_SSSPass);

			// TODO: defaultCommandList.Transparent (?)
		}

		current.Viewport->Data.SubsurfaceScattering.RenderTarget.UnBind();
	}

	void D3D_Renderer3D::InternalPreRenderReduceFilterSubsurfaceScattering()
	{
		solidNoCullingRasterizerState.Bind();

		D3D.Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		D3D.Context->OMSetBlendState(nullptr, nullptr, D3D11_DEFAULT_SAMPLE_MASK);

		D3D_TextureSampler::BindArray<1>(0, { nullptr });

		shaders.SSSFilterCopy.Bind();
		current.Viewport->Data.SubsurfaceScattering.RenderTarget.BindResource(0);
		current.Viewport->Data.SubsurfaceScattering.FilterRenderTargets[0].BindSetViewport();
		D3D.Context->Draw(RectangleVertexCount, 0);
		current.Viewport->Data.SubsurfaceScattering.FilterRenderTargets[0].UnBind();

		shaders.SSSFilterMin.Bind();
		current.Viewport->Data.SubsurfaceScattering.FilterRenderTargets[0].BindResource(0);
		current.Viewport->Data.SubsurfaceScattering.FilterRenderTargets[1].BindSetViewport();
		D3D.Context->Draw(RectangleVertexCount, 0);
		current.Viewport->Data.SubsurfaceScattering.FilterRenderTargets[1].UnBind();

		sssFilterCB.Data.TextureSize = GetPackedTextureSize(current.Viewport->Data.SubsurfaceScattering.FilterRenderTargets[1]);
		CalculateSSSCoefficients(current.Viewport->Camera, sssFilterCB.Data);
		sssFilterCB.BindPixelShader();
		sssFilterCB.UploadData();
		shaders.SSSFilterGauss2D.Bind();
		current.Viewport->Data.SubsurfaceScattering.FilterRenderTargets[1].BindResource(0);
		current.Viewport->Data.SubsurfaceScattering.FilterRenderTargets[2].BindSetViewport();
		D3D.Context->Draw(RectangleVertexCount, 0);
		current.Viewport->Data.SubsurfaceScattering.FilterRenderTargets[2].UnBind();
	}

	void D3D_Renderer3D::InternalRenderOpaqueObjCommand(ObjRenderCommand& command, RenderFlags flags)
	{
		if (command.AreAllMeshesTransparent)
			return;

		if (!(flags & RenderFlags_NoFrustumCulling) && !IntersectsCameraFrustum(command.SourceCommand.SourceObj->BoundingSphere, command))
			return;

		IterateCommandMeshes(command.SourceCommand, [&](auto& mesh)
		{
			if (!(flags & RenderFlags_NoFrustumCulling) && !IntersectsCameraFrustum(mesh.BoundingSphere, command))
				return;

			BindMeshVertexBuffers(mesh, GetMorphMesh(*command.SourceCommand.SourceObj, command.SourceCommand.SourceMorphObj, mesh));

			IterateCommandSubMeshes(command.SourceCommand, mesh, [&](auto& subMesh, auto& material)
			{
				if (IsMeshTransparent(mesh, subMesh, material))
					return;
				if ((flags & RenderFlags_SSSPass) && !(UsesSSSSkin(material) || UsesSSSSkinConst(material)))
					return;
				if (!(flags & RenderFlags_NoFrustumCulling) && !IntersectsCameraFrustum(subMesh.BoundingSphere, command))
					return;

				PrepareAndRenderSubMesh(command, mesh, subMesh, material, flags);
			});
		});
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

		BindMeshVertexBuffers(mesh, GetMorphMesh(obj, objCommand->SourceCommand.SourceMorphObj, mesh));

		auto& material = GetSubMeshMaterial(subMesh, command.ObjCommand->SourceCommand);
		cachedBlendStates.GetState(material.BlendFlags.SrcBlendFactor, material.BlendFlags.DstBlendFactor).Bind();

		PrepareAndRenderSubMesh(*command.ObjCommand, mesh, subMesh, material);
	}

	void D3D_Renderer3D::InternalRenderSilhouette()
	{
		current.Viewport->Data.Silhouette.RenderTarget.ResizeIfDifferent(current.Viewport->Parameters.RenderResolution);
		current.Viewport->Data.Silhouette.RenderTarget.BindSetViewport();
		current.Viewport->Data.Silhouette.RenderTarget.Clear(vec4(0.0f));

		if (!defaultCommandList.OpaqueAndTransparent.empty())
		{
			D3D.Context->OMSetBlendState(nullptr, nullptr, D3D11_DEFAULT_SAMPLE_MASK);

			for (auto& command : defaultCommandList.OpaqueAndTransparent)
				InternalRenderOpaqueObjCommand(command, RenderFlags_SilhouetteOutlinePass | RenderFlags_NoMaterialShader | RenderFlags_NoMaterialTextures);
		}

		current.Viewport->Data.Silhouette.RenderTarget.UnBind();
	}

	void D3D_Renderer3D::InternalRenderSilhouetteOutlineOverlay()
	{
		current.Viewport->Data.Silhouette.RenderTarget.BindResource(0);

		shaders.SilhouetteOutline.Bind();
		D3D.Context->Draw(RectangleVertexCount, 0);
	}

	void D3D_Renderer3D::InternalQueryRenderLensFlare()
	{
		const Obj* sunObj = current.Scene->LensFlare.SunObj;
		if (sunObj == nullptr)
			return;

		const vec3 cameraViewPoint = current.Viewport->Camera.ViewPoint;
		const vec3 sunPosition = current.Scene->LensFlare.SunPosition.value();

		constexpr float sunScaleDistanceFactor = 0.56f;
		const float sunDistance = glm::distance(sunPosition, cameraViewPoint);
		const float sunScale = sunDistance * sunScaleDistanceFactor;

		const quat sunLookAt = glm::quatLookAt(glm::normalize(cameraViewPoint - sunPosition), OrthographicCamera::UpDirection);
		const mat4 sunTransform = glm::translate(mat4(1.0f), sunPosition) * glm::mat4_cast(sunLookAt) * glm::scale(mat4(1.0f), vec3(sunScale));

		objectCB.Data.ModelViewProjection = glm::transpose(current.Viewport->Camera.GetViewProjection() * sunTransform);
		objectCB.UploadData();

		if (!current.Viewport->Parameters.DebugVisualizeOcclusionQuery)
			lensFlareSunQueryBlendState.Bind();

		solidNoCullingRasterizerState.Bind();
		shaders.Sun.Bind();

		if (current.Viewport->Parameters.LastFrameOcclusionQueryOptimization && !sunOcclusionQuery.IsFirstQuery())
			sunOcclusionQuery.QueryData();

		sunOcclusionQuery.BeginQuery();
		{
			for (auto& mesh : sunObj->Meshes)
			{
				BindMeshVertexBuffers(mesh, nullptr);
				for (auto& subMesh : mesh.SubMeshes)
					SubmitSubMeshDrawCall(subMesh);
			}
		}
		sunOcclusionQuery.EndQuery();

		if (!current.Viewport->Parameters.LastFrameOcclusionQueryOptimization)
			sunOcclusionQuery.QueryData();

		const vec2 normalizedSunScreenPosition = current.Viewport->Camera.ProjectPointNormalizedScreen(sunPosition);

#if COMFY_DEBUG
		const vec2 sunScreenPos = Gui::GetWindowPos() + (normalizedSunScreenPosition * Gui::GetWindowSize());
		const vec2 screenCenter = Gui::GetWindowPos() + (vec2(0.5f, 0.5f) * Gui::GetWindowSize());

		char buffer[64]; sprintf_s(buffer, "{ x=%.2f, y=%.2f } ", sunScreenPos.x, sunScreenPos.y);
		Gui::GetForegroundDrawList()->AddText(sunScreenPos, IM_COL32_BLACK, buffer);
		Gui::GetForegroundDrawList()->AddLine(sunScreenPos, screenCenter, IM_COL32_BLACK);

		Gui::DEBUG_NOSAVE_WINDOW(__FUNCTION__"(): Test", [&]
		{
			Gui::Text("sunOcclusionQuery.GetCoveredPixels(): %d", static_cast<int>(sunOcclusionQuery.GetCoveredPixels()));
		});
#endif
	}

	void D3D_Renderer3D::InternalRenderLensFlare()
	{
		shaders.LensFlare.Bind();
	}

	void D3D_Renderer3D::InternalRenderPostProcessing()
	{
		D3D.Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		constexpr uint32_t morphAttrbutesFactor = 2;
		constexpr uint32_t attributesToReset = (VertexAttribute_Count * morphAttrbutesFactor);

		std::array<ID3D11Buffer*, attributesToReset> buffers = {};
		std::array<UINT, attributesToReset> strides = {}, offsets = {};
		D3D.Context->IASetVertexBuffers(0, attributesToReset, buffers.data(), strides.data(), offsets.data());

		solidNoCullingRasterizerState.Bind();
		D3D.Context->OMSetBlendState(nullptr, nullptr, D3D11_DEFAULT_SAMPLE_MASK);

		D3D_TextureSampler::BindArray<1>(0, { nullptr });

		if (current.Viewport->Parameters.RenderBloom)
			InternalRenderBloom();

		current.Viewport->Data.Output.RenderTarget.ResizeIfDifferent(current.Viewport->Parameters.RenderResolution);
		current.Viewport->Data.Output.RenderTarget.BindSetViewport();

		if (toneMapData.NeedsUpdating(current.Scene->Glow))
			toneMapData.Update(current.Scene->Glow);

		const bool autoExposureEnabled = (current.Viewport->Parameters.AutoExposure && current.Viewport->Parameters.RenderBloom && current.Scene->Glow.AutoExposure);

		D3D_ShaderResourceView::BindArray<4>(0,
			{
				&current.Viewport->Data.Main.CurrentOrResolved(),
				(current.Viewport->Parameters.RenderBloom) ? &current.Viewport->Data.Bloom.CombinedBlurRenderTarget : nullptr,
				toneMapData.GetLookupTexture(),
				autoExposureEnabled ? &current.Viewport->Data.Bloom.ExposureRenderTargets.back() : nullptr
			});

		toneMapCB.Data.Exposure = current.Scene->Glow.Exposure;
		toneMapCB.Data.Gamma = current.Scene->Glow.Gamma;
		toneMapCB.Data.SaturatePower = static_cast<float>(current.Scene->Glow.SaturatePower);
		toneMapCB.Data.SaturateCoefficient = current.Scene->Glow.SaturateCoefficient;
		toneMapCB.Data.AlphaLerp = current.Viewport->Parameters.ToneMapPreserveAlpha ? 0.0f : 1.0f;
		toneMapCB.Data.AlphaValue = 1.0f;
		toneMapCB.Data.AutoExposure = autoExposureEnabled;
		toneMapCB.UploadData();
		toneMapCB.BindPixelShader();

		shaders.ToneMap.Bind();

		D3D.Context->Draw(RectangleVertexCount, 0);

		D3D_ShaderResourceView::BindArray<3>(0, { nullptr, nullptr, nullptr });
		current.Viewport->Data.Main.AdvanceRenderTarget();
	}

	void D3D_Renderer3D::InternalRenderBloom()
	{
		auto& bloom = current.Viewport->Data.Bloom;

		bloom.BaseRenderTarget.ResizeIfDifferent(current.Viewport->Parameters.RenderResolution / 2);

		reduceTexCB.Data.CombineBlurred = false;
		reduceTexCB.BindPixelShader();
		shaders.ReduceTex.Bind();

		for (int i = -1; i < static_cast<int>(bloom.ReduceRenderTargets.size()); i++)
		{
			auto& renderTarget = (i < 0) ? bloom.BaseRenderTarget : bloom.ReduceRenderTargets[i];
			auto& lastRenderTarget = (i < 0) ? current.Viewport->Data.Main.CurrentOrResolved() : (i == 0) ? bloom.BaseRenderTarget : bloom.ReduceRenderTargets[i - 1];

			reduceTexCB.Data.TextureSize = GetPackedTextureSize(lastRenderTarget);
			reduceTexCB.Data.ExtractBrightness = (i == 0);
			reduceTexCB.UploadData();

			renderTarget.BindSetViewport();
			lastRenderTarget.BindResource(0);

			D3D.Context->Draw(RectangleVertexCount, 0);
		}

		if (current.Viewport->Parameters.AutoExposure && current.Scene->Glow.AutoExposure)
			InternalRenderExposurePreBloom();

		CalculateGaussianBlurKernel(current.Scene->Glow, ppGaussCoefCB.Data);
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

		if (current.Viewport->Parameters.AutoExposure && current.Scene->Glow.AutoExposure)
			InternalRenderExposurePostBloom();
	}

	void D3D_Renderer3D::InternalRenderExposurePreBloom()
	{
		shaders.ExposureMinify.Bind();
		current.Viewport->Data.Bloom.ExposureRenderTargets[0].BindSetViewport();
		current.Viewport->Data.Bloom.ReduceRenderTargets.back().BindResource(0);
		D3D.Context->Draw(RectangleVertexCount, 0);
		current.Viewport->Data.Bloom.ExposureRenderTargets[0].UnBind();
	}

	void D3D_Renderer3D::InternalRenderExposurePostBloom()
	{
		CalculateExposureSpotCoefficients(exposureCB.Data);
		exposureCB.UploadData();
		exposureCB.BindPixelShader();

		shaders.ExposureMeasure.Bind();
		current.Viewport->Data.Bloom.ExposureRenderTargets[1].BindSetViewport();
		current.Viewport->Data.Bloom.ExposureRenderTargets[0].BindResource(0);
		D3D.Context->Draw(RectangleVertexCount, 0);
		current.Viewport->Data.Bloom.ExposureRenderTargets[1].UnBind();

		shaders.ExposureAverage.Bind();
		current.Viewport->Data.Bloom.ExposureRenderTargets[2].BindSetViewport();
		current.Viewport->Data.Bloom.ExposureRenderTargets[1].BindResource(0);
		D3D.Context->Draw(RectangleVertexCount, 0);
		current.Viewport->Data.Bloom.ExposureRenderTargets[2].UnBind();
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
				D3D_StaticVertexBuffer* vertexBuffer = mesh.GPU_VertexBuffers[i].get();

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
		if (flags & RenderFlags_SilhouetteOutlinePass)
		{
			if (command.SourceCommand.Flags.SilhouetteOutline)
				shaders.SolidWhite.Bind();
			else
				shaders.SolidBlack.Bind();
		}
		else if (!(flags & RenderFlags_NoMaterialShader))
		{
			((flags & RenderFlags_SSSPass) ? GetSSSMaterialShader(material) : GetMaterialShader(command, mesh, subMesh, material)).Bind();
		}

		const uint32_t boundMaterialTexturesFlags = (flags & RenderFlags_NoMaterialTextures) ? 0 : BindMaterialTextures(command, material, flags);

		if (!current.Viewport->Parameters.Wireframe && !(flags & RenderFlags_NoRasterizerState))
			SetSubMeshRasterizerState(material);

		SetObjectCBMaterialData(material, objectCB.Data.Material);
		SetObjectCBTransforms(command, mesh, subMesh, objectCB.Data);

		objectCB.Data.MorphWeight = GetObjectCBMorphWeight(command);
		objectCB.Data.ShaderFlags = GetObjectCBShaderFlags(command, mesh, subMesh, material, boundMaterialTexturesFlags);

		objectCB.UploadData();

		SubmitSubMeshDrawCall(subMesh);
	}

	uint32_t D3D_Renderer3D::MaterialTextureTypeToTextureSlot(MaterialTextureType textureType, bool secondColorMap)
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
			assert(false);
			return TextureSlot_MaterialTextureCount;
		}
	}

	uint32_t D3D_Renderer3D::BindMaterialTextures(const ObjRenderCommand& command, const Material& material, RenderFlags flags)
	{
		auto applyTextureTransform = [](mat4& outTransform, const ObjAnimationData::TextureTransform& textureTransform)
		{
			constexpr vec3 centerOffset = vec3(0.5f, 0.5f, 0.0f);
			constexpr vec3 rotationAxis = vec3(0.0f, 0.0f, 1.0f);

			outTransform =
				glm::translate(mat4(1.0f), vec3(textureTransform.Translation, 0.0f))
				* glm::translate(mat4(1.0f), +centerOffset)
				* glm::rotate(mat4(1.0f), glm::radians(textureTransform.Rotation), rotationAxis)
				* glm::translate(outTransform, -centerOffset);
		};

		const ObjAnimationData* animation = command.SourceCommand.Animation;

		objectCB.Data.DiffuseRGTC1 = false;
		objectCB.Data.DiffuseScreenTexture = false;
		objectCB.Data.AmbientTextureType = 0;

		std::array<D3D_ShaderResourceView*, TextureSlot_MaterialTextureCount> textureResources = {};
		std::array<D3D_TextureSampler*, TextureSlot_MaterialTextureCount> textureSamplers = {};

		for (auto& materialTexture : material.Textures)
		{
			const bool secondColorMap = textureResources[TextureSlot_Diffuse] != nullptr;
			const auto correspondingTextureSlot = MaterialTextureTypeToTextureSlot(materialTexture.TextureFlags.Type, secondColorMap);

			if (correspondingTextureSlot >= TextureSlot_MaterialTextureCount || textureResources[correspondingTextureSlot] != nullptr)
				continue;

			if (!GetIsTextureSlotUsed(material.ShaderType, material.UsedTexturesFlags, correspondingTextureSlot))
				continue;

			const Cached_TxpID* txpID = &materialTexture.TextureID;
			auto samplerFlags = materialTexture.SamplerFlags;

			if (animation != nullptr)
			{
				for (auto& transform : animation->TextureTransforms)
				{
					if (*txpID == transform.ID)
					{
						samplerFlags.RepeatU = transform.RepeatU.value_or<int>(samplerFlags.RepeatU);
						samplerFlags.RepeatV = transform.RepeatU.value_or<int>(samplerFlags.RepeatV);
					}
				}

				for (auto& pattern : animation->TexturePatterns)
				{
					if (*txpID == pattern.ID && pattern.IDOverride != TxpID::Invalid)
						txpID = &pattern.IDOverride;
				}
			}

			auto txp = GetTxpFromTextureID(txpID);
			if (txp == nullptr)
				continue;

			textureResources[correspondingTextureSlot] = (txp->GPU_Texture2D != nullptr) ? static_cast<D3D_TextureResource*>(txp->GPU_Texture2D.get()) : (txp->GPU_CubeMap != nullptr) ? (txp->GPU_CubeMap.get()) : nullptr;
			textureSamplers[correspondingTextureSlot] = &cachedTextureSamplers.GetSampler(samplerFlags);

			if (correspondingTextureSlot == TextureSlot_Diffuse)
			{
				objectCB.Data.Material.DiffuseTextureTransform = materialTexture.TextureCoordinateMatrix;

				if (animation != nullptr)
				{
					for (auto& textureTransform : animation->TextureTransforms)
						if (textureTransform.ID == *txpID)
							applyTextureTransform(objectCB.Data.Material.DiffuseTextureTransform, textureTransform);

					if (*txpID == animation->ScreenRenderTextureID)
					{
						objectCB.Data.DiffuseScreenTexture = true;
						textureResources[correspondingTextureSlot] = &current.Viewport->Data.Main.PreviousOrResolved();

						// HACK: Flip to adjust for the expected OpenGL texture coordinates, problematic because it also effects all other textures using the first TEXCOORD attribute
						objectCB.Data.Material.DiffuseTextureTransform *= glm::scale(mat4(1.0f), vec3(1.0f, -1.0f, 1.0f));
					}
				}

				if (txp->GetFormat() == TextureFormat::RGTC1)
					objectCB.Data.DiffuseRGTC1 = true;
			}
			else if (correspondingTextureSlot == TextureSlot_Ambient)
			{
				objectCB.Data.Material.AmbientTextureTransform = materialTexture.TextureCoordinateMatrix;

				if (animation != nullptr)
				{
					for (auto& textureTransform : animation->TextureTransforms)
						if (textureTransform.ID == *txpID)
							applyTextureTransform(objectCB.Data.Material.AmbientTextureTransform, textureTransform);
				}

				const auto blendFlags = materialTexture.SamplerFlags.Blend;
				objectCB.Data.AmbientTextureType = (blendFlags == 0b100) ? 2 : (blendFlags == 0b110) ? 1 : (blendFlags != 0b10000) ? 0 : 3;
			}
		}

		if (flags & RenderFlags_DiffuseTextureOnly)
		{
			D3D_ShaderResourceView::BindArray<1>(TextureSlot_Diffuse, { textureResources[TextureSlot_Diffuse] });
			D3D_TextureSampler::BindArray<1>(TextureSlot_Diffuse, { textureSamplers[TextureSlot_Diffuse] });
		}
		else
		{
			D3D_ShaderResourceView::BindArray(TextureSlot_Diffuse, textureResources);
			D3D_TextureSampler::BindArray(TextureSlot_Diffuse, textureSamplers);
		}

		uint32_t boundMaterialTexturesFlags = 0;
		for (size_t textureSlot = 0; textureSlot < textureResources.size(); textureSlot++)
		{
			if (textureResources[textureSlot] != nullptr)
				boundMaterialTexturesFlags |= (1 << textureSlot);
		}
		return boundMaterialTexturesFlags;
	}

	bool D3D_Renderer3D::GetIsTextureSlotUsed(Material::ShaderTypeIdentifier shaderType, Material::MaterialUsedTextureFlags usedTextureFlags, uint32_t textureSlot)
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

	void D3D_Renderer3D::SetSubMeshRasterizerState(const Material& material)
	{
		if (material.BlendFlags.DoubleSided)
			solidNoCullingRasterizerState.Bind();
		else
			solidBackfaceCullingRasterizerState.Bind();
	}

	void D3D_Renderer3D::SetObjectCBMaterialData(const Material& material, ObjectConstantData::MaterialData& outMaterialData) const
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

	void D3D_Renderer3D::SetObjectCBTransforms(const ObjRenderCommand& command, const Mesh& mesh, const SubMesh& subMesh, ObjectConstantData& outData) const
	{
		mat4 modelMatrix;
		if (current.Viewport->Parameters.ObjectBillboarding && (mesh.Flags.FaceCameraPosition || mesh.Flags.FaceCameraView))
		{
			const auto& transform = command.SourceCommand.Transform;

			// TODO: if (mesh.Flags.FaceCameraView)
			const vec3 viewPoint = current.Viewport->Camera.ViewPoint;
			const float cameraAngle = glm::atan(transform.Translation.x - viewPoint.x, transform.Translation.z - viewPoint.z);

			modelMatrix = glm::rotate(command.ModelMatrix, cameraAngle - glm::pi<float>() - glm::radians(transform.Rotation.y), vec3(0.0f, 1.0f, 0.0f));
		}
		else
		{
			modelMatrix = command.ModelMatrix;
		}

		outData.Model = glm::transpose(modelMatrix);

#if 0 // TODO:
		outData.ModelView = glm::transpose(current.Viewport->Camera.GetView() * modelMatrix);
		outData.ModelViewProjection = glm::transpose(current.Viewport->Camera.GetViewProjection() * modelMatrix);
#else
		outData.ModelView = glm::transpose(glm::transpose(sceneCB.Data.Scene.View) * modelMatrix);
		outData.ModelViewProjection = glm::transpose(glm::transpose(sceneCB.Data.Scene.ViewProjection) * modelMatrix);
#endif
	}

	vec4 D3D_Renderer3D::GetObjectCBMorphWeight(const ObjRenderCommand& command) const
	{
		if (command.SourceCommand.Animation != nullptr)
		{
			const float morphWeight = command.SourceCommand.Animation->MorphWeight;
			return vec4(morphWeight, 1.0f - morphWeight, 0.0f, 0.0f);
		}
		else
		{
			return vec4(0.0f, 0.0f, 0.0f, 0.0f);
		}
	}

	uint32_t D3D_Renderer3D::GetObjectCBShaderFlags(const ObjRenderCommand& command, const Mesh& mesh, const SubMesh& subMesh, const Material& material, uint32_t boundMaterialTexturesFlags) const
	{
		uint32_t result = 0;

		const bool hasVertexTangents = (mesh.AttributeFlags & VertexAttributeFlags_Tangent);
		const bool hasVertexTexCoords0 = (mesh.AttributeFlags & VertexAttributeFlags_TextureCoordinate0);
		const bool hasVertexTexCoords1 = (mesh.AttributeFlags & VertexAttributeFlags_TextureCoordinate1);

		if (current.Viewport->Parameters.VertexColoring)
		{
			if (mesh.AttributeFlags & VertexAttributeFlags_Color0)
				result |= ShaderFlags_VertexColor;
		}

		if (current.Viewport->Parameters.DiffuseMapping)
		{
			if (hasVertexTexCoords0 && (boundMaterialTexturesFlags & (1 << TextureSlot_Diffuse)))
				result |= ShaderFlags_DiffuseTexture;
		}

		if (current.Viewport->Parameters.AmbientOcclusionMapping)
		{
			if (hasVertexTexCoords1 && (boundMaterialTexturesFlags & (1 << TextureSlot_Ambient)))
				result |= ShaderFlags_AmbientTexture;
		}

		if (current.Viewport->Parameters.NormalMapping)
		{
			if (hasVertexTangents && (boundMaterialTexturesFlags & (1 << TextureSlot_Normal)))
				result |= ShaderFlags_NormalTexture;
		}

		if (current.Viewport->Parameters.SpecularMapping)
		{
			if (hasVertexTexCoords0 && (boundMaterialTexturesFlags & (1 << TextureSlot_Specular)))
				result |= ShaderFlags_SpecularTexture;
		}

		if (current.Viewport->Parameters.TransparencyMapping)
		{
			if (hasVertexTexCoords0 && (boundMaterialTexturesFlags & (1 << TextureSlot_Transparency)))
				result |= ShaderFlags_TransparencyTexture;
		}

		if (current.Viewport->Parameters.EnvironmentMapping)
		{
			if (boundMaterialTexturesFlags & (1 << TextureSlot_Environment))
				result |= ShaderFlags_EnvironmentTexture;
		}

		if (current.Viewport->Parameters.TranslucencyMapping)
		{
			if (hasVertexTexCoords0 && (boundMaterialTexturesFlags & (1 << TextureSlot_Translucency)))
				result |= ShaderFlags_TranslucencyTexture;
		}

		if (current.Viewport->Parameters.RenderPunchThrough)
		{
			if (material.BlendFlags.AlphaTexture && !IsMeshTransparent(mesh, subMesh, material))
				result |= ShaderFlags_PunchThrough;
		}

		if (current.Viewport->Parameters.RenderFog)
		{
			if (!material.BlendFlags.NoFog && current.Scene->Fog.Depth.Density > 0.0f)
				result |= ShaderFlags_LinearFog;
		}

		if (current.Viewport->Parameters.ObjectMorphing)
		{
			if (command.SourceCommand.SourceMorphObj != nullptr)
			{
				result |= ShaderFlags_Morph;
				result |= ShaderFlags_MorphColor;
			}
		}

		if (current.Viewport->Parameters.ShadowMapping && isAnyCommand.CastShadow && isAnyCommand.ReceiveShadow)
		{
			if (ReceivesShadows(command.SourceCommand, mesh, subMesh))
				result |= ShaderFlags_Shadow;
		}

		if (current.Viewport->Parameters.ShadowMapping && current.Viewport->Parameters.SelfShadowing && isAnyCommand.CastShadow && isAnyCommand.ReceiveShadow)
		{
			if (ReceivesSelfShadow(command.SourceCommand, mesh, subMesh))
				result |= ShaderFlags_SelfShadow;
		}

		return result;
	}

	D3D_ShaderPair& D3D_Renderer3D::GetMaterialShader(const ObjRenderCommand& command, const Mesh& mesh, const SubMesh& subMesh, const Material& material)
	{
		if (current.Viewport->Parameters.AllowDebugShaderOverride)
		{
			if (command.SourceCommand.SourceObj->Debug.UseDebugMaterial || mesh.Debug.UseDebugMaterial || subMesh.Debug.UseDebugMaterial || material.Debug.UseDebugMaterial)
				return shaders.DebugMaterial;
		}

		if (material.ShaderType == Material::ShaderIdentifiers::Blinn)
		{
			if (material.ShaderFlags.PhongShading)
			{
				return (material.UsedTexturesFlags.Normal) ? shaders.BlinnPerFrag : shaders.BlinnPerVert;
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
		else if (material.ShaderType == Material::ShaderIdentifiers::Item)
		{
			return shaders.ItemBlinn;
		}
		else if (material.ShaderType == Material::ShaderIdentifiers::Stage)
		{
			return shaders.StageBlinn;
		}
		else if (material.ShaderType == Material::ShaderIdentifiers::Skin)
		{
			return shaders.SkinDefault;
		}
		else if (material.ShaderType == Material::ShaderIdentifiers::Hair)
		{
			return (material.ShaderFlags.AnisoDirection != AnisoDirection::Normal) ? shaders.HairAniso : shaders.HairDefault;
		}
		else if (material.ShaderType == Material::ShaderIdentifiers::Cloth)
		{
			return (material.ShaderFlags.AnisoDirection != AnisoDirection::Normal) ? shaders.ClothAniso : shaders.ClothDefault;
		}
		else if (material.ShaderType == Material::ShaderIdentifiers::Tights)
		{
			return shaders.Tights;
		}
		else if (material.ShaderType == Material::ShaderIdentifiers::Sky)
		{
			return shaders.SkyDefault;
		}
		else if (material.ShaderType == Material::ShaderIdentifiers::EyeBall)
		{
			return (true) ? shaders.GlassEye : shaders.EyeBall;
		}
		else if (material.ShaderType == Material::ShaderIdentifiers::EyeLens)
		{
			return shaders.EyeLens;
		}
		else if (material.ShaderType == Material::ShaderIdentifiers::GlassEye)
		{
			return shaders.GlassEye;
		}
		else if (material.ShaderType == Material::ShaderIdentifiers::Water01 || material.ShaderType == Material::ShaderIdentifiers::Water02)
		{
			return shaders.Water;
		}
		else if (material.ShaderType == Material::ShaderIdentifiers::Floor)
		{
			return shaders.Floor;
		}
		else
		{
			return shaders.DebugMaterial;
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
		const size_t indexCount = subMesh.GetIndexCount();
		subMesh.GPU_IndexBuffer->Bind();

		D3D.Context->IASetPrimitiveTopology(GetD3DPrimitiveTopolgy(subMesh.Primitive));
		D3D.Context->DrawIndexed(static_cast<UINT>(indexCount), 0, 0);

		statistics.VerticesRendered += indexCount;
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

		constexpr float radiusPadding = 0.05f;
		const vec3 size = (max - min) / 2.0f;

		return Sphere { (min + size), (std::max(size.x, std::max(size.y, size.z))) + radiusPadding };
	}

	bool D3D_Renderer3D::IntersectsCameraFrustum(const ObjRenderCommand& command) const
	{
		if (!current.Viewport->Parameters.FrustumCulling)
			return true;

		return current.Viewport->Camera.IntersectsViewFrustum(command.TransformedBoundingSphere);
	}

	bool D3D_Renderer3D::IntersectsCameraFrustum(const Sphere& boundingSphere, const ObjRenderCommand& command) const
	{
		if (!current.Viewport->Parameters.FrustumCulling)
			return true;

		return current.Viewport->Camera.IntersectsViewFrustum(boundingSphere * command.SourceCommand.Transform);
	}

	bool D3D_Renderer3D::IntersectsCameraFrustum(const Sphere& boundingSphere, const SubMeshRenderCommand& command) const
	{
		return IntersectsCameraFrustum(boundingSphere, *command.ObjCommand);
	}

	bool D3D_Renderer3D::IsDebugRenderFlagSet(int bitIndex) const
	{
		return current.Viewport->Parameters.DebugFlags & (1 << bitIndex);
	}
}
