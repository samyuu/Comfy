#include "TextureCachedWaveform.h"
#include "ImGui/Gui.h"

namespace Comfy::Audio
{
	TextureCachedWaveform::TextureCachedWaveform(Waveform& waveform, std::array<u32, ChannelsToVisualize> channelColors)
		: waveform(waveform), channelColors(channelColors)
	{
	}

	void TextureCachedWaveform::Draw(ImDrawList* drawList, vec2 screenTL, vec2 screenBR, f32 scrollOffset, u32 colorTint)
	{
		if (!chunksInitialized)
		{
			InitializeChunkTextures();
			chunksInitialized = true;
		}

		waveformTimePerPixel = waveform.GetTimePerPixel();
		const auto scrollPixelOffset = static_cast<i64>(glm::round(scrollOffset));

		const auto waveformPixelCount = static_cast<i64>(waveform.GetPixelCount());
		const auto remainingVisibleWaveformPixels = std::clamp((waveformPixelCount - scrollPixelOffset), 0i64, static_cast<i64>(MaxSupportedRenderWidth));

		const f32 renderWidth = std::clamp((screenBR.x - screenTL.x), 0.0f, static_cast<f32>(remainingVisibleWaveformPixels));
		const f32 renderHeight = (screenBR.y - screenTL.y);

		bool firstIteration = true;
		for (i64 xOffset = 0; xOffset < static_cast<i64>(renderWidth); xOffset += PixelsPerChunk)
		{
			const auto waveformPixel = (xOffset + scrollPixelOffset);
			if (waveformPixel < -PixelsPerChunk)
				continue;

			const auto& chunk = FindCheckUpdateClosestChunk(waveformPixel);
			if (firstIteration)
			{
				xOffset -= (waveformPixel - chunk.StartPixel);
				firstIteration = false;
			}

			const f32 chunkVisibleStart = std::clamp(static_cast<f32>(xOffset), 0.0f, renderWidth);
			const f32 chunkVisibleEnd = std::clamp(static_cast<f32>(xOffset + PixelsPerChunk), 0.0f, renderWidth);

			const f32 texCoordScale = (chunkVisibleEnd - chunkVisibleStart) / static_cast<f32>(PixelsPerChunk);
			const f32 texCoordOffset = (chunkVisibleStart - xOffset) / PixelsPerChunk;

			drawList->AddImageQuad(
				chunk.Texture,
				screenTL + vec2(chunkVisibleStart, 0.0f),
				screenTL + vec2(chunkVisibleEnd, 0.0f),
				screenTL + vec2(chunkVisibleEnd, renderHeight),
				screenTL + vec2(chunkVisibleStart, renderHeight),
				vec2(0.0f, texCoordOffset),
				vec2(0.0f, texCoordOffset + texCoordScale),
				vec2(1.0f, texCoordOffset + texCoordScale),
				vec2(1.0f, texCoordOffset),
				colorTint);
		}
	}

	void TextureCachedWaveform::InvalidateAll()
	{
		for (auto& chunk : chunks)
		{
			chunk.StartPixel = 0;
			chunk.TimePerPixel = TimeSpan::Zero();
		}
	}

	void TextureCachedWaveform::InitializeChunkTextures()
	{
		for (size_t i = 0; i < ChunkCount; i++)
		{
			auto& chunk = chunks[i];
			chunk.Texture.GPU_Texture2D.DynamicResource = true;

#if COMFY_DEBUG
			chunk.Texture.Name = "WaveformChunk[" + std::to_string(i) + "]";
#endif

			auto& mip = chunk.Texture.MipMapsArray.emplace_back().emplace_back();
			mip.Size = vec2(TextureResolution, PixelsPerChunk);
			mip.Format = Graphics::TextureFormat::RGBA8;
			mip.DataSize = mip.Size.x * mip.Size.y * sizeof(u32);
			mip.Data = std::make_unique<u8[]>(mip.DataSize);
		}
	}

	i64 TextureCachedWaveform::FloorPixelToChunkBoundary(i64 pixel) const
	{
		return (pixel - (pixel % PixelsPerChunk));
	}

	const TextureCachedWaveform::WaveformChunk& TextureCachedWaveform::FindCheckUpdateClosestChunk(i64 startPixel)
	{
		const auto boundaryPixel = FloorPixelToChunkBoundary(startPixel);

		const auto* existingChunk = TryFindExistingValidChunk(boundaryPixel);
		if (existingChunk != nullptr)
			return *existingChunk;

		auto& chunkToOverride = FindBestSuitableChunkToOverride(boundaryPixel);
		UpdateRenderChunkTexture(chunkToOverride, boundaryPixel);

		return chunkToOverride;
	}

	TextureCachedWaveform::WaveformChunk* TextureCachedWaveform::TryFindExistingValidChunk(i64 startPixel)
	{
		return FindIfOrNull(chunks, [&](const auto& chunk)
		{
			return (chunk.TimePerPixel == waveformTimePerPixel) && (chunk.StartPixel == startPixel);
		});
	}

	TextureCachedWaveform::WaveformChunk& TextureCachedWaveform::FindBestSuitableChunkToOverride(i64 startPixel)
	{
		WaveformChunk* furthest = &chunks[0];

		for (auto& chunk : chunks)
		{
			if (chunk.TimePerPixel != waveformTimePerPixel)
				return chunk;

			if (glm::abs(startPixel - furthest->StartPixel) < glm::abs(startPixel - chunk.StartPixel))
				furthest = &chunk;
		}

		return *furthest;
	}

	void TextureCachedWaveform::UpdateRenderChunkTexture(WaveformChunk& chunk, i64 startPixel)
	{
#if COMFY_DEBUG && 0 // DEBUG: Important to make sure chunks aren't being updated while the waveform is static
		printf(__FUNCTION__"(): %lld px - %lld px\n", startPixel, startPixel + PixelsPerChunk);
#endif

		chunk.StartPixel = startPixel;
		chunk.TimePerPixel = waveformTimePerPixel;
		chunk.Texture.GPU_Texture2D.RequestReupload = true;

		auto rgbaPixels = reinterpret_cast<u32*>(chunk.Texture.MipMapsArray[0][0].Data.get());
		for (size_t y = 0; y < PixelsPerChunk; y++)
		{
			RenderTexturePixelRow(chunk.StartPixel + y, rgbaPixels);
			rgbaPixels += TextureResolution;
		}
	}

	void TextureCachedWaveform::RenderTexturePixelRow(i64 waveformPixel, u32* rgbaPixels)
	{
		memset(rgbaPixels, 0x00, TextureResolution * sizeof(u32));

		for (u32 c = 0; c < ChannelsToVisualize; c++)
		{
			const f32 amplitude = waveform.GetNormalizedPCMForPixel(waveformPixel, c);
			i32 amplitudePixels = static_cast<i32>(amplitude * static_cast<f32>(TextureResolution));

			if constexpr (MinAmplitudePixels > 0)
				amplitudePixels = std::max(amplitudePixels, MinAmplitudePixels);

			for (size_t x = 0; x < amplitudePixels; x++)
			{
				auto& pixel = rgbaPixels[x + (TextureResolution / 2) - (amplitudePixels / 2)];
				pixel = (pixel == 0) ? channelColors[0] : channelColors[1];
			}
		}
	}
}
