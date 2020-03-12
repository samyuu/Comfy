#include "MaterialEditor.h"
#include "ImGui/Gui.h"
#include "ImGui/Extensions/TxpExtensions.h"

#define GuiBitFieldCheckbox(name, bitFieldMember) { bool temp = bitFieldMember; if (Gui::Checkbox(name, &temp)) { bitFieldMember = temp; } }
#define GuiBitFieldInputInt(name, bitFieldMember) { int temp = bitFieldMember; if (Gui::InputInt(name, &temp)) { bitFieldMember = temp; } }
#define GuiBitFieldComboEnum(name, bitfieldMember, enumNames) { int temp = static_cast<int>(bitfieldMember); if (ComboHelper(name, &temp, ImGuiComboFlags_None, enumNames)) { bitfieldMember = static_cast<decltype(bitfieldMember)>(temp); } }

namespace Comfy::Editor
{
	template <typename ArrayType>
	bool ComboHelper(const char* label, int* currentItem, ImGuiComboFlags flags, const ArrayType& items)
	{
		const int previousCurrentItem = *currentItem;

		const char* previewValue = (*currentItem < 0 || *currentItem >= items.size()) ? "" : (items[*currentItem] == nullptr ? "" : items[*currentItem]);
		if (Gui::BeginCombo(label, previewValue, flags))
		{
			for (int i = 0; i < items.size(); i++)
			{
				if (items[i] == nullptr)
					continue;

				const bool isSelected = (*currentItem == i);
				if (Gui::Selectable(items[i], isSelected))
					*currentItem = i;
				if (isSelected)
					Gui::SetItemDefaultFocus();
			}
			Gui::EndCombo();
		}

		return (*currentItem != previousCurrentItem);
	}

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

		constexpr vec2 TxpDisplaySize = vec2(64.0f);
	}

	void MaterialEditor::DrawGui(const D3D_Renderer3D& renderer, Material& material)
	{
		Gui::Checkbox("Use Debug Material", &material.Debug.UseDebugMaterial);
		DrawUsedTexturesFlagsGui(material.UsedTexturesCount, material.UsedTexturesFlags);
		DrawShaderTypeGui(material.ShaderType);
		DrawShaderFlagsGui(material.ShaderFlags);
		DrawTextureDataGui(renderer, material);
		DrawBlendFlagsGui(material.BlendFlags);
		DrawColorGui(material.Color);
	}

	void MaterialEditor::DrawUsedTexturesFlagsGui(uint32_t& usedTexturesCount, Material::MaterialTextureFlags& texturesFlags)
	{
		if (Gui::WideTreeNodeEx("Used Textures Flags", ImGuiTreeNodeFlags_None))
		{
			Gui::InputScalar("Used Textures Count %d", ImGuiDataType_U32, &usedTexturesCount);
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
			Gui::TreePop();
		}
	}

	void MaterialEditor::DrawShaderTypeGui(Material::ShaderTypeIdentifier& shaderType)
	{
		if (Gui::InputText("Shader Type", shaderType.data(), shaderType.size(), ImGuiInputTextFlags_None))
			std::fill(std::find(shaderType.begin(), shaderType.end(), '\0'), shaderType.end(), '\0');

		Gui::ItemContextMenu("ShaderTypeContextMenu", [&]
		{
			for (const auto&[typeName, newType] : AvailableMaterialShaderTypes)
			{
				if (Gui::MenuItem(typeName))
					shaderType = newType;
			}
		});
	}

	void MaterialEditor::DrawShaderFlagsGui(Material::MaterialShaderFlags& shaderFlags)
	{
		if (Gui::WideTreeNodeEx("Shader Flags", ImGuiTreeNodeFlags_None))
		{
			GuiBitFieldComboEnum("Vertex Translation Type", shaderFlags.VertexTranslationType, VertexTranslationTypeNames);
			GuiBitFieldComboEnum("Color Source Type", shaderFlags.ColorSourceType, ColorSourceTypeNames);
			Gui::Separator();
			GuiBitFieldCheckbox("Lambert Shading", shaderFlags.LambertShading);
			GuiBitFieldCheckbox("Phong Shading", shaderFlags.PhongShading);
			GuiBitFieldCheckbox("Per Pixel Shading", shaderFlags.PerPixelShading);
			GuiBitFieldCheckbox("Double Shading", shaderFlags.DoubleShading);
			Gui::Separator();
			GuiBitFieldComboEnum("Bump Map Type", shaderFlags.BumpMapType, BumpMapTypeNames);
			GuiBitFieldInputInt("Fresnel", shaderFlags.Fresnel);
			GuiBitFieldInputInt("Line Light", shaderFlags.LineLight);
			Gui::Separator();
			GuiBitFieldCheckbox("Receives Shadows", shaderFlags.ReceivesShadows);
			GuiBitFieldCheckbox("Casts Shadows", shaderFlags.CastsShadows);
			Gui::Separator();
			GuiBitFieldComboEnum("Specular Quality", shaderFlags.SpecularQuality, SpecularQualityNames);
			GuiBitFieldComboEnum("Aniso Direction", shaderFlags.AnisoDirection, AnisoDirectionNames);
			Gui::TreePop();
		}
	}

	void MaterialEditor::DrawTextureDataGui(const D3D_Renderer3D& renderer, Material& material)
	{
		Gui::PushID(&material.Textures);
		if (Gui::WideTreeNodeEx("Texture Data", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (size_t i = 0; i < material.Textures.size(); i++)
			{
				auto& texture = material.Textures[i];
				
				char textureNodeNameBuffer[64];
				sprintf_s(textureNodeNameBuffer, "Textures[%zu]", i);

				if (Gui::WideTreeNodeEx(textureNodeNameBuffer, ImGuiTreeNodeFlags_None))
				{
					if (Gui::WideTreeNodeEx("Sampler Flags", ImGuiTreeNodeFlags_None))
					{
						auto& samplerFlags = texture.SamplerFlags;
						GuiBitFieldCheckbox("Repeat U", samplerFlags.RepeatU);
						GuiBitFieldCheckbox("Repeat V", samplerFlags.RepeatV);
						GuiBitFieldCheckbox("Mirror U", samplerFlags.MirrorU);
						GuiBitFieldCheckbox("Mirror V", samplerFlags.MirrorV);
						Gui::Separator();
						GuiBitFieldCheckbox("Ignore Alpha", samplerFlags.IgnoreAlpha);
						GuiBitFieldInputInt("Blend", samplerFlags.Blend);
						GuiBitFieldInputInt("Alpha Blend", samplerFlags.AlphaBlend);
						Gui::Separator();
						GuiBitFieldCheckbox("Border", samplerFlags.Border);
						GuiBitFieldCheckbox("Clamp To Edge", samplerFlags.ClampToEdge);
						GuiBitFieldInputInt("Filter", samplerFlags.Filter);
						Gui::Separator();
						GuiBitFieldInputInt("Mip Map", samplerFlags.MipMap);
						GuiBitFieldInputInt("Mip Map Bias", samplerFlags.MipMapBias);
						GuiBitFieldInputInt("Ansi Filters", samplerFlags.AnsiFilters);
						Gui::TreePop();
					}

					Gui::InputScalar("Texture ID", ImGuiDataType_U32, &texture.TextureID, nullptr, nullptr, "%X", ImGuiInputTextFlags_CharsHexadecimal);
					if (auto txp = renderer.GetTxpFromTextureID(&texture.TextureID); txp != nullptr)
					{
						Gui::Text("%s", txp->GetName().data());
						Gui::ImageObjTxp(txp, TxpDisplaySize);
					}

					if (Gui::WideTreeNodeEx("Texture Flags", ImGuiTreeNodeFlags_None))
					{
						auto& textureFlags = texture.TextureFlags;
						GuiBitFieldComboEnum("Type", textureFlags.Type, MaterialTextureTypeNames);
						GuiBitFieldComboEnum("UV Index", textureFlags.UVIndex, MaterialTextureUVIndexNames);
						GuiBitFieldComboEnum("UV Translation Type", textureFlags.UVTranslationType, MaterialTextureUVTranslationTypeNames);
						Gui::TreePop();
					}

					Gui::TreePop();
				}
			}

			Gui::TreePop();
		}
		Gui::PopID();
	}

	void MaterialEditor::DrawBlendFlagsGui(Material::MaterialBlendFlags& blendFlags)
	{
		if (Gui::WideTreeNodeEx("Blend Flags", ImGuiTreeNodeFlags_None))
		{
			GuiBitFieldCheckbox("Alpha Texture", blendFlags.AlphaTexture);
			GuiBitFieldCheckbox("Alpha Material", blendFlags.AlphaMaterial);
			GuiBitFieldCheckbox("Punch Through", blendFlags.PunchThrough);
			Gui::Separator();
			GuiBitFieldCheckbox("Double Sided", blendFlags.DoubleSided);
			GuiBitFieldCheckbox("Normal Direction Light", blendFlags.NormalDirectionLight);
			Gui::Separator();
			GuiBitFieldComboEnum("Source Blend Factor", blendFlags.SrcBlendFactor, BlendFactorNames);
			GuiBitFieldComboEnum("Destination Blend Factor", blendFlags.DstBlendFactor, BlendFactorNames);
			GuiBitFieldInputInt("Blend Operation", blendFlags.BlendOperation);
			Gui::Separator();
			GuiBitFieldInputInt("Z Bias", blendFlags.ZBias);
			GuiBitFieldCheckbox("No Fog", blendFlags.NoFog);
			Gui::Separator();
			GuiBitFieldInputInt("Unknown0", blendFlags.Unknown0);
			GuiBitFieldInputInt("Unknown1", blendFlags.Unknown1);
			Gui::TreePop();
		}
	}

	void MaterialEditor::DrawColorGui(Material::MaterialColor& materialColor)
	{
		Gui::PushID(&materialColor);
		if (Gui::WideTreeNodeEx("Color", ImGuiTreeNodeFlags_None))
		{
			Gui::ColorEdit3("Diffuse", glm::value_ptr(materialColor.Diffuse), ImGuiColorEditFlags_Float);
			Gui::DragFloat("Transparency", &materialColor.Transparency, 0.01f);
			Gui::ColorEdit4("Ambient", glm::value_ptr(materialColor.Ambient), ImGuiColorEditFlags_Float);
			Gui::ColorEdit3("Specular", glm::value_ptr(materialColor.Specular), ImGuiColorEditFlags_Float);
			Gui::DragFloat("Reflectivity", &materialColor.Reflectivity, 0.01f);
			Gui::ColorEdit4("Emission", glm::value_ptr(materialColor.Emission), ImGuiColorEditFlags_Float);
			Gui::DragFloat("Shininess", &materialColor.Shininess, 0.05f);
			Gui::DragFloat("Intensity", &materialColor.Intensity, 0.05f);
			Gui::TreePop();
		}
		Gui::PopID();
	}
}

#undef GuiBitFieldComboEnum
#undef GuiBitFieldInputInt
#undef GuiBitFieldCheckbox
