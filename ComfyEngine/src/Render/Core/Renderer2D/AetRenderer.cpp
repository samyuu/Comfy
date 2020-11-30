#include "AetRenderer.h"
#include "Renderer2D.h"
#include "Misc/StringUtil.h"

namespace Comfy::Render
{
	using namespace Graphics::Aet;
	namespace AetUtil = Graphics::Aet::Util;

	namespace
	{
		template <bool Exact>
		TexSprView SprSetNameStringSprGetterBase(const VideoSource& source, const Graphics::SprSet* sprSetToSearch)
		{
			if (sprSetToSearch == nullptr)
				return { nullptr, nullptr };

			if (source.SprCache != nullptr && source.SprSetCache == sprSetToSearch)
				return { sprSetToSearch->TexSet.Textures[source.SprCache->TextureIndex].get(), source.SprCache };

			// TEMP: Temporary solution, check for IDs in the future
			const auto matchingSpr = FindIfOrNull(sprSetToSearch->Sprites, [&](const auto& spr)
			{
				if constexpr (Exact)
					return (source.Name == spr.Name);
				else
					return ::Comfy::Util::EndsWith(source.Name, spr.Name);
			});

			if (matchingSpr == nullptr)
				return { nullptr, nullptr };

			source.SprCache = &(*matchingSpr);
			source.SprSetCache = sprSetToSearch;
			return { sprSetToSearch->TexSet.Textures[source.SprCache->TextureIndex].get(), source.SprCache };
		}
	}

	TexSprView SprSetNameStringSprGetter(const VideoSource& source, const Graphics::SprSet* sprSetToSearch)
	{
		return SprSetNameStringSprGetterBase<false>(source, sprSetToSearch);
	}

	TexSprView SprSetNameStringSprGetterExact(const VideoSource& source, const Graphics::SprSet* sprSetToSearch)
	{
		return SprSetNameStringSprGetterBase<true>(source, sprSetToSearch);
	}

	TexSprView NullSprGetter(const VideoSource& source)
	{
		return { nullptr, nullptr };
	}

	AetRenderer::AetRenderer(Renderer2D& renderer) : renderer2D(renderer)
	{
		sprGetter = NullSprGetter;
	}

	void AetRenderer::DrawObj(const AetUtil::Obj& obj, const Graphics::Transform2D& transform)
	{
		if (obj.Video == nullptr || !obj.IsVisible)
			return;

		if (objCallback && objCallback(obj, transform))
			return;

		auto[tex, spr] = GetSprite(obj.Video, obj.SpriteFrame);
		if (obj.SourceLayer->RenderOverride.UseTexSpr)
		{
			tex = obj.SourceLayer->RenderOverride.Tex;
			spr = obj.SourceLayer->RenderOverride.Spr;
		}

		const auto objTransform = AetUtil::CombineTransformsCopy(obj.Transform, transform);

		if (tex != nullptr && spr != nullptr)
		{
			const auto command = RenderCommand2D(
				tex,
				objTransform.Origin,
				objTransform.Position,
				objTransform.Rotation,
				objTransform.Scale,
				spr->PixelRegion,
				obj.BlendMode,
				objTransform.Opacity);

			renderer2D.Draw(command);
		}
		else if (renderNullVideos)
		{
			auto command = RenderCommand2D();
			command.Origin = objTransform.Origin;
			command.Position = objTransform.Position;
			command.Rotation = objTransform.Rotation;
			command.Scale = objTransform.Scale;
			command.SourceRegion = vec4(0.0f, 0.0f, obj.Video->Size);
			command.BlendMode = obj.BlendMode;
			command.SetColor(GetSolidVideoColor(*obj.Video, objTransform.Opacity));

			renderer2D.Draw(command);
		}
	}

	void AetRenderer::DrawObjMask(const AetUtil::Obj& maskObj, const AetUtil::Obj& obj, const Graphics::Transform2D& transform)
	{
		if (maskObj.Video == nullptr || obj.Video == nullptr || !obj.IsVisible)
			return;

		if (objMaskCallback && objMaskCallback(maskObj, obj, transform))
			return;

		auto[maskTex, maskSpr] = GetSprite(maskObj.Video, maskObj.SpriteFrame);
		if (maskObj.SourceLayer->RenderOverride.UseTexSpr)
		{
			maskTex = maskObj.SourceLayer->RenderOverride.Tex;
			maskSpr = maskObj.SourceLayer->RenderOverride.Spr;
		}

		auto[tex, spr] = GetSprite(obj.Video, obj.SpriteFrame);
		if (obj.SourceLayer->RenderOverride.UseTexSpr)
		{
			tex = obj.SourceLayer->RenderOverride.Tex;
			spr = obj.SourceLayer->RenderOverride.Spr;
		}

		const auto objTransform = AetUtil::CombineTransformsCopy(obj.Transform, transform);
		const auto maskTransform = AetUtil::CombineTransformsCopy(maskObj.Transform, transform);

		if (maskTex != nullptr && maskSpr != nullptr && tex != nullptr && spr != nullptr)
		{
			const auto command = RenderCommand2D(
				tex,
				objTransform.Origin,
				objTransform.Position,
				objTransform.Rotation,
				objTransform.Scale,
				spr->PixelRegion,
				obj.BlendMode,
				objTransform.Opacity);

			const auto maskCommand = RenderCommand2D(
				maskTex,
				maskTransform.Origin,
				maskTransform.Position,
				maskTransform.Rotation,
				maskTransform.Scale,
				maskSpr->PixelRegion,
				maskObj.BlendMode,
				maskTransform.Opacity);

			renderer2D.Draw(command, maskCommand);
		}
		else if (renderNullVideos)
		{
			auto command = RenderCommand2D();
			command.Origin = objTransform.Origin;
			command.Position = objTransform.Position;
			command.Rotation = objTransform.Rotation;
			command.Scale = objTransform.Scale;
			command.SourceRegion = vec4(0.0f, 0.0f, obj.Video->Size);
			command.BlendMode = obj.BlendMode;
			command.SetColor(GetSolidVideoColor(*obj.Video, objTransform.Opacity));

			renderer2D.Draw(command);
		}
	}

	void AetRenderer::DrawObjCache(const AetUtil::ObjCache& objCache, const Graphics::Transform2D& transform)
	{
		for (size_t i = 0; i < objCache.size(); i++)
		{
			const auto& obj = objCache[i];
			if (obj.UseTrackMatte && InBounds(i + 1, objCache))
				DrawObjMask(objCache[++i], obj, transform);
			else
				DrawObj(obj, transform);
		}
	}

	void AetRenderer::DrawLayer(const Layer& layer, frame_t frame, const Graphics::Transform2D& transform)
	{
		if (frame < layer.StartFrame || frame >= layer.EndFrame)
			return;

		objCache.clear();
		AetUtil::GetAddObjectsAt(objCache, layer, frame);

		DrawObjCache(objCache, transform);
	}

	void AetRenderer::DrawLayerLooped(const Layer& layer, frame_t frame, const Graphics::Transform2D& transform)
	{
		DrawLayer(layer, glm::mod(frame, layer.EndFrame - 1.0f), transform);
	}

	void AetRenderer::DrawLayerClamped(const Layer& layer, frame_t frame, const Graphics::Transform2D& transform)
	{
		DrawLayer(layer, (frame >= layer.EndFrame ? layer.EndFrame : frame), transform);
	}

	void AetRenderer::DrawVideo(const Video& video, i32 frameIndex, const Graphics::Transform2D& transform, Graphics::AetBlendMode blendMode)
	{
		auto[tex, spr] = GetSprite(video, frameIndex);

		RenderCommand2D command;
		command.Origin = transform.Origin;
		command.Position = transform.Position;
		command.Rotation = transform.Rotation;
		command.Scale = transform.Scale;
		command.BlendMode = blendMode;

		if (tex != nullptr && spr != nullptr)
		{
			command.TexView = tex;
			command.SourceRegion = spr->PixelRegion;
			command.SetColor(vec4(1.0f, 1.0f, 1.0f, transform.Opacity));
			renderer2D.Draw(command);
		}
		else if (renderNullVideos)
		{
			command.SourceRegion = vec4(0.0f, 0.0f, video.Size);
			command.SetColor(GetSolidVideoColor(video));
			renderer2D.Draw(command);
		}
	}

	void AetRenderer::DrawSpr(const Graphics::Tex& tex, const Graphics::Spr& spr, const Graphics::Transform2D& transform, Graphics::AetBlendMode blendMode)
	{
		RenderCommand2D command;
		command.TexView = &tex;
		command.Origin = transform.Origin;
		command.Position = transform.Position;
		command.Rotation = transform.Rotation;
		command.Scale = transform.Scale;
		command.SourceRegion = spr.PixelRegion;
		command.BlendMode = blendMode;
		command.SetColor(vec4(1.0f, 1.0f, 1.0f, transform.Opacity));

		renderer2D.Draw(command);
	}

	void AetRenderer::SetSprGetter(SprGetter value)
	{
		sprGetter = value;
	}

	void AetRenderer::SetObjCallback(AetObjCallback value)
	{
		objCallback = value;
	}

	void AetRenderer::SetObjMaskCallback(AetObjMaskCallback value)
	{
		objMaskCallback = value;
	}

	bool AetRenderer::GetRenderNullVideos() const
	{
		return renderNullVideos;
	}

	void AetRenderer::SetRenderNullVideos(bool value)
	{
		renderNullVideos = true;
	}

	TexSprView AetRenderer::GetSprite(const VideoSource& source) const
	{
		return sprGetter(source);
	}

	TexSprView AetRenderer::GetSprite(const Graphics::Aet::VideoSource* source) const
	{
		return (source != nullptr) ? GetSprite(*source) : TexSprView { nullptr, nullptr };
	}

	TexSprView AetRenderer::GetSprite(const Graphics::Aet::Video& video, i32 frameIndex) const
	{
		return InBounds(frameIndex, video.Sources) ? GetSprite(video.Sources[frameIndex]) : TexSprView { nullptr, nullptr };
	}

	TexSprView AetRenderer::GetSprite(const Graphics::Aet::Video* video, i32 frameIndex) const
	{
		return (video != nullptr) ? GetSprite(*video, frameIndex) : TexSprView { nullptr, nullptr };
	}

	vec4 AetRenderer::GetSolidVideoColor(const Graphics::Aet::Video& video, float opacity)
	{
		static constexpr vec4 dummyColor = vec4(0.79f, 0.90f, 0.57f, 0.50f);

		if (true)
			return vec4(dummyColor.r, dummyColor.g, dummyColor.b, dummyColor.a * opacity);

		return vec4(
			((video.Color >> (CHAR_BIT * 0)) & 0xFF) / 255.0f,
			((video.Color >> (CHAR_BIT * 1)) & 0xFF) / 255.0f,
			((video.Color >> (CHAR_BIT * 2)) & 0xFF) / 255.0f,
			opacity);
	}

	const AetUtil::ObjCache& AetRenderer::GetLastObjCache() const
	{
		return objCache;
	}
}
