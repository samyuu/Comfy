#pragma once
#include "Types.h"
#include "Graphics/Auth3D/ObjSet.h"
#include "Graphics/GPU/GPURenderers.h"
#include "Editor/Common/CameraController3D.h"

namespace Comfy::Editor
{
	class MaterialEditor
	{
	public:
		MaterialEditor() = default;
		~MaterialEditor() = default;

	public:
		void DrawGui(Graphics::GPU_Renderer3D& renderer, const Graphics::SceneParameters& scene, Graphics::Material& material);

	private:
		void DrawUsedTexturesFlagsGui(u32& usedTexturesCount, Graphics::Material::MaterialUsedTextureFlags& texturesFlags);
		void DrawShaderFlagsGui(Graphics::Material::ShaderTypeIdentifier& shaderType, Graphics::Material::MaterialShaderFlags& shaderFlags);
		void DrawTextureDataGui(Graphics::GPU_Renderer3D& renderer, Graphics::Material& material);
		void DrawBlendFlagsGui(Graphics::Material::MaterialBlendFlags& blendFlags);
		void DrawColorGui(Graphics::Material::MaterialColor& materialColor);

		class MaterialPreview
		{
		public:
			void DrawGui(Graphics::GPU_Renderer3D& renderer, const Graphics::SceneParameters& scene, Graphics::Material& material);

		private:
			void UpdateCameraView();
			void RenderMaterial(Graphics::GPU_Renderer3D& renderer, const Graphics::SceneParameters& scene, Graphics::Material& material);

			std::unique_ptr<Graphics::SceneViewport> viewport = nullptr;
			CameraController3D cameraController;

			std::unique_ptr<Graphics::Obj> sphereObj;
			Graphics::ObjAnimationData overrideAnimation;

			float targetRenderHeight = 120.0f;
			vec2 renderSize = vec2(targetRenderHeight, 0.0f);

			float cameraDistance = 1.45f;
			bool rotateCamera = true;
			vec2 rotationSpeed = vec2(22.0f, 0.0f);
		} preview;
	};
}
