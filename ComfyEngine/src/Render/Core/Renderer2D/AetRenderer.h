#pragma once
#include "Types.h"
#include "RenderCommand2D.h"
#include "Graphics/Auth2D/Aet/AetUtil.h"
#include "Graphics/Auth2D/SprSet.h"
#include <functional>

namespace Comfy::Render
{
	struct TexSprView
	{
		const Graphics::Tex* Tex;
		const Graphics::Spr* Spr;

		operator bool() const { return (Tex != nullptr && Spr != nullptr); }
	};

	using SprGetter = std::function<TexSprView(const Graphics::Aet::VideoSource& source)>;

	using AetObjCallback = std::function<bool(const Graphics::Aet::Util::Obj& obj, const Graphics::Transform2D& transform)>;
	using AetObjMaskCallback = std::function<bool(const Graphics::Aet::Util::Obj& maskObj, const Graphics::Aet::Util::Obj& obj, const Graphics::Transform2D& transform)>;

	TexSprView SprSetNameStringSprGetter(const Graphics::Aet::VideoSource& source, const Graphics::SprSet* sprSetToSearch);
	TexSprView SprSetNameStringSprGetterExact(const Graphics::Aet::VideoSource& source, const Graphics::SprSet* sprSetToSearch);

	TexSprView NullSprGetter(const Graphics::Aet::VideoSource& source);

	// TODO: AetCommand2D (?)

	class Renderer2D;

	class AetRenderer : NonCopyable
	{
	public:
		AetRenderer(Renderer2D& renderer);
		~AetRenderer() = default;

		// NOTE: All of these are only valid between a Rendere2D Being() and End() block
	public:
		void DrawObj(const Graphics::Aet::Util::Obj& obj, const Graphics::Transform2D& transform = vec2(0.0f));
		void DrawObjMask(const Graphics::Aet::Util::Obj& maskObj, const Graphics::Aet::Util::Obj& obj, const Graphics::Transform2D& transform = vec2(0.0f));
		void DrawObjCache(const Graphics::Aet::Util::ObjCache& objCache, const Graphics::Transform2D& transform = vec2(0.0f));

		void DrawLayer(const Graphics::Aet::Layer& layer, frame_t frame, const Graphics::Transform2D& transform = vec2(0.0f));
		void DrawLayerLooped(const Graphics::Aet::Layer& layer, frame_t frame, const Graphics::Transform2D& transform = vec2(0.0f));
		void DrawLayerClamped(const Graphics::Aet::Layer& layer, frame_t frame, const Graphics::Transform2D& transform = vec2(0.0f));

		void DrawVideo(const Graphics::Aet::Video& video, i32 frameIndex, const Graphics::Transform2D& transform, Graphics::AetBlendMode blendMode = Graphics::AetBlendMode::Normal);
		void DrawSpr(const Graphics::Tex& tex, const Graphics::Spr& spr, const Graphics::Transform2D& transform, Graphics::AetBlendMode blendMode = Graphics::AetBlendMode::Normal);

	public:
		void SetSprGetter(SprGetter value);
		void SetObjCallback(AetObjCallback value);
		void SetObjMaskCallback(AetObjMaskCallback value);

		bool GetRenderNullVideos() const;
		void SetRenderNullVideos(bool value);

		TexSprView GetSprite(const Graphics::Aet::VideoSource& source) const;
		TexSprView GetSprite(const Graphics::Aet::VideoSource* source) const;

		TexSprView GetSprite(const Graphics::Aet::Video& video, i32 frameIndex) const;
		TexSprView GetSprite(const Graphics::Aet::Video* video, i32 frameIndex) const;

		vec4 GetSolidVideoColor(const Graphics::Aet::Video& video, float opacity = 1.0f);

	public:
		const Graphics::Aet::Util::ObjCache& GetLastObjCache() const;

	private:
		Renderer2D& renderer2D;

		SprGetter sprGetter;
		AetObjCallback objCallback;
		AetObjMaskCallback objMaskCallback;
		bool renderNullVideos = false;

		Graphics::Aet::Util::ObjCache objCache;
	};
}
