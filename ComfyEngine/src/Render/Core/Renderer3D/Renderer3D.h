#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "RenderCommand3D.h"
#include "RenderTarget3D.h"
#include "Render/Core/Camera.h"
#include "Graphics/Auth3D/LightParam/Parameters.h"
#include <optional>
#include <functional>

namespace Comfy::Render
{
	using TexGetter = std::function<const Graphics::Tex*(const Cached_TexID* texID)>;

	struct SceneParam3D
	{
		struct LensFlareParam
		{
			// NOTE: Lens flare sun position
			std::optional<vec3> SunPosition;
			const Graphics::Obj* SunObj = nullptr;

			struct Textures
			{
				// NOTE: Common sun texture from effcmn
				Cached_TexID Sun = TexID::Invalid;
				// NOTE: Stage specific textures
				std::array<Cached_TexID, 2> Flares = { TexID::Invalid, TexID::Invalid };
				Cached_TexID Ghost = TexID::Invalid;
			} Textures;
		} LensFlare;

		Graphics::FogParameter Fog;
		Graphics::GlowParameter Glow;
		Graphics::LightParameter Light;
		Graphics::IBLParameters IBL;
	};

	class Renderer3D : NonCopyable
	{
	public:
		Renderer3D(TexGetter texGetter);
		~Renderer3D();

	public:
		void Begin(PerspectiveCamera& camera, RenderTarget3D& renderTarget, const SceneParam3D& sceneParam);
		void Draw(const RenderCommand3D& command);
		// TODO: DrawRect(), DrawCircle(), DrawCylinder(), DrawLine(), DrawSpriteBillboard(), etc.

		void End();

	public:
		const Graphics::Tex* GetTexFromTextureID(const Cached_TexID* textureID) const;

	public:
		static std::unique_ptr<RenderTarget3D> CreateRenderTarget();

	private:
		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}
