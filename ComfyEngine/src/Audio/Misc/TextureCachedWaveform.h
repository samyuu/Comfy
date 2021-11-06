#pragma once
#include "Types.h"
#include "Waveform.h"
#include "Graphics/TexSet.h"
#include "Time/TimeSpan.h"

// NOTE: Prevent header contamination by including ImGui into Comfy::Audio
struct ImDrawList;

namespace Comfy::Audio
{
	class TextureCachedWaveform : NonCopyable
	{
	public:
		// NOTE: Amplitude axis, should be but doesn't have to be equal to the timeline height
		static constexpr i32 TextureResolution = /*128*/ 216 /*256*/;
		static constexpr i32 MinAmplitudePixels = 1; // 0;

		// NOTE: Time axis, smaller means better performance when streaming in chunks slowly but also means having to render more
		static constexpr i32 PixelsPerChunk = /*128*/ 256 /*512*/ /*1024*/ /*2048*/;

		// NOTE: Rendering a waveform larger than this would mean having to reuse a single chunk multiple times per draw
		static constexpr i32 MaxSupportedRenderWidth = 0x2000;
		static constexpr i32 ChunkCount = (MaxSupportedRenderWidth / PixelsPerChunk) + 4;

		static constexpr u32 ChannelsToVisualize = 2;

	public:
		TextureCachedWaveform(Waveform& waveform, std::array<u32, ChannelsToVisualize> channelColors);
		~TextureCachedWaveform() = default;

	public:
		void Draw(ImDrawList* drawList, vec2 screenTL, vec2 screenBR, f32 scrollOffset, u32 colorTint = 0xFFFFFFFF, f32 heightFactor = 1.0f);
		void InvalidateAll();

	private:
		struct WaveformChunk
		{
			i64 StartPixel;
			TimeSpan TimePerPixel;
			Graphics::Tex Texture;
		};

		void InitializeChunkTextures();
		i64 FloorPixelToChunkBoundary(i64 pixel) const;

		const WaveformChunk& FindCheckUpdateClosestChunk(i64 startPixel);
		WaveformChunk* TryFindExistingValidChunk(i64 startPixel);
		WaveformChunk& FindBestSuitableChunkToOverride(i64 startPixel);

		void UpdateRenderChunkTexture(WaveformChunk& chunk, i64 startPixel);
		void RenderTexturePixelRow(i64 waveformPixel, u32* rgbaPixels);

	private:
		Audio::Waveform& waveform;
		std::array<u32, ChannelsToVisualize> channelColors;

		TimeSpan waveformTimePerPixel = {};

		bool chunksInitialized = false;
		std::array<WaveformChunk, ChunkCount> chunks = {};
	};
}
