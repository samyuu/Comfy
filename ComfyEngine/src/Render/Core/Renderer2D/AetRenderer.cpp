#include "AetRenderer.h"
#include "Misc/StringHelper.h"

namespace Comfy::Render
{
	using namespace Graphics::Aet;

	TexSpr SprSetNameStringSprGetter(const VideoSource& source, const Graphics::SprSet* sprSetToSearch)
	{
		if (sprSetToSearch == nullptr)
			return { nullptr, nullptr };

		if (source.SpriteCache != nullptr)
			return { sprSetToSearch->TexSet->Textures[source.SpriteCache->TextureIndex].get(), source.SpriteCache };

		// TEMP: Temporary solution, check for IDs in the future
		auto matchingSpr = std::find_if(sprSetToSearch->Sprites.begin(), sprSetToSearch->Sprites.end(), [&](const auto& spr)
		{
			return EndsWith(source.Name, spr.Name);
		});

		if (matchingSpr != sprSetToSearch->Sprites.end())
		{
			source.SpriteCache = &(*matchingSpr);
			return { sprSetToSearch->TexSet->Textures[source.SpriteCache->TextureIndex].get(), source.SpriteCache };
		}

		return { nullptr, nullptr };
	}

	TexSpr NullSprGetter(const VideoSource& source)
	{
		return { nullptr, nullptr };
	}

	AetRenderer::AetRenderer(Renderer2D& renderer) : renderer2D(renderer)
	{
		sprGetter = NullSprGetter;
	}

	void AetRenderer::DrawObj(const Util::Obj& obj, vec2 positionOffset, float opacity)
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
			command.Scale = obj.Transform.Scale * vec2(obj.Video->Size);
			command.BlendMode = obj.BlendMode;
			command.SetColor(GetSolidVideoColor(*obj.Video, finalOpacity));

			renderer2D.Draw(command);
		}
	}

	void AetRenderer::DrawObjMask(const Util::Obj& maskObj, const Util::Obj& obj, vec2 positionOffset, float opacity)
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
				maskObj.Transform.Position,
				maskObj.Transform.Rotation,
				maskObj.Transform.Scale,
				maskSpr->PixelRegion,
				maskObj.BlendMode,
				1.0f);

			renderer2D.Draw(command, maskCommand);
		}
		else
		{
			auto command = RenderCommand2D();
			command.Origin = obj.Transform.Origin;
			command.Position = obj.Transform.Position + positionOffset;
			command.Rotation = obj.Transform.Rotation;
			command.Scale = obj.Transform.Scale * vec2(obj.Video->Size);
			command.BlendMode = obj.BlendMode;
			command.SetColor(GetSolidVideoColor(*obj.Video, finalOpacity));

			renderer2D.Draw(command);
		}
	}

	void AetRenderer::DrawObjCache(const Util::ObjCache& objCache, vec2 position, float opacity)
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
		Util::GetAddObjectsAt(objCache, layer, frame);

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

	void AetRenderer::DrawVideo(const Video& video, i32 frameIndex, vec2 position)
	{
		auto[tex, spr] = GetSprite(video, frameIndex);
		if (tex != nullptr && spr != nullptr)
		{
			RenderCommand2D command;
			command.Texture = tex;
			command.Position = position;
			command.SourceRegion = spr->PixelRegion;
			renderer2D.Draw(command);
		}
		else
		{
			RenderCommand2D command;
			command.Position = position;
			command.Scale = vec2(video.Size);
			command.SetColor(GetSolidVideoColor(video));
			renderer2D.Draw(command);
		}
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

	TexSpr AetRenderer::GetSprite(const VideoSource& source) const
	{
		return sprGetter(source);
	}

	TexSpr AetRenderer::GetSprite(const Graphics::Aet::VideoSource* source) const
	{
		return (source != nullptr) ? GetSprite(*source) : TexSpr { nullptr, nullptr };
	}

	TexSpr AetRenderer::GetSprite(const Graphics::Aet::Video& video, i32 frameIndex) const
	{
		return InBounds(frameIndex, video.Sources) ? GetSprite(video.Sources[frameIndex]) : TexSpr { nullptr, nullptr };
	}

	TexSpr AetRenderer::GetSprite(const Graphics::Aet::Video* video, i32 frameIndex) const
	{
		return (video != nullptr) ? GetSprite(*video, frameIndex) : TexSpr { nullptr, nullptr };
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
}
