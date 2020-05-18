#pragma once
#include "Types.h"
#include "Renderer2D.h"
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

	// TODO: Should this be combined into the Renderer2D (?)
	//		 Considering the Renderer3D operates on ObjSet directly...
	//		 but doesn't on A3D...

	// TODO: AetCommand2D (?)

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

		void DrawVideo(const Graphics::Aet::Video& video, i32 frameIndex, vec2 position);

	public:
		void SetSprGetter(SprGetter value);
		void SetObjCallback(AetObjCallback value);
		void SetObjMaskCallback(AetObjMaskCallback value);

		TexSpr GetSprite(const Graphics::Aet::VideoSource& source) const;
		TexSpr GetSprite(const Graphics::Aet::VideoSource* source) const;

		TexSpr GetSprite(const Graphics::Aet::Video& video, i32 frameIndex) const;
		TexSpr GetSprite(const Graphics::Aet::Video* video, i32 frameIndex) const;

		vec4 GetSolidVideoColor(const Graphics::Aet::Video& video, float opacity = 1.0f);

	private:
		Renderer2D& renderer2D;

		SprGetter sprGetter;
		AetObjCallback objCallback;
		AetObjMaskCallback objMaskCallback;

		Graphics::Aet::Util::ObjCache objCache;
	};

	// TODO: Maybe something along the lines of... instead (?)
	//struct AetRenderContext
	//{
	//	std::vector<Graphics::Aet::Util::ObjCache> Cache;
	//};

	// TODO: Or a proper AetManager which also supports multiple layers
	// class AetManager {};
	// ... to transform Aet::Util::ObjCache into draw calls
	// for convenience sake maybe Renderer2D should own its own AetManager (?)
}
