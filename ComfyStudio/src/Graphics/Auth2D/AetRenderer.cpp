#include "AetRenderer.h"
#include "Misc/StringHelper.h"

namespace Graphics
{
	AetRenderer::AetRenderer(D3D_Renderer2D* renderer) : renderer2D(renderer)
	{
	}

	AetRenderer::~AetRenderer()
	{
	}

	D3D_Renderer2D* AetRenderer::GetRenderer2D()
	{
		return renderer2D;
	}

	void AetRenderer::SetRenderer2D(D3D_Renderer2D* value)
	{
		renderer2D = value;
	}

	SpriteGetterFunction* AetRenderer::GetSpriteGetterFunction()
	{
		return spriteGetter;
	}

	void AetRenderer::SetSpriteGetterFunction(SpriteGetterFunction* value)
	{
		spriteGetter = value;
	}

	void AetRenderer::SetCallback(const AetObjCallbackFunction& value)
	{
		objCallback = value;
	}

	void AetRenderer::SetMaskCallback(const AetObjMaskCallbackFunction& value)
	{
		objMaskCallback = value;
	}

	void AetRenderer::RenderObjCache(const AetMgr::ObjCache& obj, const vec2& positionOffset, float opacity)
	{
		if (obj.Surface == nullptr || !obj.Visible)
			return;

		if (objCallback.has_value() && objCallback.value()(obj, positionOffset, opacity))
			return;

		const Txp* txp;
		const Spr* spr;
		const bool validSprite = GetSprite(obj.Surface->GetSprite(obj.SpriteIndex), &txp, &spr);

		const vec2 finalPosition = obj.Transform.Position + positionOffset;
		const float finalOpacity = obj.Transform.Opacity * opacity;

		if (validSprite)
		{
			renderer2D->Draw(
				txp->Texture2D.get(),
				spr->PixelRegion,
				finalPosition,
				obj.Transform.Origin,
				obj.Transform.Rotation,
				obj.Transform.Scale,
				vec4(1.0f, 1.0f, 1.0f, finalOpacity),
				obj.BlendMode);
		}
		else
		{
			// TODO: Only render optionally
			renderer2D->Draw(
				nullptr,
				vec4(0.0f, 0.0f, obj.Surface->Size),
				finalPosition,
				obj.Transform.Origin,
				obj.Transform.Rotation,
				obj.Transform.Scale,
				vec4(DummyColor.r, DummyColor.g, DummyColor.b, DummyColor.a * finalOpacity),
				obj.BlendMode);
		}
	}

	void AetRenderer::RenderObjCacheMask(const AetMgr::ObjCache& maskObj, const AetMgr::ObjCache& obj, const vec2& positionOffset, float opacity)
	{
		if (maskObj.Surface == nullptr || obj.Surface == nullptr || !obj.Visible)
			return;

		if (objMaskCallback.has_value() && objMaskCallback.value()(maskObj, obj, positionOffset, opacity))
			return;

		const Txp* maskTxp;
		const Spr* maskSpr;
		const bool validMaskSprite = GetSprite(maskObj.Surface->GetSprite(maskObj.SpriteIndex), &maskTxp, &maskSpr);

		const Txp* txp;
		const Spr* spr;
		const bool validSprite = GetSprite(obj.Surface->GetSprite(obj.SpriteIndex), &txp, &spr);

		if (validMaskSprite && validSprite)
		{
			renderer2D->Draw(
				maskTxp->Texture2D.get(),
				maskSpr->PixelRegion,
				maskObj.Transform.Position,
				maskObj.Transform.Origin,
				maskObj.Transform.Rotation,
				maskObj.Transform.Scale,
				txp->Texture2D.get(),
				spr->PixelRegion,
				obj.Transform.Position + positionOffset,
				obj.Transform.Origin,
				obj.Transform.Rotation,
				obj.Transform.Scale,
				vec4(1.0f, 1.0f, 1.0f, maskObj.Transform.Opacity * obj.Transform.Opacity * opacity),
				maskObj.BlendMode);
		}
		else
		{
			renderer2D->Draw(
				nullptr,
				vec4(0.0f, 0.0f, obj.Surface->Size),
				obj.Transform.Position + positionOffset,
				obj.Transform.Origin,
				obj.Transform.Rotation,
				obj.Transform.Scale,
				vec4(DummyColor.r, DummyColor.g, DummyColor.b, DummyColor.a * maskObj.Transform.Opacity * obj.Transform.Opacity * opacity),
				maskObj.BlendMode);
		}
	}

	void AetRenderer::RenderObjCacheVector(const std::vector<AetMgr::ObjCache>& objectCache, const vec2& position, float opacity)
	{
		bool singleObject = objectCache.size() == 1;

		for (size_t i = 0; i < objectCache.size(); i++)
		{
			const AetMgr::ObjCache& obj = objectCache[i];

			if (obj.UseTextureMask && !singleObject && (i + 1 < objectCache.size()))
			{
				RenderObjCacheMask(objectCache[i + 1], obj, position, opacity);
				i++;
			}
			else
			{
				RenderObjCache(obj, position, opacity);
			}
		}
	}

	void AetRenderer::RenderLayer(const AetLayer* layer, float frame, const vec2& position, float opacity)
	{
		objectCache.clear();

		AetMgr::GetAddObjects(objectCache, layer, frame);
		RenderObjCacheVector(objectCache, position, opacity);
	}

	void AetRenderer::RenderLayerLooped(const AetLayer* layer, float frame, const vec2& position, float opacity)
	{
		RenderLayer(layer, fmod(frame, layer->EndFrame - 1.0f), position, opacity);
	}

	void AetRenderer::RenderLayerClamped(const AetLayer* layer, float frame, const vec2& position, float opacity)
	{
		RenderLayer(layer, (frame >= layer->EndFrame ? layer->EndFrame : frame), position, opacity);
	}

	void AetRenderer::RenderAetSprite(const AetSurface* surface, const AetSpriteIdentifier* identifier, const vec2& position)
	{
		const Txp* texture;
		const Spr* sprite;

		if (surface->SpriteCount() < 1 || !GetSprite(identifier, &texture, &sprite))
		{
			renderer2D->Draw(nullptr, vec4(0.0f, 0.0f, surface->Size), vec2(0.0f), vec2(0.0f), 0.0f, vec2(1.0f), AetRenderer::DummyColor);
		}
		else
		{
			renderer2D->Draw(texture->Texture2D.get(), sprite->PixelRegion, vec2(0.0f), vec2(0.0f), 0.0f, vec2(1.0f), vec4(1.0f));
		}
	}

	bool AetRenderer::SpriteNameSprSetSpriteGetter(const SprSet* sprSet, const AetSpriteIdentifier* identifier, const Txp** outTxp, const Spr** outSpr)
	{
		if (identifier == nullptr)
			return false;

		if (identifier->SpriteCache != nullptr)
		{
		from_sprite_cache:
			*outTxp = &sprSet->TxpSet->Txps[identifier->SpriteCache->TextureIndex];
			*outSpr = identifier->SpriteCache;
			return true;
		}

		for (auto& sprite : sprSet->Sprites)
		{
			if (EndsWith(identifier->Name, sprite.Name))
			{
				// TEMP: Temporary solution, check for IDs in the future
				identifier->SpriteCache = &sprite;
				goto from_sprite_cache;
			}
		}

		return false;
	}

	bool AetRenderer::GetSprite(const AetSpriteIdentifier* identifier, const Txp** outTxp, const Spr** outSpr)
	{
		assert(spriteGetter != nullptr);
		return (*spriteGetter)(identifier, outTxp, outSpr);
	}
}