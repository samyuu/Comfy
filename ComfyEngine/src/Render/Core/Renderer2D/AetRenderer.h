#pragma once
#include "Types.h"
#include "RenderCommand2D.h"
#include "Graphics/Auth2D/Aet/AetUtil.h"
#include "Graphics/Auth2D/SprSet.h"
#include <functional>

namespace Comfy::Render
{
	struct TexSpr
	{
		const Graphics::Tex* Tex;
		const Graphics::Spr* Spr;
	};

	using SprGetter = std::function<TexSpr(const Graphics::Aet::VideoSource& source)>;

	using AetObjCallback = std::function<bool(const Graphics::Aet::Util::Obj& obj, vec2 positionOffset, float opacity)>;
	using AetObjMaskCallback = std::function<bool(const Graphics::Aet::Util::Obj& maskObj, const Graphics::Aet::Util::Obj& obj, vec2 positionOffset, float opacity)>;

	TexSpr SprSetNameStringSprGetter(const Graphics::Aet::VideoSource& source, const Graphics::SprSet* sprSetToSearch);

	TexSpr NullSprGetter(const Graphics::Aet::VideoSource& source);

	// TODO: AetCommand2D (?)

	class Renderer2D;

	class AetRenderer : NonCopyable
	{
	public:
		AetRenderer(Renderer2D& renderer);
		~AetRenderer() = default;

		// NOTE: All of these are only valid between a Rendere2D Being() and End() block
	public:
		void DrawObj(const Graphics::Aet::Util::Obj& obj, vec2 positionOffset, float opacity = 1.0f);
		void DrawObjMask(const Graphics::Aet::Util::Obj& maskObj, const Graphics::Aet::Util::Obj& obj, vec2 positionOffset, float opacity = 1.0f);
		void DrawObjCache(const Graphics::Aet::Util::ObjCache& objCache, vec2 position = vec2(0.0f, 0.0f), float opacity = 1.0f);

		void DrawLayer(const Graphics::Aet::Layer& layer, frame_t frame, vec2 position = vec2(0.0f, 0.0f), float opacity = 1.0f);
		void DrawLayerLooped(const Graphics::Aet::Layer& layer, frame_t frame, vec2 position = vec2(0.0f, 0.0f), float opacity = 1.0f);
		void DrawLayerClamped(const Graphics::Aet::Layer& layer, frame_t frame, vec2 position = vec2(0.0f, 0.0f), float opacity = 1.0f);

		void DrawVideo(const Graphics::Aet::Video& video, i32 frameIndex, const Graphics::Transform2D& transform, Graphics::AetBlendMode blendMode = Graphics::AetBlendMode::Normal);
		void DrawSpr(const Graphics::Tex& tex, const Graphics::Spr& spr, const Graphics::Transform2D& transform, Graphics::AetBlendMode blendMode = Graphics::AetBlendMode::Normal);

	public:
		void SetSprGetter(SprGetter value);
		void SetObjCallback(AetObjCallback value);
		void SetObjMaskCallback(AetObjMaskCallback value);

		TexSpr GetSprite(const Graphics::Aet::VideoSource& source) const;
		TexSpr GetSprite(const Graphics::Aet::VideoSource* source) const;

		TexSpr GetSprite(const Graphics::Aet::Video& video, i32 frameIndex) const;
		TexSpr GetSprite(const Graphics::Aet::Video* video, i32 frameIndex) const;

		vec4 GetSolidVideoColor(const Graphics::Aet::Video& video, float opacity = 1.0f);

	public:
		const Graphics::Aet::Util::ObjCache& GetLastObjCache() const;

	private:
		Renderer2D& renderer2D;

		SprGetter sprGetter;
		AetObjCallback objCallback;
		AetObjMaskCallback objMaskCallback;

		Graphics::Aet::Util::ObjCache objCache;
	};
}
