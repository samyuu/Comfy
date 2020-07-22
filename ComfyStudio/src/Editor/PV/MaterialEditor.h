#pragma once
#include "Types.h"
#include "Graphics/Auth3D/ObjSet.h"
#include "Render/Render.h"
#include "Editor/Common/CameraController3D.h"
#include "ImGui/Extensions/CheckerboardTexture.h"

namespace Comfy::Studio::Editor
{
	class MaterialEditor : NonCopyable
	{
	public:
		MaterialEditor() = default;
		~MaterialEditor() = default;

	public:
		void Gui(Render::Renderer3D& renderer, const Render::SceneParam3D& scene, Graphics::Material& material);

	private:
		void GuiUsedTexturesFlags(u32& usedTexturesCount, Graphics::Material::MaterialUsedTextureFlags& texturesFlags);
		void GuiShaderFlags(Graphics::Material::ShaderTypeIdentifier& shaderType, Graphics::Material::MaterialShaderFlags& shaderFlags);
		void GuiTextureData(Render::Renderer3D& renderer, Graphics::Material& material);
		void GuiBlendFlags(Graphics::Material::MaterialBlendFlags& blendFlags);
		void GuiColor(Graphics::Material::MaterialColor& materialColor);

		Gui::CheckerboardTexture checkerboard;

		class MaterialPreview
		{
		public:
			void Gui(Render::Renderer3D& renderer, const Render::SceneParam3D& scene, Graphics::Material& material);

		private:
			void UpdateCameraView();
			void RenderMaterial(Render::Renderer3D& renderer, const Render::SceneParam3D& scene, Graphics::Material& material);

			std::unique_ptr<Render::RenderTarget3D> renderTarget = nullptr;

			Render::Camera3D camera;
			CameraController3D cameraController;

			std::unique_ptr<Graphics::Obj> sphereObj;
			Render::RenderCommand3D::DynamicData overrideDynamic;

			float targetRenderHeight = 120.0f;
			vec2 renderSize = vec2(targetRenderHeight, 0.0f);

			float cameraDistance = 1.45f;
			bool rotateCamera = true;
			vec2 rotationSpeed = vec2(22.0f, 0.0f);
		} preview;
	};
}
