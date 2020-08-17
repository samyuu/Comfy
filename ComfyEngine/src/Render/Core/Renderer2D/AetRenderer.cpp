#include "AetRenderer.h"
#include "Renderer2D.h"
#include "Misc/StringUtil.h"

namespace Comfy::Render
{
	using namespace Graphics::Aet;
	namespace AetUtil = Graphics::Aet::Util;

	TexSprView SprSetNameStringSprGetter(const VideoSource& source, const Graphics::SprSet* sprSetToSearch)
	{
		if (sprSetToSearch == nullptr)
			return { nullptr, nullptr };

		if (source.SprCache != nullptr && source.SprSetCache == sprSetToSearch)
			return { sprSetToSearch->TexSet.Textures[source.SprCache->TextureIndex].get(), source.SprCache };

		// TEMP: Temporary solution, check for IDs in the future
		const auto matchingSpr = FindIfOrNull(sprSetToSearch->Sprites, [&](const auto& spr) { return ::Comfy::Util::EndsWith(source.Name, spr.Name); });
		if (matchingSpr == nullptr)
			return { nullptr, nullptr };

		source.SprCache = &(*matchingSpr);
		source.SprSetCache = sprSetToSearch;
		return { sprSetToSearch->TexSet.Textures[source.SprCache->TextureIndex].get(), source.SprCache };
	}

	TexSprView NullSprGetter(const VideoSource& source)
	{
		return { nullptr, nullptr };
	}

	AetRenderer::AetRenderer(Renderer2D& renderer) : renderer2D(renderer)
	{
		sprGetter = NullSprGetter;
	}

	void AetRenderer::DrawObj(const AetUtil::Obj& obj, vec2 positionOffset, float opacity)
	{
		if (obj.Video == nullptr || !obj.IsVisible)
			return;

		if (objCallback && objCallback(obj, positionOffset, opacity))
			return;

		auto[tex, spr] = GetSprite(obj.Video, obj.SpriteFrame);
		const auto finalPosition = obj.Transform.Position + positionOffset;
		const auto finalOpacity = obj.Transform.Opacity * opacity;

		if (tex != nullptr && spr != nullptr)
		{
			const auto command = RenderCommand2D(
				tex,
				obj.Transform.Origin,
				finalPosition,
				obj.Transform.Rotation,
				obj.Transform.Scale,
				spr->PixelRegion,
				obj.BlendMode,
				finalOpacity);

			renderer2D.Draw(command);
		}
		else
		{
			auto command = RenderCommand2D();
			command.Origin = obj.Transform.Origin;
			command.Position = finalPosition;
			command.Rotation = obj.Transform.Rotation;
			command.Scale = obj.Transform.Scale;
			command.SourceRegion = vec4(0.0f, 0.0f, obj.Video->Size);
			command.BlendMode = obj.BlendMode;
			command.SetColor(GetSolidVideoColor(*obj.Video, finalOpacity));

			renderer2D.Draw(command);
		}
	}

	void AetRenderer::DrawObjMask(const AetUtil::Obj& maskObj, const AetUtil::Obj& obj, vec2 positionOffset, float opacity)
	{
		if (maskObj.Video == nullptr || obj.Video == nullptr || !obj.IsVisible)
			return;

		if (objMaskCallback && objMaskCallback(maskObj, obj, positionOffset, opacity))
			return;

		auto[maskTex, maskSpr] = GetSprite(maskObj.Video, maskObj.SpriteFrame);
		auto[tex, spr] = GetSprite(obj.Video, obj.SpriteFrame);

		const auto finalOpacity = maskObj.Transform.Opacity * obj.Transform.Opacity * opacity;

		if (maskTex != nullptr && maskSpr != nullptr && tex != nullptr && spr != nullptr)
		{
			const auto command = RenderCommand2D(
				tex,
				obj.Transform.Origin,
				obj.Transform.Position + positionOffset,
				obj.Transform.Rotation,
				obj.Transform.Scale,
				spr->PixelRegion,
				obj.BlendMode,
				finalOpacity);

			const auto maskCommand = RenderCommand2D(
				maskTex,
				maskObj.Transform.Origin,
				maskObj.Transform.Position + positionOffset,
				maskObj.Transform.Rotation,
				maskObj.Transform.Scale,
				maskSpr->PixelRegion,
				maskObj.BlendMode,
				finalOpacity);

			renderer2D.Draw(command, maskCommand);
		}
		else
		{
			auto command = RenderCommand2D();
			command.Origin = obj.Transform.Origin;
			command.Position = obj.Transform.Position + positionOffset;
			command.Rotation = obj.Transform.Rotation;
			command.Scale = obj.Transform.Scale;
			command.SourceRegion = vec4(0.0f, 0.0f, obj.Video->Size);
			command.BlendMode = obj.BlendMode;
			command.SetColor(GetSolidVideoColor(*obj.Video, finalOpacity));

			renderer2D.Draw(command);
		}
	}

	void AetRenderer::DrawObjCache(const AetUtil::ObjCache& objCache, vec2 position, float opacity)
	{
		for (size_t i = 0; i < objCache.size(); i++)
		{
			const auto& obj = objCache[i];
			if (obj.UseTrackMatte && InBounds(i + 1, objCache))
				DrawObjMask(objCache[++i], obj, position, opacity);
			else
				DrawObj(obj, position, opacity);
		}
	}

	void AetRenderer::DrawLayer(const Layer& layer, frame_t frame, vec2 position, float opacity)
	{
		objCache.clear();
		AetUtil::GetAddObjectsAt(objCache, layer, frame);

		DrawObjCache(objCache, position, opacity);
	}

	void AetRenderer::DrawLayerLooped(const Layer& layer, frame_t frame, vec2 position, float opacity)
	{
		DrawLayer(layer, glm::mod(frame, layer.EndFrame - 1.0f), position, opacity);
	}

	void AetRenderer::DrawLayerClamped(const Layer& layer, frame_t frame, vec2 position, float opacity)
	{
		DrawLayer(layer, (frame >= layer.EndFrame ? layer.EndFrame : frame), position, opacity);
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
			command.Texture = tex;
			command.SourceRegion = spr->PixelRegion;
			command.SetColor(vec4(1.0f, 1.0f, 1.0f, transform.Opacity));
		}
		else
		{
			command.SourceRegion = vec4(0.0f, 0.0f, video.Size);
			command.SetColor(GetSolidVideoColor(video));
		}

		renderer2D.Draw(command);
	}

	void AetRenderer::DrawSpr(const Graphics::Tex& tex, const Graphics::Spr& spr, const Graphics::Transform2D& transform, Graphics::AetBlendMode blendMode)
	{
		RenderCommand2D command;
		command.Texture = &tex;
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
