#include "MaterialEditor.h"
#include "Graphics/Auth3D/DebugObj.h"
#include "ImGui/Gui.h"
#include "ImGui/Extensions/TxpExtensions.h"
#include "ImGui/Extensions/PropertyEditor.h"

namespace Comfy::Editor
{
	using namespace Graphics;

	namespace
	{
		constexpr std::array AvailableMaterialShaderTypes =
		{
			std::make_pair("Blinn", Material::ShaderIdentifiers::Blinn),
			std::make_pair("Item", Material::ShaderIdentifiers::Item),
			std::make_pair("Stage", Material::ShaderIdentifiers::Stage),
			std::make_pair("Skin", Material::ShaderIdentifiers::Skin),
			std::make_pair("Hair", Material::ShaderIdentifiers::Hair),
			std::make_pair("Cloth", Material::ShaderIdentifiers::Cloth),
			std::make_pair("Tights", Material::ShaderIdentifiers::Tights),
			std::make_pair("Sky", Material::ShaderIdentifiers::Sky),
			std::make_pair("Eye Ball", Material::ShaderIdentifiers::EyeBall),
			std::make_pair("Eye Lens", Material::ShaderIdentifiers::EyeLens),
			std::make_pair("Glass Eye", Material::ShaderIdentifiers::GlassEye),
			std::make_pair("Water", Material::ShaderIdentifiers::Water01),
			std::make_pair("Floor", Material::ShaderIdentifiers::Floor),
		};

		constexpr vec2 TxpDisplaySize = vec2(96.0f);
	}

	void MaterialEditor::DrawGui(D3D_Renderer3D& renderer, const SceneParameters& scene, Material& material)
	{
		GuiPropertyRAII::ID id(&material);
		GuiPropertyRAII::PropertyValueColumns rootColumns;

		GuiProperty::TreeNode("Material", material.Name.data(), ImGuiTreeNodeFlags_DefaultOpen, [&]
		{
			GuiProperty::Checkbox("Use Debug Material", material.Debug.UseDebugMaterial);
			DrawUsedTexturesFlagsGui(material.UsedTexturesCount, material.UsedTexturesFlags);
			DrawShaderFlagsGui(material.ShaderType, material.ShaderFlags);
			DrawTextureDataGui(renderer, material);
			DrawBlendFlagsGui(material.BlendFlags);
			DrawColorGui(material.Color);

			GuiProperty::TreeNode("Preview", ImGuiTreeNodeFlags_DefaultOpen, [&]
			{
				preview.DrawGui(renderer, scene, material);
			});
		});
	}

	void MaterialEditor::DrawUsedTexturesFlagsGui(uint32_t& usedTexturesCount, Material::MaterialUsedTextureFlags& texturesFlags)
	{
		GuiProperty::TreeNode("Used Textures Flags", ImGuiTreeNodeFlags_None, [&]
		{
			GuiProperty::Input("Used Texture Count", usedTexturesCount);
			GuiPropertyBitFieldCheckbox("Color", texturesFlags.Color);
			GuiPropertyBitFieldCheckbox("Color A", texturesFlags.ColorA);
			GuiPropertyBitFieldCheckbox("Color L1", texturesFlags.ColorL1);
			GuiPropertyBitFieldCheckbox("Color L1 A", texturesFlags.ColorL1A);
			GuiPropertyBitFieldCheckbox("Color L2", texturesFlags.ColorL2);
			GuiPropertyBitFieldCheckbox("Color L2 A", texturesFlags.ColorL2A);
			GuiPropertyBitFieldCheckbox("Transparency", texturesFlags.Transparency);
			GuiPropertyBitFieldCheckbox("Specular", texturesFlags.Specular);
			GuiPropertyBitFieldCheckbox("Normal", texturesFlags.Normal);
			GuiPropertyBitFieldCheckbox("Normal Alt", texturesFlags.NormalAlt);
			GuiPropertyBitFieldCheckbox("Environment", texturesFlags.Environment);
			GuiPropertyBitFieldCheckbox("Color L3", texturesFlags.ColorL3);
			GuiPropertyBitFieldCheckbox("Color L3 A", texturesFlags.ColorL3A);
			GuiPropertyBitFieldCheckbox("Translucency", texturesFlags.Translucency);
			GuiPropertyBitFieldCheckbox("Unknown 0", texturesFlags.Unknown0);
			GuiPropertyBitFieldCheckbox("OverrideIBLCubeMap", texturesFlags.OverrideIBLCubeMap);
		});
	}

	void MaterialEditor::DrawShaderFlagsGui(Material::ShaderTypeIdentifier& shaderType, Material::MaterialShaderFlags& shaderFlags)
	{
		GuiProperty::TreeNode("Shader Flags", ImGuiTreeNodeFlags_None, [&]
		{
			if (GuiProperty::Input("Shader Type", shaderType.data(), shaderType.size(), ImGuiInputTextFlags_None))
				std::fill(std::find(shaderType.begin(), shaderType.end(), '\0'), shaderType.end(), '\0');

			Gui::ItemContextMenu("ShaderTypeContextMenu", [&]
			{
				for (const auto&[typeName, newType] : AvailableMaterialShaderTypes)
				{
					if (Gui::MenuItem(typeName))
						shaderType = newType;
				}
			});

			GuiPropertyBitFieldComboEnum("Vertex Translation Type", shaderFlags.VertexTranslationType, VertexTranslationTypeNames);
			GuiPropertyBitFieldComboEnum("Color Source Type", shaderFlags.ColorSourceType, ColorSourceTypeNames);
			GuiPropertyBitFieldCheckbox("Lambert Shading", shaderFlags.LambertShading);
			GuiPropertyBitFieldCheckbox("Phong Shading", shaderFlags.PhongShading);
			GuiPropertyBitFieldCheckbox("Per Pixel Shading", shaderFlags.PerPixelShading);
			GuiPropertyBitFieldCheckbox("Double Shading", shaderFlags.DoubleShading);
			GuiPropertyBitFieldComboEnum("Bump Map Type", shaderFlags.BumpMapType, BumpMapTypeNames);
			GuiPropertyBitFieldInputInt("Fresnel", shaderFlags.Fresnel);
			GuiPropertyBitFieldInputInt("Line Light", shaderFlags.LineLight);
			GuiPropertyBitFieldCheckbox("Receives Shadows", shaderFlags.ReceivesShadows);
			GuiPropertyBitFieldCheckbox("Casts Shadows", shaderFlags.CastsShadows);
			GuiPropertyBitFieldComboEnum("Specular Quality", shaderFlags.SpecularQuality, SpecularQualityNames);
			GuiPropertyBitFieldComboEnum("Aniso Direction", shaderFlags.AnisoDirection, AnisoDirectionNames);
		});
	}

	void MaterialEditor::DrawTextureDataGui(D3D_Renderer3D& renderer, Material& material)
	{
		GuiPropertyRAII::ID id(&material.Textures);

		GuiProperty::TreeNode("Texture Data", ImGuiTreeNodeFlags_DefaultOpen, [&]
		{
			for (size_t i = 0; i < material.Textures.size(); i++)
			{
				auto& texture = material.Textures[i];

				char nodePropertyNameBuffer[32];
				sprintf_s(nodePropertyNameBuffer, "Textures[%zu]", i);

				char nodeValueNameBuffer[32];
				sprintf_s(nodeValueNameBuffer, "(%s)", MaterialTextureTypeNames[static_cast<uint32_t>(texture.TextureFlags.Type)]);

				GuiProperty::TreeNode(nodePropertyNameBuffer, nodeValueNameBuffer, ImGuiTreeNodeFlags_None, [&]
				{
					GuiProperty::TreeNode("Sampler Flags", ImGuiTreeNodeFlags_None, [&]
					{
						auto& samplerFlags = texture.SamplerFlags;
						GuiPropertyBitFieldCheckbox("Repeat U", samplerFlags.RepeatU);
						GuiPropertyBitFieldCheckbox("Repeat V", samplerFlags.RepeatV);
						GuiPropertyBitFieldCheckbox("Mirror U", samplerFlags.MirrorU);
						GuiPropertyBitFieldCheckbox("Mirror V", samplerFlags.MirrorV);
						GuiPropertyBitFieldCheckbox("Ignore Alpha", samplerFlags.IgnoreAlpha);
						GuiPropertyBitFieldInputInt("Blend", samplerFlags.Blend);
						GuiPropertyBitFieldInputInt("Alpha Blend", samplerFlags.AlphaBlend);
						GuiPropertyBitFieldCheckbox("Border", samplerFlags.Border);
						GuiPropertyBitFieldCheckbox("Clamp To Edge", samplerFlags.ClampToEdge);
						GuiPropertyBitFieldInputInt("Filter", samplerFlags.Filter);
						GuiPropertyBitFieldInputInt("Mip Map", samplerFlags.MipMap);
						GuiPropertyBitFieldInputInt("Mip Map Bias", samplerFlags.MipMapBias);
						GuiPropertyBitFieldInputInt("Ansi Filters", samplerFlags.AnsiFilters);
					});

					auto tempID = static_cast<uint32_t>(texture.TextureID.ID);
					if (GuiProperty::InputHex("Texture ID", tempID))
						texture.TextureID = static_cast<TxpID>(tempID);

					auto txp = renderer.GetTxpFromTextureID(&texture.TextureID);
					GuiProperty::PropertyLabelValueFunc((txp == nullptr) ? "(No Texture)" : txp->GetName(), [&]
					{
						if (txp != nullptr)
						{
							const float width = std::clamp(Gui::GetContentRegionAvailWidth(), 1.0f, TxpDisplaySize.x);
							const float aspectRatio = static_cast<float>(txp->GetSize().x) / static_cast<float>(txp->GetSize().y);
							Gui::ImageObjTxp(txp, vec2(width, width * aspectRatio));
						}
						return false;
					});

					GuiProperty::TreeNode("Texture Flags", ImGuiTreeNodeFlags_None, [&]
					{
						auto& textureFlags = texture.TextureFlags;
						GuiPropertyBitFieldComboEnum("Type", textureFlags.Type, MaterialTextureTypeNames);
						GuiPropertyBitFieldComboEnum("UV Index", textureFlags.UVIndex, MaterialTextureUVIndexNames);
						GuiPropertyBitFieldComboEnum("UV Translation Type", textureFlags.UVTranslationType, MaterialTextureUVTranslationTypeNames);
					});
				});
			}
		});
	}

	void MaterialEditor::DrawBlendFlagsGui(Material::MaterialBlendFlags& blendFlags)
	{
		GuiProperty::TreeNode("Blend Flags", ImGuiTreeNodeFlags_None, [&]
		{
			GuiPropertyBitFieldCheckbox("Alpha Texture", blendFlags.AlphaTexture);
			GuiPropertyBitFieldCheckbox("Alpha Material", blendFlags.AlphaMaterial);
			GuiPropertyBitFieldCheckbox("Punch Through", blendFlags.PunchThrough);
			GuiPropertyBitFieldCheckbox("Double Sided", blendFlags.DoubleSided);
			GuiPropertyBitFieldCheckbox("Normal Direction Light", blendFlags.NormalDirectionLight);
			GuiPropertyBitFieldComboEnum("Source Blend Factor", blendFlags.SrcBlendFactor, BlendFactorNames);
			GuiPropertyBitFieldComboEnum("Destination Blend Factor", blendFlags.DstBlendFactor, BlendFactorNames);
			GuiPropertyBitFieldInputInt("Blend Operation", blendFlags.BlendOperation);
			GuiPropertyBitFieldInputInt("Z Bias", blendFlags.ZBias);
			GuiPropertyBitFieldCheckbox("No Fog", blendFlags.NoFog);
			GuiPropertyBitFieldInputInt("Unknown0", blendFlags.Unknown0);
			GuiPropertyBitFieldInputInt("Unknown1", blendFlags.Unknown1);
		});
	}

	void MaterialEditor::DrawColorGui(Material::MaterialColor& materialColor)
	{
		GuiPropertyRAII::ID id(&materialColor);
		GuiProperty::TreeNode("Color", ImGuiTreeNodeFlags_None, [&]
		{
			GuiProperty::ColorEdit("Diffuse", materialColor.Diffuse, ImGuiColorEditFlags_Float);
			GuiProperty::Input("Transparency", materialColor.Transparency, 0.01f, vec2(0.0f, 1.0f));
			GuiProperty::ColorEdit("Ambient", materialColor.Ambient, ImGuiColorEditFlags_Float);
			GuiProperty::ColorEdit("Specular", materialColor.Specular, ImGuiColorEditFlags_Float);
			GuiProperty::Input("Reflectivity", materialColor.Reflectivity, 0.01f, vec2(0.0f, 1.0f));
			GuiProperty::ColorEdit("Emission", materialColor.Emission, ImGuiColorEditFlags_Float);
			GuiProperty::Input("Shininess", materialColor.Shininess, 0.05f, vec2(0.0f, 128.0f));
			GuiProperty::Input("Intensity", materialColor.Intensity, 0.01f, vec2(0.0f, 1.0f));
		});
	}

	void MaterialEditor::MaterialPreview::DrawGui(D3D_Renderer3D& renderer, const SceneParameters& scene, Material& material)
	{
		Gui::Columns(1);

		if (viewport == nullptr)
		{
			viewport = MakeUnique<SceneViewport>();
			viewport->Parameters.AllowDebugShaderOverride = false;
			viewport->Parameters.FrustumCulling = false;
			viewport->Parameters.AlphaSort = false;
			viewport->Parameters.RenderBloom = true;
			viewport->Parameters.RenderLensFlare = false;
			viewport->Parameters.AutoExposure = false;
			viewport->Parameters.RenderFog = false;
			viewport->Parameters.ObjectMorphing = false;
			viewport->Parameters.ObjectSkinning = false;
			viewport->Parameters.ToneMapPreserveAlpha = true;
			viewport->Parameters.ShadowMapping = true;
			viewport->Parameters.RenderReflection = false;
			viewport->Parameters.MultiSampleCount = 2;
			viewport->Parameters.ClearColor = vec4(vec3(vec4(Gui::GetStyleColorVec4(ImGuiCol_ChildBg))), 0.0f);

			sphereObj = GenerateUploadMaterialTestSphereObj();
		}

		const vec2 cursorPos = Gui::GetCursorScreenPos();
		renderSize = vec2(Gui::GetContentRegionAvailWidth(), targetRenderHeight);
		viewport->Parameters.RenderResolution = renderSize;

		Gui::BeginChild("MaterialPreviewChid", renderSize, false, ImGuiWindowFlags_None);
		UpdateCameraView();
		RenderMaterial(renderer, scene, material);
		Gui::GetWindowDrawList()->AddImage(viewport->Data.Output.RenderTarget, cursorPos, cursorPos + renderSize);
		Gui::ItemSize(renderSize);
		Gui::EndChild();
	}

	void MaterialEditor::MaterialPreview::UpdateCameraView()
	{
		viewport->Camera.FieldOfView = 90.0f;
		viewport->Camera.AspectRatio = (renderSize.x / renderSize.y);

		cameraController.Settings.OrbitMouseScrollDistance = false;
		cameraController.OrbitData.Distance = cameraDistance;

		if (rotateCamera)
			cameraController.OrbitData.TargetRotation += (rotationSpeed * Gui::GetIO().DeltaTime);

		cameraController.Update(viewport->Camera);
	}

	void MaterialEditor::MaterialPreview::RenderMaterial(D3D_Renderer3D& renderer, const SceneParameters& scene, Material& material)
	{
		viewport->Camera.UpdateMatrices();
		renderer.Begin(*viewport, scene);
		{
			overrideAnimation.MaterialOverrides.clear();
			overrideAnimation.MaterialOverrides.push_back({ &sphereObj->Meshes.front().SubMeshes.front(), &material });

			RenderCommand renderCommand;
			renderCommand.SourceObj = sphereObj.get();
			renderCommand.Transform = Transform(viewport->Camera.Interest);
			renderCommand.Animation = &overrideAnimation;
			renderer.Draw(renderCommand);
		}
		renderer.End();
	}
}
