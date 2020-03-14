#pragma once
#include "Types.h"
#include "Graphics/Auth3D/ObjSet.h"
#include "Graphics/Direct3D/Renderer/D3D_Renderer3D.h"

namespace Comfy::Editor
{
	class MaterialEditor
	{
	public:
		MaterialEditor() = default;
		~MaterialEditor() = default;

	public:
		void DrawGui(const Graphics::D3D_Renderer3D& renderer, Graphics::Material& material);

	private:
		void DrawUsedTexturesFlagsGui(uint32_t& usedTexturesCount, Graphics::Material::MaterialTextureFlags& texturesFlags);
		void DrawShaderFlagsGui(Graphics::Material::ShaderTypeIdentifier& shaderType, Graphics::Material::MaterialShaderFlags& shaderFlags);
		void DrawTextureDataGui(const Graphics::D3D_Renderer3D& renderer, Graphics::Material& material);
		void DrawBlendFlagsGui(Graphics::Material::MaterialBlendFlags& blendFlags);
		void DrawColorGui(Graphics::Material::MaterialColor& materialColor);

		// TODO: Eventually store D3D_RenderTarget on which to draw a material preview onto a sphere
	};
}
