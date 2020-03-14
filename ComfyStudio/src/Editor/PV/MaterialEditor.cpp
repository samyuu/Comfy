#include "MaterialEditor.h"
#include "ImGui/Gui.h"
#include "ImGui/Extensions/TxpExtensions.h"
#include "ImGui/Extensions/PropertyEditor.h"

#define GuiBitFieldCheckbox(name, bitFieldMember) { bool temp = bitFieldMember; if (GuiProperty::Checkbox(name, temp)) { bitFieldMember = temp; } }
#define GuiBitFieldInputInt(name, bitFieldMember) { int temp = bitFieldMember; if (GuiProperty::Input(name, temp)) { bitFieldMember = temp; } }
#define GuiBitFieldComboEnum(name, bitfieldMember, enumNames) { int temp = static_cast<int>(bitfieldMember); if (GuiProperty::Combo(name, temp, enumNames)) { bitfieldMember = static_cast<decltype(bitfieldMember)>(temp); } }

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
			std::make_pair("EyeBall", Material::ShaderIdentifiers::EyeBall),
			std::make_pair("EyeLens", Material::ShaderIdentifiers::EyeLens),
			std::make_pair("GlassEye", Material::ShaderIdentifiers::GlassEye),
			std::make_pair("Water01", Material::ShaderIdentifiers::Water01),
			std::make_pair("Floor", Material::ShaderIdentifiers::Floor),
		};

		constexpr vec2 TxpDisplaySize = vec2(96.0f);
	}

	void MaterialEditor::DrawGui(const D3D_Renderer3D& renderer, Material& material)
	{
		GuiPropertyRAII::ID id(&material);
		GuiPropertyRAII::PropertyValueColumns rootColumns;

		char valueNodeBuffer[68];
		sprintf_s(valueNodeBuffer, "(%s)", material.Name.data());

		GuiProperty::TreeNode("Material", valueNodeBuffer, ImGuiTreeNodeFlags_DefaultOpen, [&]
		{
			GuiProperty::Separator();
			GuiProperty::Checkbox("Use Debug Material", material.Debug.UseDebugMaterial);
			GuiProperty::Separator();
			DrawUsedTexturesFlagsGui(material.UsedTexturesCount, material.UsedTexturesFlags);
			DrawShaderFlagsGui(material.ShaderType, material.ShaderFlags);
			DrawTextureDataGui(renderer, material);
			DrawBlendFlagsGui(material.BlendFlags);
			DrawColorGui(material.Color);
		});
	}

	void MaterialEditor::DrawUsedTexturesFlagsGui(uint32_t& usedTexturesCount, Material::MaterialTextureFlags& texturesFlags)
	{
		GuiProperty::TreeNode("Used Textures Flags", ImGuiTreeNodeFlags_None, [&]
		{
			GuiProperty::Separator();
			GuiProperty::Input("Used Texture Count", usedTexturesCount);
			GuiBitFieldCheckbox("Color", texturesFlags.Color);
			GuiBitFieldCheckbox("Color A", texturesFlags.ColorA);
			GuiBitFieldCheckbox("Color L1", texturesFlags.ColorL1);
			GuiBitFieldCheckbox("Color L1 A", texturesFlags.ColorL1A);
			GuiBitFieldCheckbox("Color L2", texturesFlags.ColorL2);
			GuiBitFieldCheckbox("Color L2 A", texturesFlags.ColorL2A);
			GuiBitFieldCheckbox("Transparency", texturesFlags.Transparency);
			GuiBitFieldCheckbox("Specular", texturesFlags.Specular);
			GuiBitFieldCheckbox("Normal", texturesFlags.Normal);
			GuiBitFieldCheckbox("NormalAlt", texturesFlags.NormalAlt);
			GuiBitFieldCheckbox("Environment", texturesFlags.Environment);
			GuiBitFieldCheckbox("Color L3", texturesFlags.ColorL3);
			GuiBitFieldCheckbox("Color L3 A", texturesFlags.ColorL3A);
			GuiBitFieldCheckbox("Translucency", texturesFlags.Translucency);
			GuiBitFieldCheckbox("Unknown 0", texturesFlags.Unknown0);
			GuiBitFieldCheckbox("OverrideIBLCubeMap", texturesFlags.OverrideIBLCubeMap);
		});
		GuiProperty::Separator();
	}

	void MaterialEditor::DrawShaderFlagsGui(Material::ShaderTypeIdentifier& shaderType, Material::MaterialShaderFlags& shaderFlags)
	{
		GuiProperty::TreeNode("Shader Flags", ImGuiTreeNodeFlags_None, [&]
		{
			GuiProperty::Separator();
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

			GuiProperty::Separator();
			GuiBitFieldComboEnum("Vertex Translation Type", shaderFlags.VertexTranslationType, VertexTranslationTypeNames);
			GuiBitFieldComboEnum("Color Source Type", shaderFlags.ColorSourceType, ColorSourceTypeNames);
			GuiProperty::Separator();
			GuiBitFieldCheckbox("Lambert Shading", shaderFlags.LambertShading);
			GuiBitFieldCheckbox("Phong Shading", shaderFlags.PhongShading);
			GuiBitFieldCheckbox("Per Pixel Shading", shaderFlags.PerPixelShading);
			GuiBitFieldCheckbox("Double Shading", shaderFlags.DoubleShading);
			GuiProperty::Separator();
			GuiBitFieldComboEnum("Bump Map Type", shaderFlags.BumpMapType, BumpMapTypeNames);
			GuiBitFieldInputInt("Fresnel", shaderFlags.Fresnel);
			GuiBitFieldInputInt("Line Light", shaderFlags.LineLight);
			GuiProperty::Separator();
			GuiBitFieldCheckbox("Receives Shadows", shaderFlags.ReceivesShadows);
			GuiBitFieldCheckbox("Casts Shadows", shaderFlags.CastsShadows);
			GuiProperty::Separator();
			GuiBitFieldComboEnum("Specular Quality", shaderFlags.SpecularQuality, SpecularQualityNames);
			GuiBitFieldComboEnum("Aniso Direction", shaderFlags.AnisoDirection, AnisoDirectionNames);
		});
		GuiProperty::Separator();
	}

	void MaterialEditor::DrawTextureDataGui(const D3D_Renderer3D& renderer, Material& material)
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

				GuiProperty::Separator();
				GuiProperty::TreeNode(nodePropertyNameBuffer, nodeValueNameBuffer, ImGuiTreeNodeFlags_None, [&]
				{
					GuiProperty::Separator();
					GuiProperty::TreeNode("Sampler Flags", ImGuiTreeNodeFlags_None, [&]
					{
						auto& samplerFlags = texture.SamplerFlags;
						GuiProperty::Separator();
						GuiBitFieldCheckbox("Repeat U", samplerFlags.RepeatU);
						GuiBitFieldCheckbox("Repeat V", samplerFlags.RepeatV);
						GuiBitFieldCheckbox("Mirror U", samplerFlags.MirrorU);
						GuiBitFieldCheckbox("Mirror V", samplerFlags.MirrorV);
						GuiProperty::Separator();
						GuiBitFieldCheckbox("Ignore Alpha", samplerFlags.IgnoreAlpha);
						GuiBitFieldInputInt("Blend", samplerFlags.Blend);
						GuiBitFieldInputInt("Alpha Blend", samplerFlags.AlphaBlend);
						GuiProperty::Separator();
						GuiBitFieldCheckbox("Border", samplerFlags.Border);
						GuiBitFieldCheckbox("Clamp To Edge", samplerFlags.ClampToEdge);
						GuiBitFieldInputInt("Filter", samplerFlags.Filter);
						GuiProperty::Separator();
						GuiBitFieldInputInt("Mip Map", samplerFlags.MipMap);
						GuiBitFieldInputInt("Mip Map Bias", samplerFlags.MipMapBias);
						GuiBitFieldInputInt("Ansi Filters", samplerFlags.AnsiFilters);
					});

					GuiProperty::Separator();
					auto tempID = static_cast<uint32_t>(texture.TextureID.ID);
					if (GuiProperty::InputHex("Texture ID", tempID))
						texture.TextureID = static_cast<TxpID>(tempID);

					GuiProperty::Separator();
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

					GuiProperty::Separator();
					GuiProperty::TreeNode("Texture Flags", ImGuiTreeNodeFlags_None, [&]
					{
						auto& textureFlags = texture.TextureFlags;
						GuiProperty::Separator();
						GuiBitFieldComboEnum("Type", textureFlags.Type, MaterialTextureTypeNames);
						GuiBitFieldComboEnum("UV Index", textureFlags.UVIndex, MaterialTextureUVIndexNames);
						GuiBitFieldComboEnum("UV Translation Type", textureFlags.UVTranslationType, MaterialTextureUVTranslationTypeNames);
					});
				});
			}
		});
		GuiProperty::Separator();
	}

	void MaterialEditor::DrawBlendFlagsGui(Material::MaterialBlendFlags& blendFlags)
	{
		GuiProperty::TreeNode("Blend Flags", ImGuiTreeNodeFlags_None, [&]
		{
			GuiProperty::Separator();
			GuiBitFieldCheckbox("Alpha Texture", blendFlags.AlphaTexture);
			GuiBitFieldCheckbox("Alpha Material", blendFlags.AlphaMaterial);
			GuiBitFieldCheckbox("Punch Through", blendFlags.PunchThrough);
			GuiProperty::Separator();
			GuiBitFieldCheckbox("Double Sided", blendFlags.DoubleSided);
			GuiBitFieldCheckbox("Normal Direction Light", blendFlags.NormalDirectionLight);
			GuiProperty::Separator();
			GuiBitFieldComboEnum("Source Blend Factor", blendFlags.SrcBlendFactor, BlendFactorNames);
			GuiBitFieldComboEnum("Destination Blend Factor", blendFlags.DstBlendFactor, BlendFactorNames);
			GuiBitFieldInputInt("Blend Operation", blendFlags.BlendOperation);
			GuiProperty::Separator();
			GuiBitFieldInputInt("Z Bias", blendFlags.ZBias);
			GuiBitFieldCheckbox("No Fog", blendFlags.NoFog);
			GuiProperty::Separator();
			GuiBitFieldInputInt("Unknown0", blendFlags.Unknown0);
			GuiBitFieldInputInt("Unknown1", blendFlags.Unknown1);
		});
		GuiProperty::Separator();
	}

	void MaterialEditor::DrawColorGui(Material::MaterialColor& materialColor)
	{
		GuiPropertyRAII::ID id(&materialColor);
		GuiProperty::TreeNode("Color", ImGuiTreeNodeFlags_None, [&]
		{
			GuiProperty::Separator();
			GuiProperty::ColorEdit("Diffuse", materialColor.Diffuse, ImGuiColorEditFlags_Float);
			GuiProperty::Input("Transparency", materialColor.Transparency, 0.01f, 0.0f, 1.0f);
			GuiProperty::ColorEdit("Ambient", materialColor.Ambient, ImGuiColorEditFlags_Float);
			GuiProperty::ColorEdit("Specular", materialColor.Specular, ImGuiColorEditFlags_Float);
			GuiProperty::Input("Reflectivity", materialColor.Reflectivity, 0.01f, 0.0f, 1.0f);
			GuiProperty::ColorEdit("Emission", materialColor.Emission, ImGuiColorEditFlags_Float);
			GuiProperty::Input("Shininess", materialColor.Shininess, 0.05f, 0.0f, 128.0f);
			GuiProperty::Input("Intensity", materialColor.Intensity, 0.01f, 0.0f, 1.0f);
		});
		GuiProperty::Separator();
	}
}

#undef GuiBitFieldComboEnum
#undef GuiBitFieldInputInt
#undef GuiBitFieldCheckbox
